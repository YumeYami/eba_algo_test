#pragma once
#include <iostream>
#include <vector>
#include "Location.h"

using namespace std;

class Cluster {
public:
	vector<Location*> location_list;
	int cluster_id = -1;
	double lat;
	double lng;

	Cluster();
	Cluster(Location *initLocation);
	~Cluster();
};

