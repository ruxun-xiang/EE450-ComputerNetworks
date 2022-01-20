#include "Client.h"

int main(int argc, char *argv[]){
	int sockfd, sd, rv;
	struct sockaddr_in serv_addr;
	struct hostent *scheduler;
	char recv_buff[1024];
	char send_buff[100];
	
	if (argc < 2){
		cout << "Location needed." << endl;
		exit(0);
	}
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd < 0){
		perror("Socket error");
	}

	cout << "The client is up and running." << endl;

	scheduler = gethostbyname(SERVER);

	if (scheduler == NULL){
		perror ("No such host.");
	}

	bzero((char *)&serv_addr, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;

	bcopy((char *)scheduler -> h_addr, (char *)&serv_addr.sin_addr.s_addr, scheduler->h_length);

	serv_addr.sin_port = htons(DESTPORT);

	if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
		perror("Connect error");
	}

	memset(send_buff, 0, sizeof(send_buff));
	memcpy(send_buff, argv[1], strlen(argv[1]));

	sd = send(sockfd, send_buff, sizeof(send_buff), 0);
	if (sd < 0){
		perror("Send error");
	}
	cout << "The client has sent query to Scheduler using TCP: client location " << argv[1] << "." << endl;

	// allocate memory
	bzero(recv_buff, sizeof(recv_buff));

	rv = recv(sockfd, recv_buff, sizeof(recv_buff), 0);
	if (rv < 0){
		perror("Receive error");
	}

	if (strcmp(recv_buff, "noneExist") == 0){ // indicate client's location not exist
		cout << "Location " << argv[1] << " not found." << endl;
	}else if (strcmp(recv_buff, "scoreNone") == 0) { // indicate score == None
		cout << "Score = None, No assignment." << endl;
	}else {
		cout << "The client has received results from the Scheduler: assigned to Hospital " << recv_buff << "." << endl;
	}

	close(sockfd);
	return 0;

}