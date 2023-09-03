# Variables
CC := gcc

# rdynamic makes libc stack traces better, actually shows function names
# instead of just offsets. NOTE: static functions are not linked in the 
# dynamic linkage table, so don't make something static if you want it
# to show up in the stack trace.
CFLAGS += -DDEBUG=1 -rdynamic
CFLAGS += -Wall -Wno-missing-braces -Wno-unused-variable -ggdb

# which debugger do you use?
GDB:=gf2

INCLUDES += -I. -Isrc 

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

OBJS := $(patsubst %.c,%.o,$(SRCS))
SYMBOLS := $(OBJS)

LIBWHISPER := deps/libwhisper/libwhisper.a
LIBTERMBUFFER := deps/libtermbuffer/libtermbuffer.a

STATIC := $(LIBWHISPER) $(LIBTERMBUFFER)
# include the /api headers in each build for each submodule.
API_DIRS := $(foreach lib,$(STATIC),$(patsubst %/,%/api,$(dir $(lib))))
INCLUDES += $(foreach dir,$(API_DIRS),$(addprefix -I,$(dir)))
# $(info api_dirs - $(API_DIRS))
# $(info includes - $(INCLUDES))

SYMBOLS += $(STATIC)

DEPS := $(STATIC)

REQUIREMENTS := $(SYMBOLS)

all: $(TARGET)
	@echo "Target built at path ./$(TARGET)."

# for both, just compile all the symbols into TARGET with CC
$(TARGET): $(REQUIREMENTS) $(LIB_REQUIREMENTS)
	$(CC) -o $@ $(SYMBOLS) $(LIB_REQUIREMENTS) $(CFLAGS) $(LIBS)

# just make each dependency using a generic make -C in the $(dir $(DEP)) base dir
# of the dependency.
define create_rule
$(1):
	@echo "Making dependency in $(dir $(1))"
	make -C $(dir $(1))
endef

# eval to dynamically generate the rule
# call to call a user-defined function.
# in total, this defines all the rules we need dynamically, each time the makefile is run.
$(foreach dep,$(DEPS),$(eval $(call create_rule,$(dep))))

# Pattern rule to compile .c files into .o files
%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS) $(INCLUDES)

debug: $(TARGET)
	setsid gdbserver :1234 ./$(TARGET) &
	sleep 0.1
	$(GDB) -ex "target remote localhost:1234"
	killall gdbserver

clean:
	rm -f $(shell find . -name "*.o") $(TARGET) $(STATIC)


.PHONY: clean 
