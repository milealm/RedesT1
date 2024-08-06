#ifndef TIPOS_H
#define TIPOS_H

#include <fstream>
#include <list>

void listType(int socket,struct kermit *pacote,std::list<struct kermit*>& mensagens,std::list<struct kermit*>& janela);

void mostraType(int socket, struct kermit *pacote,std::list<struct kermit*>& mensagens,std::list<struct kermit*>& janela);

int enviar_janela(int socket,std::list <struct kermit *>&janela,std::list <struct kermmit*> &mensagens);

int dadosType(int socket,std::ifstream& file, unsigned int bytesLidos,std::list<struct kermit*>& mensagens);

int baixarType(int socket, struct kermit *pacote,std::list<struct kermit*>& mensagens,std::list<struct kermit*>& janela);


#endif //tipos.h