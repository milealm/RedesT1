#include "definicoes.h"
#include "pacotes.h"
#include "tipos.h"

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

void imprimirFilas(std::list<struct kermit*>& mensagens,std::list<struct kermit*>& janela){
    if (!mensagens.empty()){
        printf ("Mensagens\n");
        for (std::list<struct kermit*>::iterator it = mensagens.begin(); it != mensagens.end(); ++it) {
            printf ("Marc: %u, Tam: %u, Seq: %u, Tipo: %u\n", (*it)->m_inicio, (*it)->tam, (*it)->seq, (*it)->type);
        }
    }
    if (!janela.empty()){
        printf ("Janela\n");
        for (std::list<struct kermit*>::iterator it = janela.begin(); it != janela.end(); ++it) {
            printf ("Marc: %u, Tam: %u, Seq: %u, Tipo: %u\n", (*it)->m_inicio, (*it)->tam, (*it)->seq, (*it)->type);
        }
    }
}

int process_resposta(int socket,struct kermit *pacote,std::list<struct kermit*>& mensagens,std::list<struct kermit*>& janela){
    if (pacote->m_inicio != 126){ //colocar um OU com o calculo do crc
        enviar_pacote(socket,TIPO_NACK,0,NULL,NULL,mensagens,janela);//tem que resolver o numero de sequencia
        return 2;
        //função para colocar na parte de dados o tipo de erro que deu para imprimir
    }
    else{
        switch (pacote->type){
            case TIPO_ACK:
                //baseado no numero de sequência do pacote, eu tiro algumas mensagens da fila e movo a janela. O negócio é como kk
                printf ("\nEu acho que vi um ACK\n");
                if (janela.empty()){
                    if (mensagens.size() <= 1){
                        return 1;
                    }
                    else{
                        for (auto it = mensagens.begin(); it != mensagens.end(); ) {
                            if ((*it)->seq == pacote->seq) {
                                it = mensagens.erase(it); // Remove o elemento e atualiza o iterador
                            } else {
                                ++it; // Avança o iterador apenas se não remover o elemento
                            }
                        }
                        return 1;
                    }
                }
                break;
            case TIPO_NACK:
                //vou ter que reenviar uma janela ou mensagem //envia uma mensagem de erro TIPO_ERRO
                printf ("Eu acho que vi um NACK\n");
                return 2;
                break;
            case TIPO_LIST:
                //o que o servidor recebe para começar a enviar os nomes dos arquivos
                printf ("\n\nEu acho que vi um LIST\n");
                listType(socket,pacote,mensagens,janela);
                return 0;
                break;
            case TIPO_BAIXAR:
                //estou no servidor
                printf ("Eu acho que vi um BAIXAR\n");
                //vou mandar um descritor de arquivo depois de mandar um ACK
                baixarType(socket,pacote,mensagens,janela);
                printf ("voltou?\n");
                return 0;
                break;
            case TIPO_MOSTRA:
                printf ("\nEu acho que vi um MOSTRA\n");
                //o que o cliente recebe com os dados que ele pediu para mostrar, vão ser várias mensagens, cada uma com um dos arquivos do servidor
                mostraType(socket,pacote,mensagens,janela);
                return 0;
                break;
            case TIPO_DESCREVE:
                printf ("Eu acho que vi um DESCREVE\n");
                //o que o cliente vai receber depois de mandar um baixar, receber um ack, indica que vai começar a mandar dados
                //nessa função já pode ter um loop no cliente para receber os dados e ir juntando (TIPO_DADOS)
                return 0;
                break;
            case TIPO_FIM:
                printf ("\nEu acho que vi um FIM\n");
                //o que o cliente recebe para saber que o vídeo já foi todo mandado e dá para dar play no que foi enviado
                if (pacote->tam == 1){ //fim 
                    printf ("Esses são todos os arquivos disponíveis\n");
                    return 3;
                }
                else{
                    printf ("hora de abrir o player :)\n");
                    return 4;
                }
                break;
        }
    }
    return 2;
}

void enviar_pacote(int socket,int tipo,int bytesLidos,char *dadosArquivo,struct kermit *anterior,std::list<struct kermit*>& mensagens,std::list<struct kermit*>& janela){ //não faz muito sentido passar o file aqui, deveria ser um buffer
    unsigned char buffer[PACOTE_MAX] = {0}; //64 + 4 bytes dos outros campso do frame (8 + 6 + 5 + 5 bits = 3 bytes)
    struct kermit *pacote = new struct kermit; //aloquei estrutura onde eu vou guardar o meu pacote
    //marcador de início
    pacote->m_inicio = MARCADOR_DE_INICIO;
    //tamanho da área de dados
    pacote->tam = bytesLidos;
    //num de sequencia
    if (anterior){
        if (tipo != TIPO_ACK && tipo!=TIPO_NACK){
            if (anterior->seq != 31){
                pacote->seq = anterior->seq+1;
            }
            else {
                pacote->seq = 0;
            }
        }
        else{
            pacote->seq = anterior->seq;
        }
    }
    else{
        pacote->seq = 0;
    }
    //tipo da mensagem
    pacote->type = tipo;
    memcpy(buffer,pacote,3); //põe os 3 primeiros bytes do pacote no buffer (marcador, tamanho, seq e tipo)
    //dados
    if (dadosArquivo){
        memcpy(pacote->dados, dadosArquivo, bytesLidos);
        memcpy(buffer+3,pacote->dados,bytesLidos); //coloca os dados no buffer
    }
    //crc INCOMPLETO
    pacote->crc = 8;
    buffer[PACOTE_MAX-1] = pacote->crc;
    mensagens.push_back(pacote); //coloquei mensagem fila de mensagens
    ssize_t status = send(socket,buffer,sizeof(buffer),0);
    if (status == (-1)){
        perror("Erro ao anviar pacote\n");
        exit (-1);
    }
    else{
        printf ("pacotes: %ld enviados com sucesso!\n",status); //numero de bytes enviados, deve ser o tamanho do buffer (67)
    }
    //imprimirFilas(mensagens,janela);
}


void analise_arquivo (int socket){
    std::ifstream file("arquivoEnvio.txt", std::ios::in | std::ios::binary);
    file.seekg(0, std::ios::end);
    std::streampos pos = file.tellg();
    int bytesLidos = static_cast<int>(pos);
    //printf ("bytesLidos:%d\n",bytesLidos);
    if (bytesLidos > 64){
        fragmentar_pacote(file,bytesLidos);
    }
    else{
        file.seekg(0,std::ios::beg);
        char *dadosArquivo = new char[bytesLidos];
        file.read (dadosArquivo,bytesLidos);
        file.close();
        //enviar_pacote(socket,TIPO_DADOS,bytesLidos, dadosArquivo);
    }

}

struct kermit *receber_pacote(int socket,std::list<struct kermit*>& mensagens,std::list<struct kermit*>& janela){
    unsigned char pacote_recebido[PACOTE_MAX] = {0};
    struct kermit *pacoteMontado = new(struct kermit);
    unsigned int byte;
    unsigned int byteShiftado;
    ssize_t byes_recebidos = recv(socket,pacote_recebido, PACOTE_MAX+1,0);
    printf("byte_recebidos: %ld\n",byes_recebidos);
    if (byes_recebidos < 4){ //menor mensagem, com todos os pacotes, é 4 bytes
        perror ("Erro ao receber mensagem\n");
        //não é um dos meus pacotes, então nem faço nada, só volto a escutar;
        return NULL;
    }
    else{
        memcpy(pacoteMontado,pacote_recebido,3);
        printf ("marcador:%u\n",pacoteMontado->m_inicio);
        printf ("tam:%u\n",pacoteMontado->tam);
        printf ("seq:%u\n",pacoteMontado->seq);
        printf ("type:%u\n",pacoteMontado->type);
        memcpy(pacoteMontado->dados,pacote_recebido+3,pacoteMontado->tam);
        printf ("conteudo %s\n",pacoteMontado->dados);
        pacoteMontado->crc = pacote_recebido[PACOTE_MAX-1];
        printf ("crc %u\n",pacote_recebido[PACOTE_MAX-1]);
        //IF CRC aqui eu calcularia o crc para ver se chegou certo, se não eu mando um NACK
        return pacoteMontado;
    }    
}
