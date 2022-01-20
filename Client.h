#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <iostream>
#include <unistd.h>

#define DESTPORT 34761
#define SERVER "127.0.0.1"
using namespace std;