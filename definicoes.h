#ifndef DEFINICOES_H
#define DEFINICOES_H

#include <iostream>
#include <unistd.h> 
#include <list>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <filesystem>
#include <list>

#define FILE_NAME "arquivoEnvio.txt"
#define MAX_BYTES 64
#define PACOTE_MAX 68
#define MARCADOR_DE_INICIO 126
#define POLINOMIO 0x9B //10011011

#define TIPO_ACK 0
#define TIPO_NACK 1
#define TIPO_LIST 10
#define TIPO_BAIXAR 11
#define TIPO_MOSTRA 16
#define TIPO_DESCREVE 17
#define TIPO_DADOS 18
#define TIPO_FIM 30
#define TIPO_ERRO 31

struct __attribute__((packed)) kermit {
    unsigned int m_inicio : 8;   // marcador de inicio
    unsigned int tam : 6;        // tamanho do campo dados em bytes
    unsigned int seq : 5;        // numero de sequência
    unsigned int type : 5;       // tipo da mensagem
    unsigned char dados[63];        //dados, tem que ter no máximo 64 bytes 
    unsigned int crc : 8;        //crc
};

#endif //definicoes.h