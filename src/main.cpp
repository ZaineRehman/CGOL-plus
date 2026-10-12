#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <chrono>
#include <thread>
#include <vector>

#include <SDL3/SDL.h>

#include "render.hpp"
#include "cell.hpp"


size_t WIDTH = 800, HEIGHT = 800;
double FPS = 60.0;


int main() {
	srand(time(0));

	bool RUNNING = true;
	std::vector<RGBA_t> screen(WIDTH*HEIGHT, RGBA_t{0,0,0,255});

	Renderer renderer {WIDTH, HEIGHT, "CGOL+"};

	//std::unordered_map<Point, Cell> cellMap, newMap {};
	std::vector<Cell> cellMap(WIDTH*HEIGHT, Cell{0}), newMap(WIDTH*HEIGHT, Cell{0});

	std::vector<Rules> rules {
		Rules{-1, -1, -1, {1, 1}}, 
		Rules{2, 4, 3, {1, 1}}
	};
	std::vector<RGBA_t> stateColors {
		RGBA_t{  0,   0,   0, 255}, 
		RGBA_t{255, 255, 255, 255}
	};

	// fill
	for (int x = 0; x < WIDTH; ++x) {
		for (int y = 0; y < HEIGHT; ++y) {
			cellMap[y*WIDTH + x] = Cell{rand() % 2};
		}
	}
	

	while (RUNNING) {
		std::chrono::time_point<std::chrono::steady_clock> timeStart = std::chrono::steady_clock::now();

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


		// TODO optimize checks

		for (size_t y = 0; y < HEIGHT; ++y) {
			for (size_t x = 0; x < WIDTH; ++x) {
				newMap[y*WIDTH + x] = cellMap[y*WIDTH + x];
				Point point {x, y};

				// check if cell
				Cell c = cellMap[y*WIDTH + x];
				if (c.state != 0) {
					const Rules& local = rules[c.state];
					// get surrounding cell outcome
					int outcome = getSurrounding(point, cellMap, local.surrounding, WIDTH, HEIGHT);

					if (outcome < local.minToSurvive || outcome >= local.minToDie) {
						// lower state
						newMap[y*WIDTH + x].state--;
					}
				} else {
					// no cell, check if life
					int outcome = getSurrounding(point, cellMap, rules[0].surrounding, WIDTH, HEIGHT);

					// cells regenerate preferring higher states
					for (size_t r = 1; r < rules.size(); ++r) {
						if (outcome == rules[r].numForReproduction) {
							// alive!
							newMap[y*WIDTH + x] = Cell{r};
						}
					}
				}

				// calculate render
				screen[point.y*WIDTH + point.x] = stateColors[newMap[y*WIDTH + x].state];
			}
		}

		cellMap.swap(newMap);

		renderer.render(screen);

		// deltatime

		const auto frameElapsed = std::chrono::steady_clock::now() - timeStart;
		const std::chrono::duration<double> delta = frameElapsed;

		if (delta.count() < 1.0/(FPS)) {
			std::this_thread::sleep_for(std::chrono::duration<double>(1.0/(FPS) - delta.count()));
		}
	}
	
	return 0;
}