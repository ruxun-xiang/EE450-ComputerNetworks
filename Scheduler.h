#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <netdb.h>
#include <iostream>

#define LOCAL_PORT_U 33761
#define LOCAL_PORT_T 34761
#define HOST "127.0.0.1"

const float INF = 1000000000;

using namespace std;

struct hBootUpMessage {
	int hCapacity;
	int hOccupancy;
};

struct hospitalSituation {
	float score;
	float distance;
};

float getMax(float array[],int count);
void onScreenBootup (const char *hospital, int hCapacity, int hOccupancy);
void onScreenMapInfo (const char *hospital, float score, float distance);
void onScreenResultSending(const char * decision, struct sockaddr_in sockaddr);
const char * compareTwoScore(const char *h1, const char *h2, struct hospitalSituation hSit1, struct hospitalSituation hSit2);
const char * compareTwoDistance(const char *h1, const char *h2, struct hospitalSituation hSit1, struct hospitalSituation hSit2);
const char * decisionMaking (struct hospitalSituation hSitA, struct hospitalSituation hSitB, struct hospitalSituation hSitC);