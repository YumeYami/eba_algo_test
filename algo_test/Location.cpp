#include "Location.h"

Location::Location() {

}

Location::Location(double lat, double lon, double time, int id) {
	this->lat = lat;
	this->lng = lon;
	this->time = time;
	this->id = id;
}


Location::~Location() {
}
