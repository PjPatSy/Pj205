CC=g++
CFLAGS=-c -Wall -Wextra -ansi -pedantic 
#~ -std=c++11 -stdlib=libc++
#CFLAGS=-c -Wall -Wextra
LDFLAGS=
SOURCES= $(wildcard src/*.cpp src/tinyxml/*.cpp)
INCLUDES = $(wildcard src/tinyxml/*.h)
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=ndet

all: $(SOURCES) $(INCLUDES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@
	
	
.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f ndet src/*.o src/*~ *~ 
  # $(EXECUTABLE)

mrporper:
	rm -f $(EXECUTABLE)
