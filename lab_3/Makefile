SOURCES = $(wildcard *.c)
BINARY = $(patsubst %.c, %, $(SOURCES))

FLAGS = -W -pedantic -std=c2x -O3 -fopenmp
LIBS = -lgomp
.PHONY = all clean

all: $(BINARY)

# Specjalna reguła dla game_smart_bot - linkuj z wszystkimi zależnościami
game_smart_bot: game_smart_bot.c heuristic.c opening_book.c
	$(CC) $(FLAGS) $^ -o $@ $(LIBS)
	strip $@

$(BINARY): %: %.c
	$(CC) $(FLAGS) $^ -o $@ $(LIBS)
	strip $@

clean:
	rm -f $(BINARY)

