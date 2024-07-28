#ifndef PACOTES_H
#define PACOTES_H
#include <fstream>
#include <list>

int codigo_crc(char *dadosArquivo, int bytesLidos);

void enviar_pacote(int socket,int tipo,int bytesLidos,char *dadosArquivo,struct kermit *anterior,std::list<struct kermit*>& mensagens);

struct kermit *receber_pacote(int socket,std::list<struct kermit*>& mensagens,std::list<struct kermit*>& janela);

int process_resposta(int socket,struct kermit *pacote,std::list<struct kermit*>& mensagens,std::list<struct kermit*>& janela);

void imprimirFilas(std::list<struct kermit*>& mensagens,std::list<struct kermit*>& janela);

void enfileirar (int tipo,int bytesLidos,char *dadosArquivo,struct kermit *anterior,std::list<struct kermit*>&mensagens,std::list<struct kermit*>&janela);

#endif //pacotes.h