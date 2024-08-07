#include "rawsocket.h"
#include "pacotes.h"
#include "definicoes.h"

int main(int argc, char *argv[]){
    //pegar o parâmetro com a interface
    printf ("Bem vindo ao seu sistema de Streaming!\n");
    size_t length = sizeof(argv[2]);
    char device[length];
    strcpy(device,argv[2]);
    //opção para a seleção na tela do usuário
    int option = 0;
    std::list<struct kermit*> mensagens;
    std::list<struct kermit*> janela;
    struct kermit*anterior = NULL;


    //diferentes execuções para servidor e para cliente
    if (argc > 1 && strcmp(argv[1], "servidor") == 0){
        int socketServer = cria_raw_socket(device);
        printf ("Você está no servidor\n");
        int decide = 0;
        while (1){
            printf ("\n\nvoltei para o menu!!\n");
            struct kermit *pacote = receber_pacote(socketServer,decide,mensagens,janela); //receber o primeiro pacote
            if (pacote != NULL){
                int sair = process_resposta(socketServer,pacote,decide,mensagens,janela);
            }
            //printf("\033[H\033[J");
        }
        close(socketServer);
    }
    else if (argc > 1 && strcmp(argv[1], "cliente") == 0){
        int socketClient = cria_raw_socket(device);
        int sair = -1;
        printf ("Você está no cliente\n");
        while(option != 3){
            printf ("Você tem as seguintes opções: \n1.Listar\n2.Baixar\n3.Sair\nSelecione uma delas para continuar\n");
            scanf("%d",&option);
            printf("\033[H\033[J");
            int decide = 0;
            struct kermit *enviar;
            switch (option){
                case 1:
                    printf ("\n\n\n\nVamos listar!\n\n");
                    enviar = montar_pacote(TIPO_LIST,0,NULL,anterior,mensagens);
                    enviar_pacote(socketClient,0,enviar,mensagens);
                    while (sair != TIPO_FIM){
                        struct kermit *pacote = receber_pacote(socketClient,decide,mensagens,janela); //receber o primeiro pacote
                        sair = process_resposta(socketClient,pacote,decide,mensagens,janela);
                        if (sair == TIPO_NACK){ //deu timeout ou é só um nack
                            printf ("Ixi? Vou mandar de novo!\n");
                            enviar_pacote(socketClient,0,enviar,mensagens);
                            decide++;
                        }
                    }
                    break;
                case 2:
                    char nomeArquivo[64];
                    unsigned int bytesLidos = 13;
                    printf ("Digite o nome do arquivo que gostaria de baixar:");
                    scanf("%s%n",nomeArquivo,&bytesLidos);
                    //strcpy(nomeArquivo,"suspeito.mp4");
                    char str1[bytesLidos+2];
                    strcpy(str1,"./");
                    strcat(str1, nomeArquivo);
                    std::string filePath = str1; // Caminho completo ou relativo do arquivo
                    std::ifstream file(filePath); // Abrir arquivo para leitura
                    if (file.is_open()) {
                        printf ("Arquivo já baixado! Voltando ao menu...\n");
                        break;
                    }
                    else{
                        //scanf("%s%n",nomeArquivo,&bytesLidos);
                        enviar = montar_pacote(TIPO_BAIXAR,bytesLidos-1,nomeArquivo,anterior,mensagens);
                        enviar_pacote(socketClient,bytesLidos-1,enviar,mensagens);
                        while (sair != FIM_TIMEOUT && sair != TIPO_FIM){
                            struct kermit *pacote = receber_pacote(socketClient,decide,mensagens,janela); //receber o primeiro pacote
                            sair = process_resposta(socketClient,pacote,decide,mensagens,janela);
                            if (sair == TIPO_NACK){ //deu timeout ou é só um nack
                                printf ("Ixi? Vou mandar de novo!\n");
                                enviar_pacote(socketClient,bytesLidos-1,enviar,mensagens);
                                decide++;
                            }
                            else {
                                if (sair == TIPO_NOTFOUND){
                                    break;
                                }
                            }
                        }
                        break;
                    }
            }
            mensagens.clear();
            janela.clear();
        }
        close(socketClient);
    }
    return 0;
}