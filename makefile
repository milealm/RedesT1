#parametrosCompilacao=-Wall #-Wshadow
nomePrograma=trab

all: $(nomePrograma)

$(nomePrograma): main.o rawsocket.o
	g++ -o $(nomePrograma) main.o rawsocket.o

main.o: main.cpp
	g++ -c main.cpp

rawsocket.o: rawsocket.h rawsocket.cpp
	g++ -c rawsocket.cpp

clean:
	rm -f *.o *.gch $(nomePrograma)
