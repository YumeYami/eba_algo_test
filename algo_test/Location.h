#pragma once
#include <iostream>

using namespace std;

class Location {
public:
	

	double lat;
	double lon;
	double time;
	int id;

	Location();
	Location(double lat, double lon, double time, int id);
	~Location();
};

