#include <iostream>
#include <stdlib.h>
#include <vector>
#include <unistd.h>
#include <ncurses.h>
#include <map>
#include<string>
#include <unordered_map>

#define THRESHOLD 2
#define SECOND 1000

unsigned short int g_windowWidth, g_windowHeight;
unsigned short int g_screenWidth, g_screenHeight;
short int keyPressed;
unsigned short int TIMEOUT;
int slope, spaceBetween;

long double current_ticks = 0;

enum : int {
    ASCEND_SLOPE = -1,
    DESCEND_SLOPE = 1,
    FROM_LEFT = 1,
    FROM_RIGHT = -1,

    /*
    *   @
    *   
    *   @
    */
    BLOCK_LEFT_TWO_1,
    BLOCK_RIGHT_TWO_1,
    BLOCK_LEFT_TWO_2,
    BLOCK_RIGHT_TWO_2,

    /*
    *   @  @  @
    *   ------
    *   @  @  @
    */
    BLOCK_LEFT_THREE_1,
    BLOCK_RIGHT_THREE_1,
    BLOCK_LEFT_THREE_2,
    BLOCK_RIGHT_THREE_2,

    /*
    *        @ | @
    *      @   |   @
    *    @     |     @
    *  @       |       @
    */
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
    char chr;
    int quantity;
    int directionFrom;
    int additionX;
    int additionY;
    int positionY;
} b;

struct BaseCluster {
    int posX, posY;
    int velocityX, velocityY;
    bool bVisible;
} be;

struct BaseSequenceBlock {
    std::vector<int> listBlock;
    bool bRevealed;
} bsb;

class Entity {
    public:
        virtual void init() = 0;
        virtual void update() = 0;
    private:
};

class FloatingBlock : public Entity
{
public: 

    FloatingBlock(Block* block)
    : block(block), clusterBlock(new BaseCluster[block->quantity]), m_bOutOfScope(false)
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
            clusterBlock[i].posX = (i * spaceBetween * block->additionX) + positionX;
            clusterBlock[i].posY = (i * slope * block->additionY) + block->positionY;
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
                mvwaddch(stdscr, clusterBlock[i].posY, clusterBlock[i].posX, block->chr);
                wmove(stdscr, g_screenHeight, g_screenWidth);
            }

            // * Bounding checking 
            if(
                clusterBlock[i].posX + clusterBlock[i].velocityX <= 0 || 
                clusterBlock[i].posX + clusterBlock[i].velocityX > g_screenWidth - THRESHOLD || 
                clusterBlock[i].posY + clusterBlock[i].velocityY < THRESHOLD || 
                clusterBlock[i].posY > g_screenHeight - THRESHOLD 
            ){
                clusterBlock[i].bVisible = false;
            } else{
                clusterBlock[i].bVisible = true;
            }

            clusterBlock[i].posX += clusterBlock[i].velocityX;
            clusterBlock[i].posY += clusterBlock[i].velocityY;

            if(
                (block->directionFrom == FROM_LEFT && clusterBlock[i].posX > g_screenWidth) ||
                (block->directionFrom == FROM_RIGHT && clusterBlock[i].posX < 0)
            ){
                ++countVisible;
            } 
        }

        // * call destructor when cluster out of scope
        if(countVisible == block->quantity){
            m_bOutOfScope = true;
            this->~FloatingBlock();
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
    
    bool bOutOfScope()
    {
        return m_bOutOfScope;
    }

private:
    bool m_bOutOfScope;
    Block* block;
    BaseCluster *clusterBlock;
    
    int positionX;
};

/*
 * Global variables
 */
std::unordered_map<int, Block> g_listShape;
std::vector<FloatingBlock> g_listBlock;
std::unordered_map<long double, BaseSequenceBlock> sequenceRevealBlock;
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
        this->drawIntro();
        this->drawBoard();
    }

    void drawBoard()
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

    void drawIntro()
    {
        printw("Welcome to the Game");
        printw("\n");
    }

private:

};

class Player : public Entity
{
public:

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
        // ?
        // this->onCheckingCollision();
   }

   void onListenKeyPressed()
   {
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
       for (std::vector<FloatingBlock>::iterator it = g_listBlock.begin(); it != g_listBlock.end(); it++){
            BaseCluster* cluster = it->getCluster();
            Block* block = it->getBlock();
            
            if(it->bOutOfScope()){
                continue;
            }

            for(int i = 0; i < block->quantity; ++i){
                if(
                    (posX == cluster[i].posX + 1 && posY == cluster[i].posY + 1) || 
                    (posX == cluster[i].posX - 1 && posY == cluster[i].posY - 1) || 
                    (posX == cluster[i].posX + 1 && posY == cluster[i].posY) ||
                    (posX == cluster[i].posX - 1 && posY == cluster[i].posY) ||
                    (posX == cluster[i].posX && posY == cluster[i].posY + 1) ||
                    (posX == cluster[i].posX && posY == cluster[i].posY - 1) ||
                    (posX == cluster[i].posX && posY == cluster[i].posY)
                ) {
                    g_isPlaying = false;
                }
            }
        }
   }

private:
    bool g_isPlaying;
    int velocityX, velocityY;
    int posX, posY;
    char chr;
};

class Core : public Entity
{
public:

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
        this->initializeBlockShape();
        this->initializeSequenceRevealShape();
        this->initializeInstance();
    }

    void update()
    {   
        // * Render
        board->update();
        player->update();
        for (std::vector<FloatingBlock>::iterator block = g_listBlock.begin(); block != g_listBlock.end(); block++){
            if(!block->bOutOfScope()){
                block->update();
            }
        }

        // * Update
        this->computeTicks();
        this->checkTickForRevealingBlock();
        g_bRunning = player->isPlaying();
    }

    void getTerminalDimension()
    {
        getmaxyx(stdscr, g_windowHeight, g_windowWidth);
        g_screenWidth = g_windowWidth * 0.7;
        g_screenHeight = g_screenWidth * 0.2;
    }

    void computeTicks()
    {
        current_ticks += (double) TIMEOUT / SECOND;
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
        } else if(g_windowWidth <= 140){
            TIMEOUT = 35;
            slope = 3;
            spaceBetween = 15;
        } else if(g_windowWidth <= 160){
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
        // g_listBlock.push_back(FloatingBlock(&g_listShape[BLOCK_LEFT_TWO_1]));
    }

    void assignBlockShape(int typeBlock, char chr, int quantity, int directionFrom, int additionX, int additionY, int positionY)
    {
        g_listShape[typeBlock].chr = chr;
        g_listShape[typeBlock].quantity = quantity;
        g_listShape[typeBlock].directionFrom = directionFrom;
        g_listShape[typeBlock].additionX = additionX;
        g_listShape[typeBlock].additionY = additionY;
        g_listShape[typeBlock].positionY = positionY;
    }

    void assignSequenceShape(long double ticks, int typeBlock)
    {
        sequenceRevealBlock[ticks].listBlock.push_back(typeBlock);
        sequenceRevealBlock[ticks].bRevealed = false;
    }

    void checkTickForRevealingBlock()
    {
        // ?
        mvprintw(g_screenHeight + 2, 5, std::to_string((long)current_ticks).c_str());

        bool bRevealed = sequenceRevealBlock[(long)current_ticks].bRevealed;
        std::vector<int> vt = sequenceRevealBlock[(long)current_ticks].listBlock;

        if(vt.size() == 0 || bRevealed){
            return;
        }

        for (std::vector<int>::iterator it = vt.begin(); it != vt.end(); it++){
            g_listBlock.push_back(FloatingBlock(&g_listShape[*it]));
        }

        sequenceRevealBlock[(long)current_ticks].bRevealed = true;
    }

    void initializeBlockShape()
    {
        // TWO
        this->assignBlockShape(BLOCK_LEFT_TWO_1, '^', 2, FROM_LEFT, 0, 3, slope + 3);
        this->assignBlockShape(BLOCK_RIGHT_TWO_1, '^', 2, FROM_RIGHT, 0, 3, slope + 3);
        this->assignBlockShape(BLOCK_LEFT_TWO_2, '^', 2, FROM_LEFT, 0, 4, slope + 1);
        this->assignBlockShape(BLOCK_RIGHT_TWO_2, '^', 2, FROM_RIGHT, 0, 4, slope + 1);

        // THREE
        this->assignBlockShape(BLOCK_LEFT_THREE_1, '^', 3, FROM_LEFT, 2, 0, slope + 1);
        this->assignBlockShape(BLOCK_RIGHT_THREE_1, '^', 3, FROM_RIGHT, 2, 0, slope + 1);
        this->assignBlockShape(BLOCK_LEFT_THREE_2, '^', 3, FROM_LEFT, 2, 0, g_screenHeight - (slope + 1));
        this->assignBlockShape(BLOCK_RIGHT_THREE_2, '^', 3, FROM_RIGHT, 2, 0, g_screenHeight - (slope + 1));

        // FOUR
        this->assignBlockShape(BLOCK_LEFT_FOUR_1, '^', 4, FROM_LEFT, 1, ASCEND_SLOPE, g_screenHeight - slope - 1);
        this->assignBlockShape(BLOCK_LEFT_FOUR_2, '^', 4, FROM_LEFT, 1, DESCEND_SLOPE, slope + 3);
        this->assignBlockShape(BLOCK_RIGHT_FOUR_1, '^', 4, FROM_RIGHT, 1, ASCEND_SLOPE, g_screenHeight - slope - 1);
        this->assignBlockShape(BLOCK_RIGHT_FOUR_2, '^', 4, FROM_RIGHT, 1, DESCEND_SLOPE, slope + 3);

    }

    void initializeSequenceRevealShape()
    {
        this->assignSequenceShape(1, BLOCK_LEFT_TWO_1);
        this->assignSequenceShape(2, BLOCK_LEFT_TWO_1);
        this->assignSequenceShape(3, BLOCK_RIGHT_FOUR_2);
        this->assignSequenceShape(5, BLOCK_LEFT_THREE_1);
    }

private:
    Player* player;
    Board* board;
    bool g_bRunning;
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
        keyPressed = getch();
        wclear(stdscr);
        core->update();
        refresh();
    }

    endwin();
}

