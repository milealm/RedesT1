#ifndef TIPOS_H
#define TIPOS_H

#include <fstream>
#include <list>

void listType(int socket,struct kermit *pacote,std::list<struct kermit*>& mensagens,std::list<struct kermit*>& janela);

void mostraType(int socket, struct kermit *pacote,std::list<struct kermit*>& mensagens,std::list<struct kermit*>& janela);

void dadosType(int socket,std::ifstream& file,std::list<struct kermit*>& mensagens,std::list<struct kermit*>& janela);

void baixarType(int socket, struct kermit *pacote,std::list<struct kermit*>& mensagens,std::list<struct kermit*>& janela);

#endif //tipos.h