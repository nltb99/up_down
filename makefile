SRCS  = $(shell find ./src -type f -name *.cpp)
OBJS = $(SRCS:.cpp=.cpp)

GCC ?= g++
VERSION_FLAG := --std=c++17
NCURSES_FLAG := -lncurses
SQLITE3_FLAG := -lsqlite3
NAME_IN := up_down.cpp
NAME_OUT := ./src/up_down

runmac:
	$(GCC) $(NCURSES_FLAG) $(SQLITE3_FLAG) $(OBJS) -o $(NAME_OUT); $(NAME_OUT)

runwin:
	$(GCC) src\up_down.cpp $(NCURSES_FLAG) $(SQLITE3_FLAG) -o src\up_down && src\up_down

install: 
	brew install gcc gpp


