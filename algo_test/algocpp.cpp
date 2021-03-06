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

/// from paper
#define VARIANCE_LAT 0.00002017
#define VARIANCE_LNG 0.00003082
#define VARIANCE_TIME 4.0
#define K_TIME 0.0004
#define DIST_THRESHOLD 0.0025

///
#define PI 3.141592653589793238462643383279502884
#define TIME_DESTINATION_THRESHOLD 24

/// for predicting 24h
#define MAX_PREDICTING_TIME 24 /// from requirement
#define PREDICTING_TIME_INTERVAL 1.0 /// form requirement
#define TIME_24H_DESTINATION_THRESHOLD 0.5
#define TIME_COMPARE_INTERVAL 1.0

/// file "1", "2", "3", "3-1", "3-2", "3-3"
string file_num = "2";
string FILE_NAME = "location-" + file_num + ".txt";
string FILE_TRAIN = "location-" + file_num + "-train.txt";
string FILE_TEST = "location-" + file_num + "-test.txt";
string OUTPUT_FILE_NAME = "out-location-" + file_num + ".txt";

int location_id_counter = 0;
int cluster_id_counter = 0;

fstream fileLocation;
fstream file_test;
ofstream out(OUTPUT_FILE_NAME);

///-------------------- read file part
void ReadFileLocation(fstream & file_location, vector<Location> & location_list) {
	string temp;
	while ( !file_location.eof() ) {
		getline(file_location, temp);
		stringstream ss(temp);
		double lat, lng, timeH, timeM;
		ss >> timeH;
		ss >> timeM;
		ss >> lat;
		ss >> lng;

		int id = location_id_counter++;
		location_list.push_back(Location(lat, lng, timeH + timeM / 60, id));
	}
}

///-------------------- clustering and define destination part
bool isClosedLocation(Location sLocation, Location location2) {
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
	vector<int> location_status;
	location_status.resize(location_list.size(), 0);
	vector<Cluster> new_cluster_list;
	cluster_id_counter = 0;
	location_id_counter = 0;
	for ( unsigned int i = 0; i < location_list.size(); i++ ) {
		if ( location_status[i] != 0 ) {
			continue;
		}
		Location temp = location_list[i];
		location_status[i] = 1;
		location_list[i].parent_cluster_id = cluster_id_counter;
		Cluster cluster = Cluster(&location_list[i]);
		queue<Location> QLocation;
		QLocation.push(temp);
		while ( !QLocation.empty() ) {
			//out << "q execute\n";
			Location sLocation = QLocation.front();
			QLocation.pop();
			for ( unsigned int j = 0; j < location_list.size(); j++ ) {
				if ( location_status[j] != 0 ) {
					continue;
				}
				if ( isClosedLocation(sLocation, location_list[j]) ) {
					QLocation.push(location_list[j]);
					location_status[j] = 1;
					location_list[j].parent_cluster_id = cluster_id_counter;
					cluster.location_list.push_back(&location_list[j]);
				}
			}
		}
		cluster.cluster_id = cluster_id_counter++;
		new_cluster_list.push_back(cluster);
	}
	cluster_list = new_cluster_list;
}

//void printCluster(vector<Cluster> cluster_list) {
//	out << "number of cluster: " << cluster_list.size() << "\n";
//	for ( unsigned int i = 0; i < cluster_list.size(); i++ ) {
//		out << "cluster" << i << ": " << cluster_list[i].location_list.size() << "- ";
//		for ( unsigned int j = 0; j < cluster_list[i].location_list.size(); j++ ) {
//			out << (*cluster_list[i].location_list[j]).id << " ";
//		}
//		out << "\n";
//	}
//}

void calClusterLocation(vector<Cluster> & cluster_list) {
	for ( unsigned int i = 0; i < cluster_list.size(); i++ ) {
		double lat = 0, lng = 0;
		for ( unsigned int j = 0; j < cluster_list[i].location_list.size(); j++ ) {
			lat += (*cluster_list[i].location_list[j]).lat;
			lng += (*cluster_list[i].location_list[j]).lng;
		}
		lat /= cluster_list[i].location_list.size();
		lng /= cluster_list[i].location_list.size();
		cluster_list[i].lat = lat;
		cluster_list[i].lng = lng;
	}
}

void generateDestinationRef(vector<Location> & location_list) {
	for ( unsigned int i = 0; i < location_list.size() - 1; i++ ) {
		if ( abs(location_list[i].time - location_list[i + 1].time) <= TIME_DESTINATION_THRESHOLD )
			location_list[i].destination_cluster_id = location_list[i + 1].parent_cluster_id;
	}
}

// void printDestination(vector<Cluster> cluster_list, vector<Location> location_list) {
// 	for ( unsigned int i = 0; i < location_list.size(); i++ ) {
// 		if ( location_list[i].parent_cluster_id != location_list[i].destination_cluster_id )
// 			out << "location " << i << ": " << location_list[i].parent_cluster_id << "   \t->\t" << location_list[i].destination_cluster_id << "\n";
// 	}
// }

///-------------------- predicting part
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

bool isClosedTime(double time1, double time2, double time_threshold) {
	double diff = abs(time1 - time2);
	if ( diff >= 12 )
		diff = 24 - diff;
	return diff <= time_threshold;
}

int predictNextLocation(Location current_location, vector<Cluster> cluster_list, vector<Location> location_list) {
	Cluster current_cluster = cluster_list[current_location.parent_cluster_id];
	vector<double> score_of_place;
	score_of_place.resize(cluster_list.size(), 0);
	for ( unsigned int i = 0; i < current_cluster.location_list.size(); i++ ) {
		int dest_cluster_id = (*current_cluster.location_list[i]).destination_cluster_id;
		if ( dest_cluster_id < 0 ) {
			continue;
		}
		double score = calDestinationScore(current_location, *current_cluster.location_list[i]);
		score_of_place[dest_cluster_id] += score;
	}
	double maxScore = 0;
	int clusterID = -1;
	for ( unsigned int i = 0; i < score_of_place.size(); i++ ) {
		//out << "score cluster " << i << ": " << scoreOfPlace[i] << "\n";
		if ( score_of_place[i] > maxScore ) {
			maxScore = score_of_place[i];
			clusterID = i;
		}
	}
	return clusterID;
}

void predictNext24hLocation(Location current_location, vector<Cluster> cluster_list, vector<Location> location_list, vector<Location> & predicted_location_list) {
	vector<Location> newList;
	predicted_location_list = newList;
	int cluster_num = predictNextLocation(current_location, cluster_list, location_list);
	int time_count = PREDICTING_TIME_INTERVAL;
	double time_temp = PREDICTING_TIME_INTERVAL;
	while ( time_count <= 24 ) {
		if ( cluster_num == -1 ) {
			break;
		}
		bool is_possible_time = false;
		double predicted_time = current_location.time + time_temp;
		for ( unsigned int i = 0; i < cluster_list[cluster_num].location_list.size(); i++ ) {
			if ( isClosedTime(predicted_time, (*cluster_list[cluster_num].location_list[i]).time, TIME_24H_DESTINATION_THRESHOLD) ) {
				is_possible_time = true;
				break;
			}
		}
		if ( is_possible_time ) {
			/// create new predicted_location and use it as current_location for next predicting
			current_location = Location(cluster_list[cluster_num].lat, cluster_list[cluster_num].lng, predicted_time, location_id_counter++);
			current_location.parent_cluster_id = cluster_num;
			predicted_location_list.push_back(current_location);
			/// calculate next predicted location
			cluster_num = predictNextLocation(current_location, cluster_list, location_list);
			time_count = time_count + 2;
			/// reset time_temp for next prediction
			time_temp = 2;
		}
		else {
			time_count = time_count + 2;
			time_temp = time_temp + 2;
		}
	}
}

///-------------------- add new location and evaluating the accuracy part
void reExecute(vector<Location> & location_list, vector<Cluster> & cluster_list) {
	clustering(location_list, cluster_list);
	calClusterLocation(cluster_list);
	//printCluster(cluster_list);
	generateDestinationRef(location_list);
	//out << "\n-\n";
}

//void printPredictedLocationList(vector<Location> predicted_location_list) {
//	for ( unsigned int i = 0; i < predicted_location_list.size(); i++ ) {
//		out << "time: " << predicted_location_list[i].time << " cluster: " << predicted_location_list[i].parent_cluster_id << "\n";
//	}
//	out << "\n---\n";
//}

void addTestList(vector<Location> & location_list, vector<Cluster> & cluster_list, vector<Location> input_location_list, vector<Location> & predicted_location_list) {
	int count_correct = 0;
	int cannot_predicted = 0;
	int cannot_compare_result = 0;
	int count_hourly_number = 0;
	for ( unsigned int i = 0; i < input_location_list.size(); i++ ) {
		std::cout << "test number " << i + 1 << " from " << input_location_list.size() << "\n";
		location_list.push_back(input_location_list[i]);
		reExecute(location_list, cluster_list);
		Location new_input = location_list[location_list.size() - 1];
		/// predict 24h next location
		if ( predicted_location_list.size() == 0 )
			cannot_predicted++;
		else {
			bool comparable = false;
			count_hourly_number += predicted_location_list.size();
			for ( unsigned int j = 0; j < predicted_location_list.size(); j++ ) {
				if ( isClosedTime(new_input.time, predicted_location_list[j].time, TIME_COMPARE_INTERVAL) ) {
					comparable = true;
					if ( predicted_location_list[j].parent_cluster_id == new_input.parent_cluster_id ) {
						comparable = true;
						count_correct++;
						break;
					}
					else {
						/// not correct
					}
				}
				else {
					/// not match time, continue to next predicted location
				}
			}
			if ( !comparable ) {
				cannot_compare_result++;
			}
		}

		predictNext24hLocation(new_input, cluster_list, location_list, predicted_location_list);
		//printPredictedLocationList(predicted_location_list);
	}

	/// print result to out file
	//out << "total number of prediction result: " << count_hourly_number << "\n";
	int can_predict = input_location_list.size() - cannot_predicted;
	int can_compare = can_predict - cannot_compare_result;
	out << "cannot predict location: " << cannot_predicted << "/" << input_location_list.size() << "\n";
	out << "cannot compared result: " << cannot_compare_result << "/" << can_predict << "\n";

	out << "accuracy: " << 100.0 * count_correct / can_compare << " ( " << count_correct << "/" << can_compare << " compare with comparable result )\n";
	out << "accuracy: " << 100.0 * count_correct / can_predict << " ( " << count_correct << "/" << can_predict << " compare with predictable result )\n";
}

int main() {
	vector<Location> location_list;
	vector<Cluster> cluster_list;
	vector<Location> predicted_location_list;
	Location predicted_location;

	fileLocation.open(FILE_TRAIN);
	//fileLocation.open(FILE_NAME);

	/// import new user location
	ReadFileLocation(fileLocation, location_list);
	reExecute(location_list, cluster_list);
	//printCluster(cluster_list);


	/// predict next location
	predictNext24hLocation(location_list[location_list.size() - 1], cluster_list, location_list, predicted_location_list);


	///// test by training data set
	//int testNum = 0;
	//int countCorrect = 0;
	//for ( unsigned int i = 0; i < location_list.size(); i++ ) {
	//	Location test = location_list[i];
	//	predictNext24hLocation(location_list[i], cluster_list, location_list, predicted_location_list);
	//	out << "location_number: " << i << " lat-" << location_list[i].lat << " lng-" << location_list[i].lng << " time-" << location_list[i].time << "\n";
	//	printPredictedLocationList(predicted_location_list);
	//	//cout << "destination cluster id: " << predicted_cluster_id << "\n";
	//}
	//out << "accuracy" << 1.0*countCorrect / location_list.size() << "\n";

	/// test by increasing
	file_test.open(FILE_TEST);
	vector<Location> test_location_list;
	ReadFileLocation(file_test, test_location_list);
	addTestList(location_list, cluster_list, test_location_list, predicted_location_list);

	out.close();
}