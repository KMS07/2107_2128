#ifndef FIFO_H
#define FIFO_H
#include "student.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>

#define FIFO "/tmp/fifo.1"
#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

enum API_Type {
    ADD_STUDENT,
    MODIFY_STUDENT,
    DELETE_STUDENT,
    ADD_COURSE,
    MODIFY_COURSE,
    DELETE_COURSE
};

union API_Data {
    struct {
        int rNo;
       	char name[nameSize];
       	float cgpa;
       	int noOfSubjects;
    } api_add_student;

    struct {
        int rNo;
        float cgpa;
    } api_modify_student;

    struct {
        int rNo;
    } api_delete_student;
    
    struct {
        int rNo;
        int courseCode;
        int marks;
    } api_add_course;

    struct {
        int rNo;
        int courseCode;
        int marks;
    } api_modify_course;

    struct {
        int rNo;
        int courseCode;
    } api_delete_course;
};


struct API_Request {
    enum API_Type api_type; 
    union API_Data data;     
};

extern int writefd,readfd;

#endif
