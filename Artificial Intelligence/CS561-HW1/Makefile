csci561_hw1: csci561_hw1.o graph.o
	g++ -o csci561_hw1 -g csci561_hw1.o graph.o

csci561_hw1.o: csci561_hw1.cpp friends.h graph.cpp
	g++ -g -c -Wall csci561_hw1.cpp

graph.o: graph.cpp graph.h
	g++ -g -c -Wall graph.cpp

clean:
	rm -f *.o csci561_hw1



