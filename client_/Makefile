# compiler
CC = gcc
# Compile options
CFLAGS = -g -Wall
# Αρχεία .o
OBJS = remoteClient.o
HEADER = client.h 


# Το εκτελέσιμο πρόγραμμα
EXEC = remoteClient

# Παράμετροι για δοκιμαστική εκτέλεση
ARGS = -i 192.168.1.5 -p 12502 -d ../../../../server

$(EXEC): $(OBJS)
	$(CC) $(OBJS) -o $(EXEC)
	@if [ -f $(EXEC).exe ]; then ln -sf $(EXEC).exe $(EXEC); fi

clean:
	rm -f $(OBJS) $(EXEC)

run: $(EXEC)
	./$(EXEC) $(ARGS)

valgrind: $(EXEC)
	valgrind --error-exitcode=1 --leak-check=full --show-leak-kinds=all ./$(EXEC) $(ARGS)