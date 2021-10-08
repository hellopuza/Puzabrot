####

CC = g++
CFLAGS = -c -openmp -O3 -std=c++17
LDFLAGS =
LIBS = -lsfml-system -lsfml-graphics -lsfml-window -lsfml-audio
SOURCES = main.cpp Puzabrot.cpp Calculator/Calculator.cpp
OBJECTS = $(SOURCES:.cpp=.o)
EXECUTABLE = Puzabrot

all: $(SOURCES) $(EXECUTABLE) clean

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) $(LIBS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm $(OBJECTS)


