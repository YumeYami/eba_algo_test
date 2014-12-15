#include "Cluster.h"


Cluster::Cluster() {
}
Cluster::Cluster(Location *initLocation) {
	locationList.push_back(initLocation);
}


Cluster::~Cluster() {
}
