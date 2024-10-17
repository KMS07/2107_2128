#ifndef CLIENT_H
#define CLIENT_H

#include "socket.h"

// Function declarations
int openClientSocket(char *SERVER_IP,int SERVER_PORT);
void sendRequest(struct API_Request request);
void addStudent(int rollNo, char *name, float CGPA, int noOfSubjects);
void modifyStudent(int rollNo, float newCGPA);
void deleteStudent(int rollNo);
void modifyCourse(int rollNo, int courseCode, int marks);
void addCourse(int rollNo, int courseCode, int marks);
void deleteCourse(int rollNo, int courseCode);

#endif 














