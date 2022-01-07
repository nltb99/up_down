SRCS  = $(shell find ./src     -type f -name *.cpp)
OBJS = $(SRCS:.cpp=.o)
OBJS2 = $(SRCS:.cpp=.cpp)

GCC ?= g++
VERSION_FLAG := --std=c++17
NCURSES_FLAG := -lncurses
NAME_IN := main.cpp
NAME_OUT := ./src/a
NAME_OUT2 := ./src/a

run:
	$(GCC) $(VERSION_FLAG) $(NCURSES_FLAG) $(OBJS2) -o $(NAME_OUT); $(NAME_OUT2)



