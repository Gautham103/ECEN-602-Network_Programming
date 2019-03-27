CC=gcc
CFLAGS=-c -Wall -Iinclude
header=include/common.h
SOURCES=$(wildcard src/*.c)
SRCDIR=src
OBJECT=$(patsubst $(SRCDIR)/%,$(OBJDIR)/%,$(SOURCES:.c=.o))
OBJDIR=obj
serverDep=$(OBJDIR)/proxy_server.o $(OBJDIR)/helper.o 
OUTPUTDIR=bin
proxy_server=proxy_server
all: $(proxy_server)

$(proxy_server): $(OBJECT)
	$(CC) -o $(OUTPUTDIR)/$@ obj/helper.o obj/proxy_server.o -lpthread 

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean 
clean:
	rm -f $(OBJDIR)/*.o $(OUTPUTDIR)/$(proxy_server)
