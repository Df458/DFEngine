#include "Game.h"
#include "Util.h"

Game* g_game;

int main(int argc, char* argv[])
{
    g_game = new DFBaseGame();

    if(!g_game->initialize()) {
        error("Failed to initialize application.");
        return 1;
    }
    g_game->mainLoop();
    g_game->cleanup();
    
    return 0;
}
