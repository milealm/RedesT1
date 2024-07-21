#include <iostream>
#include <unistd.h> 
#include <fstream>
#include <list>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h> // Para htons
#include "rawsocket.h"

#define FILE_NAME "arquivoEnvio.txt"
#define MAX_BYTES 64
#define PACOTE_MAX 68
#define MARCADOR_DE_INICIO 126
#define POLINOMIO 0x9B //10011011

#define TIPO_ACK 0
#define TIPO_NACK 1
#define TIPO_LIST 10
#define TIPO_BAIXAR 11
#define TIPO_SHOW 16
#define TIPO_DADOS 18
#define TIPO_FIM 30
#define TIPO_ERRO 31

struct kermit_protocol {
    unsigned int m_inicio : 8;   // marcador de inicio
    unsigned int tam : 6;        // tamanho do campo dados em bytes
    unsigned int seq : 5;        // numero de sequência
    unsigned int type : 5;       // tipo da mensagem
    unsigned char dados[63];        //dados, tem que ter no máximo 64 bytes 
    unsigned int crc : 8;        //crc
};

void fragmentar_pacote(std::ifstream& file, int bytesLidos){
    printf ("kk muito grande\n");
}

int codigo_crc(char *dadosArquivo, int bytesLidos){
    // for (int i = 0;i< bytesLidos;i++){
    //     char byte = dadosArquivo[i];
    //     for (int j = 7; i>=0;i--){ //indo bit a bit

    //     }
    // }
    return 0;
}

void enviar_pacote(int socket,int bytesLidos,char *dadosArquivo){ //não faz muito sentido passar o file aqui, deveria ser um buffer
    unsigned char buffer[67]; //63 + 4 bytes dos outros campso do frame (8 + 6 + 5 + 5 bits = 3 bytes)
    struct kermit_protocol *pacote = new struct kermit_protocol; //aloquei estrutura onde eu vou guardar o meu pacote
    //marcador de início
    pacote->m_inicio = MARCADOR_DE_INICIO;
    //tamanho da área de dados
    pacote->tam = bytesLidos;
    //num de sequencia
    pacote->seq = 0;
    //tipo da mensagem
    pacote->type = TIPO_ACK;
    memcpy(buffer,&pacote,3); //põe os 3 primiros bytes do pacote no buffer (marcador, tamanho, seq e tipo)
    //dados
    memcpy(pacote->dados, dadosArquivo, bytesLidos);
    memcpy(buffer+3,pacote->dados,bytesLidos); //coloca o tamanho no buffer
    //crc INCOMPLETO
    pacote->crc = 0;
    int valor_crc=0;
    memcpy(buffer + 3 + bytesLidos,&valor_crc,1);

    ssize_t status = send(socket,buffer,sizeof(buffer),0);
    if (status == (-1)){
        perror("Erro ao anviar pacote\n");
        exit (-1);
    }
    else{
        printf ("pacotes: %ld\n",status); //numero de bytes enviados, deve ser o tamanho do buffer (67)
    }
}


void analise_pacote (int socket){
    std::ifstream file("arquivoEnvio.txt", std::ios::in | std::ios::binary);
    file.seekg(0, std::ios::end);
    std::streampos pos = file.tellg();
    int bytesLidos = static_cast<int>(pos);
    printf ("bytesLidos:%d\n",bytesLidos);
    if (bytesLidos > 64){
        fragmentar_pacote(file,bytesLidos);
    }
    else{
        file.seekg(0,std::ios::beg);
        char *dadosArquivo = new char[bytesLidos];
        file.read (dadosArquivo,bytesLidos);
        file.close();
        enviar_pacote(socket,bytesLidos, dadosArquivo);
    }

}

void receber_pacote(int socket){
    unsigned char *pacote_recebido = (unsigned char *)malloc(PACOTE_MAX);
    struct kermit_protocol *pacoteMontado = new(struct kermit_protocol);
    ssize_t byes_recebidos = recv(socket,pacote_recebido, PACOTE_MAX,0);
    printf("byte_recebidos: %ld\n",byes_recebidos);
    if (byes_recebidos < 4){ //menor mensagem, com todos os pacotes, é 4 bytes
        perror ("Erro ao receber mensagem\n");
    }
    else{
        char marcador_de_inicio = pacote_recebido[0]; //pegando primeiros 8 bits/byte do pacote, que deve ser o marcador_de_inicio
        int numInicio = static_cast<int>(marcador_de_inicio);

    printf ("acabou porque eu não sei se esta dando certo\n");
    }

    //agora eu tenho que decompor esse pacote para ver o que eu faço;
}

int main(int argc, char *argv[]){
    const char* temp_device1 = "eno1";
    char* device1 = (char*)malloc(strlen(temp_device1) + 1); // +1 para o terminador nulo
    if (device1 == NULL) {
        fprintf(stderr, "Erro ao alocar memória\n");
        return 1;
    }
    strcpy(device1, temp_device1);

    const char* temp_device2 = "enp1s0";
    char* device2 = (char*)malloc(strlen(temp_device2) + 1); // +1 para o terminador nulo
    if (device2 == NULL) {
        fprintf(stderr, "Erro ao alocar memória\n");
        return 1;
    }
    strcpy(device2, temp_device2);
    int option = 0;
    if (argc > 1 && strcmp(argv[1], "servidor") == 0){
        int socketServer = cria_raw_socket(device1);
        while (1){
            printf ("Você tem as seguintes opções: 1.Nada 2.Receber 3.Enviar\n");
            scanf ("\n%d",&option);
            switch (option){
            case 1:
                printf ("ue\n");
                break;
            case 2:
                receber_pacote(socketServer);
                break;
            case 3:
                analise_pacote(socketServer);
            }
        }
        close(socketServer);
    }
    if (argc > 1 && strcmp(argv[1], "cliente") == 0){
        int socketClient = cria_raw_socket(device2);
        while(1){
            printf ("Você tem as seguintes opções: 1.Nada 2.Receber 3.Enviar\n");
            scanf ("%d",&option);
            switch (option){
            case 1:
                printf ("ue\n");
                break;
            case 2:
                receber_pacote(socketClient);
                break;
            case 3:
                analise_pacote(socketClient);
            }
        }
        close(socketClient);
    }

}