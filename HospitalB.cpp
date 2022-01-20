#include "Hospital.h"

int main(int argc, char *argv[]){
	// socket variables
    int sockfd, sd, rv;
    socklen_t socklen;
    struct sockaddr_in scheduler, fromplace, hospitalB;
    struct hostent *hp;
    char recv_buff[BUFFER_SIZE];
	char send_buff[BUFFER_SIZE];

	// hospital A variables
    struct bootUpMessage bootB;
    struct hospitalSituation hsB;
	int locationB, capacityB, occupancyB;
    int cLocation;

    float score;
    float availability;
    float distance;

    //graph variables
    float loc_graph[MAX_LOC][MAX_LOC];
    int countLoc;
    map <int, int> index_map;
    // vector <int> dis(countLoc);
    
 
    if (argc != 4){
        cout << "Format: Location B, total Capacity B, initial occupancy B";
        exit(1);
    }

	locationB = atoi(argv[1]);
	capacityB = atoi(argv[2]);
	occupancyB = atoi(argv[3]);

    if (capacityB < occupancyB){
        cout << "Parameters illgal." << endl;
        exit(1);
    }

    map_process(loc_graph, countLoc, index_map);
    
    float dis[countLoc];

    shortest_distance(loc_graph, index_map[locationB], countLoc, dis);

    cout << "Hospital B is up and running UDP on port " << SOURCE_PORT_B << "." << endl;
    cout << "Hospital B has total capacity " << capacityB << " and initial occupancy " << occupancyB << "." <<endl; 

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0){
        perror("hospitalB: socket");
    }


    hp = gethostbyname(HOST);
    if (hp == 0){
        perror("Unknown host");
    }

    // destination 
    bcopy((char *)hp->h_addr, (char *)&scheduler.sin_addr, hp->h_length);
    scheduler.sin_family = AF_INET;
    scheduler.sin_port = htons(DEST_PORT);
    socklen = sizeof(struct sockaddr_in);

    // source
    bcopy((char *)hp->h_addr, (char *)&hospitalB.sin_addr, hp->h_length);
    bzero(&hospitalB,sizeof(hospitalB));
    hospitalB.sin_family = AF_INET;
    hospitalB.sin_addr.s_addr = INADDR_ANY;
    hospitalB.sin_port = htons(SOURCE_PORT_B);

    if (bind(sockfd,(struct sockaddr *)&hospitalB,socklen) < 0)
    {
        perror("HospitalB binding");
    }

    // send bootup information
    bootB.capacity = capacityB;
	bootB.occupancy = occupancyB;

    // cout << bootB.capacity << endl;
    // cout << bootB.occupancy << endl;

	memset(send_buff, 0, sizeof(send_buff));
	memcpy(send_buff, &bootB, sizeof(bootB));

    sd = sendto(sockfd, send_buff, sizeof(send_buff), 0, (struct sockaddr *)&scheduler, socklen);
    if (sd < 0){
        perror("first sendto");
    }

    bool cont = true;

    while (cont){
        // receive client's location
        rv = recvfrom(sockfd, recv_buff, sizeof(recv_buff), 0, (struct sockaddr *)&fromplace, &socklen);

        if(rv < 0){
            perror("recvfrom");
        }
        cout << "Hospital B has received input from client at location " << recv_buff << "." << endl;

        cLocation = atoi(recv_buff);


        // calculate distance value or None
        if (index_map.find(cLocation) != index_map.end()){
            if (cLocation != locationB){
                distance = dis[index_map[cLocation]];
                cout << "Hospital B has found the shortest path to client, distance = " << distance << "." << endl;
            }else{
                distance = 0;
                cout << "Client location is the same as hospital." << endl; 
            }
        } else{
            distance = INF;
            cout << "Hospital B does not have the location " << cLocation << " in map." << endl;
        }

        // calculate availability value or None
        float temp_avai = (1.0 * (capacityB - occupancyB)) / capacityB;

        if (temp_avai >= 0 && temp_avai <= 1){
            availability = temp_avai;
            cout << "Hospital B has capacity = " << capacityB << ", occupation = " << occupancyB << ", availability = " << availability << "." << endl;
        } else {
            availability = -1;
        }
        
        if (!(distance == INF || distance == 0 || availability == -1)){
            score = 1 / (distance * (1.1 - availability));
            cout << "Hospital B has the score = " << score << "." << endl;
        } else {
            score = -1;
            cout << "Hospital B has the score = None." << endl;
        }

        hsB.distance = distance;
        hsB.score = score;


        // hospital sends distance and score
        memset(send_buff, 0, sizeof(send_buff));
        memcpy(send_buff, &hsB, sizeof(hsB));

        sd = sendto(sockfd, send_buff, sizeof(send_buff), 0, (struct sockaddr *)&scheduler, socklen);
        if (sd < 0){
            perror("second sendto");
        }

        if (index_map.find(cLocation) == index_map.end()){
            cout << "Hospital B has sent \"location not found\" to the Scheduler." << endl; 
        }
        if (score != -1 && distance != INF && distance != 0){ // normal values
            cout << "Hospital B has sent score = " << score << " and distance = " << distance << " to the Scheduler." << endl;
        } else if (score == -1){ // score value illgal
            if (distance != INF && distance != 0){
                cout << "Hospital B has sent score = None and distance = " << distance << " to the Scheduler." << endl;
            } else {
                cout << "Hospital B has sent score = None and distance = None to the Scheduler." << endl;
            }
        } else if (distance == INF || distance == 0) { // distance value illgal
            if (score != -1){
                cout << "Hospital B has sent score = " << score << " and distance = None to the Scheduler." << endl;
            } else {
                cout << "Hospital B has sent score = None and distance = None to the Scheduler." << endl;
            }
        }

        memset(recv_buff, 0, sizeof(recv_buff));


        // receive assignment 
        rv = recvfrom(sockfd, recv_buff, sizeof(recv_buff), 0, (struct sockaddr *)&fromplace, &socklen);
        if (rv < 0){
            perror("Receive error");
        }

        if (strcmp(recv_buff, "B") == 0){
            occupancyB++;
            availability = (1.0 * (capacityB - occupancyB)) / capacityB;
            cout << "Hospital B has been assigned to a client, occupation is updated to " << occupancyB;
            cout << ", availability is updated to ​" << availability << "." << endl;
        }
    }


    return 0;
}

void map_process(float loc_graph[MAX_LOC][MAX_LOC], int &countLoc, map<int, int>& indexMap) {
	vector <int> v;
	int row, col;
	float weight;
	ifstream map_file;

	map_file.open(FILE_PATH, ios::in);
    if(!map_file.is_open()){
        cout << "Open file failure" << endl;
	}

	// eliminate duplicates
	while (!map_file.eof()){
		map_file >> row >> col >> weight;
		v.push_back(row);
		v.push_back(col);
	}
	sort(v.begin(), v.end());
	v.erase(unique(v.begin(), v.end()), v.end());
    countLoc = v.size();
	map_file.close();

	// map index
	for(int i = 0; i < countLoc; i++){
		indexMap[v[i]] = i;
	}

    for (int i = 0; i < MAX_LOC; i++){
        for (int j = 0; j < MAX_LOC; j++)
        {
            if (i == j)
            {
                loc_graph[i][j] = 0;
            }
            else
            {
                loc_graph[i][j] = INF;
            }
        }
    }

	// fill graph
	map_file.open(FILE_PATH, ios::in);
	if(!map_file.is_open()){
        cout << "Open file failure" << endl;
	}
	while (!map_file.eof()){
		map_file >> row >> col >> weight;
		loc_graph[indexMap[row]][indexMap[col]] = weight;
		loc_graph[indexMap[col]][indexMap[row]] = weight;
	}

}

void shortest_distance(float loc_graph[MAX_LOC][MAX_LOC], int source, int countLoc, float dis[]){
    bool vis[countLoc];

    fill(dis, dis+countLoc, INF);
    dis[source] = 0;

    for (int i = 0; i < countLoc; i++){
        int u = -1; 
        int min_dis = INF;
        for (int j = 0; j < countLoc; j++){
            if (vis[j] == false && dis[j] < min_dis){
                u = j;
                min_dis = dis[j];
            }
        }
        if (u == -1){
            return;
        }
        vis[u] = true;
        for(int v = 0; v < countLoc; v++){
            if (vis[v] == false && dis[u] + loc_graph[u][v] < dis[v]){
                dis[v] = dis[u] + loc_graph[u][v];
            }
        }
    }
}