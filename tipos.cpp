#include "definicoes.h"
#include "pacotes.h"
#include "tipos.h"


void listType(int socket,struct kermit *pacote,std::list<struct kermit*>& mensagens,std::list<struct kermit*>& janela){
    //estou no servidor e recebi um TYPE_LIST, tenho que enviar um ACK
    printf ("Entendido capitão!\n");
    enviar_pacote(socket,TIPO_ACK,0,NULL,pacote,mensagens);
    int count = 0;
    struct kermit *anterior = NULL;
    char nomeArq[63];
    std::string path = "./Videos"; // Diretório atual, você pode mudar para qualquer caminho desejado
    try {
        for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(path)) {
            if (entry.is_regular_file()) {
                std::filesystem::path filePath = entry.path();
                std::string extension = filePath.extension().string();

                // Verifica se a extensão é .mp4 ou .avi
                if (extension == ".mp4" || extension == ".avi") {
                    ++count;
                    std::string fileName = filePath.filename().string();
                    std::size_t length = fileName.length();
                    if (fileName.length() <= sizeof(nomeArq)) {
                        std::strcpy(nomeArq, fileName.c_str());
                        std::size_t length = fileName.length();
                        unsigned int lengthAsInt = static_cast<unsigned int>(length);
                        if (!mensagens.empty()){
                            anterior = mensagens.back();
                        }
                        int result = 0;
                        while (result != 1 ){
                            if (result == 2){
                                mensagens.pop_front(); //evitar ter mensagens duplicadas na lista
                            }
                            printf ("Enviando Mostra!\n");
                            enviar_pacote(socket,TIPO_MOSTRA,lengthAsInt,nomeArq,anterior,mensagens);
                            struct kermit *pacoteMontado = receber_pacote(socket,mensagens,janela);
                            result = process_resposta(socket,pacoteMontado,mensagens,janela);
                        }
                        //recebi o ack, vou continuar a mandar
                    }
                }
            }
            //espero o ack para mandar o próximo pacote
        }
        printf ("Por hoje é isso pessoal!\n");
        enviar_pacote(socket,TIPO_FIM,1,NULL,anterior,mensagens);//vou definir que quando o tam é 1, é final do mostra, e 2 é final dos dados
    } catch (const std::filesystem::filesystem_error& err) {
        //erro, sla qual
    }
}

void mostraType(int socket, struct kermit *pacote,std::list<struct kermit*>& mensagens,std::list<struct kermit*>& janela){
    //envia um ack, agora vou printar oq eu recebi
    //quando eu colocar os times eu testo o nack
    enviar_pacote(socket,TIPO_ACK,0,NULL,pacote,mensagens);
    printf ("Nome: %s \n",pacote->dados);
    struct kermit *pacoteMontado = receber_pacote(socket,mensagens,janela); //para receber os próximos nomes de arquivo ou um fim de tx
    process_resposta(socket,pacoteMontado,mensagens,janela);
}

void dadosType(int socket,std::ifstream& file,unsigned int bytesLidos,std::list<struct kermit*>& mensagens,std::list<struct kermit*>& janela){
    printf ("Dados\n");
    unsigned int numEnvios = (bytesLidos + 64 -1) / 64; //arredondar para cima se tiver resto
    unsigned int resto = bytesLidos % 64;
    struct kermit *anterior = NULL;
    printf ("%d numEnvios\n",numEnvios);
    file.seekg(0,std::ios::beg); //colocar ponteiro na posição 0
    char dadosArquivo[63];
    for (int i;i<=numEnvios;i++){
        file.read(dadosArquivo,sizeof(dadosArquivo));
        std::streamsize arqLido = file.gcount();
        // Converter para int se necessário
        int arqLidoInt = static_cast<int>(arqLido);
        if (!mensagens.empty()){
            anterior = mensagens.back();
        }
        enfileirar(TIPO_DADOS,arqLidoInt,dadosArquivo,anterior,mensagens,janela);
    }
}

void baixarType(int socket, struct kermit *pacote,std::list<struct kermit*>& mensagens,std::list<struct kermit*>& janela){
    //enviei um ack para mostrar que eu entendi, agora vou mandar o descritor
    enviar_pacote(socket,TIPO_ACK,0,NULL,pacote,mensagens);
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
        unsigned int bytesLidos = static_cast<unsigned int>(pos);
        char bytesLidosStr[11]; // Tamanho suficiente para conter a representação de um unsigned int
        sprintf(bytesLidosStr, "%u", bytesLidos);
        if (!mensagens.empty()){
           anterior = mensagens.back();
        }
        int result = 0;
        while (result != 1 ){
            if (result == 2){
                mensagens.pop_front(); //evitar ter mensagens duplicadas na lista
            }
            printf ("Enviando Descreve!\n");
            enviar_pacote(socket,TIPO_DESCREVE,0,bytesLidosStr,anterior,mensagens);
            struct kermit *pacoteMontado = receber_pacote(socket,mensagens,janela);
            result = process_resposta(socket,pacoteMontado,mensagens,janela);
        }
        dadosType(socket,file,bytesLidos,mensagens,janela);
        file.close();
        imprimirFilas(mensagens,janela);
    }
    printf ("teste\n");
    free(str1);
}

