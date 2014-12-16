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
#define PLACE_THRESHOLD_SIZE 5
#define TIME_DESTINATION_THRESHOLD 24
#define VARIANCE_LAT 0.00002017
#define VARIANCE_LNG 0.00003082
#define VARIANCE_TIME 4.0

//#define FILE_NAME "location-2.txt"
#define FILE_TRAIN "location-3-train.txt"
#define FILE_TEST "location-3-test.txt"
#define OUTPUT_FILE_NAME "out-location-3.txt"
// #define FILE_NAME "location-2.txt"

int location_id_counter = 0;
int cluster_id_counter = 0;

fstream fileLocation;
fstream fileTest;
ofstream out(OUTPUT_FILE_NAME);

void ReadFileLocation(fstream &fileLocation, vector<Location> &locationList) {
	string temp;
	while ( !fileLocation.eof() ) {
		getline(fileLocation, temp);
		stringstream ss(temp);
		double lat, lng, timeH, timeM;
		ss >> timeH;
		ss >> timeM;
		ss >> lat;
		ss >> lng;

		int id = location_id_counter++;
		locationList.push_back(Location(lat, lng, timeH + timeM / 60, id));
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
	double dLng = temp1.lng - temp2.lng;
	double dTime = min(temp1.time - temp2.time, 24 - temp1.time + temp2.time);
	double dist = dLat*dLat + dLng*dLng + K_TIME*K_TIME*dTime*dTime;
	if ( dist < DIST_THRESHOLD ) {
		return true;
	}
	else {
		return false;
	}
}

void clustering(vector<Location> & location_list, vector<Cluster> & cluster_list) {
	if ( location_list.empty() ) return;
	vector<int> locationStatus;
	locationStatus.resize(location_list.size(), 0);
	vector<Cluster> newClusterList;
	cluster_id_counter = 0;
	location_id_counter = 0;
	for ( unsigned int i = 0; i < location_list.size(); i++ ) {
		if ( locationStatus[i] != 0 ) {
			continue;
		}
		Location temp = location_list[i];
		locationStatus[i] = 1;
		location_list[i].parent_cluster_id = cluster_id_counter;
		Cluster cluster = Cluster(&location_list[i]);
		queue<Location> QLocation;
		QLocation.push(temp);
		while ( !QLocation.empty() ) {
			//out << "q execute\n";
			Location sLocation = QLocation.front();
			QLocation.pop();
			for ( unsigned int j = 0; j < location_list.size(); j++ ) {
				if ( locationStatus[j] != 0 ) {
					continue;
				}
				if ( isInCluster(sLocation, location_list[j]) ) {
					QLocation.push(location_list[j]);
					locationStatus[j] = 1;
					location_list[j].parent_cluster_id = cluster_id_counter;
					cluster.locationList.push_back(&location_list[j]);
				}
			}
		}
		cluster.cluster_id = cluster_id_counter++;
		newClusterList.push_back(cluster);
	}
	cluster_list = newClusterList;
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

void calClusterDistribution(vector<Cluster> clusterList, double & average, double & variance, double & placeThreshold) {
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
	placeThreshold = PLACE_THRESHOLD_SIZE;
}

void calClusterLocation(vector<Cluster> &clusterList) {
	for ( unsigned int i = 0; i < clusterList.size(); i++ ) {
		double lat = 0, lng = 0, time = 0;
		for ( unsigned int j = 0; j < clusterList[i].locationList.size(); j++ ) {
			lat += (*clusterList[i].locationList[j]).lat;
			lng += (*clusterList[i].locationList[j]).lng;
		}
		lat /= clusterList[i].locationList.size();
		lng /= clusterList[i].locationList.size();
		clusterList[i].lat = lat;
		clusterList[i].lng = lng;
	}
}

void classifyCluster(vector<Cluster> &clusterList, double placeThreshold) {
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

		if ( clusterList[i].type == PLACE ) {
			out << "cluster " << i << ": ";
			out << "PLACE\n";
		}
		else {
			//out << "ROUTE\n";
		}
	}
}

void generateDestinationRef(vector<Location> &locationList) {
	for ( unsigned int i = 0; i < locationList.size() - 1; i++ ) {
		if ( abs(locationList[i].time - locationList[i + 1].time) <= TIME_DESTINATION_THRESHOLD )
			locationList[i].destination_cluster_id = locationList[i + 1].parent_cluster_id;
	}
}

void printDestination(vector<Cluster> clusterList, vector<Location> locationList) {
	for ( unsigned int i = 0; i < locationList.size(); i++ ) {
		if ( locationList[i].parent_cluster_id != locationList[i].destination_cluster_id )
			out << "location " << i << ": " << locationList[i].parent_cluster_id << "   \t->\t" << locationList[i].destination_cluster_id << "\n";
	}
}

double calDestinationScore(Location current_location, Location location) {
	double score = 0;
	double diff_lat = current_location.lat - location.lat;
	double diff_lng = current_location.lng - location.lng;
	double diff_time = current_location.time - location.time;
	if ( abs(current_location.time - location.time) > 12.0 && current_location.time > location.time ) {
		diff_time -= 12.0;
	}
	else if ( abs(current_location.time - location.time) > 12.0 && current_location.time <= location.time ) {
		diff_time += 12.0;
	}
	score = 1.0 / pow(2 * PI, 1.5) / sqrt(VARIANCE_LAT + VARIANCE_LNG + VARIANCE_TIME) * exp(-1.0 / 2 * ((diff_lat*diff_lat*VARIANCE_LAT) + (diff_lng*diff_lng*VARIANCE_LNG) + (diff_time*diff_time*VARIANCE_TIME)));
	return score;
}

int predictNextLocation(Location currentLocation, vector<Cluster> clusterList, vector<Location> locationList) {
	Cluster currentCluster = clusterList[currentLocation.parent_cluster_id];
	vector<double> scoreOfPlace;
	scoreOfPlace.resize(clusterList.size(), 0);
	for ( unsigned int i = 0; i < currentCluster.locationList.size(); i++ ) {
		int dest_cluster_id = (*currentCluster.locationList[i]).destination_cluster_id;
		if ( dest_cluster_id < 0 ) {
			continue;
		}
		double score = calDestinationScore(currentLocation, *currentCluster.locationList[i]);
		scoreOfPlace[dest_cluster_id] += score;
	}
	double maxScore = 0;
	int clusterID = -1;
	for ( unsigned int i = 0; i < scoreOfPlace.size(); i++ ) {
		//out << "score cluster " << i << ": " << scoreOfPlace[i] << "\n";
		if ( scoreOfPlace[i] > maxScore ) {
			maxScore = scoreOfPlace[i];
			clusterID = i;
		}
	}
	return clusterID;
}
//
//void addSingleLocation(Location newLocation, vector<Location> &location_list, vector<Cluster> &cluster_list) {
//	for ( unsigned int i = 0; i < location_list.size(); i++ ) {
//		if ( isInCluster(newLocation, location_list[i]) ) {
//			int clusterNum = location_list[i].parent_cluster_id;
//			int clusterSize = cluster_list[clusterNum].location_list.size();
//			newLocation.parent_cluster_id = clusterNum;
//			cluster_list[clusterNum].lat = (cluster_list[clusterNum].lat * clusterSize + newLocation.lat) / (clusterSize + 1);
//			cluster_list[clusterNum].lng = (cluster_list[clusterNum].lng * clusterSize + newLocation.lng) / (clusterSize + 1);
//			location_list.push_back(newLocation);
//			cluster_list[clusterNum].location_list.push_back(&location_list[location_list.size() - 1]);
//			break;
//		}
//	}
//}
//
//void addLocationList(vector<Location> newLocationList, vector<Location> &location_list, vector<Cluster> &cluster_list) {
//	for ( unsigned int i = 0; i < newLocationList.size(); i++ ) {
//
//	}
//}

void reExecute(vector<Location> & location_list, vector<Cluster> & cluster_list, double & average, double & variance, double & place_threshold) {
	clustering(location_list, cluster_list);
	calClusterDistribution(cluster_list, average, variance, place_threshold);
	calClusterLocation(cluster_list);
	classifyCluster(cluster_list, place_threshold);
	generateDestinationRef(location_list);
	out << "\n-\n";
}
void reExecuteWithPrint(vector<Location> & location_list, vector<Cluster> & cluster_list, double & average, double & variance, double & place_threshold) {
	clustering(location_list, cluster_list);
	printCluster(cluster_list);
	calClusterDistribution(cluster_list, average, variance, place_threshold);
	calClusterLocation(cluster_list);
	classifyCluster(cluster_list, place_threshold);
	generateDestinationRef(location_list);
	printDestination(cluster_list, location_list);
	out << "\n-\n";
}

int addTestList(vector<Location> & location_list, vector<Cluster> & cluster_list, double & average, double & variance, double & place_threshold, Location predicted_location, vector<Location> test_location_list) {
	int count_correct = 0;
	for ( unsigned int i = 0; i < test_location_list.size() - 1; i++ ) {
		location_list.push_back(test_location_list[i]);
		reExecute(location_list, cluster_list, average, variance, place_threshold);
		out << "predicted cluster: " << predicted_location.parent_cluster_id << " \t new location cluster: " << location_list[location_list.size() - 1].parent_cluster_id << "\n";
		if ( predicted_location.parent_cluster_id == location_list[location_list.size() - 1].parent_cluster_id ) {
			count_correct++;
		}
		cout << "test number " << i << "from" << test_location_list.size() << "\n";
	}
	return count_correct;
}

int main() {
	vector<Location> location_list;
	vector<Cluster> cluster_list;
	vector<Location> predicted_location_list;
	Location predicted_location;

	double average = 0;
	double variance = 0;
	double place_threshold = 0;

	//fileLocation.open(FILE_NAME);
	fileLocation.open(FILE_TRAIN);

	/// import new user location
	ReadFileLocation(fileLocation, location_list);
	reExecuteWithPrint(location_list, cluster_list, average, variance, place_threshold);


	/// predict next location
	int predicted_next_cluster_id = predictNextLocation(location_list[location_list.size() - 1], cluster_list, location_list);
	predicted_location = Location(cluster_list[predicted_next_cluster_id].lat, cluster_list[predicted_next_cluster_id].lng, location_list[location_list.size() - 1].time + 1, location_id_counter++);
	predicted_location.parent_cluster_id = predicted_next_cluster_id;


	///// test by train
	//int testNum = 54;
	//int countCorrect = 0;
	//for ( unsigned int i = 0; i < location_list.size()-1; i++ ) {
	//	Location test = location_list[i];
	//	int predicted_cluster_id = predictNextLocation(test, cluster_list, location_list);
	//	if ( predicted_cluster_id == location_list[i].destination_cluster_id ) {
	//		countCorrect++;
	//	}
	//	//cout << "destination cluster id: " << predicted_cluster_id << "\n";
	//}
	//out<<"accuracy" << 1.0*countCorrect/location_list.size()<<"\n";

	/// test by increasing
	int countCorrect = 0;
	fileTest.open(FILE_TEST);
	vector<Location> test_location_list;
	ReadFileLocation(fileTest, test_location_list);
	countCorrect = addTestList(location_list, cluster_list, average, variance, place_threshold, predicted_location, test_location_list);
	out << "accuracy: " << 100.0*countCorrect / test_location_list.size() << "\n";
	out.close();
}