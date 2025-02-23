SOURCES = main.cpp analyzeDir.cpp
CPPC = g++
CPPFLAGS = -c -Wall -O2
LDLIBS = 
OBJECTS = $(SOURCES:.cpp=.o)
TARGET = analyzeDir

all: $(TARGET)

# ensure analyzeDir.o and main.o are rebuilt if analyzeDir.h changes
analyzeDir.o: analyzeDir.h
main.o: analyzeDir.h
%.o : %.c
$(OBJECTS): Makefile 

.cpp.o:
	$(CPPC) $(CPPFLAGS) $< -o $@

$(TARGET): $(OBJECTS)
	$(CPPC) -o $@ $(OBJECTS) $(LDLIBS)

.PHONY: clean
clean:
	rm -f *~ *.o $(TARGET) 

