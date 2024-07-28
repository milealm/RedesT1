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
        while (1){
            struct kermit *pacote = receber_pacote(socketServer,mensagens,janela); //receber o primeiro pacote
            process_resposta(socketServer,pacote,mensagens,janela);
            //printf("\033[H\033[J");
        }
        close(socketServer);
    }
    if (argc > 1 && strcmp(argv[1], "cliente") == 0){
        int socketClient = cria_raw_socket(device);
        int sair = -1;
        printf ("Você está no cliente\n");
        while(option != 3){
            printf ("Você tem as seguintes opções: \n1.Listar\n2.Baixar\n3.Sair\nSelecione uma delas para continuar\n");
            scanf("%d",&option);
            printf("\033[H\033[J");
            switch (option){
            case 1:
                printf ("\n\n\n\nVamos listar!\n\n");
                enviar_pacote(socketClient,TIPO_LIST,0,NULL,anterior,mensagens,janela);
                while (sair != 3){
                    struct kermit *pacote = receber_pacote(socketClient,mensagens,janela); //receber o primeiro pacote
                    sair = process_resposta(socketClient,pacote,mensagens,janela);
                }

                break;
            case 2:
                enviar_pacote(socketClient,TIPO_BAIXAR,0,NULL,anterior,mensagens,janela);
                while (sair != 0){
                    struct kermit *pacote = receber_pacote(socketClient,mensagens,janela); //receber o primeiro pacote
                    sair = process_resposta(socketClient,pacote,mensagens,janela);
                }
                break;
            }
        }
        close(socketClient);
    }
}