SRCS  = $(shell find ./src     -type f -name *.cpp)
OBJS = $(SRCS:.cpp=.o)
OBJS2 = $(SRCS:.cpp=.cpp)

GCC ?= g++
VERSION_FLAG := --std=c++17
NCURSES_FLAG := -lncurses
NAME_IN := main.cpp
NAME_OUT := ./src/up_down
NAME_OUT2 := ./src/up_down

run:
	$(GCC) $(VERSION_FLAG) $(NCURSES_FLAG) $(OBJS2) -o $(NAME_OUT); $(NAME_OUT2)



