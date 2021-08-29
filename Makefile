CC=g++
CFLAGS=-Wall -Iinclude -std=c++17 -g

SRCS=$(shell find ./src/ -type f -name '*.cpp')
HDRS=$(shell find ./include/ -type f -name '*.hpp')
OBJS=${SRCS:.cpp=.o}

TARGET=a.out

.PHONY: run clean debug valgrind

$(TARGET): $(OBJS) $(HDRS) main.cpp
	$(CC) $(CFLAGS) -o $@ $(OBJS) main.cpp

%.o: %.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

run: $(TARGET)
	./$<

clean:
	$(RM) $(TARGET)
	$(RM) $(OBJS)

debug: $(TARGET)
	gdb --args $< tests/a.c

valgrind: $(TARGET)
	valgrind --leak-check=full --track-origins=yes /home/bg/Documents/Programming/cpp/cc/$< /home/bg/Documents/Programming/cpp/cc/tests/a.c &> debug
