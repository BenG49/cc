CC=g++
CFLAGS=-Wall -Wno-switch -Iinclude -std=c++17 -g

SRCS=$(shell find ./src/ -type f -name '*.cpp')
HDRS=$(shell find ./include/ -type f -name '*.hpp')
OBJS=${SRCS:.cpp=.o}

ARGS=
TARGET=a.out

ASM_TARGET=out.s
OBJ_TARGET=${ASM_TARGET:.s=.o}
BIN_TARGET=prog.out

TEST_SCRIPT=test.sh

.PHONY: run clean debug valgrind

$(TARGET): $(OBJS) $(HDRS) main.cpp
	$(CC) $(CFLAGS) -o $@ $(OBJS) main.cpp

%.o: %.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

test: $(TARGET)
	./$< $(ARGS)
	gcc $(ASM_TARGET) -o $(BIN_TARGET)

	./test.sh $(BIN_TARGET) $(ARGS)

clean:
	$(RM) $(TARGET)
	$(RM) $(OBJS)
	$(RM) $(ASM_TARGET)

debug: $(TARGET)
	gdb --args $< tests/a.c

valgrind: $(TARGET)
	valgrind --leak-check=full --track-origins=yes /home/bg/Documents/Programming/cpp/cc/$< /home/bg/Documents/Programming/cpp/cc/tests/a.c &> debug
