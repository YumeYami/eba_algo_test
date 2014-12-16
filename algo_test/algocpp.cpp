#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <list>
#include <queue>
#include "Location.h"
#include "Cluster.h"
#include <cmath>

using namespace std;

#define K_TIME 0.0004
#define DIST_THRESHOLD 0.0025
#define PI 3.141592653589793238462643383279502884
#define PLACE_THRESHOLD 5
#define TIME_DESTINATION_THRESHOLD 3

#define FILE_NAME "location-3.txt"
#define OUTPUT_FILE_NAME "out-location-3.txt"
// #define FILE_NAME "location-2.txt"

vector<Location> locationList;
vector<Cluster> clusterList;
double average = 0;
double variance = 0;
double placeThreshold = 0;

int location_id_counter = 0;
int cluster_id_counter = 0;

fstream fileLocation;
ofstream out(OUTPUT_FILE_NAME);

void ReadFileLocation(fstream &fileLocation, vector<Location> &locationList) {
	string temp;
	while ( !fileLocation.eof() ) {
		getline(fileLocation, temp);
		stringstream ss(temp);
		double lat, lon, timeH, timeM;
		ss >> timeH;
		ss >> timeM;
		ss >> lat;
		ss >> lon;

		int id = location_id_counter++;
		locationList.push_back(Location(lat, lon, timeH + timeM / 60, id));
	}
}

bool isInCluster(Location sLocation, Location location2) {
	Location temp1, temp2;
	if ( sLocation.time > location2.time ) {
		temp1 = sLocation;
		temp2 = location2;
	}
	else {
		temp1 = location2;
		temp2 = sLocation;
	}
	double dLat = temp1.lat - temp2.lat;
	double dLon = temp1.lon - temp2.lon;
	double dTime = min(temp1.time - temp2.time, 24 - temp1.time + temp2.time);
	double dist = dLat*dLat + dLon*dLon + K_TIME*K_TIME*dTime*dTime;
	if ( dist < DIST_THRESHOLD ) {
		return true;
	}
	else {
		return false;
	}
}

void clustering(vector<Location> &locationList, vector<Cluster> &clusterList) {
	if ( locationList.empty() ) return;
	vector<int> locationStatus;
	locationStatus.resize(locationList.size(), 0);
	for ( unsigned int i = 0; i < locationList.size(); i++ ) {
		if ( locationStatus[i] != 0 ) {
			continue;
		}
		Location temp = locationList[i];
		locationStatus[i] = 1;
		locationList[i].parent_cluster_id = cluster_id_counter;
		Cluster cluster = Cluster(&locationList[i]);
		queue<Location> QLocation;
		QLocation.push(temp);
		while ( !QLocation.empty() ) {
			//out << "q execute\n";
			Location sLocation = QLocation.front();
			QLocation.pop();
			for ( unsigned int j = 0; j < locationList.size(); j++ ) {
				if ( locationStatus[j] != 0 ) {
					continue;
				}
				if ( isInCluster(sLocation, locationList[j]) ) {
					QLocation.push(locationList[j]);
					locationStatus[j] = 1;
					locationList[j].parent_cluster_id = cluster_id_counter;
					cluster.locationList.push_back(&locationList[j]);
				}
			}
		}
		cluster.cluster_id = cluster_id_counter++;
		clusterList.push_back(cluster);
	}
}

void printCluster(vector<Cluster> clusterList) {
	out << "number of cluster: " << clusterList.size() << "\n";
	for ( unsigned int i = 0; i < clusterList.size(); i++ ) {
		out << "cluster" << i << ": " << clusterList[i].locationList.size() << "- ";
		for ( unsigned int j = 0; j < clusterList[i].locationList.size(); j++ ) {
			out << (*clusterList[i].locationList[j]).id << " ";
		}
		out << "\n";
	}
}

void calClusterDistribution(vector<Cluster> clusterList) {
	average = 0;
	for ( unsigned int i = 0; i < clusterList.size(); i++ ) {
		average += clusterList[i].locationList.size();
	}
	average /= clusterList.size();

	variance = 0;
	for ( unsigned int i = 0; i < clusterList.size(); i++ ) {
		int diff = clusterList[i].locationList.size() - average;
		variance += diff*diff;
	}
	variance /= clusterList.size();
	variance /= clusterList.size();
	out << "average: " << average << "\tvariance: " << variance << "\n";
	//placeThreshold = average + sqrt(variance)/2;
	placeThreshold = PLACE_THRESHOLD;
}

void calClusterLocation(vector<Cluster> &clusterList) {
	for ( unsigned int i = 0; i < clusterList.size(); i++ ) {
		double lat = 0, lon = 0, time = 0;
		for ( unsigned int j = 0; j < clusterList[i].locationList.size(); j++ ) {
			lat += (*clusterList[i].locationList[j]).lat;
			lon += (*clusterList[i].locationList[j]).lon;
		}
		lat /= clusterList[i].locationList.size();
		lon /= clusterList[i].locationList.size();
		clusterList[i].lat = lat;
		clusterList[i].lon = lon;
	}
}

void classifyCluster(vector<Cluster> &clusterList) {
	for ( unsigned int i = 0; i < clusterList.size(); i++ ) {
		double n = clusterList[i].locationList.size();
		//out << "n: " << n << "\n";
		//double fn = 1.0 * exp(-((n - average)*(n - average) / 2.0 / variance)) / sqrt(2.0 * PI * variance);
		if ( n > placeThreshold )
			clusterList[i].type = PLACE;
		else
			clusterList[i].type = ROUTE;
		//out << "fn " << i << ": " << fn << "\n";
	}
	for ( unsigned int i = 0; i < clusterList.size(); i++ ) {
		out << "cluster " << i << ": ";
		if ( clusterList[i].type == PLACE ) {
			out << "PLACE\n";
		}
		else {
			out << "ROUTE\n";
		}
	}
}

void generateDestinationRef(vector<Location> &locationList) {
	for ( unsigned int i = 0; i < locationList.size() - 1; i++ ) {
		if ( abs(locationList[i].time - locationList[i + 1].time) <= TIME_DESTINATION_THRESHOLD )
			locationList[i].destination_cluster_id = locationList[i + 1].parent_cluster_id;
	}
}

void printDestination(vector<Cluster> clusterList) {
	for ( unsigned int i = 0; i < locationList.size(); i++ ) {
		out << "location " << i << ": " << locationList[i].parent_cluster_id << "   \t->\t" << locationList[i].destination_cluster_id;
		if ( locationList[i].destination_cluster_id == UNKNOWN - 1 )
			out << "\t  ";
		else if ( clusterList[locationList[i].parent_cluster_id].type == PLACE )
			out << "\tp   ";
		else
			out << "\tr ";
		if ( locationList[i].destination_cluster_id != -1 && locationList[i].parent_cluster_id != locationList[i].destination_cluster_id )
			out << "x";
		out << "\n";
	}
}

double calDestinationScore(Location current_location, Location destLocation) {
	double score = 0;
	double t;
	if ( abs(current_location.time - destLocation.time) <= 12.0 ) {

	}
	else if ( abs(current_location.time - destLocation.time) > 12 && current_location.time > destLocation.time ) {

	}
	else {

	}
	return score;
}

//not completed
void predictNextLocation(Location current_location, vector<Cluster> clusterList, vector<Location> locationList) {
	double maxScore = 0;
	int maxClusterID = 0;
	for ( unsigned int i = 0; i < clusterList.size(); i++ ) {
		double score = 0;
		for ( unsigned int j = 0; j < clusterList[i].locationList.size(); j++ ) {
			score += calDestinationScore(current_location, (*clusterList[i].locationList[j]));
		}
	}
}

void addSingleLocation(Location newLocation, vector<Location> &locationList, vector<Cluster> &clusterList) {
	for ( unsigned int i = 0; i < locationList.size(); i++ ) {
		if ( isInCluster(newLocation, locationList[i]) ) {
			int clusterNum = locationList[i].parent_cluster_id;
			int clusterSize = clusterList[clusterNum].locationList.size();
			newLocation.parent_cluster_id = clusterNum;
			clusterList[clusterNum].lat = (clusterList[clusterNum].lat * clusterSize + newLocation.lat) / (clusterSize + 1);
			clusterList[clusterNum].lon = (clusterList[clusterNum].lon * clusterSize + newLocation.lon) / (clusterSize + 1);
			locationList.push_back(newLocation);
			clusterList[clusterNum].locationList.push_back(&locationList[locationList.size() - 1]);
			break;
		}
	}
}

void addLocationList(vector<Location> newLocation, vector<Location> &locationList, vector<Cluster> &clusterList) {

}

int main() {

	fileLocation.open(FILE_NAME);
	// import new user location
	ReadFileLocation(fileLocation, locationList);
	clustering(locationList, clusterList);
	printCluster(clusterList);
	calClusterDistribution(clusterList);
	calClusterLocation(clusterList);
	classifyCluster(clusterList);
	generateDestinationRef(locationList);
	printDestination(clusterList);
	predictNextLocation(locationList[locationList.size() - 1], clusterList, locationList);

	//update new location (individual)
// 	Location newLocation = Location(35.0, 140.0, 12.30, location_id_counter++);
// 	addSingleLocation(newLocation, locationList, clusterList);
	out.close();
}