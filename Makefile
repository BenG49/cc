CC=g++
CFLAGS=-Wall -Wno-switch -Iinclude -std=c++17 -g

SRCS=$(shell find ./src/ -type f -name '*.cpp')
HDRS=$(shell find ./include/ -type f -name '*.hpp')
OBJS=${SRCS:.cpp=.o}

ARGS=
TEST=
TARGET=cc.out

ASM_TARGET=out.s
OBJ_TARGET=${ASM_TARGET:.s=.o}

TEST_SCRIPT=test.sh

.PHONY: test clean debug

$(TARGET): $(OBJS) $(HDRS) main.cpp
	$(CC) $(CFLAGS) -o $@ $(OBJS) main.cpp

%.o: %.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

test: $(TARGET)
	cd ~/write_a_c_compiler; ./test_compiler.sh ~/Documents/Programming/cpp/cc/bcc $(TEST)

clean:
	$(RM) $(TARGET)
	$(RM) $(OBJS)
	$(RM) $(ASM_TARGET)

debug: $(TARGET)
	gdb --args $< tests/a.c
