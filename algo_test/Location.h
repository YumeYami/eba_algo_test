#pragma once
#include <iostream>

using namespace std;

#define NO_DESTINATION -1

class Location {
public:
	
	double lat;
	double lng;
	double time;
	int id;
	int person_id;
	int destination_cluster_id = NO_DESTINATION;
	int parent_cluster_id = -1;

	Location();
	Location(double lat, double lon, double time, int id);
	~Location();
};

