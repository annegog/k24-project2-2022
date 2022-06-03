# compiler
CC = gcc
# Compile options
CFLAGS = -g -Wall
FLAGT = -lpthread
# Αρχεία .o
OBJS = dataServer.o
HEADER = server.h 


# Το εκτελέσιμο πρόγραμμα
EXEC = dataServer

# Παράμετροι για δοκιμαστική εκτέλεση
ARGS = -p 12501 -s 2 -q 2 -b 543

$(EXEC): $(OBJS)
	$(CC) $(OBJS) -o $(EXEC) $(FLAGT)
	@if [ -f $(EXEC).exe ]; then ln -sf $(EXEC).exe $(EXEC); fi

clean:
	rm -f $(OBJS) $(EXEC)

run: $(EXEC)
	./$(EXEC) $(ARGS)

valgrind: $(EXEC)
	valgrind --error-exitcode=1 --leak-check=full --show-leak-kinds=all ./$(EXEC) $(ARGS)