#include "Hospital.h"

int main(int argc, char *argv[]){
	// socket variables
    int sockfd, sd, rv;
    socklen_t socklen;
    struct sockaddr_in scheduler, fromplace, hospitalC;
    struct hostent *hp;
    char recv_buff[BUFFER_SIZE];
	char send_buff[BUFFER_SIZE];

	// hospital A variables
    struct bootUpMessage bootC;
    struct hospitalSituation hsC;
	int locationC, capacityC, occupancyC;
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
        cout << "Format: Location C, total Capacity C, initial occupancy C";
        exit(1);
    }

	locationC = atoi(argv[1]);
	capacityC = atoi(argv[2]);
	occupancyC = atoi(argv[3]);

    if (capacityC < occupancyC){
        cout << "Parameters illgal." << endl;
        exit(1);
    }

    map_process(loc_graph, countLoc, index_map);
    
    float dis[countLoc];

    shortest_distance(loc_graph, index_map[locationC], countLoc, dis);

    cout << "Hospital C is up and running UDP on port " << SOURCE_PORT_C << "." << endl;
    cout << "Hospital C has total capacity " << capacityC << " and initial occupancy " << occupancyC << "." <<endl; 

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0){
        perror("hospitalC: socket");
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
    bcopy((char *)hp->h_addr, (char *)&hospitalC.sin_addr, hp->h_length);
    bzero(&hospitalC,sizeof(hospitalC));
    hospitalC.sin_family = AF_INET;
    hospitalC.sin_addr.s_addr = INADDR_ANY;
    hospitalC.sin_port = htons(SOURCE_PORT_C);

    if (bind(sockfd,(struct sockaddr *)&hospitalC,socklen) < 0)
    {
        perror("HospitalC binding");
    }

    // send bootup information to scheduler
    bootC.capacity = capacityC;
	bootC.occupancy = occupancyC;

	memset(send_buff, 0, sizeof(send_buff));
	memcpy(send_buff, &bootC, sizeof(bootC));

    sd = sendto(sockfd, send_buff, sizeof(send_buff), 0, (struct sockaddr *)&scheduler, socklen);
    if (sd < 0){
        perror("first sendto");
    }

    bool cont = true;

    while (cont){
        // receive client's location until press control c
        rv = recvfrom(sockfd, recv_buff, sizeof(recv_buff), 0, (struct sockaddr *)&fromplace, &socklen);
        if(rv < 0){
            perror("recvfrom");
        }
        cout << "Hospital C has received input from client at location " << recv_buff << "." << endl;

        cLocation = atoi(recv_buff); // get client's location


        // calculate distance value or None
        if (index_map.find(cLocation) != index_map.end()){ // found client's location on the map
            if (cLocation != locationC){ // client's location doesn't overlap with hospital
                distance = dis[index_map[cLocation]]; // get distance 
                cout << "Hospital C has found the shortest path to client, distance = " << distance << "." << endl;
            }else{ 
                distance = 0; // indicate overlapping location
                cout << "Client location is the same as hospital." << endl; 
            }
        } else{ // client's location not found
            distance = INF; // indicate location not found
            cout << "Hospital C does not have the location " << cLocation << " in map." << endl;
        }

        // calculate availability value or None
        float temp_avai = (1.0 * (capacityC - occupancyC)) / capacityC;

        if (temp_avai >= 0 && temp_avai <= 1){ // legal availability value 
            availability = temp_avai;
            cout << "Hospital C has capacity = " << capacityC << ", occupation = " << occupancyC << ", availability = " << availability << "." << endl;
        } else {
            availability = -1; // indicate illegal availability value
        }
        
        if (!(distance == INF || distance == 0 || availability == -1)){ // legal values
            score = 1 / (distance * (1.1 - availability)); 
            cout << "Hospital C has the score = " << score << "." << endl;
        } else {
            score = -1; // indicate illegal score value
            cout << "Hospital C has the score = None." << endl;
        }

        // wrap the values
        hsC.distance = distance;
        hsC.score = score; 


        // hospital sends distance and score
        memset(send_buff, 0, sizeof(send_buff));
        memcpy(send_buff, &hsC, sizeof(hsC));

        sd = sendto(sockfd, send_buff, sizeof(send_buff), 0, (struct sockaddr *)&scheduler, socklen);
        if (sd < 0){
            perror("second sendto");
        }

        if (index_map.find(cLocation) == index_map.end()){
            cout << "Hospital C has sent \"location not found\" to the Scheduler." << endl; 
        }

        if (score != -1 && distance != INF && distance != 0){
            cout << "Hospital C has sent score = " << score << " and distance = " << distance << " to the Scheduler." << endl;
        } else if (score == -1){
            if (distance != INF && distance != 0){
                cout << "Hospital C has sent score = None and distance = " << distance << " to the Scheduler." << endl;
            } else {
                cout << "Hospital C has sent score = None and distance = None to the Scheduler." << endl;
            }
        } else if (distance == INF || distance == 0) {
            if (score != -1){
                cout << "Hospital C has sent score = " << score << " and distance = None to the Scheduler." << endl;
            } else {
                cout << "Hospital C has sent score = None and distance = None to the Scheduler." << endl;
            }
        }

        // clear recv_buff
        memset(recv_buff, 0, sizeof(recv_buff));


        // receive assignment 
        rv = recvfrom(sockfd, recv_buff, sizeof(recv_buff), 0, (struct sockaddr *)&fromplace, &socklen);
        if (rv < 0){
            perror("Receive error");
        }

        if (strcmp(recv_buff, "C") == 0){
            occupancyC++;
            availability = (1.0 * (capacityC - occupancyC)) / capacityC;
            cout << "Hospital C has been assigned to a client, occupation is updated to " << occupancyC;
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

	// fill the graph
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