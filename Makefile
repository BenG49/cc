CUR_DIR := $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

CC := g++
CFLAGS := -Wall -Wno-switch -I$(CUR_DIR)/include -std=c++17 -g

SRCS := $(shell find $(CUR_DIR)/src/ -type f -name '*.cpp')
HDRS := $(shell find $(CUR_DIR)/include/ -type f -name '*.hpp')
OBJS := ${SRCS:.cpp=.o}

# input vars, set on cmd line
ARGS=
TEST=

TARGET=cc.out

ASM_TARGET=out.s
OBJ_TARGET=${ASM_TARGET:.s=.o}

.PHONY: test clean debug

$(TARGET): $(OBJS) $(HDRS) $(CUR_DIR)/main.cpp
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(CUR_DIR)/main.cpp

%.o: %.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

# write a c compiler needs to be in ~, or just change the dir
test: $(TARGET)
	cd ~/write_a_c_compiler; ./test_compiler.sh $(CUR_DIR)/bcc $(TEST)

clean:
	$(RM) $(TARGET)
	$(RM) $(OBJS)
	$(RM) tests/a

debug: $(TARGET)
	gdb --args $< tests/a.c
