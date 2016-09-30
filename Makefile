CC=gcc
CFLAGS = -lpcre -lcurl
LDFLAGS = -lpcre -lcurl

OBJECTS = connector.o parserIEEE.o rep.o parserACM.o lombrico.o outputDblpParser.o

all: $(OBJECTS) $(CC) $(CFLAGS) $(OBJECTS) $(LDFLAGS) $(LDFLAGS) -o lombrico

.PHONY: clean
clean: rm -f *~ *.o lombrico
    
    
    
    