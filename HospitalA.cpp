#include "Hospital.h"

int main(int argc, char *argv[]){
	// socket variables
    int sockfd, sd, rv;
    socklen_t socklen;
    struct sockaddr_in scheduler, fromplace, hospitalA;
    struct hostent *hp;
    char recv_buff[BUFFER_SIZE];
	char send_buff[BUFFER_SIZE];

	// hospital A variables
    struct bootUpMessage bootA;
    struct hospitalSituation hsA;
	int locationA, capacityA, occupancyA;
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
        cout << "Format: Location A, total Capacity A, initial occupancy A";
        exit(1);
    }

	locationA = atoi(argv[1]);
	capacityA = atoi(argv[2]);
	occupancyA = atoi(argv[3]);

    if (capacityA < occupancyA){
        cout << "Parameters illgal." << endl;
        exit(1);
    }

    map_process(loc_graph, countLoc, index_map);// read map file, record the map into a graph
    
    float dis[countLoc]; // use to record the shortest distance

    shortest_distance(loc_graph, index_map[locationA], countLoc, dis);

    cout << "Hospital A is up and running UDP on port " << SOURCE_PORT_A << "." << endl;
    cout << "Hospital A has total capacity " << capacityA << " and initial occupancy " << occupancyA << "." <<endl; 

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0){
        perror("hospitalA: socket");
    }


    hp = gethostbyname(HOST);
    if (hp == 0){
        perror("Unknown host");
    }

    // destination parameter setting
    bcopy((char *)hp->h_addr, (char *)&scheduler.sin_addr, hp->h_length);
    scheduler.sin_family = AF_INET;
    scheduler.sin_port = htons(DEST_PORT);
    socklen = sizeof(struct sockaddr_in);

    // source parameter setting
    bcopy((char *)hp->h_addr, (char *)&hospitalA.sin_addr, hp->h_length);
    bzero(&hospitalA,sizeof(hospitalA));
    hospitalA.sin_family = AF_INET;
    hospitalA.sin_addr.s_addr = INADDR_ANY;
    hospitalA.sin_port = htons(SOURCE_PORT_A);

    if (bind(sockfd,(struct sockaddr *)&hospitalA,socklen) < 0)
    {
        perror("HospitalA binding");
    }

    // send bootup information
    bootA.capacity = capacityA;
	bootA.occupancy = occupancyA;

	memset(send_buff, 0, sizeof(send_buff));
	memcpy(send_buff, &bootA, sizeof(bootA));

    sd = sendto(sockfd, send_buff, sizeof(send_buff), 0, (struct sockaddr *)&scheduler, socklen);
    if (sd < 0){
        perror("first sendto");
    }

    bool cont = true;

    while(cont){
        // receive client's location
        rv = recvfrom(sockfd, recv_buff, sizeof(recv_buff), 0, (struct sockaddr *)&fromplace, &socklen);

        if(rv < 0){
            perror("recvfrom");
        }
        cout << "Hospital A has received input from client at location " << recv_buff << "." << endl;

        cLocation = atoi(recv_buff);


        // calculate distance value or None
        if (index_map.find(cLocation) != index_map.end()){
            if (cLocation != locationA){
                distance = dis[index_map[cLocation]];
                cout << "Hospital A has found the shortest path to client, distance = " << distance << "." << endl;
            }else{
                distance = 0;
                cout << "Client location is the same as hospital." << endl; 
            }
        } else{
            distance = INF;
            cout << "Hospital A does not have the location " << cLocation << " in map." << endl;
        }

        // calculate availability value or None
        float temp_avai = (1.0 * (capacityA - occupancyA)) / capacityA;

        if (temp_avai >= 0 && temp_avai <= 1){
            availability = temp_avai;
            cout << "Hospital A has capacity = " << capacityA << ", occupation = " << occupancyA << ", availability = " << availability << "." << endl;
        } else {
            availability = -1;
        }
        
        if (!(distance == INF || distance == 0 || availability == -1)){ // legal value
            score = 1 / (distance * (1.1 - availability));
            cout << "Hospital A has the score = " << score << "." << endl;
        } else {
            score = -1;
            cout << "Hospital A has the score = None." << endl;
        }

        hsA.distance = distance;
        hsA.score = score;


        // hospital sends distance and score
        memset(send_buff, 0, sizeof(send_buff));
        memcpy(send_buff, &hsA, sizeof(hsA));

        sd = sendto(sockfd, send_buff, sizeof(send_buff), 0, (struct sockaddr *)&scheduler, socklen);
        if (sd < 0){
            perror("second sendto");
        }

        if (index_map.find(cLocation) == index_map.end()){ // client's location not in the index map
            cout << "Hospital A has sent \"location not found\" to the Scheduler." << endl; 
        }

        if (score != -1 && distance != INF && distance != 0){ // legal values
            cout << "Hospital A has sent score = " << score << " and distance = " << distance << " to the Scheduler." << endl;
        } else if (score == -1){ // score == None
            if (distance != INF && distance != 0){ 
                cout << "Hospital A has sent score = None and distance = " << distance << " to the Scheduler." << endl;
            } else {
                cout << "Hospital A has sent score = None and distance = None to the Scheduler." << endl;
            }

        } else if (distance == INF || distance == 0) {
            if (score != -1){
                cout << "Hospital A has sent score = " << score << " and distance = None to the Scheduler." << endl;
            } else {
                cout << "Hospital A has sent score = None and distance = None to the Scheduler." << endl;
            }
        }

        memset(recv_buff, 0, sizeof(recv_buff));


        // receive assignment from scheduler
        rv = recvfrom(sockfd, recv_buff, sizeof(recv_buff), 0, (struct sockaddr *)&fromplace, &socklen);
        if (rv < 0){
            perror("Receive error");
        }


        if (strcmp(recv_buff, "A") == 0){
            occupancyA++;
            availability = (1.0 * (capacityA - occupancyA)) / capacityA; // update information
            cout << "Hospital A has been assigned to a client, occupation is updated to " << occupancyA;
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

    // read in map file
	map_file.open(FILE_PATH, ios::in);
    if(!map_file.is_open()){
        cout << "Open file failure" << endl;
	}


	while (!map_file.eof()){
		map_file >> row >> col >> weight;
		v.push_back(row);
		v.push_back(col);
	}
    // eliminate duplicates
	sort(v.begin(), v.end());
	v.erase(unique(v.begin(), v.end()), v.end());
    countLoc = v.size();
	map_file.close();

	// map location re-index
	for(int i = 0; i < countLoc; i++){
		indexMap[v[i]] = i;
	}

    // map initialization
    for (int i = 0; i < MAX_LOC; i++){
        for (int j = 0; j < MAX_LOC; j++){
            if (i == j){
                loc_graph[i][j] = 0;
            }else{
                loc_graph[i][j] = INF;
            }
        }
    }

	map_file.open(FILE_PATH, ios::in);
	if(!map_file.is_open()){
        cout << "Open file failure" << endl;
	}
    // fill the graph using the new location index
	while (!map_file.eof()){
		map_file >> row >> col >> weight;
		loc_graph[indexMap[row]][indexMap[col]] = weight;
		loc_graph[indexMap[col]][indexMap[row]] = weight;
	}

}

// implement Dijskra
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