SRCS  = $(shell find ./src -type f -name *.cpp)
OBJS = $(SRCS:.cpp=.cpp)

GCC ?= g++
VERSION_FLAG := --std=c++17
NCURSES_FLAG := -lncurses
NAME_IN := main.cpp
NAME_OUT := ./src/up_down

runmac:
	$(GCC) $(VERSION_FLAG) $(NCURSES_FLAG) $(OBJS) -o $(NAME_OUT); $(NAME_OUT)

runwin:
	$(GCC) src\main.cpp $(NCURSES_FLAG) -o src\up_down && src\up_down

install: 
	brew install gcc gpp


