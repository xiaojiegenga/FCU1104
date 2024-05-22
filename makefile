DST=all
SRC=main.c
SRC+=mailbox.c
SRC+=uart.c
SRC+=mqtt.c
SRC+=csv.c
SRC+=queue.c
SRC+=list.c
CC=gcc 
FLAG=-g -lpaho-mqtt3c -lrt -pthread 
#LIBS+= /home/linux/nfs/paho_lib 
LIBS+= /home/linux/nfs/paho.mqtt.c-master/build/output 
LIBS+= -L/home/linux/nfs/ssl_lib 
LIBS+= -L /mnt/paho.mqtt.c-master/build/output
#INC+=/usr/local/include
INC+=/home/linux/nfs/paho.mqtt.c-master/src

$(DST):$(SRC)
	$(CC) $(SRC) $(FLAG)
arm:$(SRC)
	arm-linux-gnueabihf-gcc $(SRC) $(FLAG) -L $(LIBS) -I $(INC) 
clean:
	rm $(DST)
