#parametrosCompilacao=-Wall #-Wshadow
nomePrograma=trab

all: $(nomePrograma)

$(nomePrograma): main.o rawsocket.o pacotes.o tipos.o
	g++ -o $(nomePrograma) main.o rawsocket.o pacotes.o tipos.o

main.o: main.cpp
	g++ -c main.cpp

rawsocket.o: rawsocket.h rawsocket.cpp
	g++ -c rawsocket.cpp

pacotes.o: pacotes.h definicoes.h tipos.h pacotes.cpp
	g++ -c pacotes.cpp

tipos.o: tipos.h definicoes.h pacotes.h tipos.cpp
	g++ -c tipos.cpp
clean:
	rm -f *.o *.gch $(nomePrograma)
