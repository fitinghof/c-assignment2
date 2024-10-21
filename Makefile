# Compiler and Linking Variables
CC = gcc
CFLAGS = -Wall -fPIC -g -pthread
LIB_NAME = libmemory_manager.so

# Source and Object Files
SRC = memory_manager.c
OBJ = $(SRC:.c=.o)

# Default target
all: mmanager list test_mmanager test_list

ifeq ($(USE_TSAN), 1)
    CFLAGS += -fsanitize=thread
    LDFLAGS += -fsanitize=thread
endif

# Rule to create the dynamic library
$(LIB_NAME): $(OBJ)
	$(CC) -shared -o $@ $(OBJ) $(LDFLAGS)

# Rule to compile source files into object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Build the memory manager
mmanager: $(LIB_NAME)

# Build the linked list
list: linked_list.o

# Test target to run the memory manager test program
test_mmanager: $(LIB_NAME)
	$(CC) $(CFLAGS) -o test_memory_manager test_memory_manager.c -L. -lmemory_manager -lm -g

# Test target to run the linked list test program
test_list: $(LIB_NAME) linked_list.o
	$(CC) $(CFLAGS) -o test_linked_list linked_list.c test_linked_list.c -L. -lmemory_manager -g

#run tests
run_tests: run_test_mmanager run_test_list

# run test cases for the memory manager
run_test_mmanager:
	export LD_LIBRARY_PATH=. && TSAN_OPTIONS=" verbosity=2 history_size=7" && ./test_memory_manager 2

# run test cases for the linked list
run_test_list:
	export LD_LIBRARY_PATH=. && ./test_linked_list 0

# Clean target to clean up build files
clean:
	rm -f $(OBJ) $(LIB_NAME) test_memory_manager test_linked_list linked_list.o