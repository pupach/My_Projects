OBJECTS = $(DIR_O)/main.o $(DIR_O)/graphviz_dump.o $(DIR_O)/list.o $(DIR_O)/work_with_file.o

CC = g++
CC = g++
CFLAGS = -c -Wshadow -Winit-self -Wredundant-decls -Wcast-align -Wundef -Wfloat-equal -Winline -Wunreachable-code -Wmissing-declarations -Wmissing-include-dirs -Wswitch-enum -Wswitch-default -Weffc++ -Wmain -Wextra -Wall -g -pipe -fexceptions -Wcast-qual -Wconversion -Wctor-dtor-privacy -Wempty-body -Wformat-security -Wformat=2 -Wignored-qualifiers -Wlogical-op -Wno-missing-field-initializers -Wnon-virtual-dtor -Woverloaded-virtual -Wpointer-arith -Wsign-promo -Wstack-usage=8192 -Wstrict-aliasing -Wstrict-null-sentinel -Wtype-limits -Wwrite-strings -Werror=vla -D_DEBUG -D_EJUDGE_CLIENT_SIDE
LFLAGS = -fsanitize=address
DIR_O = ./object
DIR_SOUR = ./source
DIR_STACK = ../Stack
DIR_ONEGIN_SOUR= ../str_project/source
DIR_ONEGIN_O = ../str_project/object
DIR_LIB = ../my_lib


# 1) use wildcard to get all cpps
# 2) add a rule to make .o from .cpp
# 3) add var for list of all .o or .cpp files needed to make a project
# 4) target <executable>: $(OBJFILES)
#	g++ $(OBJFILES) -o <
# 5)

kotik.elf: $(OBJECTS)
	$(CC) $(OBJECTS) $(LFLAGS) -o kotik.elf

$(DIR_O)/main.o: $(DIR_SOUR)/main.cpp
	$(CC) $(CFLAGS) $< -o $@

$(DIR_O)/graphviz_dump.o: $(DIR_SOUR)/graphviz_dump.cpp
	$(CC) $(CFLAGS) $< -o $@

$(DIR_O)/work_with_file.o: $(DIR_LIB)/work_with_file.cpp
	$(CC) $(CFLAGS) $< -o $@

$(DIR_O)/list.o: $(DIR_SOUR)/list.cpp
	$(CC) $(CFLAGS) $< -o $@
