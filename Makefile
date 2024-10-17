CC = gcc
CFLAGS = -Wall

CLIENT_SRCS = main.c parser.c client.c
CLIENT_OBJS = $(CLIENT_SRCS:.c=.o)

SERVER_SRCS = student.c writer.c server.c
SERVER_OBJS = $(SERVER_SRCS:.c=.o)

SERVER_TARGET = server
CLIENT_TARGET = client

# Rule to build the server target
$(SERVER_TARGET): $(SERVER_OBJS)
	$(CC) $(CFLAGS) -o $(SERVER_TARGET) $(SERVER_OBJS)

# Rule to build the client target
$(CLIENT_TARGET): $(CLIENT_OBJS)
	$(CC) $(CFLAGS) -o $(CLIENT_TARGET) $(CLIENT_OBJS)

# Pattern rule to compile .c files to .o files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Define dependencies for the source files
main.o: parser.h parser.c 
parser.o: parser.h client.h client.c
client.o: client.h socket.h client.c

server.o: student.h writer.h socket.h student.c writer.c
student.o: student.h
writer.o: writer.h

# Clean up generated files
clean:
	rm -f $(SERVER_TARGET) $(CLIENT_TARGET) $(CLIENT_OBJS) $(SERVER_OBJS)
