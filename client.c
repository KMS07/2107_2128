#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "fifo.h"
#include "client.h"

#define SERVER_CONNECTION_TIMEOUT 5
#define MAX_RETRIES 5
#define RETRY_DELAY 100000
#define PIPE_RETRY 3

struct API_Request apiRequest;

int writefd;

// Signal handler for SIGPIPE
void handle_sigpipe(int sig) {
    int pipeRetry = 1;
    fprintf(stderr, "Received SIGPIPE signal\n");

    while(pipeRetry <= PIPE_RETRY){
       ssize_t bytes_written = write(writefd, &apiRequest, sizeof(apiRequest));
       fprintf(stderr,"Retrying to write(%d/%d)\n", pipeRetry, PIPE_RETRY);
       pipeRetry++;
       if (bytes_written == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // FIFO is full
                pipeRetry++;
                fprintf(stderr,"FIFO is full, retrying(%d/%d)\n", pipeRetry, MAX_RETRIES);
                usleep(RETRY_DELAY); 
            } else if (errno == EPIPE) {
                // No readers on the FIFO
                fprintf(stderr, "Error: Server is not running or has closed the pipe\n");
                pipeRetry++;
                usleep(RETRY_DELAY); // Sleep before retrying
            } else {
                // Some other error occurred
                perror("write");
                close(writefd);
                exit(EXIT_FAILURE);
            }
        } else {
            printf("Message written to FIFO\n");
            pipeRetry = 1; // Reset retries 
            break; // exiting loop after write is successful
        }
    }
    if (pipeRetry >= PIPE_RETRY){
        printf("Couldn't write");
        exit(1);
    }
}

void handle_timeout(int sig) {
    fprintf(stderr, "Timeout: No reader connected within %d seconds. Exiting...\n", SERVER_CONNECTION_TIMEOUT);
    exit(EXIT_FAILURE);
}

int openClientFIFO() {

    // Setting up signal handler for SIGPIPE
    struct sigaction sa;
    sa.sa_handler = handle_sigpipe;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGPIPE, &sa, NULL);


    if(signal(SIGALRM, handle_timeout) == SIG_ERR){
        perror("signal FAILED");
        exit(EXIT_FAILURE);
    }


    alarm(SERVER_CONNECTION_TIMEOUT);

    if ((writefd = open(FIFO, O_WRONLY)) == -1) {
        fprintf(stderr, "Error opening FIFO %s(%s)\n", FIFO, strerror(errno));
        return -1;
    }

    alarm(0);

    return writefd;
}


//send the data to fifo
void sendRequest(struct API_Request request){
    int retries = 0;
    while (retries < MAX_RETRIES) {
        ssize_t bytes_written = write(writefd, &request, sizeof(request));
        if (bytes_written == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // FIFO is full
                retries++;
                fprintf(stderr,"FIFO is full, retrying(%d/%d)\n", retries, MAX_RETRIES);
                usleep(RETRY_DELAY); 
            } else if (errno == EPIPE) {
                // No readers on the FIFO
                fprintf(stderr, "Error: Server is not running or has closed the pipe");
                retries++;
                usleep(RETRY_DELAY); // Sleep before retrying
            } else {
                // Some other error occurred
                perror("write");
                close(writefd);
                exit(EXIT_FAILURE);
            }
        } else {
            printf("Message written to FIFO\n");
            retries = 0; // Reset retries 
            break; // exiting loop after write is successful
        }
    }

    if (retries >= MAX_RETRIES) {
        printf("Max retries reached, exiting program.\n");
        close(writefd);
    }

}
void addStudent(int rollNo, char *name, float CGPA, int noOfSubjects)
{
    apiRequest.api_type = ADD_STUDENT;
    apiRequest.data.api_add_student.rNo = rollNo;
    strncpy(apiRequest.data.api_add_student.name, name, nameSize-1);
    apiRequest.data.api_add_student.cgpa = CGPA;
    apiRequest.data.api_add_student.noOfSubjects = noOfSubjects;

    //write into fifo
    sendRequest(apiRequest);
}

void modifyStudent(int rollNo, float newCGPA){
    apiRequest.api_type = MODIFY_STUDENT;
    apiRequest.data.api_modify_student.rNo = rollNo;
    apiRequest.data.api_modify_student.cgpa = newCGPA;

    //write into fifo
    sendRequest(apiRequest);
}

void deleteStudent(int rollNo)
{
    apiRequest.api_type = DELETE_STUDENT;
    apiRequest.data.api_delete_student.rNo = rollNo;

    //write into fifo
    sendRequest(apiRequest);
}

void modifyCourse(int rollNo, int courseCode, int marks)
{
    apiRequest.api_type = MODIFY_COURSE;
    apiRequest.data.api_modify_course.rNo = rollNo;
    apiRequest.data.api_modify_course.courseCode = courseCode;
    apiRequest.data.api_modify_course.marks = marks;

    //write into fifo
    sendRequest(apiRequest);
}
void addCourse(int rollNo, int courseCode, int marks){
    apiRequest.api_type = ADD_COURSE;
    apiRequest.data.api_add_course.rNo = rollNo;
    apiRequest.data.api_add_course.courseCode = courseCode;
    apiRequest.data.api_add_course.marks = marks;

    //write into fifo
    sendRequest(apiRequest);
}

void deleteCourse(int rollNo, int courseCode)
{
    apiRequest.api_type = DELETE_COURSE;
    apiRequest.data.api_delete_course.rNo = rollNo;
    apiRequest.data.api_delete_course.courseCode = courseCode;

    //write into fifo
    sendRequest(apiRequest);
}