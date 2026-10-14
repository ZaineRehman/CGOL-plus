#pragma once

#include <iostream>
#include <unordered_map>
#include <vector>


struct Point {
	int x, y;

	Point() = default;
	Point(int _x, int _y) : x(_x), y(_y) {}
	Point(size_t _x, size_t _y) : x(static_cast<int>(_x)), y(static_cast<int>(_y)) {}
	Point(const Point& other) = default;

	bool operator==(const Point& other) const {
		return x == other.x && y == other.y;
	}

	Point operator+(const Point& other) const {
		return Point{x+other.x, y+other.y};
	}
};

// custom hash function for Point
template <>
struct std::hash<Point> {
	std::size_t operator()(const Point& point) const {
		std::size_t h1 = std::hash<int>{}(point.x);
		std::size_t h2 = std::hash<int>{}(point.y);

		std::size_t seed = h1;
		seed ^= h2 + 0x9e3779b9 + (seed << 6) + (seed >> 2);

		return seed;
	}
};


class Cell {
public:
	//Point coords {};
	uint16_t state;

	Cell() = default;
	Cell(int _state) : state(_state) {}
	Cell(size_t _state) : state(static_cast<int>(_state)) {}
	//Cell(int _state/**/, const Point& _point) : state(_state), coords(_point) {}
	Cell(const Cell& other) = default;
};


// rules for a given cell state
struct Rules {
	// minimum surrounding to survivwe
	int minToSurvive;
	// minimum surrounding for overpopulation death
	int minToDie;
	// min amount of surrounding for reproduction
	int minForReproduction;
	// max amount of surrounding for reproduction
	int maxForReproduction;
	// surrounding values taken from each cell state
	// index 0 = for empty cells, index 1 = state 1, etc. 
	std::vector<int> surrounding;

	Rules() = default;
	Rules(int _minToSurvive, int _minToDie, int _minForReproduction, int _maxForReproduction, const std::vector<int>& _surrounding) 
		: minToSurvive(_minToSurvive), minToDie(_minToDie), minForReproduction(_minForReproduction), maxForReproduction(_maxForReproduction), surrounding(_surrounding) {}
	Rules(const Rules& other) = default;
};

// returns the number of surrounding cells
int getSurrounding(const Point& point, const std::vector<Cell>& cells, const std::vector<int>& surroundingRules, size_t WIDTH, size_t HEIGHT) {
	int total = 0;

	static const Point toCheck[] = {
		{-1, -1}, {-1, 0}, {-1, 1},  // left row
		{0, -1}, {0, 1},             // middle row
		{1, -1}, {1, 0}, {1, 1}      // right row
	};

	for (size_t i = 0; i < 8; ++i) {
		// if cell exists
		int newY = point.y + toCheck[i].y, newX = point.x + toCheck[i].x;

		if (newX >= 0 && newX < static_cast<int>(WIDTH) && newY >= 0 && newY < static_cast<int>(HEIGHT)) {
			int cstate = cells[newY*static_cast<int>(WIDTH) + newX].state;
			total += surroundingRules[cstate];
		}
	}

	return total;
}


// cell calculation
void doCellCalc(
	size_t yStart, size_t yEnd, 
	size_t xStart, size_t xEnd, 
	size_t cellsHeight, size_t cellsWidth, 
	const std::vector<Cell>& cellMap, std::vector<Cell>& newMap, 
	const std::vector<Rules>& rules, const std::vector<RGBA_t>& stateColors, 
	std::vector<RGBA_t>& screen, 
	size_t pixelsPerCell, size_t WIDTH, size_t HEIGHT
) {
	for (size_t y = yStart; y < yEnd; ++y) {
		for (size_t x = xStart; x < xEnd; ++x) {
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
					if (rules[r].minForReproduction <= outcome && outcome <= rules[r].maxForReproduction) {
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
}
