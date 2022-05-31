####
#Anna Gogoula
###

# compiler
CC = gcc
# Compile options
CFLAGS = -g -Wall
# Αρχεία .o
OBJS = remoteClient.o dataServer.o
#HEADER = 


# Το εκτελέσιμο πρόγραμμα
EXEC = s

# Παράμετροι για δοκιμαστική εκτέλεση
#ARGS = 

$(EXEC): $(OBJS)
	$(CC) $(OBJS) -o $(EXEC)
	@if [ -f $(EXEC).exe ]; then ln -sf $(EXEC).exe $(EXEC); fi

clean:
	rm -f $(OBJS) $(EXEC) $(OUT1) $(OUT2)

run: $(EXEC)
	./$(EXEC)

valgrind: $(EXEC)
	valgrind --error-exitcode=1 --leak-check=full --show-leak-kinds=all ./$(EXEC) $(ARGS)