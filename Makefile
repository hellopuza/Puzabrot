####

CC = g++
CFLAGS = -c -march=native -msse4.2 -openmp -O3 -std=c++17
LDFLAGS =
LIBS = -lsfml-system -lsfml-graphics -lsfml-window
SOURCES = main.cpp Puzabrot.cpp Calculator/Calculator.cpp StringLib/StringLib.cpp
OBJECTS = $(SOURCES:.cpp=.o)
EXECUTABLE = Puzabrot

all: $(SOURCES) $(EXECUTABLE) clean

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) $(LIBS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm $(OBJECTS)


