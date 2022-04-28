TARGET = restaurantFork restaurantThread

all: ${TARGET}

restaurantFork: restaurantFork.cpp
	g++ restaurantFork.cpp -o restaurantFork -pthread -O2 -I.

restaurantThread: restaurantThreads.cpp
	g++ restaurantThreads.cpp -o restaurantThread -pthread -O2 -I.

runFork: all
	./restaurantFork

runThreads: all
	./restaurantThread

.PHONY: clean
clean:
	rm -f *.o ${TARGET}
