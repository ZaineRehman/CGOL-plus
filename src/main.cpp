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


size_t WIDTH = 1000, HEIGHT = 1000;
double FPS = 60.0;

int minRuleVal = -8, maxRuleVal = 8;

size_t numThreads = 16;

// length/width of square for each cell
size_t pixelsPerCell = 3;


int main() {
	srand(time(0));

	bool RUNNING = true;
	std::vector<RGBA_t> screen(WIDTH*HEIGHT, RGBA_t{0,0,0,255});

	Renderer renderer {WIDTH, HEIGHT, "CGOL+"};

	size_t cellsWidth = WIDTH/pixelsPerCell;
	size_t cellsHeight = HEIGHT/pixelsPerCell;

	//std::unordered_map<Point, Cell> cellMap, newMap {};
	std::vector<Cell> cellMap(cellsWidth*cellsHeight, Cell{0}), 
					  newMap (cellsWidth*cellsHeight, Cell{0});

	std::vector<Rules> rules {
		Rules{-1, -1, -1, -1, 0, {0, 0, 1}}, 
		Rules{ 2,  3, -1, -1, 0, {0, 1, 2}}, 
		Rules{ 2,  5,  4,  4, 0, {0, 0, 1}}, 
		//Rules{ 2,  4,  3, 3, {1, 1, 1}},  // STANDARD
	};
	std::vector<RGBA_t> stateColors {
		RGBA_t{  0,   0,   0, 255}, 
		RGBA_t{255,   0,   0, 255}, 
		RGBA_t{255, 255, 255, 255}, 
	};

	bool paused = false;

	// delay for cell calc per frame
	int frameDelay = 0;
	int maxFrameDelay = 0;

	// show rules to terminal?
	bool presentRules = true;
	// index in rules vec
	int edit_stateIndex = 0;
	// which specific rule
	int edit_ruleIndex = 0;

	std::string 
		ansi_reset = "\033[0m", 
		ansi_red = "\033[91m", 
		ansi_blue = "\033[94m", 
		ansi_green = "\033[92m", 
		ansi_active = "\033[1;38:5:46m", 
		ansi_inactive = "\033[1m";

	// get color for parameters when outputting
	auto getPresentColor = [&](int stateIndex, int ruleIndex) {
		if (stateIndex == edit_stateIndex && ruleIndex == edit_ruleIndex) return ansi_active;
		return ansi_inactive;
	};


	// fill with random
	for (size_t x = 0; x < cellsWidth; ++x) {
		for (size_t y = 0; y < cellsHeight; ++y) {
			cellMap[y*cellsWidth + x] = Cell{rand() % 2};
		}
	}
	

	while (RUNNING) {
		std::chrono::time_point<std::chrono::steady_clock> timeStart = std::chrono::steady_clock::now();

		if (presentRules) {
			system("cls");

			// output rules
			for (size_t ri = 0; ri < rules.size(); ++ri) {
				std::cout 
					<< "State " << ri << ": "
					<< getPresentColor(ri, 0) << padString(rules[ri].minToSurvive, 2) << ansi_reset << ","
					<< getPresentColor(ri, 1) << padString(rules[ri].minToDie, 2) << ansi_reset << " ~ "
					<< getPresentColor(ri, 2) << padString(rules[ri].minForReproduction, 2) << ansi_reset << ","
					<< getPresentColor(ri, 3) << padString(rules[ri].maxForReproduction, 2) << ansi_reset << " ~ "
					<< getPresentColor(ri, 4) << padString(rules[ri].outOfBounds, 2) << ansi_reset << " {";
				for (size_t si = 0; si < rules[ri].surrounding.size(); ++si) {
					std::cout 
						<< getPresentColor(ri, 5 + si) 
						<< padString(rules[ri].surrounding[si], 2) 
						<< ansi_reset << (si == rules[ri].surrounding.size()-1 ? "}  [": ",");
				}
				// color
				std::cout 
					<< ansi_red   << padString(stateColors[ri].r, 3) << ansi_reset << " "
					<< ansi_green << padString(stateColors[ri].g, 3) << ansi_reset << " "
					<< ansi_blue  << padString(stateColors[ri].b, 3) << ansi_reset << "]\n";
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
		// mouse position
		float mouseX, mouseY;

		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_EVENT_QUIT) {
				RUNNING = false;
			}
			else if (event.type == SDL_EVENT_KEY_DOWN) {
				switch (event.key.key) {
					case SDL_EVENT_MOUSE_BUTTON_DOWN:  // place cell
						if (paused) {
							SDL_GetMouseState(&mouseX, &mouseY);
							if (mouseX >= 0 && mouseX < WIDTH && mouseY >= 0 && mouseY < HEIGHT) {
								// add to both cellMap AND render

								for (size_t py = 0; py < pixelsPerCell; ++py) {
									for (size_t px = 0; px < pixelsPerCell; ++px) {
										screen[(pixelsPerCell*mouseY+py)*WIDTH + (pixelsPerCell*mouseX+px)] = stateColors[newMap[mouseY*cellsWidth + mouseX].state];
									}
								}
							}
						}
						break;

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
						clamp_rollover(--edit_ruleIndex, 0, static_cast<int>(rules.size()+4));
						presentRules = true;
						break;
					case SDLK_RIGHT: 
						clamp_rollover(++edit_ruleIndex, 0, static_cast<int>(rules.size()+4));
						presentRules = true;
						break;
					
					case SDLK_MINUS:  // decrement rule value
						edit_desire = -1;
						break;
					case SDLK_EQUALS:  // increment rule value
						edit_desire = 1;
						break;
					
					case SDLK_Q:  // increase red
						stateColors[edit_stateIndex].r = static_cast<uint8_t>(min((int)stateColors[edit_stateIndex].r + 16, 255));
						presentRules = true;
						break;
					case SDLK_W:  // increase green
						stateColors[edit_stateIndex].g = static_cast<uint8_t>(min((int)stateColors[edit_stateIndex].g + 16, 255));
						presentRules = true;
						break;
					case SDLK_E:  // increase blue
						stateColors[edit_stateIndex].b = static_cast<uint8_t>(min((int)stateColors[edit_stateIndex].b + 16, 255));
						presentRules = true;
						break;
					case SDLK_A:  // decrease red
						stateColors[edit_stateIndex].r = static_cast<uint8_t>(max((int)stateColors[edit_stateIndex].r - 16, 0));
						presentRules = true;
						break;
					case SDLK_S:  // decrease green
						stateColors[edit_stateIndex].g = static_cast<uint8_t>(max((int)stateColors[edit_stateIndex].g - 16, 0));
						presentRules = true;
						break;
					case SDLK_D:  // decrease blue
						stateColors[edit_stateIndex].b = static_cast<uint8_t>(max((int)stateColors[edit_stateIndex].b - 16, 0));
						presentRules = true;
						break;
					
					case SDLK_SPACE:  // pause
						paused = !paused;
						break;
					
					case SDLK_LEFTBRACKET:  // decrease frame delay
						maxFrameDelay -= 2;
						clamp(maxFrameDelay, 0, 256);
						break;
					case SDLK_RIGHTBRACKET:  // increase frame delay
						maxFrameDelay += 2;
						clamp(maxFrameDelay, 0, 256);
						break;
					
					case SDLK_SLASH:  // fill with random
						for (size_t x = 0; x < cellsWidth; ++x) {
							for (size_t y = 0; y < cellsHeight; ++y) {
								cellMap[y*cellsWidth + x] = Cell{rand() % rules.size()};
							}
						}
						break;
					
					case SDLK_COMMA:  // decrement cell pixel size
						clamp(--pixelsPerCell, (size_t)1, (size_t)16);
						cellsWidth = WIDTH/pixelsPerCell;
						cellsHeight = HEIGHT/pixelsPerCell;

						cellMap.clear();
						cellMap.resize(cellsWidth*cellsHeight);
						cellMap.assign(cellsWidth*cellsHeight, Cell{0});

						newMap.clear();
						newMap.resize(cellsWidth*cellsHeight);
						newMap.assign(cellsWidth*cellsHeight, Cell{0});

						break;
					case SDLK_PERIOD:  // increment cell pixel size
						clamp(++pixelsPerCell, (size_t)1, (size_t)16);
						cellsWidth = WIDTH/pixelsPerCell;
						cellsHeight = HEIGHT/pixelsPerCell;

						cellMap.clear();
						cellMap.resize(cellsWidth*cellsHeight);
						cellMap.assign(cellsWidth*cellsHeight, Cell{0});

						newMap.clear();
						newMap.resize(cellsWidth*cellsHeight);
						newMap.assign(cellsWidth*cellsHeight, Cell{0});

						break;
					
					case SDLK_BACKSPACE:  // delete topmost state
						if (rules.size() != 1) {
							rules.pop_back();
							stateColors.pop_back();

							// remove reference from surrounding
							for (Rules& rule : rules) {
								rule.surrounding.pop_back();
							}

							// destroy all of those cells
							for (Cell& cell : cellMap) {
								if (cell.state == rules.size()) cell.state = 0;
							}

							// make sure state index isnt out of bounds
							if (edit_stateIndex >= rules.size()) edit_stateIndex = rules.size()-1;

							presentRules = true;
						}

						break;
					
					case SDLK_RETURN:  // add topmost state
						Rules defaultRuleset {2, 4, 3, 3, 0, {0}};
						LOOP(rules.size()-1) defaultRuleset.surrounding.push_back(1);

						rules.push_back(defaultRuleset);
						stateColors.push_back(RGBA_t{255,255,255,255});

						// add surrounding rules to each state
						for (Rules& rule : rules) {
							rule.surrounding.push_back(1);
						}

						presentRules = true;
						break;
				}
			}
		}

		// edit rules
		if (edit_desire != 0) {
			if (edit_ruleIndex == 0) {  // minToSurvive
				rules[edit_stateIndex].minToSurvive += edit_desire;
				clamp(rules[edit_stateIndex].minToSurvive, minRuleVal, maxRuleVal);
			} else if (edit_ruleIndex == 1) {  // minToDie
				rules[edit_stateIndex].minToDie += edit_desire;
				clamp(rules[edit_stateIndex].minToDie, minRuleVal, maxRuleVal);
			} else if (edit_ruleIndex == 2) {  // minForReproduction
				rules[edit_stateIndex].minForReproduction += edit_desire;
				clamp(rules[edit_stateIndex].minForReproduction, minRuleVal, maxRuleVal);
			} else if (edit_ruleIndex == 3) {  // maxForReproduction
				rules[edit_stateIndex].maxForReproduction += edit_desire;
				clamp(rules[edit_stateIndex].maxForReproduction, minRuleVal, maxRuleVal);
			} else if (edit_ruleIndex == 4) {  // outOfBounds
				rules[edit_stateIndex].outOfBounds += edit_desire;
				clamp(rules[edit_stateIndex].outOfBounds, minRuleVal, maxRuleVal);
			} else {  // surrounding rules
				rules[edit_stateIndex].surrounding[edit_ruleIndex-5] += edit_desire;
				clamp(rules[edit_stateIndex].surrounding[edit_ruleIndex-5], minRuleVal, maxRuleVal);
			}

			presentRules = true;
		}


		// TODO optimize checks

		if (frameDelay == 0 && !paused) {
			std::vector<std::thread> threads {};

			// chunks are vertical slices
			int xChunkSize = cellsWidth / numThreads;
			int overflow = cellsWidth % numThreads;

			for (size_t t = 0; t < numThreads; ++t) {
				threads.push_back(std::thread(doCellCalc, 
					0, cellsHeight, 
					xChunkSize*t, xChunkSize*(t+1) + (t == numThreads-1 ? overflow : 0), 
					cellsHeight, cellsWidth, cellMap, std::ref(newMap), rules, stateColors, std::ref(screen), 
					pixelsPerCell, WIDTH, HEIGHT
				));
			}

			for (std::thread& t : threads) t.join();

			cellMap.swap(newMap);
		}

		renderer.render(screen);

		frameDelay--;
		if (frameDelay < 0) frameDelay = maxFrameDelay;

		// deltatime

		const auto frameElapsed = std::chrono::steady_clock::now() - timeStart;
		const std::chrono::duration<double> delta = frameElapsed;

		if (0 && delta.count() < 1.0/(FPS)) {
			std::this_thread::sleep_for(std::chrono::duration<double>(1.0/(FPS) - delta.count()));
		}
	}
	
	return 0;
}