#include <iostream>
#include <stdlib.h>
#include <vector>
#include <unistd.h>
#include <ncurses.h>
#include <map>
#include<string>

#define TIMEOUT 10

int g_windowHeight, g_windowWidth;
int g_screenHeight, g_screenWidth;

class Entity {
    public:
        virtual void init() = 0;
        virtual void render() = 0;
        virtual void update() = 0;
    private:
};

class Board
{
public:

   void printBoard()
   {
       for(int i = 0; i < g_screenHeight; ++i){
           for(int j = 0; j < g_screenWidth; ++j){
               if(i == 0 || i == g_screenHeight - 1 || j == 0 || j == g_screenWidth - 1){
                   printw("*");
               } else {
                   printw(" ");
               }
           }
           printw("\n");
       }
   }

private:

};

class Player : public Entity
{
public:

   static const unsigned int threshold = 3;

   int velocityX, velocityY;
   int speedX, speedY;

   Player()
   : velocityX(0), velocityY(1), speedX((g_screenWidth + g_screenHeight) / 2), speedY(threshold)
   {
   }

   ~Player()
   {
   }

   void init()
   {
   }

   void render()
   {
   }

   void update()
   {
       if(speedY <= 1 || (speedY > g_screenHeight - threshold)){
           velocityY = -velocityY;
       }

       if(speedX + velocityX > 0 && speedX + velocityX <= g_screenWidth - threshold){
           speedX += velocityX;
       }

       speedY += 1 * velocityY;
   }


   void moveSnake()
   {
        int KeyPressed = getch();
        switch(KeyPressed)
        {
            case KEY_LEFT:
                this->onMoveLeft();
                break;
            case KEY_RIGHT:
                this->onMoveRight();
                break;
        }

   }

   void onMoveLeft()
   {
       velocityX = -1;
   }
   
   void onMoveRight()
   {
       velocityX = 1;
   }

private:

};

class Enemy : public Entity
{
public:

    Enemy()
    {

    }

    ~Enemy()
    {

    }

    void init()
    {

    }

    void render()
    {

    }

    void update()
    {

    }


private:

};

class Core : public Entity
{
public:

    std::unique_ptr<Player> player;
    std::unique_ptr<Board> board;
    std::unique_ptr<Enemy> enemy;

    Core()
    {
        
    }

    ~Core()
    {

    }

    void init()
    {
        this->getTerminalDimension();
        this->initializeInstance();
    }

    void render()
    {
        printw("Welcome to the Game");
        printw("\n");
        board->printBoard();
        mvwaddch(stdscr, player->speedY, player->speedX, '#');
        wmove(stdscr, g_screenHeight, g_screenWidth);
        // mvprintw(g_screenHeight + 5, 5, "hellollllllllllllll");
    }

    void update()
    {
        player->moveSnake();
        player->update();
    }

    void getTerminalDimension()
    {
        getmaxyx(stdscr, g_windowHeight, g_windowWidth);
        g_screenHeight = g_windowHeight * 0.4;
        g_screenWidth = g_windowWidth * 0.7;
    }

    void initializeInstance()
    {
        player = std::make_unique<Player>();
        board = std::make_unique<Board>();
        enemy = std::make_unique<Enemy>();
    }



private:
    
};

int main(int argc, const char * argv[]) 
{
    initscr();			
    cbreak();
    noecho();
    timeout(TIMEOUT);
    keypad(stdscr, TRUE);

    Core* core = new Core();

    core->init();

    while(getch() != KEY_F(1)) {
        wclear(stdscr);
        core->render();
        core->update();
        wrefresh(stdscr);
        refresh();
    }
    endwin();
}

