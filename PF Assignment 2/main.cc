#include <iostream>

#include "renderer.h"

#include <chrono>
#include <thread>

int main()
{
    srand(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    renderer rndr(ivector2{ 45,21 });
    
    while (true)
    {
        rndr.set_room(ivector2{ rand(), rand() });
        rndr.construct_room();
        rndr.draw_screen();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}