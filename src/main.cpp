#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <chrono>
#include <thread>
#include <vector>
#include <format>
#include <string>

#include <SDL3/SDL.h>

#include "render.hpp"
#include "cell.hpp"
#include "util.hpp"


size_t WIDTH = 800, HEIGHT = 800;
double FPS = 60.0;

// length/width of square for each cell
size_t pixelsPerCell = 4;


int main() {
	srand(time(0));

	bool RUNNING = true;
	std::vector<RGBA_t> screen(WIDTH*HEIGHT, RGBA_t{0,0,0,255});

	Renderer renderer {WIDTH, HEIGHT, "CGOL+"};

	//std::unordered_map<Point, Cell> cellMap, newMap {};
	std::vector<Cell> cellMap((WIDTH/pixelsPerCell)*(HEIGHT/pixelsPerCell), Cell{0}), 
					  newMap ((WIDTH/pixelsPerCell)*(HEIGHT/pixelsPerCell), Cell{0});

	std::vector<Rules> rules {
		Rules{-1, -1, -1, {1, 0, 1}}, 
		Rules{ 2,  3, -1, {1, 1, 2}}, 
		Rules{ 2,  5,  4, {1, 0, 1}}, 
		//Rules{ 2,  4,  3, {1, 1, 1}},  // STANDARD
	};
	std::vector<RGBA_t> stateColors {
		RGBA_t{  0,   0,   0, 255}, 
		RGBA_t{255,   0,   0, 255}, 
		RGBA_t{255, 255, 255, 255}, 
	};

	// paused?
	bool paused = false;

	// show rules to terminal?
	bool presentRules = true;
	// index in rules vec
	int edit_stateIndex = 0;
	// which specific rule
	int edit_ruleIndex = 0;
	// if editing surrounding rules, which index
	int edit_surroundingIndex = -1;

	std::string ansi_reset = "\033[0m";
	std::string ansi_active = "\033[1;38:5:46m";
	std::string ansi_inactive = "\033[1m";

	// get color for parameters when outputting
	auto getPresentColor = [&](int stateIndex, int ruleIndex) {
		if (stateIndex == edit_stateIndex && ruleIndex == edit_ruleIndex) return ansi_active;
		return ansi_inactive;
	};


	// fill with random
	for (size_t x = 0; x < WIDTH/pixelsPerCell; ++x) {
		for (size_t y = 0; y < HEIGHT/pixelsPerCell; ++y) {
			cellMap[y*WIDTH/pixelsPerCell + x] = Cell{rand() % 2};
		}
	}
	

	while (RUNNING) {
		std::chrono::time_point<std::chrono::steady_clock> timeStart = std::chrono::steady_clock::now();

		if (presentRules) {
			system("cls");

			for (size_t ri = 0; ri < rules.size(); ++ri) {
				std::cout << "State " << ri << ": ";
				std::cout << getPresentColor(ri, 0) << padString(rules[ri].minToSurvive, 2) << ansi_reset << ",";
				std::cout << getPresentColor(ri, 1) << padString(rules[ri].minToDie, 2) << ansi_reset << ",";
				std::cout << getPresentColor(ri, 2) << padString(rules[ri].numForReproduction, 2) << ansi_reset << " {";
				for (size_t si = 0; si < rules[ri].surrounding.size(); ++si) {
					std::cout 
					<< getPresentColor(ri, 3 + si) 
					<< padString(rules[ri].surrounding[si], 2) 
					<< ansi_reset << (si == rules[ri].surrounding.size()-1 ? "}\n": ",");
				}
			}

			/* TODO look into why this crashes
			for (size_t ri = 0; ri < rules.size(); ++ri) {
				std::cout
					<< "State " << ri << ":"
					<< std::format("{:2}", rules[ri].minToSurvive) << ","
					<< std::format("{:2}", rules[ri].minToDie) << ","
					<< std::format("{:2}", rules[ri].numForReproduction) << " {";
				for (size_t si = 0; si < rules[ri].surrounding.size(); ++si) {
					std::cout << std::format("{:2}", rules[ri].surrounding[si]) << (si == rules[ri].surrounding.size()-1 ? "}\n": ",");
				}
			}*/

			presentRules = false;
		}

		// poll events
		// key buffer, not key states
		SDL_Event event;
		// desire to change indexed rule
		int edit_desire = 0;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_EVENT_QUIT) {
				RUNNING = false;
			}
			else if (event.type == SDL_EVENT_KEY_DOWN) {
				switch (event.key.key) {
					case SDLK_ESCAPE: 
						RUNNING = false;
						break;
					
					case SDLK_UP: 
						clamp_rollover(--edit_stateIndex, 0, static_cast<int>(rules.size()-1));
						presentRules = true;
						break;
					case SDLK_DOWN: 
						clamp_rollover(++edit_stateIndex, 0, static_cast<int>(rules.size()-1));
						presentRules = true;
						break;
					case SDLK_LEFT: 
						clamp_rollover(--edit_ruleIndex, 0, static_cast<int>(2*rules.size()-1));
						presentRules = true;
						break;
					case SDLK_RIGHT: 
						clamp_rollover(++edit_ruleIndex, 0, static_cast<int>(2*rules.size()-1));
						presentRules = true;
						break;
					
					case SDLK_MINUS: 
						edit_desire = -1;
						break;
					case SDLK_EQUALS: 
						edit_desire = 1;
						break;
					
					case SDLK_SPACE: 
						paused = !paused;
						break;
				}
			}
		}

		// edit rules
		if (edit_desire != 0) {
			if (edit_ruleIndex == 0) {
				rules[edit_stateIndex].minToSurvive += edit_desire;
				clamp(rules[edit_stateIndex].minToSurvive, -1, 8);
			} else if (edit_ruleIndex == 1) {
				rules[edit_stateIndex].minToDie += edit_desire;
				clamp(rules[edit_stateIndex].minToDie, -1, 8);
			} else if (edit_ruleIndex == 2) {
				rules[edit_stateIndex].numForReproduction += edit_desire;
				clamp(rules[edit_stateIndex].numForReproduction, -1, 8);
			} else {
				// editing surrounding rules
				rules[edit_stateIndex].surrounding[edit_ruleIndex-3] += edit_desire;
				clamp(rules[edit_stateIndex].surrounding[edit_ruleIndex-3], -1, 8);
			}

			presentRules = true;
		}

		//cellMap ((WIDTH/pixelsPerCell)*(HEIGHT/pixelsPerCell), Cell{0});
		//newMap ((WIDTH/pixelsPerCell)*(HEIGHT/pixelsPerCell), Cell{0});

		size_t cellsWidth = WIDTH/pixelsPerCell, cellsHeight = HEIGHT/pixelsPerCell;


		// TODO optimize checks

		for (size_t y = 0; y < cellsHeight; ++y) {
			for (size_t x = 0; x < cellsWidth; ++x) {
				newMap[y*cellsWidth + x] = cellMap[y*cellsWidth + x];
				Point point {x, y};

				// check if cell
				Cell c = cellMap[y*cellsWidth + x];
				if (c.state != 0) {
					const Rules& local = rules[c.state];
					// get surrounding cell outcome
					int outcome = getSurrounding(point, cellMap, local.surrounding, cellsWidth, cellsHeight);

					if (outcome < local.minToSurvive) {
						// lower state
						newMap[y*cellsWidth + x].state--;
					} else if (outcome >= local.minToDie) {
						// raise state
						newMap[y*cellsWidth + x].state++;
						// state too big? kill it
						if (newMap[y*cellsWidth + x].state == rules.size()) {
							newMap[y*cellsWidth + x].state = 0;
						}
					}
				} else {
					// no cell, check if life
					int outcome = getSurrounding(point, cellMap, rules[0].surrounding, cellsWidth, cellsHeight);

					// cells regenerate preferring higher states
					for (size_t r = 1; r < rules.size(); ++r) {
						if (outcome == rules[r].numForReproduction) {
							// alive!
							newMap[y*cellsWidth + x] = Cell{r};
						}
					}
				}

				// calculate render
				for (size_t py = 0; py < pixelsPerCell; ++py) {
					for (size_t px = 0; px < pixelsPerCell; ++px) {
						screen[(pixelsPerCell*point.y+py)*WIDTH + (pixelsPerCell*point.x+px)] = stateColors[newMap[y*cellsWidth + x].state];
					}
				}
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