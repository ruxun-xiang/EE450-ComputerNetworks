#include "Scheduler.h"

int main(){

    // UDP variables
    int sockfd_u, rv_u, sd_u;
    struct sockaddr_in scheduler;
    struct sockaddr_in fromhosp;
    struct sockaddr_in sockaddrA, sockaddrB, sockaddrC;
    socklen_t socklen, hosplen;
    char recv_buff_u[1024];
    char send_buff_u[100];
    int recv_port;

    struct hBootUpMessage hBootA, hBootB, hBootC;
    struct hospitalSituation hSitA, hSitB, hSitC;
    bool hospitalA = false;
    bool hospitalB = false;
    bool hospitalC = false;

    // TCP variables
    int sockfd_t, chdsockfd, sd_t, rv_t;
    struct sockaddr_in serv_addr, c_addr;
    char recv_buff_t[1024];
    char send_buff_t[1024];
    struct sockaddr_storage cli_addr;
    socklen_t cli_len, c_len;

    // char decision[20];
    const char * decision;

    // Scheduler connects to hospital
    sockfd_u = socket(AF_INET, SOCK_DGRAM,0);

    if (sockfd_u < 0){
        perror("scheduler: socket");
    }

    socklen = sizeof(scheduler);
    hosplen = sizeof(struct sockaddr_in);
    scheduler.sin_family = AF_INET;
    scheduler.sin_addr.s_addr = INADDR_ANY;
    scheduler.sin_port = htons(LOCAL_PORT_U);

    if (bind(sockfd_u, (struct sockaddr *)&scheduler, socklen) < 0){
        close(sockfd_u);
        perror("scheduler: bind");
    }

    cout << "The Scheduler is up and running." << endl;

    memset(recv_buff_u, 0, sizeof(recv_buff_u)); // clear recv_buff_u

    // receive hospitals' bootup information
    while (!(hospitalA && hospitalB && hospitalC)){
        rv_u = recvfrom(sockfd_u, recv_buff_u, 1024, 0, (struct sockaddr *)&fromhosp, &hosplen);
        if (rv_u < 0){
            perror("recvfrom");
            exit(1);
        }

        recv_port = ntohs(fromhosp.sin_port); // differentiate hospital by its port number
        if (recv_port == 30761){ // hospital A
            hospitalA = true;
            memcpy(&hBootA, recv_buff_u, sizeof(hBootUpMessage));
            sockaddrA = fromhosp;
            onScreenBootup("A", hBootA.hCapacity, hBootA.hOccupancy);
        } else if (recv_port == 31761){ // hospital B
            hospitalB = true;
            memcpy(&hBootB, recv_buff_u, sizeof(hBootUpMessage));
            sockaddrB = fromhosp;
            onScreenBootup("B", hBootB.hCapacity, hBootB.hOccupancy);
        } else if (recv_port == 32761){ // hospital C
            hospitalC = true;
            memcpy(&hBootC, recv_buff_u, sizeof(hBootUpMessage));
            sockaddrC = fromhosp;
            onScreenBootup("C", hBootC.hCapacity, hBootC.hOccupancy);
        } 


    }

    // scheduler listen to patient
    sockfd_t = socket(AF_INET, SOCK_STREAM, 0);
    
    if (sockfd_t < 0){
        perror ("Socket error");
    }

    bzero((char *)&serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(LOCAL_PORT_T);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd_t, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
        perror("Bind error");
    }

    listen(sockfd_t, 10);

    cli_len = sizeof(cli_addr);

    bool cont = true;

    while (cont){ // receive from client until press control c
        chdsockfd = accept(sockfd_t, (struct sockaddr *) &cli_addr, &cli_len);
        if (chdsockfd < 0){
            perror("Children socket error");
        }

        c_len = sizeof(c_addr);
        getpeername(chdsockfd, (struct sockaddr *)&c_addr, &c_len); // get client's address information

        bzero(recv_buff_t, 1024);

        rv_t = recv(chdsockfd, recv_buff_t, 1023, 0);

        if (rv_t < 0){
            perror("Receive error");
        }
        cout << "The scheduler has received client at location " << recv_buff_t << " from the client using TCP over port " << LOCAL_PORT_T << "." << endl; 



        // Scheduler queries hospital using client's location
        memset(send_buff_u, 0, sizeof(send_buff_u));
        memcpy(send_buff_u, recv_buff_t, sizeof(recv_buff_t));

        if (hBootA.hCapacity != hBootA.hOccupancy){
            sd_u = sendto(sockfd_u, send_buff_u, sizeof(send_buff_u), 0, (struct sockaddr *)&sockaddrA, hosplen);
            if (sd_u < 0){
                perror("sendtoA");
            }
            cout << "The Scheduler has sent client location to Hospital A using UDP over port " << ntohs(scheduler.sin_port) << "." << endl;
            hospitalA = false; // set flags for the next while loop
        }
        if (hBootB.hCapacity != hBootB.hOccupancy){
            sd_u = sendto(sockfd_u, send_buff_u, sizeof(send_buff_u), 0, (struct sockaddr *)&sockaddrB, hosplen);
            if (sd_u < 0){
                perror("sendtoB");
            }
            cout << "The Scheduler has sent client location to Hospital B using UDP over port " << ntohs(scheduler.sin_port) << "." << endl;
            hospitalB = false;
        }
        if (hBootC.hCapacity != hBootC.hOccupancy){
            sd_u = sendto(sockfd_u, send_buff_u, sizeof(send_buff_u), 0, (struct sockaddr *)&sockaddrC, hosplen);
            if (sd_u < 0){
                perror("sendtoC");
            }
            cout << "The Scheduler has sent client location to Hospital C using UDP over port " << ntohs(scheduler.sin_port) << "." << endl;
            hospitalC = false;
        }

        // hospital reply to scheduler score and distance
        memset(recv_buff_u, 0, sizeof(recv_buff_u));


        // initialize hospital situation values
        hSitA.score = -1;
        hSitA.distance = -1;
        hSitB.score = -1;
        hSitB.distance = -1;
        hSitC.score = -1;
        hSitC.distance = -1;


        while (!(hospitalA && hospitalB && hospitalC)){
            rv_u = recvfrom(sockfd_u, recv_buff_u, 1024, 0, (struct sockaddr *)&fromhosp, &hosplen);
            if (rv_u < 0){
                perror("recvfrom");
                exit(1);
            }

            recv_port = ntohs(fromhosp.sin_port);
            if (recv_port == 30761){ 
                hospitalA = true;
                memcpy(&hSitA, recv_buff_u, sizeof(hospitalSituation));
                onScreenMapInfo("A", hSitA.score, hSitA.distance); // print out on screen information
            } else if (recv_port == 31761){
                hospitalB = true;
                memcpy(&hSitB, recv_buff_u, sizeof(hospitalSituation));
                onScreenMapInfo("B", hSitB.score, hSitB.distance);
            } else if (recv_port == 32761){
                hospitalC = true;
                memcpy(&hSitC, recv_buff_u, sizeof(hospitalSituation));
                onScreenMapInfo("C", hSitC.score, hSitC.distance);
            } 
        }

        decision = decisionMaking(hSitA, hSitB, hSitC); // use the received information to make a decision 
        const char * scoreNone = "scoreNone";
        const char * noneExist = "noneExist";
 
        if (decision == scoreNone || decision == noneExist){
            cout << "The Scheduler has assigned Hospital None to the client."  << endl;
        } else {
            cout << "The Scheduler has assigned Hospital " << decision << " to the client."  << endl;
        }
        

        memset(send_buff_t, 0, sizeof(send_buff_t));
        memcpy(send_buff_t, decision, strlen(decision));

        // scheduler reply to client the decision
        sd_t = send(chdsockfd, send_buff_t, sizeof(send_buff_t), 0);
        if (sd_t < 0){
            perror("send to client");
        }
        cout << "The Scheduler has sent the result to client using TCP over port " << ntohs(serv_addr.sin_port) << "."  << endl;
        close(chdsockfd);


        // scheduler reply to hospitals the decision
        memset(send_buff_u, 0, sizeof(send_buff_u));
        memcpy(send_buff_u, decision, strlen(decision));

        if (hBootA.hCapacity != hBootA.hOccupancy){
        sd_u = sendto(sockfd_u, send_buff_u, sizeof(send_buff_u), 0, (struct sockaddr *)&sockaddrA, hosplen);
            if (sd_u < 0){
                perror("sendtoA");
            }
        }

        if (hBootB.hCapacity != hBootB.hOccupancy){
            sd_u = sendto(sockfd_u, send_buff_u, sizeof(send_buff_u), 0, (struct sockaddr *)&sockaddrB, hosplen);
            if (sd_u < 0){
                perror("sendtoB");
            }
        }
        
        if (hBootC.hCapacity != hBootC.hOccupancy){
            sd_u = sendto(sockfd_u, send_buff_u, sizeof(send_buff_u), 0, (struct sockaddr *)&sockaddrC, hosplen);
            if (sd_u < 0){
                perror("sendtoC");
            }
        }

        // on screen message of result sending to hospital
        const char * resultA = "A";
        const char * resultB = "B";
        const char * resultC = "C";
        if (decision == resultA){
            onScreenResultSending(decision, scheduler);
        } else if (decision == resultB){
            onScreenResultSending(decision, scheduler);
        } else if (decision == resultC) {
            onScreenResultSending(decision, scheduler);
        }

    }
    return 0;
}

void onScreenResultSending(const char * decision, struct sockaddr_in sockaddr){
    cout << "The Scheduler has sent the result to Hospital " << decision <<  " using UDP over port " << ntohs(sockaddr.sin_port) << "." << endl;
}


float getMax(float array[],int count){ // get the max score out of a list of scores
    float max=array[0];
    for(int i = 0;i < count; i++)
        if(max < array[i])
            max = array[i]; 
    return max;
}


void onScreenBootup (const char *hospital, int hCapacity, int hOccupancy){
    cout << "The Scheduler has received information from Hospital " <<  hospital << ": ";
    cout << "total capacity is " << hCapacity << " and initial occupancy is " << hOccupancy << "." << endl;
}

void onScreenMapInfo (const char *hospital, float score, float distance){
    cout << "The Scheduler has received map information from Hospital " <<  hospital <<  ", the score = " ;
    if (score == -1){
        cout << "None";
    } else {
        cout << score;
    }
    cout << " and the distance = "; 
    if (distance == INF || distance == 0){
        cout << "None." << endl;
    } else {
        cout << distance << "." << endl;
    }
}

const char * compareTwoScore(const char *h1, const char *h2, struct hospitalSituation hSit1, struct hospitalSituation hSit2){
    const char *decision;
    if (hSit1.score == hSit2.score){
        decision = compareTwoDistance(h1, h2, hSit1, hSit2);
    } else if(hSit1.score > hSit2.score){
        decision = h1;
    } else {
        decision = h2;
    }
    return decision;
}

const char * compareTwoDistance(const char *h1, const char *h2, struct hospitalSituation hSit1, struct hospitalSituation hSit2){
    const char * decision;
    const char * scoreNone = "scoreNone";
    if (hSit1.distance < hSit2.distance){  
        decision = h1;
    } else if (hSit1.distance > hSit2.distance){
        decision = h2;
    } else if (hSit1.distance == hSit2.distance && hSit1.distance != -1){
        decision = h1;
    } else {
        decision = scoreNone;
    }
    return decision;
}

const char * decisionMaking(struct hospitalSituation hSitA, struct hospitalSituation hSitB, struct hospitalSituation hSitC){
    const char *decision;
    float score;
    if (!(hSitA.distance == INF || hSitA.distance == 0 || 
        hSitB.distance == INF || hSitB.distance == 0 || 
        hSitC.distance == INF || hSitC.distance == 0)){ // normal returned distance value
            if (hSitA.score == -1){ // A with score None
                decision = compareTwoScore("B", "C", hSitB, hSitC);
            } else if (hSitB.score == -1){ // B with score None
                decision = compareTwoScore("A", "C", hSitA, hSitC);
            } else if (hSitC.score == -1){ // C with score None
                decision = compareTwoScore("A", "B", hSitA, hSitB);
            } else if (!(hSitA.score == hSitB.score || hSitA.score == hSitC.score || hSitC.score == hSitB.score)){
                // scores all legal
                float score_arr[3];
                score_arr[0] = hSitA.score;
                score_arr[1] = hSitB.score;
                score_arr[2] = hSitC.score;
                score = getMax(score_arr, 3); // get max score out of the three

                if (score == hSitA.score && score == hSitB.score){ // ties for highest score or two score are both == -1
                    decision = compareTwoDistance("A", "B", hSitA, hSitB);
                } else if (score == hSitB.score && score == hSitC.score){
                    decision = compareTwoDistance("B", "C", hSitB, hSitC);
                } else if (score == hSitA.score && score == hSitC.score){
                    decision = compareTwoDistance("A", "C", hSitA, hSitC);
                } else { // No ties
                    if (score == hSitA.score){
                        decision = "A";
                    } else if(score == hSitB.score){
                        decision = "B";
                    } else{
                        decision = "C";
                    }
                }
            } else { // scores all illegal
                decision = "scoreNone";
            }
        }else if (hSitA.distance == INF || hSitB.distance == INF || hSitC.distance == INF){ // distance illgal
            decision = "noneExist";
        }else if (hSitA.distance == 0 || hSitB.distance == 0 || hSitC.distance == 0){
            decision = "scoreNone";
        }else {
            decision = "scoreNone";
        }
    return decision;
}