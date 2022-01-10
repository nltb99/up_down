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

   /*
    *   @
    *   
    * 
    *     
    */
    BLOCK_LEFT_ONE_ANIMATION_1,
    BLOCK_LEFT_ONE_ANIMATION_2,
    BLOCK_RIGHT_ONE_ANIMATION_1,
    BLOCK_RIGHT_ONE_ANIMATION_2,
};

/*
 *  *
 *
 */
struct Block {
    char chr;
    int quantity;
    int directionX;
    int multiplicationX;
    int multiplicationY;
    int initPositionY;
    int nextTickTriggerY;
    int minPositionY;
    int maxPostionY;
    int directionY;
    bool useReverseDirectionY;
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

    FloatingBlock(Block* block, long double tickReveal)
    : block(block), tickReveal(tickReveal), clusterBlock(new BaseCluster[block->quantity]), m_bOutOfScope(false)
    {
        this->init();
    }

    ~FloatingBlock()
    {
    }

    void init()
    {   
        if(block->directionX == FROM_LEFT){
            positionX = 0 - (spaceBetween * block->quantity) + (THRESHOLD * block->quantity);
        } else if(block->directionX == FROM_RIGHT){
            positionX = g_screenWidth;
        }

        for(int i = 0; i < block->quantity; ++i){
            clusterBlock[i].posX = (i * spaceBetween * block->multiplicationX) + positionX;
            clusterBlock[i].posY = (i * slope * block->multiplicationY) + block->initPositionY;
            clusterBlock[i].velocityX = block->directionX;
            clusterBlock[i].velocityY = 0;
        }
    }

    void update()
    {
        unsigned short int countOutOfScope = 0;

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

            // * Update position
            clusterBlock[i].posX += clusterBlock[i].velocityX;
            
            if(block->nextTickTriggerY > 0){
                if((long)current_ticks - tickReveal == block->nextTickTriggerY){
                    if(clusterBlock[i].posY >= block->maxPostionY || clusterBlock[i].posY <= block->minPositionY){
                        if(block->useReverseDirectionY){
                            block->nextTickTriggerY = block->nextTickTriggerY + block->nextTickTriggerY;
                            block->directionY = -block->directionY;
                        } else{
                            block->directionY = 0;
                        }
                    }
                    if(clusterBlock[i].posY + block->directionY >= block->maxPostionY) {
                        clusterBlock[i].posY += block->maxPostionY - clusterBlock[i].posY;
                    } else {
                        clusterBlock[i].posY += block->directionY;
                    }
                }
            } 

            // * Increment Count
            if(
                (block->directionX == FROM_LEFT && clusterBlock[i].posX > g_screenWidth) ||
                (block->directionX == FROM_RIGHT && clusterBlock[i].posX < 0)
            ){
                ++countOutOfScope;
            } 
        }

        // * call destructor when cluster out of scope
        if(countOutOfScope == block->quantity){
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
    long double tickReveal;
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
        this->onCheckingCollision();
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
        this->appendBlockBasedOnTicks();
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
        if(g_windowWidth <= 100 || g_windowHeight < g_screenWidth * 0.2) {
            g_bRunning = false;
        }

        if(g_windowWidth <= 120){
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
    }

    void assignBlockShape(
        int typeBlock, 
        char chr, 
        int quantity, 
        int directionX, 
        int multiplicationX, 
        int multiplicationY, 
        int initPositionY
    ) {
        g_listShape[typeBlock].chr = chr;
        g_listShape[typeBlock].quantity = quantity;
        g_listShape[typeBlock].directionX = directionX;
        g_listShape[typeBlock].multiplicationX = multiplicationX;
        g_listShape[typeBlock].multiplicationY = multiplicationY;
        g_listShape[typeBlock].initPositionY = initPositionY;
    }

    void assignBlockShapeWithAnimation(
        int typeBlock, 
        char chr, 
        int quantity, 
        int directionX, 
        int initPositionY, 
        int nextTickTriggerY, 
        int minPositionY, 
        int maxPostionY, 
        int directionY,
        bool useReverseDirectionY
    ) {
        g_listShape[typeBlock].chr = chr;
        g_listShape[typeBlock].quantity = quantity;
        g_listShape[typeBlock].directionX = directionX;
        g_listShape[typeBlock].initPositionY = initPositionY;
        g_listShape[typeBlock].nextTickTriggerY = nextTickTriggerY;
        g_listShape[typeBlock].minPositionY = minPositionY;
        g_listShape[typeBlock].maxPostionY = maxPostionY;
        g_listShape[typeBlock].directionY = directionY;
        g_listShape[typeBlock].useReverseDirectionY = useReverseDirectionY;
    }

    void assignSequenceShape(long double ticks, int typeBlock)
    {
        sequenceRevealBlock[ticks].listBlock.push_back(typeBlock);
        sequenceRevealBlock[ticks].bRevealed = false;
    }

    void appendBlockBasedOnTicks()
    {
        // ?
        mvprintw(g_screenHeight + 2, 5, std::to_string((long)current_ticks).c_str());

        bool bRevealed = sequenceRevealBlock[(long)current_ticks].bRevealed;
        std::vector<int> vt = sequenceRevealBlock[(long)current_ticks].listBlock;

        if(vt.size() == 0 || bRevealed) return;

        for (std::vector<int>::iterator tick = vt.begin(); tick != vt.end(); tick++){
            g_listBlock.push_back(FloatingBlock(&g_listShape[*tick], (long)current_ticks));
        }

        sequenceRevealBlock[(long)current_ticks].bRevealed = true;
    }

    void initializeBlockShape()
    {
        // TWO
        this->assignBlockShape(BLOCK_LEFT_TWO_1, '^', 2, FROM_LEFT, 0, 3, slope + 3);
        this->assignBlockShape(BLOCK_LEFT_TWO_2, '^', 2, FROM_LEFT, 0, 4, slope + 1);
        this->assignBlockShape(BLOCK_RIGHT_TWO_1, '^', 2, FROM_RIGHT, 0, 3, slope + 3);
        this->assignBlockShape(BLOCK_RIGHT_TWO_2, '^', 2, FROM_RIGHT, 0, 4, slope + 1);

        // THREE
        this->assignBlockShape(BLOCK_LEFT_THREE_1, '^', 3, FROM_LEFT, 2, 0, slope + 1);
        this->assignBlockShape(BLOCK_LEFT_THREE_2, '^', 3, FROM_LEFT, 2, 0, g_screenHeight - (slope + 1));
        this->assignBlockShape(BLOCK_RIGHT_THREE_1, '^', 3, FROM_RIGHT, 2, 0, slope + 1);
        this->assignBlockShape(BLOCK_RIGHT_THREE_2, '^', 3, FROM_RIGHT, 2, 0, g_screenHeight - (slope + 1));

        // FOUR
        this->assignBlockShape(BLOCK_LEFT_FOUR_1, '^', 4, FROM_LEFT, 1, ASCEND_SLOPE, g_screenHeight - slope - 1);
        this->assignBlockShape(BLOCK_LEFT_FOUR_2, '^', 4, FROM_LEFT, 1, DESCEND_SLOPE, slope + 3);
        this->assignBlockShape(BLOCK_RIGHT_FOUR_1, '^', 4, FROM_RIGHT, 1, ASCEND_SLOPE, g_screenHeight - slope - 1);
        this->assignBlockShape(BLOCK_RIGHT_FOUR_2, '^', 4, FROM_RIGHT, 1, DESCEND_SLOPE, slope + 3);

        // ANIMATION
        this->assignBlockShapeWithAnimation(BLOCK_LEFT_ONE_ANIMATION_1, '^', 1, FROM_LEFT, slope + 3, 1, 0 + (THRESHOLD * 2), g_screenHeight - (THRESHOLD * 2), 1, true);
        this->assignBlockShapeWithAnimation(BLOCK_LEFT_ONE_ANIMATION_2, '^', 1, FROM_LEFT, slope + 3, 1, 0 + (THRESHOLD * 2), g_screenHeight - (THRESHOLD * 2), 1, true);
    }

    void initializeSequenceRevealShape()
    {
        // this->assignSequenceShape(1, BLOCK_LEFT_ONE_ANIMATION_2);

        this->assignSequenceShape(0, BLOCK_LEFT_TWO_1);
        this->assignSequenceShape(1, BLOCK_LEFT_TWO_1);
        this->assignSequenceShape(4, BLOCK_RIGHT_FOUR_2);
        this->assignSequenceShape(7, BLOCK_RIGHT_THREE_2);
        this->assignSequenceShape(8, BLOCK_LEFT_THREE_1);
        this->assignSequenceShape(11, BLOCK_RIGHT_TWO_1);
        this->assignSequenceShape(12, BLOCK_RIGHT_TWO_1);
        this->assignSequenceShape(13, BLOCK_LEFT_FOUR_1);
        this->assignSequenceShape(14, BLOCK_RIGHT_FOUR_2);
        this->assignSequenceShape(17, BLOCK_RIGHT_THREE_2);
        this->assignSequenceShape(17, BLOCK_RIGHT_THREE_1);
        this->assignSequenceShape(19, BLOCK_LEFT_TWO_2);
        this->assignSequenceShape(20, BLOCK_LEFT_THREE_1);
        this->assignSequenceShape(22, BLOCK_RIGHT_FOUR_2);
        this->assignSequenceShape(24, BLOCK_RIGHT_FOUR_2);
        this->assignSequenceShape(26, BLOCK_LEFT_TWO_2);
        this->assignSequenceShape(26, BLOCK_LEFT_TWO_2);
        this->assignSequenceShape(29, BLOCK_LEFT_TWO_1);
        this->assignSequenceShape(30, BLOCK_RIGHT_TWO_1);
        this->assignSequenceShape(31, BLOCK_RIGHT_TWO_2);
        this->assignSequenceShape(34, BLOCK_LEFT_FOUR_1);
        this->assignSequenceShape(36, BLOCK_LEFT_FOUR_2);
        this->assignSequenceShape(39, BLOCK_RIGHT_FOUR_1);
        this->assignSequenceShape(41, BLOCK_RIGHT_FOUR_2);
        this->assignSequenceShape(43, BLOCK_RIGHT_TWO_2);
        this->assignSequenceShape(43, BLOCK_LEFT_TWO_1);
        this->assignSequenceShape(45, BLOCK_RIGHT_TWO_1);
        this->assignSequenceShape(45, BLOCK_LEFT_TWO_2);
        this->assignSequenceShape(47, BLOCK_LEFT_FOUR_2);
        this->assignSequenceShape(49, BLOCK_LEFT_FOUR_1);
        this->assignSequenceShape(52, BLOCK_RIGHT_FOUR_2);
        this->assignSequenceShape(54, BLOCK_RIGHT_FOUR_1);
        this->assignSequenceShape(57, BLOCK_LEFT_TWO_1);
        this->assignSequenceShape(57, BLOCK_RIGHT_TWO_1);
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

