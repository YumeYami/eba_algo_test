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
#define FILE_NAME "location-2.txt"
#define PI 3.141592653589793238462643383279502884
#define PLACE_THRESHOLD 0.8413

vector<Location> personList;
vector<Cluster> clusterList;
double average = 0;
double variance = 0;


void clustering(vector<Location> locationList, vector<Cluster> &clusterList) {
	if ( locationList.empty() ) return;
	vector<int> locationStatus;
	locationStatus.resize(locationList.size(), 0);
	for ( unsigned int i = 0; i < locationList.size(); i++ ) {
		if ( locationStatus[i] != 0 ) {
			continue;
		}
		Location temp = locationList[i];
		locationStatus[i] = 1;

		Cluster cluster = Cluster(locationList[i]);
		queue<Location> QLocation;
		QLocation.push(temp);
		while ( !QLocation.empty() ) {
			//cout << "q execute\n";
			Location sLocation = QLocation.front();
			QLocation.pop();
			for ( unsigned int j = 0; j < locationList.size(); j++ ) {
				if ( locationStatus[j] != 0 ) {
					continue;
				}
				Location temp1, temp2;
				if ( sLocation.time > locationList[j].time ) {
					temp1 = sLocation;
					temp2 = locationList[j];
				}
				else {
					temp1 = locationList[j];
					temp2 = sLocation;
				}
				double dLat = temp1.lat - temp2.lat;
				double dLon = temp1.lon - temp2.lon;
				double dTime = min(temp1.time - temp2.time, 24 - temp1.time + temp2.time);
				double dist = dLat*dLat + dLon*dLon + K_TIME*K_TIME*dTime*dTime;
				//cout << "dist: " << dist << "\n";
				if ( dist < DIST_THRESHOLD ) {
					QLocation.push(locationList[j]);
					locationStatus[j] = 1;
					cluster.locationList.push_back(locationList[j]);
				}
			}
		}
		clusterList.push_back(cluster);
	}
}

void printCluster(vector<Cluster> clusterList) {
	cout << "number of cluster: " << clusterList.size() << "\n";
	for ( unsigned int i = 0; i < clusterList.size(); i++ ) {
		cout << "cluster" << i << ": ";
		for ( unsigned int j = 0; j < clusterList[i].locationList.size(); j++ ) {
			cout << clusterList[i].locationList[j].id << " ";
		}
		cout << "\n";
	}
}

void calCluster(vector<Cluster> clusterList) {
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
	cout << "average: " << average << "\tvariance: " << variance<<"\n";
}

void classifyCluster(vector<Cluster> &clusterList) {
	for ( unsigned int i = 0; i < clusterList.size(); i++ ) {
		double n = clusterList[i].locationList.size();
		cout << "n: " << n << "\n";
		double fn = 1.0 * exp(-((n - average)*(n - average) / 2.0 / variance)) / sqrt(2.0 * PI * variance);
		if ( fn > PLACE_THRESHOLD )
			clusterList[i].type = PLACE;
		else
			clusterList[i].type = ROUTE;
		cout << "fn " << i << ": " << fn << "\n";
	}
	for ( unsigned int i = 0; i < clusterList.size(); i++ ) {
		cout << "cluster " << i << ": ";
		if ( clusterList[i].type == PLACE ) {
			cout << "PLACE\n";
		}
		else {
			cout << "ROUTE\n";
		}
	}
}

int main() {
	fstream fileLocation;
	fileLocation.open(FILE_NAME);
	string temp;
	int locationID = 0;


	while ( !fileLocation.eof() ) {
		getline(fileLocation, temp);
		stringstream ss(temp);
		double lat, lon, time;
		ss >> time;
		ss >> lat;
		ss >> lon;

		int id = locationID++;
		personList.push_back(Location(lat, lon, time, id));
	}
	for ( unsigned int i = 0; i < personList.size(); i++ ) {
		Location x = personList[i];
		cout << x.id << ":\t" << x.time << "\t" << x.lat << "\t" << x.lon << "\n";
	}
	clustering(personList, clusterList);
	printCluster(clusterList);
	calCluster(clusterList);
	classifyCluster(clusterList);

}