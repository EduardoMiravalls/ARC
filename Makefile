#
# Author: Eduardo Miravalls Sierra
#
# Date: 2014-06-11 20:35

.PHONY: doc all clean tests
.PRECIOUS: obj/%.o

# Variables
SRC      :=$(shell find src -name '*.c')
TEST     :=$(shell find test -name '*.c')

SRC_OBJ  :=$(addprefix obj/,$(SRC:.c=.o))

TESTS    :=$(TEST:.c=)

CC := gcc
CFLAGS := -Wall -Wextra -Iinclude -Isrc

all:	lib/libARC.a
	@echo DONE!

lib/libARC.a:	CFLAGS += -O2
lib/libARC.a:	$(SRC_OBJ)

# build tests and execute them
tests:	lib/libARC.a
	@for test in $(TESTS) ; do \
		echo "\nExecuting $$test:\n" ; \
		mkdir -p $$(dirname bin/$$test) ; \
		$(CC) $(CFLAGS) -g $$test.c -o bin/$$test -Llib -lARC; \
		valgrind --leak-check=full bin/$$test ; \
	done

print:
	@echo $(TESTS)
	@echo $(TEST_OBJ)

clean:
	rm -f lib/*
	find obj/ -type f -delete

doc:
	doxygen doc/Doxyfile &> /dev/null
	@echo documentation generated in doc/

# Rules
obj/%.o:	%.c
	@mkdir -p $(@D) 
	@echo "#----------------------------------------"
	@echo "#Building $@:"
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@
	@echo ""

lib/%.a:
	@mkdir -p $(@D)
	@echo "#----------------------------------------"
	@echo "#Generating static library $@:"
	ar rcs $@ $^
	@echo ""

# sources' dependencies
DEP      :=$(shell find obj/ -type f -name '*.d')
-include $(DEP)
