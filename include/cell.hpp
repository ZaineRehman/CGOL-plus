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
	// amount of surrounding for reproduction
	int numForReproduction;
	// surrounding values taken from each cell state
	// index 0 = for empty cells, index 1 = state 1, etc. 
	std::vector<int> surrounding;
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
			if (cstate != 0) {
				total += surroundingRules[cstate];
			}
		}
	}

	return total;
}