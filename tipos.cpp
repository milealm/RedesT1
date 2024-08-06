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
    char nomeArq[64];
    std::string path = "./Videos"; // Diretório atual, você pode mudar para qualquer caminho desejado
    //try {
        for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(path)) {
            printf ("sera que tem arquivo aqui?\n");
            if (entry.is_regular_file()) {
                std::filesystem::path filePath = entry.path();
                std::string extension = filePath.extension().string();

                // Verifica se a extensão é .mp4 ou .avi
                if (extension == ".mp4" || extension == ".avi" || extension == ".txt") {
                    ++count;
                    std::string fileName = filePath.filename().string();
                    std::size_t length = fileName.length();
                    if (fileName.length() <= sizeof(nomeArq)) {
                        std::strcpy(nomeArq, fileName.c_str());
                        std::size_t length = fileName.length();
                        printf ("nomeArq %s\n",nomeArq);
                        unsigned int lengthAsInt = static_cast<unsigned int>(length);
                        if (!mensagens.empty()){
                            anterior = mensagens.back();
                        }
                        int volta = 0;
                        while (result != TIPO_ACK ){
                            if (result == TIPO_NACK || (pacote == NULL && volta > 0)){ //envio de novo se receber nack ou se não receber a mensagem
                                mensagens.pop_front(); //evitar ter mensagens duplicadas na lista
                            }
                            printf ("Enviando Mostra! decide-%d\n",decide);
                            struct kermit *enviar = montar_pacote(TIPO_MOSTRA,lengthAsInt,nomeArq,anterior,mensagens);
                            enviar_pacote(socket,lengthAsInt,enviar,mensagens);
                            struct kermit *pacoteMontado = NULL;
                            printf ("aguardo resposta...\n");
                            pacoteMontado = receber_pacote(socket,decide,mensagens,janela);
                            result = process_resposta(socket,pacoteMontado,decide,mensagens,janela);
                            if (decide == 4 || result == FIM_TIMEOUT){
                                printf ("não vou possível receber este pacote\n");
                                break;
                            }
                            decide++;
                            volta++;
                        }
                        result = -1;
                        //recebi o ack, vou continuar a mandar
                    }
                }
            }
            //espero o ack para mandar o próximo pacote
        }
        printf ("Por hoje é isso pessoal!\n");
        result = -1;
        decide = 0;
        int volta = 0;
        if (!mensagens.empty()){
            anterior = mensagens.back();
        }
        while (result != TIPO_ACK ){
            if (result == TIPO_NACK || (pacote == NULL && volta > 0)){ //vai reenviar, tira da filta
                mensagens.pop_front(); //evitar ter mensagens duplicadas na lista
            }
            printf ("Enviando Fim!\n");
            enviar = montar_pacote(TIPO_FIM,1,NULL,anterior,mensagens);
            printf ("Aguardando ultima mensagem...\n");
            enviar_pacote(socket,1,enviar,mensagens);
            struct kermit *pacoteMontado = receber_pacote(socket,decide,mensagens,janela);
            result = process_resposta(socket,pacoteMontado,decide,mensagens,janela);
            if (decide == 4 || result == FIM_TIMEOUT){
                printf ("não vou possível receber este pacote\n");
                break;
            }
            decide++;
            volta++;
        }
}

void mostraType(int socket, struct kermit *pacote,std::list<struct kermit*>& mensagens,std::list<struct kermit*>& janela){
    //envia um ack, agora vou printar oq eu recebi
    //quando eu colocar os times eu testo o nack
    std::list <struct kermit *> mostras;
    struct kermit *enviar = montar_pacote(TIPO_ACK,0,NULL,pacote,mensagens); //
    enviar_pacote(socket,0,enviar,mensagens);
    int status = pacote->type;
    if (status != TIPO_FIM){
        printf ("enviei o ack!\n");
        //printf ("Nome: %s \n",pacote->dados);
        int decide = 0;
        int status = 0;
        int volta = 0;
        struct kermit *pacoteMontado = NULL;
        while (pacoteMontado == NULL){
            printf ("esperando...\n");
            pacoteMontado = receber_pacote(socket,decide,mensagens,janela); //aqui eu só espero
        }
        if (pacoteMontado->type == TIPO_MOSTRA){
            if (mostras.back()->seq < pacoteMontado->seq){
                mostras.push_back(pacoteMontado);
                enviar = montar_pacote(TIPO_ACK,0,NULL,pacoteMontado,mensagens); //
                enviar_pacote(socket,0,enviar,mensagens);
            }
        }
    }
    for (struct kermit *elemento : mostras){
        if (elemento != NULL){
            printf ("Nome: %s",(char*)elemento->dados);
        }
    }
}

void enviar_janela(int socket,std::list <struct kermit *>&janela,std::list <struct kermit*> &mensagens){
    printf ("size janela %ld\n",janela.size());
    struct kermit *pacote = NULL;
    int demora = 0;
    for (struct kermit *elemento :janela){
        if (elemento != NULL){
            printf (" SEQ: %d \n",elemento->seq);
            enviar_pacote(socket,elemento->tam,elemento,mensagens);
        }
    }
    while (pacote == NULL){
        printf ("esperando... %d\n",demora);
        pacote = receber_pacote(socket,demora,mensagens,janela);
        if (pacote != NULL){
            printf ("pacote->type %d\n",pacote->type);
        }
        demora++;
        if (demora > 1){
            printf ("vou reenviar\n");
            for (struct kermit *elemento :janela){
                if (elemento != NULL){
                    printf ("SEQ %d \n",elemento->seq);
                    enviar_pacote(socket,elemento->tam,elemento,mensagens);
                }
            }
            demora = 0;
        }
    }
    //int result = process_resposta(socket,pacote,demora,mensagens,janela);
    //printf ("result %d\n",result);
    if (pacote != NULL){
        if (pacote->type == TIPO_ACK){
            printf ("recebi um ack!\n");
            janela.clear();
            printf ("janela size:%ld\n",janela.size());
        }
        else if (pacote->type == TIPO_NACK){
            printf ("recebi um nack\n");
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
    char dadosArquivo[32];
    char dadosExtrabyte[64];
    struct kermit *elementoJan = NULL;
    int i = 0;
    while (!file.eof()){
        printf ("Dados Totais: bytes lidos %d - %d -\n",bytesLidos,i);
        i = i + 32;
        file.read(dadosArquivo,sizeof(dadosArquivo));
        std::streamsize arqLido = file.gcount();
        //printf ("-- %s --\n",dadosArquivo);
        int arqLidoInt = static_cast<int>(arqLido);
        int i = 0;
        int j = 0;
        while(i < 64){
            dadosExtrabyte[i] = dadosArquivo[j];
            dadosExtrabyte[i+1] = 0xFF;
            //printf ("j %d\n",j);
            i+=2;
            j++;
        }
        // Converter para int se necessário
        //printf ("arqLidoInt %ld\n",arqLido);
        if (!mensagens.empty()){
            anterior = mensagens.back();
        }
        if (janela.size() < 5){
            if (!janela.empty()){
                anterior = janela.back();
            }
            elementoJan = montar_pacote(TIPO_DADOS,arqLidoInt + 32,dadosExtrabyte,anterior,mensagens);
            janela.push_back(elementoJan);
            if (janela.size() == 5 || file.eof()){
                if (file.eof()){
                    if (!mensagens.empty()){
                        anterior = mensagens.back();
                    }
                    elementoJan = montar_pacote(TIPO_FIM,0,NULL,anterior,mensagens);
                    janela.push_back(elementoJan);
                }
                enviar_janela(socket,janela,mensagens);
            }
        }
    }
    printf ("enviei tudo");
    if (!mensagens.empty()){
        anterior = mensagens.back();
    }
    // struct kermit *enviar = montar_pacote(TIPO_FIM,0,NULL,anterior,mensagens);
    // enviar_pacote(socket,0,enviar,mensagens);
    // printf ("enviei o FIM\n");
}

void baixarType(int socket, struct kermit *pacote,std::list<struct kermit*>& mensagens,std::list<struct kermit*>& janela){
    //enviei um ack para mostrar que eu entendi, agora vou mandar o descritor
    struct kermit *enviar = montar_pacote(TIPO_ACK,0,NULL,pacote,mensagens);
    enviar_pacote(socket,0,enviar,mensagens);
    printf ("%s e %d\n",pacote->dados,pacote->tam);
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
            //printf ("ainda aqui? result %d\n",result);
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

void descreverType (int socket, struct kermit *pacote,std::list<struct kermit*>&mensagens,std::list <struct kermit*>&janela){
    std::list <struct kermit*> janelaClient;
    int demora = 0;
    int numJanela = 0;
    struct kermit *pacoteJanela;
    struct kermit *enviar;

    janelaClient.clear();
    pacoteJanela = NULL;
    while(pacoteJanela == NULL){
        pacoteJanela = receber_pacote(socket,demora,mensagens,janela); //tem que fazer o negocio do timeout aqui
    }
    janelaClient.push_back(pacoteJanela);
    numJanela++;
    demora = 0;
    while (numJanela < 5){
        while ((janelaClient.size() < 5 && pacoteJanela->type != TIPO_FIM)){
            pacoteJanela = receber_pacote(socket,demora,mensagens,janela); //tem que fazer o negocio do timeout aqui
            while(pacoteJanela == NULL){
                printf ("esperando..%d\n",demora);
                pacoteJanela = receber_pacote(socket,demora,mensagens,janela); //tem que fazer o negocio do timeout aqui
                demora++;
            }
            printf ("num seq %d\n",pacoteJanela->seq);
            if (!janelaClient.empty()){
                printf ("janela front seq %d e pacoteRecebido %d e demora %d\n",janelaClient.back()->seq, pacoteJanela->seq,demora);
                if (((janelaClient.back()->seq != (pacoteJanela->seq-1)) && ((janelaClient.back()->seq == 31) && (pacoteJanela->seq != 0))) || (demora > 1)){
                    printf ("limpa limpa tudo\n");
                    janelaClient.clear();
                    printf ("aqui?\n");
                }
            }
            if (demora <= 1){
                printf ("coloca na janela\n");
                janelaClient.push_back(pacoteJanela);
            }
            numJanela++;
            demora = 0;
        }
        //janelaClient.push_back(pacoteJanela);
        numJanela++;
        if (janelaClient.size() == 5 || pacoteJanela->type == TIPO_FIM){
            //printf("sera ack ou nack?\n");
            verifica_janela(socket,(char*)pacote->dados,janelaClient,mensagens,janela);
            //printf ("antes\n");
            if (pacoteJanela->type == TIPO_FIM){
                numJanela = 6;
            }
            else{
                numJanela = 0;
                //printf ("aqui\n");
            }
        }
        //printf ("proximo\n");
    }
}