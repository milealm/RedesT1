#parametrosCompilacao=-Wall #-Wshadow
nomePrograma=trab

all: $(nomePrograma)

$(nomePrograma): main.o rawsocket.o pacotes.o
	g++ -o $(nomePrograma) main.o rawsocket.o pacotes.o

main.o: main.cpp
	g++ -c main.cpp

rawsocket.o: rawsocket.h rawsocket.cpp
	g++ -c rawsocket.cpp

pacotes.o: pacotes.h definicoes.h pacotes.cpp
	g++ -c pacotes.cpp

clean:
	rm -f *.o *.gch $(nomePrograma)
