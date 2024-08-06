#include "definicoes.h"
#include "pacotes.h"
#include "tipos.h"

long long timestamp() {
    struct timeval tp;
    gettimeofday(&tp, NULL);
    return tp.tv_sec*1000 + tp.tv_usec/1000;
}

int codigo_crc(unsigned char *buffer){
    unsigned char crc = 0; // Inicializa o CRC
    for (int i = 0; i < PACOTE_MAX; i++) {
        crc ^= buffer[i]; // XOR o byte atual do buffer com o CRC

        for (int j = 0; j < 8; j++) { // Processar cada bit
            if (crc & 0x80) { // Se o bit mais significativo for 1
                crc = (crc << 1) ^ 0x7; // Desloca à esquerda e aplica o polinômio
            } else {
                crc <<= 1; // Apenas desloca à esquerda
            }
        }
    }
    printf ("crc %d\n",crc);    
    return crc; // Retorna o CRC calculados
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

void verifica_janela(int socket,char *nomeArquivo,std::list <struct kermit*>&janelaClient,std::list <struct kermit*> mensagens, std::list <struct kermit*>janela){
    struct kermit *elementoJan;
    while (!janelaClient.empty()){
        elementoJan = janelaClient.front();
        janelaClient.pop_front(); 
        if (elementoJan->m_inicio != 126){ //incluir checagem crc
            printf ("veio errado ;-;\n");
            struct kermit *enviar = montar_pacote(TIPO_NACK,0,NULL,elementoJan,mensagens);
            enviar_pacote(socket,0,enviar,mensagens);
        }
        else {
            if (elementoJan->type != TIPO_FIM && elementoJan->type == TIPO_DADOS){
                std::fstream file;
                file.open(nomeArquivo, std::ios::out | std::ios::app);
                if (!file){
                    printf ("falha ao abrir arquivo\n");
                    exit (1);
                }
                else{
                    char buffer[63];
                    memcpy(buffer, elementoJan->dados,64);
                    char bufferSemExtra[31];
                    int i = 0;
                    int j = 0;
                    while(i < 64){
                        bufferSemExtra[i] = buffer[j];
                        i++;
                        j+=2;
                    }
                    //buffer[65] = '\0'; // Adicione o caractere nulo
                    file.write(bufferSemExtra, elementoJan->tam-31); // Use write para evitar escrever caracteres extras
                    //file << elementoJan->dados;
                }
            }
            else {
                printf ("recebi um tipo FIM\n");
            }
        }
    }
    janela.clear();
    //printf ("ack!\n");
    struct kermit *enviar = montar_pacote(TIPO_ACK,0,NULL,elementoJan,mensagens);
    enviar_pacote(socket,0,enviar,mensagens);
    printf ("enviei um ack!\n");
}

int process_resposta(int socket,struct kermit *pacote,int decide,std::list<struct kermit*>& mensagens,std::list<struct kermit*>& janela){
    printf ("recebi algo! vamos processar!!\n");
    if (pacote == NULL){
        return TIPO_NACK;
    }
    if (pacote == NULL && decide == 4){
        return FIM_TIMEOUT; //acabou, não tenta enviar de novo;
    }
    else{
        struct kermit *enviar;
        int demora = 0;
        switch (pacote->type){
            case TIPO_ACK:

                //baseado no numero de sequência do pacote, eu tiro algumas mensagens da fila e movo a janela. O negócio é como kk
                printf ("\nEu acho que vi um ACK\n");
                if (janela.empty()){
                    if (mensagens.size() <= 1){ //para deixar uma mensagem na fila para usar de referência para o num se sequencia
                        return TIPO_ACK;
                    }
                    else{
                        for (auto it = mensagens.begin(); it != mensagens.end(); ) {
                            if ((*it)->seq == pacote->seq) {
                                it = mensagens.erase(it); // Remove o elemento e atualiza o iterador
                            } else {
                                ++it; // Avança o iterador apenas se não remover o elemento
                            }
                        }
                        return TIPO_ACK;
                    }
                }
                break;

            case TIPO_NACK:

                //vou ter que reenviar uma janela ou mensagem //envia uma mensagem de erro TIPO_ERRO
                printf ("Eu acho que vi um NACK\n");
                return TIPO_NACK;
                break;

            case TIPO_LIST:

                //o que o servidor recebe para começar a enviar os nomes dos arquivos
                printf ("\n\nEu acho que vi um LIST\n");
                listType(socket,pacote,mensagens,janela);
                printf ("\nvamos voltar a escutar...\n");
                return TIPO_LIST;
                break;

            case TIPO_BAIXAR:

                //estou no servidor
                printf ("Eu acho que vi um BAIXAR\n");
                //vou mandar um descritor de arquivo depois de mandar um ACK
                baixarType(socket,pacote,mensagens,janela);
                //printf ("voltou?\n");
                return TIPO_BAIXAR;
                break;

            case TIPO_MOSTRA:

                printf ("\nEu acho que vi um MOSTRA\n");
                //o que o cliente recebe com os dados que ele pediu para mostrar, vão ser várias mensagens, cada uma com um dos arquivos do servidor
                mostraType(socket,pacote,mensagens,janela);
                return TIPO_MOSTRA;
                break;

            case TIPO_DESCREVE:

                printf ("Eu acho que vi um DESCREVE\n");
                //o que o cliente vai receber depois de mandar um baixar, receber um ack, indica que vai começar a mandar dados
                enviar = montar_pacote(TIPO_ACK,0,NULL,NULL,mensagens);
                enviar_pacote(socket,0,enviar,mensagens);
                printf ("Seu video -%s- começará a ser baixado agora\n",pacote->dados);
                //nessa função já pode ter um loop no cliente para receber os dados e ir juntando (TIPO_DADOS)
                descreverType(socket,pacote,mensagens,janela);
                printf ("acabou! da pra abrir o player eu acho k\n");
                return TIPO_FIM;
                break;

            case TIPO_FIM:
                printf ("\nEu acho que vi um FIM\n");
                enviar = montar_pacote(TIPO_ACK,0,NULL,pacote,mensagens);
                enviar_pacote(socket,0,enviar,mensagens);
                //o que o cliente recebe para saber que o vídeo já foi todo mandado e dá para dar play no que foi enviado
                if (pacote->tam == 1){ //fim 
                    printf ("Esses são todos os arquivos disponíveis\n");
                    return TIPO_MOSTRA;
                }
                else{
                    printf ("hora de abrir o player :)\n");
                    return TIPO_FIM;
                }
                break;
        }
    }
    return TIPO_NACK;
}

struct kermit *montar_pacote(int tipo,int bytesLidos,char *dadosArquivo,struct kermit*anterior,std::list<struct kermit *> &mensagens){
    unsigned char buffer[PACOTE_MAX] = {0}; //64 + 4 bytes dos outros campso do frame (8 + 6 + 5 + 5 bits = 3 bytes)
    struct kermit *pacote = new struct kermit; //aloquei estrutura onde eu vou guardar o meu pacote
    //marcador de início
    pacote->m_inicio = MARCADOR_DE_INICIO;
    //tamanho da área de dados
    pacote->tam = bytesLidos;
    //num de sequencia
    if (anterior){
        if ((tipo != TIPO_ACK) && (tipo!=TIPO_NACK)){
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
    //dados
    if (dadosArquivo!= NULL){
        memcpy(pacote->dados, dadosArquivo, bytesLidos);
    }
    //crc INCOMPLETO
    pacote->crc = 0;
    return pacote;
}

void enviar_pacote(int socket,int bytesLidos,struct kermit *pacote,std::list<struct kermit*>& mensagens){
    unsigned char buffer[PACOTE_MAX] = {0};
    memcpy(buffer,pacote,3); //põe os 3 primeiros bytes do pacote no buffer (marcador, tamanho, seq e tipo)
    //dados
    if (pacote->dados != 0){
        memcpy(buffer+3,pacote->dados,bytesLidos); //coloca os dados no buffer
    }
    //buffer[PACOTE_MAX-1] = pacote->crc;
    int crc = codigo_crc(buffer);
    buffer[PACOTE_MAX-1] = crc;
    printf ("ENVIANDO...");
    print_buffer(buffer,PACOTE_MAX);
    printf ("inicio: %d, tam: %d, seq: %d, crc: %d\n\n", pacote->m_inicio,pacote->tam,pacote->seq, buffer[PACOTE_MAX-1]);
    if (pacote->type!= TIPO_ACK || TIPO_NACK){
        mensagens.push_back(pacote); //coloquei mensagem fila de mensagens
    }
    ssize_t status = send(socket,buffer,sizeof(buffer),0);
    if (status == (-1)){
        perror("Erro ao anviar pacote\n");
        exit (-1);
    }
    else{
        //printf ("pacotes: %ld enviados com sucesso!\n",status); //numero de bytes enviados, deve ser o tamanho do buffer (67)
    }
}

struct kermit *receber_pacote(int socket,int demora,std::list<struct kermit*>& mensagens,std::list<struct kermit*>& janela){
    long long comeco = timestamp();
    struct timeval timeout = { .tv_sec = TIMEOUT_MILLIS/1000, .tv_usec = (TIMEOUT_MILLIS%1000) * 1000 };
    setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (char*) &timeout, sizeof(timeout));
    
    unsigned char pacote_recebido[PACOTE_MAX] = {0};
    struct kermit *pacoteMontado = new(struct kermit);
    unsigned int byte;  
    unsigned int byteShiftado;
    ssize_t bytes_recebidos = 0;
    int timeoutDaVez = 1;
    for (int j = 0; j <= demora; j++){
        timeoutDaVez = timeoutDaVez * TIMEOUT_MILLIS * (demora+1); //exponencial, só n é pq demora muito
    }
    while (timestamp() - comeco <= timeoutDaVez && bytes_recebidos <= 0){
        bytes_recebidos = recv(socket,pacote_recebido, PACOTE_MAX+1,0);
    }
    if ((timestamp()- comeco > timeoutDaVez) || (bytes_recebidos < 68)){
        if (demora == 4){
            printf ("Agora já deu, não deu pra enviar e pronto ;-; volte para o início");
        }
        return NULL;
    }
    else{
        memcpy(pacoteMontado,pacote_recebido,3);
        memcpy(pacoteMontado->dados,pacote_recebido+3,pacoteMontado->tam);
        pacoteMontado->crc = pacote_recebido[PACOTE_MAX-1];
        int crc = codigo_crc(pacote_recebido);
        print_buffer(pacote_recebido,PACOTE_MAX);
        printf ("inicio: %d, tam: %d, seq: %d, crc: %d e %d, crcCalculado: %d\n\n", pacoteMontado->m_inicio,pacoteMontado->tam,pacoteMontado->seq,pacoteMontado->crc, pacote_recebido[PACOTE_MAX-1],crc);
        if (crc != pacoteMontado->crc){
            struct kermit *enviar = montar_pacote(TIPO_NACK,0,NULL,pacoteMontado,mensagens);
            enviar_pacote(socket,0,enviar,mensagens);
            return NULL;
        }
        return pacoteMontado;
    }    
}

void print_buffer(unsigned char* buffer, int length) {
    for (size_t i = 0; i < length; i++) {
        printf("%02X ", buffer[i]);
        if ((i + 1) % 16 == 0) {
            printf("\n"); // Adiciona uma nova linha a cada 16 bytes para melhor legibilidade
        }
    }
    printf("\n"); // Adiciona uma nova linha ao final para separação
}

