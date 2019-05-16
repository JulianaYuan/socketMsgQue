CC = g++

#AR = ar

CFLAG = -O2 -W -Wall -fPIC

INCLUDE =
TARGET = libskmsgque.so
OBJ = msgsocket.o remsg.o remsgque.o
SRC = msgsocket.c remsg.c remsgque.c


all:$(TARGET)
$(OBJ):$(SRC)
	$(CC) $(CFLAG) $(INCLUDE) $(LIB) -c $(SRC)

$(TARGET):$(OBJ)
	$(AR) cq $@ $^

clean:
	rm -f *.o $(TARGET)



