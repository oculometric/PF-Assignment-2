#include "game.h"

#include <iostream>
#include <chrono>
#include <thread>

void game::main()
{
    srand(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    game_renderer = new renderer(ivector2{ 45,21 });
    player* game_player = new player(22, 17);
    game_renderer->set_player(game_player);

    while (true)
    {
        game_renderer->set_room(ivector2{ rand(), rand() });
        game_renderer->construct_room();
        tick();
        game_renderer->draw_screen();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
