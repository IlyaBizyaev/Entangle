CC=gcc
CFLAGS=-std=c99 -c -Wall -I/home/rostyslav/Projects/modix/modwm/src/
LDFLAGS=-lX11
SOURCES=main.c src/modwm.c src/log.c src/error.c src/event_process.c src/window.c src/frame.c
OBJECTS=$(SOURCES:.c=.o)
DEPS=src/modwm.h src/log.h src/error.h src/event_process.h src/window.h src/frame.h
EXECUTABLE=bin/modwm

all: $(SOURCES) $(DEPS) $(EXECUTABLE)
    
$(EXECUTABLE): $(OBJECTS) ;  $(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o: $(DEPS) ; $(CC) $(CFLAGS) $< -o $@

clean: ; for o in $(OBJECTS) rm o done
