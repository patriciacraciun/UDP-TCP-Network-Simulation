CC = gcc
CFLAGS = -Wall -Wextra -g
TARGET = client

OBJS = buffer.o client.o helpers.o parson.o requests.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

buffer.o: buffer.c buffer.h
	$(CC) $(CFLAGS) -c buffer.c

client.o: client.c helpers.h requests.h buffer.h parson.h
	$(CC) $(CFLAGS) -c client.c

helpers.o: helpers.c helpers.h
	$(CC) $(CFLAGS) -c helpers.c

parson.o: parson.c parson.h
	$(CC) $(CFLAGS) -c parson.c

requests.o: requests.c requests.h helpers.h buffer.h
	$(CC) $(CFLAGS) -c requests.c

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
