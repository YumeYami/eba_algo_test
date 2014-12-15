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
	vector<Location*> locationList;
	int type = UNKNOWN;
	int cluster_id = -1;

	Cluster();
	Cluster(Location *initLocation);
	~Cluster();
};

