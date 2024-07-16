#include <iostream>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <netinet/if_ether.h> // ETH_P_ALL
#include <linux/if_packet.h>
#include <net/if.h>
#include <unistd.h>
#include <arpa/inet.h>

//função para configurar raw socket em modo promíscuo para uma interface de rede 
int cria_raw_socket(char *nome_interface_rede) { //"ip addr" listar todas as interfaces de rede de um sistema linux
    // Cria arquivo para o socket sem qualquer protocolo
    
    int soquete = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (soquete == -1) {
        fprintf(stderr, "Erro ao criar socket: Verifique se você é root!\n");
        exit(-1);
    }
 
    int ifindex = if_nametoindex(nome_interface_rede); //obtém o índice da interface de rede chamada 
 
    struct sockaddr_ll endereco = {0}; //iniciar struct com zeros, será usada para especificar as informações de endereço do socket.
    endereco.sll_family = AF_PACKET; //familia usada por raw_sockets
    endereco.sll_protocol = htons(ETH_P_ALL);
    endereco.sll_ifindex = ifindex;
    // Inicializa socket
    if (bind(soquete, (struct sockaddr*) &endereco, sizeof(endereco)) == -1) { //liga/bind o socket à interface de rede especificada
        fprintf(stderr, "Erro ao fazer bind no socket\n");
        exit(-1);
    }
 
    //modo promíscuo
    struct packet_mreq mr = {0}; //outra struct inicializada com 0, usada para definir opçõs do pacote AF_PACKET
    mr.mr_ifindex = ifindex;
    mr.mr_type = PACKET_MR_PROMISC;
    // Não joga fora o que identifica como lixo: Modo promíscuo
    if (setsockopt(soquete, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) == -1) { //setando opções para o socket
        fprintf(stderr, "Erro ao fazer setsockopt: "
            "Verifique se a interface de rede foi especificada corretamente.\n");
        exit(-1);
    }
 
    return soquete; //retorna um inteiro para o SO vai usar para identificar o socket dentro do sistema
}