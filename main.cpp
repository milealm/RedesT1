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
    unsigned char *dados;        //dados, tem que ter no máximo 64 bytes 
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
    struct kermit_protocol *pacote = new (struct kermit_protocol); //aloquei estrutura onde eu vou guardar o meu pacote

    //marcador de início
    pacote->m_inicio = htons(static_cast<unsigned int>(MARCADOR_DE_INICIO));
    //tamanho da área de dados
    pacote->m_inicio = htons(static_cast<unsigned int>(bytesLidos));
    //num de sequencia
    pacote->m_inicio = htons(static_cast<unsigned int>(0));
    //tipo da mensagem
    pacote->m_inicio = htons(static_cast<unsigned int>(TIPO_ACK));
    //dados
    pacote->dados = new unsigned char[bytesLidos];
    memcpy(pacote->dados, dadosArquivo, bytesLidos);
    //crc INCOMPLETO
    pacote->crc = htons(static_cast<unsigned int>(0));

    ssize_t status = send(socket,pacote,(sizeof(pacote) + bytesLidos),0);
    if (status == (-1)){
        perror("Erro ao anviar pacote\n");
        exit (-1);
    }
    else{
        printf ("pacotes: %ld\n",status);
    }

    delete[] pacote->dados;
    delete[] dadosArquivo;
    delete pacote;
}


void analise_pacote (int socket){
    std::ifstream file("arquivoEnvio.txt", std::ios::in | std::ios::binary);
    file.seekg(0, std::ios::end);
    std::streampos pos = file.tellg();
    int bytesLidos = static_cast<int>(pos);
    
    if (bytesLidos < 64){
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
    if (byes_recebidos < 4){ //menor mensagem, com todos os pacotes, é 4 bytes
        perror ("Erro ao receber mensagem");
    }
    else{
        char marcador_de_inicio = pacote_recebido[0]; //pegando primeiros 8 bits/byte do pacote, que deve ser o marcador_de_inicio
        int numInicio = static_cast<int>(marcador_de_inicio);

    printf ("acabou porque eu não sei se esta dando certo");
    }

    //agora eu tenho que decompor esse pacote para ver o que eu faço;
}

int main(int argc, char *argv[]){
    const char* temp_device1 = "enp1s0";
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

    if (argv[1] == "servidor"){
        while (1){
            int socketServer = cria_raw_socket(device1);
            receber_pacote(socketServer);
            close(socketServer);
        }
    }
    if (argv[1] == "cliente"){
        while(1){
            int socketClient = cria_raw_socket(device2);
            analise_pacote(socketClient); //por aqui eu envio
            close(socketClient);
        }

    }

}