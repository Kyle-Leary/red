# Variables
CC := gcc

CFLAGS += -DDEBUG=1
CFLAGS += -Wall -Wno-missing-braces -Wno-unused-variable -g

INCLUDES += -I. -Isrc -Ideps/libwhisper/api

# ignore the whole backend folder, and only add in the backend subfolder.
ALL_BACKEND := src/backends

# The final target (change this to your desired target name)
TARGET := red

# modify the include variables AFTER config.mk.
LIBS := -lm -lpthread 

# Directories
SRC_DIR := src 

# Get a list of all .c files
SRCS := $(shell find $(SRC_DIR) -name '*.c' -type f)
# Filter out .test.c files from the list
SRCS := $(filter-out %.test.c, $(SRCS))

# Object files (corresponding .o files in the obj/ directory)
OBJS := $(patsubst %.c,%.o,$(SRCS))
SYMBOLS := $(OBJS)

LIBWHISPER := deps/libwhisper/libwhisper.a

REQUIREMENTS := $(SYMBOLS)
LIB_REQUIREMENTS := $(LIBWHISPER)

$(info OBJS is $(OBJS))

all: $(TARGET)
	@echo "Target built at path ./$(TARGET)."

# for both, just compile all the symbols into TARGET with CC
$(TARGET): $(REQUIREMENTS) $(LIB_REQUIREMENTS)
	$(CC) -o $@ $(SYMBOLS) $(LIB_REQUIREMENTS) $(CFLAGS) $(LIBS)

$(LIBWHISPER):
	make -C deps/libwhisper

# Pattern rule to compile .c files into .o files
%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS) $(INCLUDES) -g

clean:
	rm -f $(shell find . -name "*.o") $(TARGET) 

# always rebuild test.c
# always rebuild shaders, it's cheap.
.PHONY: clean 
