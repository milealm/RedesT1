#ifndef PACOTES_H
#define PACOTES_H
#include <fstream>
#include <list>

long long timestamp();

int codigo_crc(unsigned char *buffer);

void enviar_pacote(int socket,int bytesLidos,struct kermit *pacote,std::list<struct kermit*>& mensagens);

struct kermit *montar_pacote(int tipo,int bytesLidos,char *dadosArquivo,struct kermit*anterior,std::list<struct kermit *>&mensagens);

struct kermit *receber_pacote(int socket, int demora, std::list<struct kermit*>& mensagens,std::list<struct kermit*>& janela);

int process_resposta(int socket,struct kermit *pacote,int decide,std::list<struct kermit*>& mensagens,std::list<struct kermit*>& janela);

void imprimirFilas(std::list<struct kermit*>& mensagens,std::list<struct kermit*>& janela);

//void enfileirar (int tipo,int bytesLidos,char *dadosArquivo,struct kermit *anterior,std::list<struct kermit*>&mensagens,std::list<struct kermit*>&janela);

void verifica_janela(int socket,char *nomeArquivo,std::list <struct kermit*>&janelaClient,std::list <struct kermit*> mensagens, std::list <struct kermit*>janela);

#endif //pacotes.h