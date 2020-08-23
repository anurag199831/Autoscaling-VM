all:
	g++ client.cpp -o client -pthread
	gcc -g -Wall monitor.c -o monitor -lvirt
