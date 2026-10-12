#pragma once

#include <iostream>


struct Point {
	uint32_t x, y;
};

class Cell {
public:
	Point coords {};
	int state;

	Cell(int _state, const Point& _point) : state(_state), coords(_point) {}
};