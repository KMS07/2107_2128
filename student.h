#ifndef STUDENT_H
#define STUDENT_H

#include <stdbool.h>
#include <stdio.h>

#define nameSize 400

struct student
{
    int rollNo;
    char name[nameSize];
    float CGPA;
    int noOfSubjects;

    struct student *nextStudent;
    struct student *prevStudent;
    struct course *course;
};

struct course
{
    int courseCode;
    int marks;

    struct course *nextCourse;
    struct course *prevCourse;
};

extern struct student *head; // we are not keeping the data in the head node.

bool searchStudent(int rollNo);
int searchStudentCourse(int rollNo, int courseCode);
void addStudent(int rollNo, char *name, float CGPA, int noOfSubjects);
void modifyStudent(int rollNo, float newCGPA);
void deleteStudent(int rollNo);
void modifyCourse(int rollNo, int courseCode, int marks);
void addCourse(int rollNo, int courseCode, int marks);
void deleteCourse(int rollNo, int courseCode);


#endif