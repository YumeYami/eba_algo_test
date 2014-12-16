#include "Cluster.h"


Cluster::Cluster() {
}
Cluster::Cluster(Location *initLocation) {
	location_list.push_back(initLocation);
}


Cluster::~Cluster() {
}
