objects = webserver.o config.o
webserver : $(objects)
	gcc -o webserver $(objects) -pthread -lrt
webserver.o : webserver.c 
config.o : config.c
$(object) : config.h
c : 
	rm webserver.o config.o
cc :
	rm webserver
