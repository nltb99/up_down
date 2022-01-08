#include <iostream>
#include <stdlib.h>
#include <vector>
#include <unistd.h>
#include <ncurses.h>
#include <map>
#include<string>

#define THRESHOLD 2

unsigned short int g_windowWidth, g_windowHeight;
unsigned short int g_screenWidth, g_screenHeight;
unsigned short int TIMEOUT;

enum : int {
    DIR_LEFT,
    DIR_RIGHT,
    DIR_UP,
    DIR_DOWN,
    FROM_LEFT,
    FROM_RIGHT,
};

class Entity {
    public:
        virtual void init() = 0;
        virtual void render() = 0;
        virtual void update() = 0;
    private:
};

class Board : public Entity
{
public:

    Board()
    {

    }

    ~Board()
    {
        this->init();
    }

    void init()
    {

    }
     
    void render()
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

    void update()
    {

    }

private:

};

class Player : public Entity
{
public:

   int velocityX, velocityY;
   int posX, posY;
   char chr;

   Player(char chr)
   : velocityX(0), velocityY(1), posX((g_screenWidth + g_screenHeight) / 2), posY(THRESHOLD), chr(chr)
   {
       this->init();
   }

   ~Player()
   {
   }

   void init()
   {
   }

   void render()
   {
        mvwaddch(stdscr, posY, posX, chr);
        wmove(stdscr, g_screenHeight, g_screenWidth);
        // mvprintw(g_screenHeight + 5, 5, "hello");
   }

   void update()
   {
       this->onListenKeyPressed();

        // * Reverse direction Y
        if(posY + (1 * velocityY) < THRESHOLD || (posY > g_screenHeight - THRESHOLD)){
           velocityY = -velocityY;
        }
    
        // * Bounding checking direction X
        if(posX + velocityX > 0 && posX + velocityX <= g_screenWidth - THRESHOLD){
           posX += velocityX;
        }

       posY += 1 * velocityY;
   }

   void onListenKeyPressed()
   {
        int keyPressed = getch();
        switch(keyPressed)
        {
            case KEY_LEFT:
                this->onMoveLeft();
                break;
            case KEY_RIGHT:
                this->onMoveRight();
                break;
            case KEY_UP:
            case KEY_DOWN:
                this->onStand();
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

   void onStand()
   {
       velocityX = 0;
   }

private:

};

class Enemy : public Entity
{
public:
    struct BaseEnemy {
        int posX, posY;
        int velocityX, velocityY;
        bool bVisible;
    } be;

    static const unsigned int cluster2 = 2;
    static const unsigned int cluster3 = 3;
    static const unsigned int cluster4 = 4;

    BaseEnemy enemies[cluster4];

    char chr;
    int fromDirection, waveDirection;
    int initPosX, initPosY, spaceBetween;
    float proportionX, proportionY;
    float slope;
    int initVelocityX, initVelocityY;

    Enemy(char chr, unsigned int fromDirection, unsigned int waveDirection, float proportionX, float proportionY)
    : chr(chr), fromDirection(fromDirection), waveDirection(waveDirection), proportionX(proportionX), proportionY(proportionY)
    {
        this->init();
    }

    ~Enemy()
    {
    }

    // { 3, 15}
    void init()
    {   
        this->getResponsiveScale();
        // ?
        // proportionY = (float)((cluster4 - 1)  * (cluster4 - 1)) / g_screenHeight;
        float tempProportion = (float) cluster4 / g_screenHeight;
        proportionY = (float) 1 - tempProportion;
        // proportionY = 0.;
        spaceBetween = 15;
        enemies[0].bVisible = true;

        if(fromDirection == FROM_LEFT){
            initPosX = 20;
            initVelocityX = 1;
        } else if(fromDirection == FROM_RIGHT){
            initPosX = g_screenWidth - THRESHOLD;
            initVelocityX = -1;
        }
       
        if(fromDirection == FROM_LEFT){
            initPosY = g_screenHeight * proportionY;
        } else {
            initPosY = g_screenHeight * proportionY;
        }
        
        short int waveValue;
        if(waveDirection == DIR_UP){
            waveValue = -1;
        } else if(waveDirection == DIR_DOWN){
            waveValue = 1;
        }
        
        for(int i = 0; i < cluster4; ++i){
            enemies[i].posX = (i * spaceBetween) + initPosX;
            enemies[i].posY = (i * slope) * waveValue + initPosY;
            enemies[i].velocityX = initVelocityX;
            enemies[i].velocityY = 0;
        }
    }

    void render()
    {
        // ?
        int sumX = 0, sumY = 0;
        
        for(int i = 0; i < cluster4; ++i){
            sumX += enemies[i].posX;
            sumY += enemies[i].posY;

            if(enemies[i].bVisible){
                mvwaddch(stdscr, enemies[i].posY, enemies[i].posX, chr);
                wmove(stdscr, g_screenHeight, g_screenWidth);
            }
        }

        int sum = (cluster4 - 1)  * (cluster4 - 1);

        mvprintw(g_screenHeight + 5, 5, std::to_string(enemies[0].posY).c_str());
        mvprintw(g_screenHeight + 10, 5, std::to_string(enemies[1].posY).c_str());
        mvprintw(g_screenHeight + 15, 5, std::to_string(enemies[2].posY).c_str());
        mvprintw(g_screenHeight + 20, 5, std::to_string(enemies[3].posY).c_str());
        mvprintw(g_screenHeight + 20, 10, std::to_string(sum).c_str());
        mvprintw(g_screenHeight + 20, 15, std::to_string(g_screenHeight).c_str());
        float ok = (float)((cluster4 - 1) * (cluster4 - 1)) / g_screenHeight;
        mvprintw(g_screenHeight + 20, 25, std::to_string(ok).c_str());

    }

    void update()
    {
        unsigned short int countVisible = 0;

        for(int i = 0; i < cluster4; ++i){
            // * Bounding checking 
            // if(
            //     enemies[i].posX + enemies[i].velocityX <= 0 || // Bounding X LEFT
            //     enemies[i].posX + enemies[i].velocityX > g_screenWidth - THRESHOLD || // Bounding X RIGHT
            //     enemies[i].posY + enemies[i].velocityY < THRESHOLD || // Bounding Y Top
            //     enemies[i].posY > g_screenHeight - THRESHOLD // Bounding Y DOWN
            // ){
            //     enemies[i].bVisible = false;
            // } else{
            //     enemies[i].bVisible = true;
            // }
            enemies[i].bVisible = true;

            // enemies[i].posX += enemies[i].velocityX;
            // enemies[i].posY += enemies[i].velocityY;

            if(!enemies[i].bVisible){
                ++countVisible;
            }
        }

        // * call destructor when cluster out of scope
        if(countVisible == cluster4){
            this->~Enemy();
        }
    }

    void getResponsiveScale()
    {
        if(g_windowWidth <= 80){
            slope = 1;
        } else if(g_windowWidth <= 100){
            slope = 1;
        } else if(g_windowWidth <= 120){
            slope = 1;
        } else if(g_windowWidth <= 150){
            slope = 1;
        } else if(g_windowWidth <= 170){
            slope = 1;
        } else {
            slope = 1;
        }
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
        this->getTimeout();
        this->initializeInstance();
    }

    void render()
    {
        printw("Welcome to the Game");
        printw("\n");
        board->render();
        player->render();
        enemy->render();
    }

    void update()
    {
        player->update();
        enemy->update();
    }

    void getTerminalDimension()
    {
        getmaxyx(stdscr, g_windowHeight, g_windowWidth);
        g_screenWidth = g_windowWidth * 0.7;
        g_screenHeight = g_screenWidth * 0.2;
    }

    void getTimeout()
    {
        if(g_windowWidth <= 80){
            TIMEOUT = 55;
        } else if(g_windowWidth <= 100){
            TIMEOUT = 50;
        } else if(g_windowWidth <= 120){
            TIMEOUT = 45;
        } else if(g_windowWidth <= 150){
            TIMEOUT = 35;
        } else if(g_windowWidth <= 170){
            TIMEOUT = 30;
        } else {
            TIMEOUT = 25;
        }
    }

    void initializeInstance()
    {
        player = std::make_unique<Player>('#');
        board = std::make_unique<Board>();
        enemy = std::make_unique<Enemy>('^', FROM_LEFT, DIR_UP, 5, 0.9);
    }

private:
    
};

int main(int argc, const char * argv[]) 
{
    initscr();			
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    Core* core = new Core();
    core->init();

    timeout(TIMEOUT);

    while(true) {
        wclear(stdscr);
        core->render();
        core->update();
        refresh();
    }
    endwin();
}

