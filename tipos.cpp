#include "definicoes.h"
#include "pacotes.h"
#include "tipos.h"


void listType(int socket,struct kermit *pacote,std::list<struct kermit*>& mensagens,std::list<struct kermit*>& janela){
    //estou no servidor e recebi um TYPE_LIST, tenho que enviar um ACK
    printf ("Entendido capitão!\n");
    struct kermit *enviar = montar_pacote(TIPO_ACK,0,NULL,pacote,mensagens);
    enviar_pacote(socket,0,enviar,mensagens);
    int count = 0;
    int result = -1;
    int decide = 0;
    struct kermit *anterior = NULL;
    char nomeArq[63];
    std::string path = "./Videos"; // Diretório atual, você pode mudar para qualquer caminho desejado
    //try {
        for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(path)) {
            printf ("sera que tem arquivo aqui?\n");
            if (entry.is_regular_file()) {
                std::filesystem::path filePath = entry.path();
                std::string extension = filePath.extension().string();

                // Verifica se a extensão é .mp4 ou .avi
                if (extension == ".mp4" || extension == ".avi") {
                    ++count;
                    printf ("nao entrei aqui?\n");
                    std::string fileName = filePath.filename().string();
                    std::size_t length = fileName.length();
                    if (fileName.length() <= sizeof(nomeArq)) {
                        printf ("e aqui?\n");
                        std::strcpy(nomeArq, fileName.c_str());
                        std::size_t length = fileName.length();
                        unsigned int lengthAsInt = static_cast<unsigned int>(length);
                        if (!mensagens.empty()){
                            anterior = mensagens.back();
                        }
                        while (result != TIPO_ACK ){
                            if (result == TIPO_NACK){
                                mensagens.pop_front(); //evitar ter mensagens duplicadas na lista
                            }
                            printf ("Enviando Mostra!\n");
                            struct kermit *enviar = montar_pacote(TIPO_MOSTRA,lengthAsInt,nomeArq,anterior,mensagens);
                            enviar_pacote(socket,lengthAsInt,enviar,mensagens);
                            struct kermit *pacoteMontado = receber_pacote(socket,decide,mensagens,janela);
                            result = process_resposta(socket,pacoteMontado,decide,mensagens,janela);
                            if (decide == 4 || result == FIM_TIMEOUT){
                                printf ("não vou possível receber este pacote\n");
                                break;
                            }
                            decide++;
                        }
                        //recebi o ack, vou continuar a mandar
                    }
                }
            }
            //espero o ack para mandar o próximo pacote
        }
        printf ("Por hoje é isso pessoal!\n");
        result = -1;
        decide = 0;
        if (!mensagens.empty()){
            anterior = mensagens.back();
        }
        while (result != TIPO_ACK ){
            if (result == TIPO_NACK){
                mensagens.pop_front(); //evitar ter mensagens duplicadas na lista
            }
            printf ("Enviando Fim!\n");
            enviar = montar_pacote(TIPO_FIM,1,NULL,anterior,mensagens);
            enviar_pacote(socket,1,enviar,mensagens);
            struct kermit *pacoteMontado = receber_pacote(socket,decide,mensagens,janela);
            result = process_resposta(socket,pacoteMontado,decide,mensagens,janela);
            if (decide == 4 || result == FIM_TIMEOUT){
                printf ("não vou possível receber este pacote\n");
                break;
            }
            decide++;
        }
        //vou definir que quando o tam é 1, é final do mostra, e 2 é final dos dados
    // } catch (const std::filesystem::filesystem_error& err) {
        
    // }
}

void mostraType(int socket, struct kermit *pacote,std::list<struct kermit*>& mensagens,std::list<struct kermit*>& janela){
    //envia um ack, agora vou printar oq eu recebi
    //quando eu colocar os times eu testo o nack
    struct kermit *enviar = montar_pacote(TIPO_ACK,0,NULL,pacote,mensagens);
    enviar_pacote(socket,0,enviar,mensagens);
    printf ("Nome: %s \n",pacote->dados);
    int decide = 0;
    int status = 0;
    while (status != TIPO_ACK || decide<4){
        struct kermit *pacoteMontado = receber_pacote(socket,decide,mensagens,janela); //para receber os próximos nomes de arquivo ou um fim de tx
        status = process_resposta(socket,pacoteMontado,decide,mensagens,janela);
        if (decide == 4 || status == FIM_TIMEOUT){
            printf ("Não foi possível receber este pacote\n");
            break;
        }
        decide ++;
    }
}

void enviar_janela(int socket,std::list <struct kermit *>janela,std::list <struct kermit*> mensagens){
    for (struct kermit *elemento :janela){
        if (elemento){
            enviar_pacote(socket,elemento->tam,elemento,mensagens);
        }
    }
    int demora = 0;
    struct kermit *pacote = receber_pacote(socket,demora,mensagens,janela);
    int result = process_resposta(socket,pacote,demora,mensagens,janela);
    if (pacote != NULL){
        if (pacote->type = TIPO_ACK){
            janela.clear();
        }
        else if (pacote->type = TIPO_NACK){
            int numSequencia = pacote->seq;
            for (auto it = janela.begin(); it != janela.end(); ) {
                if ((*it)->seq < numSequencia) {
                    // Remove o elemento da lista e obtém o próximo iterador
                    it = janela.erase(it);
                } else {
                    ++it;
                }
            }
            for (auto it = mensagens.begin(); it != mensagens.end(); ){
                if ((*it)->seq < numSequencia){
                    it = mensagens.erase(it);
                } else {
                    ++it;
                }
            }
        }
    }
}

void dadosType(int socket,std::ifstream& file,unsigned int bytesLidos,std::list<struct kermit*>& mensagens){
    printf ("Dados totais:%d\n",bytesLidos);
    std::list <struct kermit *> janela;
    unsigned int numEnvios = (bytesLidos + 64 -1) / 64; //arredondar para cima se tiver resto
    unsigned int resto = bytesLidos % 64;
    struct kermit *anterior = NULL;
    printf ("%d numEnvios\n",numEnvios);
    file.seekg(0,std::ios::beg); //colocar ponteiro na posição 0
    char dadosArquivo[63];
    for (int i = 0;i< numEnvios;i++){
        printf ("%d\n",i);
        file.read(dadosArquivo,sizeof(dadosArquivo));
        std::streamsize arqLido = file.gcount();
        // Converter para int se necessário
        int arqLidoInt = static_cast<int>(arqLido);
        if (!mensagens.empty()){
            anterior = mensagens.back();
        }
        if (janela.size() < 5){
            struct kermit *elementoJan = montar_pacote(TIPO_DADOS,sizeof(dadosArquivo),dadosArquivo,anterior,mensagens);
            janela.push_back(elementoJan);
            if (janela.size() == 5 || i == (numEnvios -1 )){
                enviar_janela(socket,janela,mensagens);
            }
        }
    }
    printf ("enviei tudo");
    if (!mensagens.empty()){
        anterior = mensagens.back();
    }
    struct kermit *enviar = montar_pacote(TIPO_FIM,0,NULL,anterior,mensagens);
    enviar_pacote(socket,0,enviar,mensagens);
}

void baixarType(int socket, struct kermit *pacote,std::list<struct kermit*>& mensagens,std::list<struct kermit*>& janela){
    //enviei um ack para mostrar que eu entendi, agora vou mandar o descritor
    struct kermit *enviar = montar_pacote(TIPO_ACK,0,NULL,pacote,mensagens);
    enviar_pacote(socket,0,enviar,mensagens);
    printf ("%s\n e %d",pacote->dados,pacote->tam);
    char *str1 = (char*)malloc(pacote->tam + 9 +1);
    struct kermit *anterior = NULL;
    strcpy(str1,"./Videos/");
    strcat(str1, (char*)pacote->dados);
    std::string filePath = str1; // Caminho completo ou relativo do arquivo
    printf ("filepath:%s\n",str1);
    std::ifstream file(filePath); // Abrir arquivo para leitura
    if (!file.is_open()) {
        std::cerr << "Arquivo não encontrado!" << std::endl;
        //enviar nack com erro
    }
    else{
        file.seekg(0, std::ios::end);
        std::streampos pos = file.tellg();
        ssize_t bytesLidos = static_cast<ssize_t>(pos);
        char bytesLidosStr[21]; // Tamanho suficiente para conter a representação de um ssize_t
        sprintf(bytesLidosStr, "%zd", bytesLidos);
        if (!mensagens.empty()){
           anterior = mensagens.back();
        }
        int result = -1;
        int demora = 0;
        while (result != TIPO_ACK ){
            if (result == TIPO_NACK){
                mensagens.pop_front(); //evitar ter mensagens duplicadas na lista
            }
            printf ("Enviando Descreve!\n");
            struct kermit *enviar = montar_pacote(TIPO_DESCREVE,pacote->tam,(char*)pacote->dados,anterior,mensagens); //enviando o nome do pacote
            enviar_pacote(socket,pacote->tam,enviar,mensagens);
            struct kermit *pacoteMontado = receber_pacote(socket,demora,mensagens,janela);
            printf ("teste\n");
            result = process_resposta(socket,pacoteMontado,demora,mensagens,janela);
            printf ("ainda aqui? result %d\n",result);
            if (demora == 4 && result == FIM_TIMEOUT){
                printf ("Não foi possível receber esta mensagem :(");
                break;
            }
            demora++;
        }
        printf ("e vamos para os dados\n");
        dadosType(socket,file,bytesLidos,mensagens);
        file.close();
    }
    printf ("teste\n");
    free(str1);
}

