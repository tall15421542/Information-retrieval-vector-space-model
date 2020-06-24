CFLAGS=-O1
Main: main.o query.o model.o vsm.o 
	g++ $(CFLAGS) -o Main main.o query.o model.o vsm.o
main.o: main.cpp query.hpp model.hpp
	g++ $(CFLAGS) -c main.cpp
query.o: query.cpp query.hpp vsm.hpp common.hpp
	g++ $(CFLAGS) -c query.cpp
vsm.o: vsm.cpp vsm.hpp model.hpp common.hpp
	g++ $(CFLAGS) -c vsm.cpp
model.o: model.cpp model.hpp
	g++ $(CFLAGS) -c model.cpp
clean:
	rm *.o Main
