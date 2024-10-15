#ifndef CLIENT_H
#define CLIENT_H

#include "fifo.h"

// Function declarations
int openClientFIFO();
void sendRequest(struct API_Request request);
void addStudent(int rollNo, char *name, float CGPA, int noOfSubjects);
void modifyStudent(int rollNo, float newCGPA);
void deleteStudent(int rollNo);
void modifyCourse(int rollNo, int courseCode, int marks);
void addCourse(int rollNo, int courseCode, int marks);
void deleteCourse(int rollNo, int courseCode);

#endif /* FIFO_H */














