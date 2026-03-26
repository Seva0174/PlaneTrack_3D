CC      = gcc
CFLAGS  = -Wall -Wextra -std=c99 -O2
LIBS    = -lGL -lGLU -lglut -lm
TARGET  = jeu_avion
SRCS    = main.c avion.c

OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean