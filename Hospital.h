#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <map>
#include <vector> 

#define DEST_PORT 33761
#define SOURCE_PORT_A 30761
#define SOURCE_PORT_B 31761
#define SOURCE_PORT_C 32761
#define HOST "127.0.0.1"
#define MAX_LOC 100
#define BUFFER_SIZE 1024
#define FILE_PATH "./map/map_hard.txt"

const float INF = 1000000000;

using namespace std;

struct bootUpMessage {
	int capacity;
	int occupancy;
};

struct hospitalSituation {
	float score;
	float distance;
};

void map_process(float loc_graph[MAX_LOC][MAX_LOC], int &countLoc, map<int, int>& indexMap);
void shortest_distance(float loc_graph[MAX_LOC][MAX_LOC], int source, int countLoc, float dis[]);
