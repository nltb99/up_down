#include <iostream>
#include <stdlib.h>
#include <vector>
#include <unistd.h>
#include <ncurses.h>
#include <map>
#include<string>
#include <unordered_map>

#define THRESHOLD 2

unsigned short int g_windowWidth, g_windowHeight;
unsigned short int g_screenWidth, g_screenHeight;
unsigned short int TIMEOUT;
int slope, spaceBetween;

enum : int {
    ASCEND_SLOPE = -1,
    DESCEND_SLOPE = 1,
    FROM_LEFT = 1,
    FROM_RIGHT = -1,

    BLOCK_LEFT_FOUR_1,
    BLOCK_LEFT_FOUR_2,
    BLOCK_RIGHT_FOUR_1,
    BLOCK_RIGHT_FOUR_2,
};

/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */
struct Block {
    int quantity;
    int directionFrom;
    int directionWave;
    int positionY;
} b;

struct BaseCluster {
    int posX, posY;
    int velocityX, velocityY;
    bool bVisible;
} be;

class Entity {
    public:
        virtual void init() = 0;
        virtual void update() = 0;
    private:
};

class FloatingBlock : public Entity
{
public: 
    Block* block;
    BaseCluster *clusterBlock;
    
    char chr;
    int positionX;

    FloatingBlock(char chr, Block* block)
    : chr(chr), block(block), clusterBlock(new BaseCluster[block->quantity]), m_shouldDestroy(false)
    {
        this->init();
    }

    ~FloatingBlock()
    {
    }

    void init()
    {   
        if(block->directionFrom == FROM_LEFT){
            positionX = 0 - (spaceBetween * block->quantity) + (THRESHOLD * block->quantity);
        } else if(block->directionFrom == FROM_RIGHT){
            positionX = g_screenWidth;
        }

        for(int i = 0; i < block->quantity; ++i){
            clusterBlock[i].posX = (i * spaceBetween) + positionX;
            clusterBlock[i].posY = (i * slope * block->directionWave) + block->positionY;
            clusterBlock[i].velocityX = block->directionFrom;
            clusterBlock[i].velocityY = 0;
        }
    }

    void update()
    {
        unsigned short int countVisible = 0;

        for(int i = 0; i < block->quantity; ++i){
            
            // * Render
            if(clusterBlock[i].bVisible){
                mvwaddch(stdscr, clusterBlock[i].posY, clusterBlock[i].posX, chr);
                wmove(stdscr, g_screenHeight, g_screenWidth);
            }

            // * Bounding checking 
            if(
                clusterBlock[i].posX + clusterBlock[i].velocityX <= 0 || // Bounding X LEFT
                clusterBlock[i].posX + clusterBlock[i].velocityX > g_screenWidth - THRESHOLD || // Bounding X RIGHT
                clusterBlock[i].posY + clusterBlock[i].velocityY < THRESHOLD || // Bounding Y Top
                clusterBlock[i].posY > g_screenHeight - THRESHOLD // Bounding Y DOWN
            ){
                clusterBlock[i].bVisible = false;
            } else{
                clusterBlock[i].bVisible = true;
            }

            clusterBlock[i].posX += clusterBlock[i].velocityX;
            clusterBlock[i].posY += clusterBlock[i].velocityY;

            if(!clusterBlock[i].bVisible){
                ++countVisible;
            }
        }

        // * call destructor when cluster out of scope
        if(countVisible == block->quantity){
            this->~FloatingBlock();
            m_shouldDestroy = true;
        }
    }

    BaseCluster* getCluster()
    {
        return clusterBlock;
    }

    Block* getBlock()
    {
        return block;
    }
    
    bool shouldDestroy()
    {
        return m_shouldDestroy;
    }

private:
    bool m_shouldDestroy;
};

/*
 * Global variables
 */
std::unordered_map<int, Block> g_listShape;
std::vector<FloatingBlock> g_listBlock;

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

    void update()
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

   int velocityX, velocityY;
   int posX, posY;
   char chr;
   bool g_isPlaying;
   int tempX, tempY;

   Player(char chr)
   : chr(chr), g_isPlaying(true), velocityX(0), velocityY(1), posX((g_screenWidth + g_screenHeight) / 2), posY(THRESHOLD)
   {
       this->init();
   }

   ~Player()
   {
   }

   bool isPlaying()
   {
       return g_isPlaying;
   }

   void init()
   {
   }

   void update()
   {    
        // * Render 
        mvwaddch(stdscr, posY, posX, chr);
        wmove(stdscr, g_screenHeight, g_screenWidth);

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
    //    this->onCheckingCollision();
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

   void onCheckingCollision()
   {
       // ?
       int lengthList = g_listBlock.size();
       for (std::vector<FloatingBlock>::iterator it = g_listBlock.begin(); it != g_listBlock.end(); it++){
            BaseCluster* cluster = it->getCluster();
            Block* block = it->getBlock();

            for(int i = 0; i < block->quantity; ++i){
                if(
                    // posX >= cluster[i].posX - THRESHOLD + 1 &&
                    // posX <= cluster[i].posX - 1 &&
                    // posY == cluster[i].posY - THRESHOLD + 1 &&
                    // posY == cluster[i].posY - 1
                    posX == cluster[i].posX &&
                    posY == cluster[i].posY
                ) {
                    // g_isPlaying = false;
                    tempX = posX;
                    tempY = posY;
                }
                // mvprintw(g_screenHeight + i * 2 + 2, 5, std::to_string(cluster[i].posX).c_str());
                // mvprintw(g_screenHeight + i * 2 + 2, 10, std::to_string(cluster[i].posY).c_str());
                // mvprintw(g_screenHeight + 5, 20, std::to_string(posX).c_str());
                // mvprintw(g_screenHeight + 5, 25, std::to_string(posY).c_str());

                // mvprintw(g_screenHeight + 5, 30, std::to_string(tempX).c_str());
                // mvprintw(g_screenHeight + 5, 35, std::to_string(tempY).c_str());
                // mvprintw(g_screenHeight + 5, 50, std::to_string(lengthList).c_str());
            }
        }
   }

private:

};

class Core : public Entity
{
public:
    Player* player;
    Board* board;
    bool g_bRunning;

    Core()
        : g_bRunning(true)
    {
    }

    ~Core()
    {
    }

    bool isRunning()
    {
        return g_bRunning;
    }

    void init()
    {
        this->getTerminalDimension();
        this->getReponsiveMetrics();
        this->setBlockLevel();
        this->initializeInstance();
    }

    void update()
    {   
        // * Render
        printw("Welcome to the Game");
        printw("\n");
        board->update();
        player->update();
        for (std::vector<FloatingBlock>::iterator block = g_listBlock.begin(); block != g_listBlock.end(); block++){
            block->update();
            if(block->shouldDestroy()){
            }
        }

        g_bRunning = player->isPlaying();
    }

    void getTerminalDimension()
    {
        getmaxyx(stdscr, g_windowHeight, g_windowWidth);
        g_screenWidth = g_windowWidth * 0.7;
        g_screenHeight = g_screenWidth * 0.2;
    }

    void getReponsiveMetrics()
    {   
        //TODO
        // Need to popup dialog alert not eligible dimension here
        if(g_windowHeight < g_screenWidth * 0.2) {

        }

        if(g_windowWidth <= 80){
            TIMEOUT = 55;
            slope = 1;
            spaceBetween = 10;
        } else if(g_windowWidth <= 100){
            TIMEOUT = 50;
            slope = 1;
            spaceBetween = 10;
        } else if(g_windowWidth <= 120){
            TIMEOUT = 45;
            slope = 2;
            spaceBetween = 10;
        } else if(g_windowWidth <= 150){
            TIMEOUT = 35;
            slope = 3;
            spaceBetween = 15;
        } else if(g_windowWidth <= 170){
            TIMEOUT = 30;
            slope = 4;
            spaceBetween = 20;
        } else {
            TIMEOUT = 25;
            slope = 4;
            spaceBetween = 20;
        }
    }

    void initializeInstance()
    {
        player = new Player('#');
        board = new Board();
        g_listBlock.push_back(FloatingBlock('^', &g_listShape[BLOCK_LEFT_FOUR_1]));
        // g_listBlock.push_back(FloatingBlock('^', &g_listShape[BLOCK_RIGHT_FOUR_1]));
    }

    void setBlockLevel()
    {
        // FOUR/LEFT/ASCEND
        g_listShape[BLOCK_LEFT_FOUR_1].quantity = 4;
        g_listShape[BLOCK_LEFT_FOUR_1].directionFrom = FROM_LEFT;
        g_listShape[BLOCK_LEFT_FOUR_1].directionWave = ASCEND_SLOPE;
        g_listShape[BLOCK_LEFT_FOUR_1].positionY = 
            g_listShape[BLOCK_LEFT_FOUR_1].directionWave == ASCEND_SLOPE ? g_screenHeight - slope - 1 : 
            g_listShape[BLOCK_LEFT_FOUR_1].directionWave == DESCEND_SLOPE ? slope + 3 : 0;

        // FOUR/LEFT/DESCEND
        g_listShape[BLOCK_LEFT_FOUR_2].quantity = 4;
        g_listShape[BLOCK_LEFT_FOUR_2].directionFrom = FROM_LEFT;
        g_listShape[BLOCK_LEFT_FOUR_2].directionWave = DESCEND_SLOPE;
        g_listShape[BLOCK_LEFT_FOUR_2].positionY = 
            g_listShape[BLOCK_LEFT_FOUR_2].directionWave == ASCEND_SLOPE ? g_screenHeight - slope - 1 : 
            g_listShape[BLOCK_LEFT_FOUR_2].directionWave == DESCEND_SLOPE ? slope + 3 : 0;

        // FOUR/RIGHT/ASCEND
        g_listShape[BLOCK_RIGHT_FOUR_1].quantity = 4;
        g_listShape[BLOCK_RIGHT_FOUR_1].directionFrom = FROM_RIGHT;
        g_listShape[BLOCK_RIGHT_FOUR_1].directionWave = DESCEND_SLOPE;
        g_listShape[BLOCK_RIGHT_FOUR_1].positionY = 
            g_listShape[BLOCK_RIGHT_FOUR_1].directionWave == ASCEND_SLOPE ? g_screenHeight - slope - 1 : 
            g_listShape[BLOCK_RIGHT_FOUR_1].directionWave == DESCEND_SLOPE ? slope + 3 : 0;
    }

private:
    
};

int main(int argc, const char * argv[]) 
{
    initscr();			
    cbreak();
    echo();
    keypad(stdscr, TRUE);

    Core* core = new Core();
    core->init();

    timeout(TIMEOUT);

    while(core->isRunning()) {
        wclear(stdscr);
        core->update();
        refresh();
    }

    endwin();
}

