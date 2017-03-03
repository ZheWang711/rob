//
// Created by Zhe Wang on 3/1/17.
//


// A simple socket server, refer to: UCSD CSE124 HW2

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <string>
#include <map>
#include "Controller.h"
#include "Command.h"
#include <thread>

using std::string;
using std::to_string;

const int data_length = 8;

static const int MAXPENDING = 5; // Maximum outstanding connection requests (pending commands)
const int BUFSIZE = 1024; // TCP datagram buffer
KobukiController controller; // Kobuki controller

void DieWithSystemMessage(const char *msg) {
  perror(msg);
  exit(1);
}

string serilize(int *x, int len){
    string res = to_string(x[0]);
    for (int i = 1; i < len; ++i){
        res += ("," + to_string(x[i]));
    }
    return res;
}

void handleGetData(int clntSocket){
    char buffer[BUFSIZE]; // Buffer recving command
    ssize_t numBytesRcvd;
    size_t totaBytesRcvd = 0;

    // // Keep receiving until it got a null char
    while (true) {
      // Receive message from client
      printf("call recv() \n");
      numBytesRcvd = recv(clntSocket, buffer + totaBytesRcvd, BUFSIZE - totaBytesRcvd, 0);
      totaBytesRcvd += numBytesRcvd;
      if (numBytesRcvd < 0)
        DieWithSystemMessage("recv() failed");
      // '\0' is guaranteed to on the boundary at some time
      if (buffer[totaBytesRcvd - 1] == '\0')
        break;
    }

    int* data_buffer = new int[data_length];
    controller.getSensorData(data_buffer);
    string msg = serilize(data_buffer, data_length);
    char* send_buffer = new char [msg.length() + 1];
    memcpy(send_buffer, msg.c_str(), msg.length());
    send_buffer[msg.length()] = '\0';

    int *x = data_buffer;
    printf("data_buffer %x, %x, %x, %x, %x, %x, %x, %x \n", x[0], x[1], x[2], x[3], x[4], x[5], x[6], x[7]);
    printf("send buffer %s\n", send_buffer); // TODO delete debug
    send(clntSocket, send_buffer, msg.length() + 1 , 0); // todo check sent byte error
    delete data_buffer;
    delete send_buffer;
    close(clntSocket); // Close client socket

}

void handleCommand(const char* cmd){
    Command command = Command(cmd);
    printf("handing command \n");


    switch (command.getCmdType()) {

        case Constants::MOVE: {
            controller.move(atof(command.getPara(Constants::speed).c_str()), atoi(command.getPara(Constants::time).c_str()));
            if (command.getPara(Constants::stop) == "true"){
                controller.stop();
            }
            break;
        }
        case Constants::ROTATE: {
            controller.rotate(atof(command.getPara(Constants::speed).c_str()), atoi(command.getPara(Constants::time).c_str()));
            if (command.getPara(Constants::stop) == "true"){
                controller.stop();
            }
            break;
        }
        case Constants::STOP: {
            controller.stop();
            break;
        }
        // invalid command
        assert(false);

    }
    printf("heading\n %lf", controller.getHeading());
    controller.resetOdometry();

}



void HandleTCPClient(int clntSocket, int n) {
  printf("call HandleTCPClient() \n");
  char buffer[BUFSIZE]; // Buffer recving command
  ssize_t numBytesRcvd;
  size_t totaBytesRcvd = 0;

  // // Keep receiving until it got a null char
  while (true) {
    // Receive message from client
    printf("call recv() \n");
    numBytesRcvd = recv(clntSocket, buffer + totaBytesRcvd, BUFSIZE - totaBytesRcvd, 0);
    totaBytesRcvd += numBytesRcvd;
    if (numBytesRcvd < 0)
      DieWithSystemMessage("recv() failed");
    // '\0' is guaranteed to on the boundary at some time
    if (buffer[totaBytesRcvd - 1] == '\0')
      break;
  }
  handleCommand(buffer);
  string response = string(buffer) + "received";
  send(clntSocket, response.c_str(), response.size(), 0); // todo check sent byte error
  close(clntSocket); // Close client socket
}



void getDataThread(int servPort) {
    printf("get data thread on %d\n", servPort);

    // Create socket for incoming connections
    int servSock; // Socket descriptor for server
    if ((servSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
      DieWithSystemMessage("socket() failed");

    // Construct local address structure
    struct sockaddr_in servAddr;                  // Local address
    memset(&servAddr, 0, sizeof(servAddr));       // Zero out structure
    servAddr.sin_family = AF_INET;                // IPv4 address family
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY); // Any incoming interface
    servAddr.sin_port = htons(servPort);          // Local port

    // Bind to the local address
    if (bind(servSock, (struct sockaddr*) &servAddr, sizeof(servAddr)) < 0)
      DieWithSystemMessage("bind() failed");

    // Mark the socket so it will listen for incoming connections
    if (listen(servSock, MAXPENDING) < 0)
      DieWithSystemMessage("listen() failed");


      for (;;) { // Run forever
        struct sockaddr_in clntAddr; // Client address
        // Set length of client address structure (in-out parameter)
        socklen_t clntAddrLen = sizeof(clntAddr);

        // Wait for a client to connect
        printf("call accept() \n");
        int clntSock = accept(servSock, (struct sockaddr *) &clntAddr, &clntAddrLen);
        if (clntSock < 0)
          DieWithSystemMessage("accept() failed");

        // clntSock is connected to a client!

        char clntName[INET_ADDRSTRLEN]; // String to contain client address
        if (inet_ntop(AF_INET, &clntAddr.sin_addr.s_addr, clntName,
                      sizeof(clntName)) != NULL)
          printf("Handling client %s/%d\n", clntName, ntohs(clntAddr.sin_port));
        else
          puts("Unable to get client address");

        handleGetData(clntSock);
      }
}



int main(int argc, char *argv[]) {

  if (argc != 3) // Test for correct number of arguments
    DieWithSystemMessage("Parameter(s) <Server Port> <N>");

  in_port_t servPort = atoi(argv[1]); // First arg:  local port
  int N = atoi(argv[2]);
  (void)N; // suppress the unused parameter warning

  printf("%s\n", "spawing thread");
  std::thread get_data_thread(getDataThread, N);

  // Create socket for incoming connections
  int servSock; // Socket descriptor for server
  if ((servSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    DieWithSystemMessage("socket() failed");

  // Construct local address structure
  struct sockaddr_in servAddr;                  // Local address
  memset(&servAddr, 0, sizeof(servAddr));       // Zero out structure
  servAddr.sin_family = AF_INET;                // IPv4 address family
  servAddr.sin_addr.s_addr = htonl(INADDR_ANY); // Any incoming interface
  servAddr.sin_port = htons(servPort);          // Local port

  // Bind to the local address
  if (bind(servSock, (struct sockaddr*) &servAddr, sizeof(servAddr)) < 0)
    DieWithSystemMessage("bind() failed");

  // Mark the socket so it will listen for incoming connections
  if (listen(servSock, MAXPENDING) < 0)
    DieWithSystemMessage("listen() failed");

  for (;;) { // Run forever
    struct sockaddr_in clntAddr; // Client address
    // Set length of client address structure (in-out parameter)
    socklen_t clntAddrLen = sizeof(clntAddr);

    // Wait for a client to connect
    printf("call accept() \n");
    int clntSock = accept(servSock, (struct sockaddr *) &clntAddr, &clntAddrLen);
    if (clntSock < 0)
      DieWithSystemMessage("accept() failed");

    // clntSock is connected to a client!

    char clntName[INET_ADDRSTRLEN]; // String to contain client address
    if (inet_ntop(AF_INET, &clntAddr.sin_addr.s_addr, clntName,
                  sizeof(clntName)) != NULL)
      printf("Handling client %s/%d\n", clntName, ntohs(clntAddr.sin_port));
    else
      puts("Unable to get client address");

    HandleTCPClient(clntSock, N);
  }
  // NOT REACHED
}
