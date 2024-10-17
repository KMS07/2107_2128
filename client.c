#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include "socket.h"
#include "client.h"

#define SERVER_CONNECTION_TIMEOUT 5
#define MAX_RETRIES 5
#define RETRY_DELAY 100000
#define SOCKET_RETRY 3

struct API_Request apiRequest;

int socket_fd;

// Signal handler for SIGPIPE
void handle_sigpipe(int sig) {
    int socketRetry = 1;
    fprintf(stderr, "Received SIGPIPE signal\n");

    while(socketRetry <= SOCKET_RETRY){
       ssize_t bytes_written = write(socket_fd, &apiRequest, sizeof(apiRequest));
       fprintf(stderr,"Retrying to write(%d/%d)\n", socketRetry, SOCKET_RETRY);
       socketRetry++;
       if (bytes_written == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                socketRetry++;
                fprintf(stderr,"Socket send buffer is full, retrying(%d/%d)\n", socketRetry, MAX_RETRIES);
                usleep(RETRY_DELAY); 
            } else if (errno == EPIPE) {
                fprintf(stderr, "Error: Server has closed the connection\n");
                socketRetry++;
                usleep(RETRY_DELAY); // Sleep before retrying
            } else {
                // Some other error occurred
                perror("write");
                close(socket_fd);
                exit(EXIT_FAILURE);
            }
        } else {
            printf("Message written to socket\n");
            socketRetry = 1; // Reset retries 
            break; // exiting loop after write is successful
        }
    }
    if (socketRetry >= SOCKET_RETRY){
        printf("Couldn't write");
        exit(1);
    }
}

void handle_timeout(int sig) {
    fprintf(stderr, "Timeout: No server connected within %d seconds. Exiting...\n", SERVER_CONNECTION_TIMEOUT);
    exit(EXIT_FAILURE);
}

int openClientSocket(char *SERVER_IP,int SERVER_PORT) {

    //installing signal handler for SIGPIPE
    struct sigaction sa;
    sa.sa_handler = handle_sigpipe;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGPIPE, &sa, NULL);

    //signal handler for connection timeout
    if (signal(SIGALRM, handle_timeout) == SIG_ERR) {
        perror("signal FAILED");
        exit(EXIT_FAILURE);
    }

    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        return -1;
    }

    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(SERVER_PORT);

    // Convert the IP address from text to binary form
    if (inet_pton(AF_INET, SERVER_IP, &server_address.sin_addr) <= 0) {
        fprintf(stderr, "Invalid server IP address\n");
        close(socket_fd);
        return -1;
    }

    //alarm for connection timeout
    alarm(SERVER_CONNECTION_TIMEOUT);

    //connect to the server
    if (connect(socket_fd, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        perror("Connection to server failed");
        close(socket_fd);
        return -1;
    }

    alarm(0);

    return socket_fd;
}


void receiveFromSocket()
{
    struct API_Response response;
    int bytesReceived;
    fd_set read_fds;
    struct timeval timeout;

    timeout.tv_sec = 5;
    timeout.tv_usec = 0;

    // Clear set and add socket_fd to the set
    FD_ZERO(&read_fds);
    FD_SET(socket_fd, &read_fds);

    //to wait for data to be available
    int select_result = select(socket_fd + 1, &read_fds, NULL, NULL, &timeout);
    if (select_result < 0)
    {
        perror("Error with select()");
        return;
    }
    else if (select_result == 0)
    {
        printf("Timeout occurred! No data received in 5 seconds.\n");
        return;
    }

    // Data is available to be read
    bytesReceived = read(socket_fd, &response, sizeof(response));
    if (bytesReceived < 0)
    {
        perror("Error receiving data");
    }
    else if (bytesReceived == 0)
    {
        printf("Connection closed by the server\n");
    }
    else
    {
        // Successfully received data
        printf("Received response from server\n");
        printf("Status: %d, Message: %s\n", response.status, response.message);
    }
}


//send the data to socket
void sendRequest(struct API_Request request){
    int retries = 0;
    while (retries < MAX_RETRIES) {
        ssize_t bytes_written = write(socket_fd, &request, sizeof(request));
        if (bytes_written == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // socket is full
                retries++;
                fprintf(stderr,"Socket send buffer is full, retrying(%d/%d)\n", retries, MAX_RETRIES);
                usleep(RETRY_DELAY); 
            } else if (errno == EPIPE) {
                fprintf(stderr, "Error: Server has closed the connection");
                retries++;
                usleep(RETRY_DELAY); // Sleep before retrying
            } else {
                // Some other error occurred
                perror("write");
                close(socket_fd);
                exit(EXIT_FAILURE);
            }
        } else {
            printf("Message written to socket\n");
            retries = 0; // Reset retries 
            break; // exiting loop after write is successful
        }
    }

    if (retries >= MAX_RETRIES) {
        printf("Max retries reached, exiting program.\n");
        close(socket_fd);
    }

}
void addStudent(int rollNo, char *name, float CGPA, int noOfSubjects)
{
    apiRequest.api_type = ADD_STUDENT;
    apiRequest.data.api_add_student.rNo = rollNo;
    strncpy(apiRequest.data.api_add_student.name, name, nameSize-1);
    apiRequest.data.api_add_student.cgpa = CGPA;
    apiRequest.data.api_add_student.noOfSubjects = noOfSubjects;

    //write into socket
    sendRequest(apiRequest);
    receiveFromSocket();
}

void modifyStudent(int rollNo, float newCGPA){
    apiRequest.api_type = MODIFY_STUDENT;
    apiRequest.data.api_modify_student.rNo = rollNo;
    apiRequest.data.api_modify_student.cgpa = newCGPA;

    //write into socket
    sendRequest(apiRequest);
    receiveFromSocket();
}

void deleteStudent(int rollNo)
{
    apiRequest.api_type = DELETE_STUDENT;
    apiRequest.data.api_delete_student.rNo = rollNo;

    //write into socket 
    sendRequest(apiRequest);
    receiveFromSocket();
}

void modifyCourse(int rollNo, int courseCode, int marks)
{
    apiRequest.api_type = MODIFY_COURSE;
    apiRequest.data.api_modify_course.rNo = rollNo;
    apiRequest.data.api_modify_course.courseCode = courseCode;
    apiRequest.data.api_modify_course.marks = marks;

    //write into socket
    sendRequest(apiRequest);
    receiveFromSocket();
}
void addCourse(int rollNo, int courseCode, int marks){
    apiRequest.api_type = ADD_COURSE;
    apiRequest.data.api_add_course.rNo = rollNo;
    apiRequest.data.api_add_course.courseCode = courseCode;
    apiRequest.data.api_add_course.marks = marks;

    //write into socket
    sendRequest(apiRequest);
    receiveFromSocket();
}

void deleteCourse(int rollNo, int courseCode)
{
    apiRequest.api_type = DELETE_COURSE;
    apiRequest.data.api_delete_course.rNo = rollNo;
    apiRequest.data.api_delete_course.courseCode = courseCode;

    //write into socket
    sendRequest(apiRequest);
    receiveFromSocket();
}