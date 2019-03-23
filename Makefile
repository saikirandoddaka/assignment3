CC = gcc
COMPILER_FLAGS = -g -std=gnu99
LINKER_FLAGS = -g -lpthread -lm
BINARYOSS = master
BINARYUSER = palin
OBJSOSS = oss.o common.o
OBJSUSER = user.o common.o
HEADERS = common.h

all: $(BINARYOSS) $(BINARYUSER)

$(BINARYOSS): $(OBJSOSS)
	$(CC) -o $(BINARYOSS) $(OBJSOSS) $(LINKER_FLAGS)

$(BINARYUSER): $(OBJSUSER)
	$(CC) -o $(BINARYUSER) $(OBJSUSER) $(LINKER_FLAGS)

%.o: %.c $(HEADERS)
	$(CC) $(COMPILER_FLAGS) -c $<

clean:
	/bin/rm -f $(OBJSOSS) $(OBJSUSER) $(BINARYOSS) $(BINARYUSER)

dist:
	zip -r palin.zip *.c *.h Makefile README
