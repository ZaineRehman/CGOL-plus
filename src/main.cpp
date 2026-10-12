#include <iostream>
#include <unordered_map>

#include <SDL3/SDL.h>

#include "render.hpp"


size_t WIDTH = 1200, HEIGHT = 800;


int main() {
	srand(time(0));

	bool RUNNING = true;
	std::vector<RGBA_t> screen(WIDTH*HEIGHT, RGBA_t{0,0,0,255});

	Renderer renderer {WIDTH, HEIGHT, "Stereogram"};

	
	

	while (RUNNING) {
		// poll events
		// key buffer, not key states
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_EVENT_QUIT) {
				RUNNING = false;
			}
			else if (event.type == SDL_EVENT_KEY_DOWN) {
				switch (event.key.key) {
					case SDLK_ESCAPE: 
						RUNNING = false;
						break;
				}
			}
		}

		renderer.render(screen);
	}
	
	return 0;
}