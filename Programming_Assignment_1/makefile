CC=gcc
CFLAGS=-c -Wall -Iinclude
header=include/common.h
SOURCES=$(wildcard src/*.c)
SRCDIR=src
OBJECT=$(patsubst $(SRCDIR)/%,$(OBJDIR)/%,$(SOURCES:.c=.o))
OBJDIR=obj
serverDep=$(OBJDIR)/server.o $(OBJDIR)/helper.o 
OUTPUTDIR=bin
SERVER=echos
CLIENT=echo
all: $(SERVER) $(CLIENT)

$(SERVER): $(OBJECT)
	$(CC) -o $(OUTPUTDIR)/$@ obj/helper.o obj/server.o 

$(CLIENT): $(OBJECT)
	$(CC) -o $(OUTPUTDIR)/$@ obj/helper.o obj/client.o

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean 
clean:
	rm -f $(OBJDIR)/*.o $(OUTPUTDIR)/$(SERVER) $(OUTPUTDIR)/$(CLIENT)