#include <iostream>
#include <stdlib.h>
#include <vector>
#include <ncurses.h>
#include <map>
#include<string>
#include <unordered_map>
#include <unistd.h>

#define THRESHOLD 2
#define SECOND 1000
#define PLAYER_CHARACTER '#'
#define TOTAL_MENU 4

namespace STATUS
{
    const std::string GAME_MENU_START = "GAME_MENU_START";
    const std::string GAME_ASK_PLAY_AGAIN = "GAME_ASK_PLAY_AGAIN";
    const std::string GAME_PLAYING = "GAME_PLAYING";
    const std::string GAME_SELECT_LEVEL = "GAME_SELECT_LEVEL";
    const std::string GAME_BEST_SCORE = "GAME_BEST_SCORE";
    const std::string GAME_NOT_MEET_DIMENSION_REQUIREMENT = "GAME_NOT_MEET_DIMENSION_REQUIREMENT";
    const std::string GAME_STOP = "GAME_STOP";
} 

namespace LEVEL
{
    const int EASY = 1;
    const int NORMAL = 2;
    const int HARD = 3;
}

namespace MENU
{
    const int START_GAME = 1;
    const int LEVEL = 2;
    const int BEST_SCORE = 3;
    const int EXIT = 4;
}

uint16_t g_windowWidth, g_windowHeight;
uint16_t g_screenWidth, g_screenHeight;
uint16_t TIMEOUT;
short int keyPressed;
uint8_t slope, spaceBetween;
long double current_ticks = 0;
uint8_t index_menu_selected = 1; // min 1
uint8_t level_selected = LEVEL::NORMAL;
uint16_t score;
std::string listRandomChar[5] = { "~", "-", "=", "*", "+"};

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
    *   @                     @
    *     ~                 ~
    *       ~             ~
    *         ~         ~
    *           @ - - @
    */          
    BLOCK_LEFT_ONE_ANIMATION_REVERSE_1,
    BLOCK_LEFT_ONE_ANIMATION_REVERSE_2,
    BLOCK_RIGHT_ONE_ANIMATION_REVERSE_1,
    BLOCK_RIGHT_ONE_ANIMATION_REVERSE_2,

    BLOCK_LEFT_ONE_ANIMATION_NO_REVERSE_1,
    BLOCK_LEFT_ONE_ANIMATION_NO_REVERSE_2,
    BLOCK_RIGHT_ONE_ANIMATION_NO_REVERSE_1,
    BLOCK_RIGHT_ONE_ANIMATION_NO_REVERSE_2,
};

int all_kind_of_blocks[20] = { 
    BLOCK_LEFT_TWO_1,
    BLOCK_RIGHT_TWO_1,
    BLOCK_LEFT_TWO_2,
    BLOCK_RIGHT_TWO_2,

    BLOCK_LEFT_THREE_1,
    BLOCK_RIGHT_THREE_1,
    BLOCK_LEFT_THREE_2,
    BLOCK_RIGHT_THREE_2,

    BLOCK_LEFT_FOUR_1,
    BLOCK_LEFT_FOUR_2,
    BLOCK_RIGHT_FOUR_1,
    BLOCK_RIGHT_FOUR_2,

    BLOCK_LEFT_ONE_ANIMATION_REVERSE_1,
    BLOCK_LEFT_ONE_ANIMATION_REVERSE_2,
    BLOCK_RIGHT_ONE_ANIMATION_REVERSE_1,
    BLOCK_RIGHT_ONE_ANIMATION_REVERSE_2,

    BLOCK_LEFT_ONE_ANIMATION_NO_REVERSE_1,
    BLOCK_LEFT_ONE_ANIMATION_NO_REVERSE_2,
    BLOCK_RIGHT_ONE_ANIMATION_NO_REVERSE_1,
    BLOCK_RIGHT_ONE_ANIMATION_NO_REVERSE_2,
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

    ~FloatingBlock() {}

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
                    clusterBlock[i].posY += block->directionY;
                    if(clusterBlock[i].posY >= block->maxPostionY || clusterBlock[i].posY <= block->minPositionY){
                        if(block->useReverseDirectionY){
                            block->nextTickTriggerY = block->nextTickTriggerY + block->nextTickTriggerY;
                            block->directionY = -block->directionY;
                        } else{
                            block->directionY = 0;
                        }
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

class Context
{
public:

    Context() {}

    ~Context() {}

    void drawPlayground()
    {
        printw("Up & Down Game");
        printw("\n");

        this->drawRectangle(g_screenHeight, g_screenWidth, "*");
    }

    void drawRectangle(const int height, const int width, const std::string str)
    {
        for(int i = 0; i < height; ++i){
           for(int j = 0; j < width; ++j){
               if(i == 0 || i == height - 1 || j == 0 || j == width - 1){
                   printw(str.c_str());
               } else {
                   printw(" ");
               }
           }
           printw("\n");
       }
    }

    void drawMenuStartGame()
    {
        usleep(99999);
        u_int8_t heightBox = 13;
        u_int8_t widthBox = 50;
        std::string titleMenu = "Start Game";
        std::string level = "Level";
        std::string bestScore = "Best Score";
        std::string exit = "Exit";
        switch(index_menu_selected){
            case MENU::START_GAME:
                titleMenu += " *";
                break;
            case MENU::LEVEL:
                level += " *";
                break;
            case MENU::BEST_SCORE:
                bestScore += " *";
                break;
            case MENU::EXIT:
                exit += " *";
                break;
        }
        
        this->drawMenuBox(heightBox, widthBox);
        mvprintw(2, (widthBox / 2) - 3, "Up & Down");
        mvprintw(4, (widthBox / 2) - 4, titleMenu.c_str());
        mvprintw(6, (widthBox / 2) - 2, level.c_str());
        mvprintw(8, (widthBox / 2) - 4, bestScore.c_str());
        mvprintw(10, (widthBox / 2) - 1, exit.c_str());
    }

    void drawMenuPlayAgain()
    {
        usleep(99999);
        u_int8_t heightBox = 17;
        u_int8_t widthBox = 50;
        std::string titleMenu = "Play Again";
        std::string level = "Level";
        std::string bestScore = "Best Score";
        std::string exit = "Exit";
        switch(index_menu_selected){
            case MENU::START_GAME:
                titleMenu += " *";
                break;
            case MENU::LEVEL:
                level += " *";
                break;
            case MENU::BEST_SCORE:
                bestScore += " *";
                break;
            case MENU::EXIT:
                exit += " *";
                break;
        }

        this->drawMenuBox(heightBox, widthBox);
        mvprintw(2, (widthBox / 2) - 3, "Up & Down");
        mvprintw(4, (widthBox / 2) - 4, "Your Score");
        mvprintw(6, (widthBox / 2), std::to_string(score).c_str());
        mvprintw(8, (widthBox / 2) - 4, titleMenu.c_str());
        mvprintw(10, (widthBox / 2) - 2, level.c_str());
        mvprintw(12, (widthBox / 2) - 4, bestScore.c_str());
        mvprintw(14, (widthBox / 2) - 1, exit.c_str());
    }

    void drawMenuSelectLevel()
    {
        usleep(99999);
        u_int8_t heightBox = 15;
        u_int8_t widthBox = 50;
        std::string titleMenu = "Select Level";
        std::string easy = "Easy";
        std::string normal = "Normal";
        std::string hard = "Hard";
        std::string back = "Back";
        switch(index_menu_selected){
            case LEVEL::EASY:
                easy += " *";
                break;
            case LEVEL::NORMAL:
                normal += " *";
                break;
            case LEVEL::HARD:
                hard += " *";
                break;
            case MENU::EXIT:
                back += " *";
                break;
        }

        switch(level_selected){
            case LEVEL::EASY:
                easy = "@ " + easy;
                break;
            case LEVEL::NORMAL:
                normal = "@ " + normal;
                break;
            case LEVEL::HARD:
                hard = "@ " + hard;
                break;
        }

        this->drawMenuBox(heightBox, widthBox);
        mvprintw(2, (widthBox / 2) - 3, "Up & Down");
        mvprintw(4, (widthBox / 2) - 4, titleMenu.c_str());
        mvprintw(6, (widthBox / 2) - 1, easy.c_str());
        mvprintw(8, (widthBox / 2) - 2, normal.c_str());
        mvprintw(10, (widthBox / 2) - 1, hard.c_str());
        mvprintw(12, (widthBox / 2) - 1, back.c_str());
    }

    void drawMenuBox(const u_int8_t heightBox, const u_int8_t widthBox)
    {
        uint8_t lenListChar = sizeof(listRandomChar)/sizeof(listRandomChar[0]);
        uint8_t randomIndex = std::rand() % lenListChar;
        this->drawRectangle(heightBox, widthBox, listRandomChar[randomIndex]);
    }

    void drawAlertNotMetRequirement()
    {
        printw("Ouch~ Looks like your Terminal/CMD display screen isn't wide enough. Let's try to enlarge the screen and run it again.");
    }

private:

};

class Player : public Entity
{
public:

    Player(char chr)
    : chr(chr), velocityX(0), velocityY(1), posY(THRESHOLD)
    {
        int randomX = (0 + (THRESHOLD * 3)) + (std::rand() % ( (g_screenWidth - (THRESHOLD * 3)) - (0 + (THRESHOLD * 3)) + 1));
        posX = randomX;
        this->init();
    }

    ~Player()
    {
    }

    void init()
    {
    }

    void update()
    {    
        // * Render 
        mvwaddch(stdscr, posY, posX, chr);

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

   int getPosX()
   {
       return posX;
   }

   int getPosY()
   {
       return posY;
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
    int posX, posY;
    int velocityX, velocityY;
    char chr;
};

class Core : public Entity
{
public:

    Core(std::string g_sCoreStatus)
    : g_sCoreStatus(g_sCoreStatus)
    {}

    ~Core() {}

    std::string getStatusGame()
    {
        return g_sCoreStatus;
    }

    void init()
    {
        this->getTerminalDimension();
        this->initializeReponsiveMetrics();
        this->initializeBlockShape();
        this->initializeSequenceRevealShape();
        this->initializeInstance();
    }

    void update()
    {   
        // * Listen key pressed
        this->onListenKeyPressed();
        
        if(g_sCoreStatus == STATUS::GAME_MENU_START){
            context->drawMenuStartGame();
        } else if(g_sCoreStatus == STATUS::GAME_ASK_PLAY_AGAIN){
            context->drawMenuPlayAgain();
        } else if(g_sCoreStatus == STATUS::GAME_SELECT_LEVEL){
            context->drawMenuSelectLevel();
        } else if(g_sCoreStatus == STATUS::GAME_BEST_SCORE){
            // context->drawMenuPlayAgain();
        } else if(g_sCoreStatus == STATUS::GAME_NOT_MEET_DIMENSION_REQUIREMENT){
            context->drawAlertNotMetRequirement();
        } else if(g_sCoreStatus == STATUS::GAME_PLAYING){
            // * Render
            context->drawPlayground();
            player->update();
            for (std::vector<FloatingBlock>::iterator block = listFloatingBlock.begin(); block != listFloatingBlock.end(); block++){
                if(!block->bOutOfScope()){
                    block->update();
                }
            }      

            // * Update
            this->computeTicks();
            // ?
            this->appendBlockBasedOnTicks();
            this->checkingPlayerCollision();
        } 
        // * Move curse out of screen
        wmove(stdscr, g_windowHeight - THRESHOLD, g_windowWidth - THRESHOLD);
    }

private:
    Player* player;
    Context* context;
    std::string g_sCoreStatus;
    int marked_tick_append_last_block;

    std::unordered_map<int, Block> listShape;
    std::unordered_map<long double, BaseSequenceBlock> sequenceRevealBlock;
    std::vector<FloatingBlock> listFloatingBlock;

    void onListenKeyPressed()
    {
        bool isInMenuScreen = 
            g_sCoreStatus == STATUS::GAME_MENU_START || 
            g_sCoreStatus == STATUS::GAME_SELECT_LEVEL ||
            g_sCoreStatus == STATUS::GAME_BEST_SCORE ||
            g_sCoreStatus == STATUS::GAME_ASK_PLAY_AGAIN;

        switch(keyPressed){
            case KEY_LEFT:
                player->onMoveLeft();
                break;
            case KEY_RIGHT:
                player->onMoveRight();
                break;
            case KEY_UP:
                if(isInMenuScreen){
                    if(index_menu_selected - 1 < 1) {
                        index_menu_selected = TOTAL_MENU;
                    } else {
                        --index_menu_selected;
                    }
                } else if(g_sCoreStatus == STATUS::GAME_PLAYING){
                    player->onStand();
                }
                break;
            case KEY_DOWN:
                if(isInMenuScreen){
                    if(index_menu_selected + 1 > TOTAL_MENU) {
                        index_menu_selected = 1;
                    } else {
                        ++index_menu_selected;
                    }
                } else if(g_sCoreStatus == STATUS::GAME_PLAYING){
                    player->onStand();
                }
                break;
            case 10: // ENTER
                if(isInMenuScreen){
                    if(g_sCoreStatus == STATUS::GAME_SELECT_LEVEL){
                        if(index_menu_selected != MENU::EXIT){
                            level_selected = index_menu_selected;
                        }
                        index_menu_selected = MENU::LEVEL;
                        g_sCoreStatus = STATUS::GAME_MENU_START;
                    } else {
                        switch(index_menu_selected){
                            case MENU::START_GAME:
                                g_sCoreStatus = STATUS::GAME_PLAYING;
                                break;
                            case MENU::LEVEL:
                                index_menu_selected = level_selected;
                                g_sCoreStatus = STATUS::GAME_SELECT_LEVEL;
                                break;
                            case MENU::BEST_SCORE:
                                g_sCoreStatus = STATUS::GAME_BEST_SCORE;
                                break;
                            case MENU::EXIT:
                                g_sCoreStatus = STATUS::GAME_STOP;
                                break;
                        }
                    }
                }
                break;
            default:
                if(keyPressed != -1 && g_sCoreStatus == STATUS::GAME_NOT_MEET_DIMENSION_REQUIREMENT){
                    g_sCoreStatus = STATUS::GAME_STOP;
                }
        }
    }

    void checkingPlayerCollision()
    {
        bool bCollision = false;
        for (std::vector<FloatingBlock>::iterator it = listFloatingBlock.begin(); it != listFloatingBlock.end(); it++){
            BaseCluster* cluster = it->getCluster();
            Block* block = it->getBlock();
            
            if(it->bOutOfScope()){
                continue;
            }

            for(int i = 0; i < block->quantity; ++i){
                if(
                    (player->getPosX() == cluster[i].posX + 1 && player->getPosY() == cluster[i].posY + 1) || 
                    (player->getPosX() == cluster[i].posX - 1 && player->getPosY() == cluster[i].posY - 1) || 
                    (player->getPosX() == cluster[i].posX + 1 && player->getPosY() == cluster[i].posY) ||
                    (player->getPosX() == cluster[i].posX - 1 && player->getPosY() == cluster[i].posY) ||
                    (player->getPosX() == cluster[i].posX && player->getPosY() == cluster[i].posY + 1) ||
                    (player->getPosX() == cluster[i].posX && player->getPosY() == cluster[i].posY - 1) ||
                    (player->getPosX() == cluster[i].posX && player->getPosY() == cluster[i].posY)
                ) {
                    g_sCoreStatus = STATUS::GAME_ASK_PLAY_AGAIN;
                    bCollision = true;
                    break;
                }
            }

            if(bCollision) break;
        }

        if(bCollision) {
            this->resetGame();
        }
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

    void resetTicks()
    {
        current_ticks = 0;
    }

    void resetGame()
    {
        player->~Player();
        delete player;

        listFloatingBlock.clear();
        sequenceRevealBlock.clear();
        listFloatingBlock.shrink_to_fit();

        player = new Player(PLAYER_CHARACTER);
        this->initializeSequenceRevealShape();

        score = current_ticks;
        this->resetTicks();
    }

    void initializeReponsiveMetrics()
    {   
        if(g_windowWidth <= 115 || g_windowHeight < g_screenWidth * 0.2) {
            g_sCoreStatus = STATUS::GAME_NOT_MEET_DIMENSION_REQUIREMENT;
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
        } else if(g_windowWidth <= 180){
            TIMEOUT = 25;
            slope = 4;
            spaceBetween = 20;
        } else if(g_windowWidth <= 200){
            TIMEOUT = 20;
            slope = 4;
            spaceBetween = 20;
        } else {
            TIMEOUT = 20;
            slope = 6;
            spaceBetween = 25;
        }
    }

    void initializeInstance()
    {
        player = new Player(PLAYER_CHARACTER);
        context = new Context();
    }

    void assignBlockShape(
        const int typeBlock, 
        const char chr, 
        const int quantity, 
        const int directionX, 
        const int multiplicationX, 
        const int multiplicationY, 
        const int initPositionY
    ) {
        listShape[typeBlock].chr = chr;
        listShape[typeBlock].quantity = quantity;
        listShape[typeBlock].directionX = directionX;
        listShape[typeBlock].multiplicationX = multiplicationX;
        listShape[typeBlock].multiplicationY = multiplicationY;
        listShape[typeBlock].initPositionY = initPositionY;
    }

    void assignBlockShapeWithAnimation(
        const int typeBlock, 
        const char chr, 
        const int quantity, 
        const int directionX, 
        const int initPositionY, 
        const int nextTickTriggerY, 
        const int minPositionY, 
        const int maxPostionY, 
        const int directionY,
        const bool useReverseDirectionY
    ) {
        listShape[typeBlock].chr = chr;
        listShape[typeBlock].quantity = quantity;
        listShape[typeBlock].directionX = directionX;
        listShape[typeBlock].initPositionY = initPositionY;
        listShape[typeBlock].nextTickTriggerY = nextTickTriggerY;
        listShape[typeBlock].minPositionY = minPositionY;
        listShape[typeBlock].maxPostionY = maxPostionY;
        listShape[typeBlock].directionY = directionY;
        listShape[typeBlock].useReverseDirectionY = useReverseDirectionY;
    }

    void assignSequenceShape(const long double ticks, const int typeBlock)
    {
        sequenceRevealBlock[ticks].listBlock.push_back(typeBlock);
        sequenceRevealBlock[ticks].bRevealed = false;
    }

    void appendBlockBasedOnTicks()
    {
        mvprintw(g_screenHeight + 2, 5, std::to_string((long)current_ticks).c_str());

        // bool bRevealed = sequenceRevealBlock[(long)current_ticks].bRevealed;
        // std::vector<int> vt = sequenceRevealBlock[(long)current_ticks].listBlock;

        // if(vt.size() == 0 || bRevealed) return;

        // for (std::vector<int>::iterator tick = vt.begin(); tick != vt.end(); tick++){
        //     listFloatingBlock.push_back(FloatingBlock(&listShape[*tick], (long)current_ticks));
        // }

        // sequenceRevealBlock[(long)current_ticks].bRevealed = true;

        int8_t nextTickAppend = 0;
        switch (level_selected) {
            case LEVEL::EASY:
                nextTickAppend = 3;
                break;
            case LEVEL::NORMAL:
                nextTickAppend = 2;
                break;
            case LEVEL::HARD:
                nextTickAppend = 1;
                break;
        }

        if((long)current_ticks - marked_tick_append_last_block == nextTickAppend) {
            uint8_t randomBlock = std::rand() % 20;
            listFloatingBlock.push_back(FloatingBlock(&listShape[all_kind_of_blocks[randomBlock]], (long)current_ticks));
            marked_tick_append_last_block = current_ticks;
        }
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

        // ONE
        this->assignBlockShapeWithAnimation(BLOCK_LEFT_ONE_ANIMATION_REVERSE_1, '^', 1, FROM_LEFT, 4, 1, 4, g_screenHeight - 2, 1, true);
        this->assignBlockShapeWithAnimation(BLOCK_LEFT_ONE_ANIMATION_REVERSE_2, '^', 1, FROM_LEFT, g_screenHeight - 2, 1, 4, g_screenHeight - 2, -1, true);
        this->assignBlockShapeWithAnimation(BLOCK_RIGHT_ONE_ANIMATION_REVERSE_1, '^', 1, FROM_RIGHT, 4, 1, 4, g_screenHeight - 2, 1, true);
        this->assignBlockShapeWithAnimation(BLOCK_RIGHT_ONE_ANIMATION_REVERSE_2, '^', 1, FROM_RIGHT, g_screenHeight - 2, 1, 4, g_screenHeight - 2, -1, true);

        this->assignBlockShapeWithAnimation(BLOCK_LEFT_ONE_ANIMATION_NO_REVERSE_1, '^', 1, FROM_LEFT, 4, 1, 4, g_screenHeight - 2, 1, false);
        this->assignBlockShapeWithAnimation(BLOCK_LEFT_ONE_ANIMATION_NO_REVERSE_2, '^', 1, FROM_LEFT, g_screenHeight - 2, 1, 4, g_screenHeight - 2, -1, false);
        this->assignBlockShapeWithAnimation(BLOCK_RIGHT_ONE_ANIMATION_NO_REVERSE_1, '^', 1, FROM_RIGHT, 4, 1, 4, g_screenHeight - 2, 1, false);
        this->assignBlockShapeWithAnimation(BLOCK_RIGHT_ONE_ANIMATION_NO_REVERSE_2, '^', 1, FROM_RIGHT, g_screenHeight - 2, 1, 4, g_screenHeight - 2, -1, false);
    }

    void initializeSequenceRevealShape()
    {
        this->assignSequenceShape(0, BLOCK_LEFT_TWO_1);
    }
};

int main(int argc, const char * argv[]) 
{
    initscr();			
    cbreak();
    echo();
    keypad(stdscr, TRUE);
    std::srand(time(NULL));

    Core* core = new Core(STATUS::GAME_MENU_START);
    core->init();

    timeout(TIMEOUT);

    while(core->getStatusGame() != STATUS::GAME_STOP) {
        keyPressed = getch();
        wclear(stdscr);
        core->update();
        wrefresh(stdscr);
    }

    endwin();
}

