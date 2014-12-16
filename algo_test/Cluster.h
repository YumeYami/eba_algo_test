#pragma once
#include <iostream>
#include <vector>
#include "Location.h"

using namespace std;
#define UNKNOWN 0
#define ROUTE -1
#define PLACE -2

class Cluster {
public:
	vector<Location*> location_list;
	int type = UNKNOWN;
	int cluster_id = -1;
	double lat;
	double lng;

	Cluster();
	Cluster(Location *initLocation);
	~Cluster();
};

