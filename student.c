#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "student.h"

struct student *head = NULL;
bool searchStudent(int rollNo)
{
    struct student *nextStudentNode = (struct student *)malloc(sizeof(struct student));
    
    if (head == NULL)
    {
        return false;
    }
    else
    {
        nextStudentNode = head->nextStudent;
        while (nextStudentNode != NULL)
        {
            if (nextStudentNode->rollNo == rollNo)
            {
                return true;
            }
            else
            {
                nextStudentNode = nextStudentNode->nextStudent;
            }
        }
        return false;
    }
}

int searchStudentCourse(int rollNo, int courseCode)
{
    struct student *nextStudentNode = (struct student *)malloc(sizeof(struct student));
    struct course *nextCourseNode = (struct course *)malloc(sizeof(struct course));
    nextStudentNode = head;
    if (nextStudentNode == NULL)
    {
        return -1;
    }
    else
    {
        nextStudentNode = head->nextStudent;
        while (nextStudentNode != NULL)
        {
            if (nextStudentNode->rollNo == rollNo)
            {
                if (nextStudentNode->course == NULL)
                {
                    return -2;
                }
                else
                {
                    nextCourseNode = nextStudentNode->course;
                    while (nextCourseNode != NULL)
                    {
                        if (nextCourseNode->courseCode == courseCode)
                        {
                            return -3;
                        }
                        else
                        {
                            nextCourseNode = nextCourseNode->nextCourse;
                        }
                    }
                    return -2;
                }
            }
            else
            {
                nextStudentNode = nextStudentNode->nextStudent;
            }
        }
        return -1;
    }
}

void addStudent(int rollNo, char *name, float CGPA, int noOfSubjects, struct API_Response *response)
{
    char buffer[256];
    struct student *tempNode = (struct student *)malloc(sizeof(struct student));
    struct student *nextNode = (struct student *)malloc(sizeof(struct student));
    if (tempNode == NULL || nextNode == NULL)
    {
        response->status = 0;
        strcpy(response->message, "Error in memory allocation in Data Structure\n");
        return ;
    }
    tempNode->rollNo = rollNo;
    strncpy(tempNode->name, name, nameSize - 1);
    tempNode->name[nameSize - 1] = '\0';
    tempNode->CGPA = CGPA;
    tempNode->noOfSubjects = noOfSubjects;
    tempNode->prevStudent = NULL;
    tempNode->nextStudent = NULL;
    tempNode->course = NULL;

    if (head == NULL)
    {
        head = (struct student *)malloc(sizeof(struct student));
        head->nextStudent = tempNode;
        tempNode->prevStudent = head;

        sprintf(buffer, "Student added succesfully with roll no. %d!", rollNo);
        response->status = 1;
        strcpy(response->message, buffer);
    }
    else
    {
        if (searchStudent(rollNo))
        {
            sprintf(buffer, "Student already exists with roll no. %d!(add student)\n", rollNo);
            response->status = 0;
            strcpy(response->message, buffer);
        }
        else
        {
            nextNode = head->nextStudent;
            while (nextNode->nextStudent != NULL)
            {
                nextNode = nextNode->nextStudent;
            }
            nextNode->nextStudent = tempNode;
            tempNode->prevStudent = nextNode;
            sprintf(buffer, "Student added succesfully with roll no. %d!", rollNo);
            response->status = 1;
            strcpy(response->message, buffer);
        }
    }

    return;
}

void modifyStudent(int rollNo, float newCGPA,struct API_Response *response)
{
    char buffer[256];
    if (searchStudent(rollNo))
    {
        struct student *nextNode = (struct student *)malloc(sizeof(struct student));
        nextNode = head->nextStudent;
        while (nextNode->rollNo != rollNo)
        {
            nextNode = nextNode->nextStudent;
        }
        nextNode->CGPA = newCGPA;
        sprintf(buffer, "Student's CGPA with roll no %d modified successfuly\n", rollNo);
        response->status = 1;
        strcpy(response->message, buffer);
    }
    else
    {
        sprintf(buffer, "Student with %d does not exist to modify CGPA(modify student)\n", rollNo);
        response->status = 0;
        strcpy(response->message, buffer);
    }
    return;
}

void deleteStudent(int rollNo, struct API_Response *response)
{
    char buffer[256];
    if (searchStudent(rollNo))
    {
        struct student *tempNode = (struct student *)malloc(sizeof(struct student));
        struct student *prevNode = (struct student *)malloc(sizeof(struct student));
        tempNode = head->nextStudent;
        while (tempNode->rollNo != rollNo)
        {
            tempNode = tempNode->nextStudent;
        }

        struct student *deleteStudent = tempNode;
        if (tempNode->prevStudent->prevStudent == NULL && tempNode->rollNo == rollNo)
        {
            head->nextStudent = tempNode->nextStudent;
            tempNode->nextStudent->prevStudent = head;
            free(deleteStudent);
        }
        else if (tempNode->nextStudent == NULL)
        {
            tempNode->prevStudent->nextStudent = NULL;
            free(deleteStudent);
        }
        else
        {
            prevNode = tempNode->prevStudent;
            tempNode = tempNode->nextStudent;
            prevNode->nextStudent = tempNode;
            tempNode->prevStudent = prevNode;
            free(deleteStudent);
        }
        sprintf(buffer, "Student with roll no %d deleted successfully\n", rollNo);
        response->status = 1;
        strcpy(response->message, buffer);
    }
    else
    {
        sprintf(buffer, "Student with %d does not exist to Delete(delete student)\n", rollNo);
        response->status = 0;
        strcpy(response->message, buffer);
        return;
    }
}

void modifyCourse(int rollNo, int courseCode, int marks,struct API_Response *response)
{
    char buffer[256];
    int searchCase = searchStudentCourse(rollNo, courseCode);
    if (searchCase == -3)
    {
        struct student *tempStudentNode = (struct student *)malloc(sizeof(struct student));
        tempStudentNode = head;
        while (tempStudentNode->rollNo != rollNo)
        {
            tempStudentNode = tempStudentNode->nextStudent;
        }

        struct course *tempCourseNode = (struct course *)malloc(sizeof(struct course));
        tempCourseNode = tempStudentNode->course;
        while (tempCourseNode->courseCode != courseCode)
        {
            tempCourseNode = tempCourseNode->nextCourse;
        }
        if (tempCourseNode->courseCode == courseCode)
        {
            tempCourseNode->marks = marks;
            sprintf(buffer, "Student's course with roll no %d modified successfully\n", rollNo);
            response->status = 1;
            strcpy(response->message, buffer);
        }
    }
    else if (searchCase == -2)
    {
        sprintf(buffer, "Student with rollno. %d exists but course with code %d does not exists(modify course)\n", rollNo, courseCode);
        response->status = 0;
        strcpy(response->message, buffer);
        return;
    }
    else if (searchCase == -1)
    {
        sprintf(buffer, "Student with rollno. %d does not exist(modify course)\n", rollNo);
        response->status = 0;
        strcpy(response->message, buffer);
    }
}

void addCourse(int rollNo, int courseCode, int marks, struct API_Response *response)
{
    char buffer[256];
    int searchCase = searchStudentCourse(rollNo, courseCode);
    if (searchCase == -3)
    {
        sprintf(buffer, "Student with rollNo %d and course code %d already exists.(addCourse)\n", rollNo, courseCode);
        response->status = 0;
        strcpy(response->message, buffer);
        return;
    }
    else if (searchCase == -2)
    {
        struct student *tempStudentNode = (struct student *)malloc(sizeof(struct student));
        struct course *tempCourseNode = (struct course *)malloc(sizeof(struct course));
        tempStudentNode = head->nextStudent;
        while (tempStudentNode->rollNo != rollNo)
        {
            tempStudentNode = tempStudentNode->nextStudent;
        }
        tempStudentNode->noOfSubjects = tempStudentNode->noOfSubjects + 1;

        tempCourseNode = tempStudentNode->course;

        struct course *additionNode = (struct course *)malloc(sizeof(struct course));
        additionNode->courseCode = courseCode;
        additionNode->marks = marks;
        additionNode->nextCourse = NULL;
        additionNode->prevCourse = NULL;
        if (tempCourseNode == NULL)
        {
            tempStudentNode->course = additionNode;
        }
        else
        {
            while (tempCourseNode->nextCourse != NULL)
            {
                tempCourseNode = tempCourseNode->nextCourse;
            }
            tempCourseNode->nextCourse = additionNode;
            additionNode->prevCourse = tempCourseNode;
        }
        sprintf(buffer, "Student with rollNo %d and course code %d added successfully\n", rollNo, courseCode);
        response->status = 1;
        strcpy(response->message, buffer);        
        return;
    }
    else if (searchCase == -1)
    {
        sprintf(buffer, "Student with rollNo %d does not exists!(addCourse)\n", rollNo);
        response->status = 0;
        strcpy(response->message, buffer);        
    }
    else
    {
        fprintf(stderr, "Error in return from searchcase!");
    }
}

void deleteCourse(int rollNo, int courseCode, struct API_Response *response)
{
    char buffer[256];
    int searchCase = searchStudentCourse(rollNo, courseCode);
    if (searchCase == -3)
    {
        struct student *tempStudentNode = (struct student *)malloc(sizeof(struct student));
        tempStudentNode = head->nextStudent;
        while (tempStudentNode->rollNo != rollNo)
        {
            tempStudentNode = tempStudentNode->nextStudent;
        }

        tempStudentNode->noOfSubjects = tempStudentNode->noOfSubjects - 1;
        struct course *tempCourseNode = (struct course *)malloc(sizeof(struct course));
        struct course *prevCourseNode = (struct course *)malloc(sizeof(struct course));
        tempCourseNode = tempStudentNode->course;
        while (tempCourseNode->courseCode != courseCode)
        {
            tempCourseNode = tempCourseNode->nextCourse;
        }

        if (tempCourseNode->prevCourse == NULL) // at the begining
        {
            struct course *deleteNode = tempCourseNode;
            tempCourseNode = tempCourseNode->nextCourse;
            tempStudentNode->course = tempCourseNode;
            tempCourseNode->prevCourse = NULL;
            free(deleteNode);
        }
        else if (tempCourseNode->nextCourse == NULL) // at the end
        {
            struct course *deleteNode = tempCourseNode;
            tempCourseNode = tempCourseNode->prevCourse;
            tempCourseNode->nextCourse = NULL;
            free(deleteNode);
        }
        else // in between
        {
            struct course *deleteNode = tempCourseNode;
            prevCourseNode = tempCourseNode->prevCourse;
            tempCourseNode = tempCourseNode->nextCourse;

            prevCourseNode->nextCourse = tempCourseNode;
            tempCourseNode->prevCourse = prevCourseNode;
            free(deleteNode);
        }
        sprintf(buffer, "Student's course with code %d and with rollNo %d deleted successfully\n", courseCode, rollNo);
        response->status = 1;
        strcpy(response->message, buffer); 
    }
    else if (searchCase == -2)
    {
        sprintf(buffer, "student id %d exists but course code %d does not exist(delete course)\n", rollNo, courseCode);
        response->status = 0;
        strcpy(response->message, buffer);
    }
    else if (searchCase == -1)
    {
        sprintf(buffer, "student id %d does not exist(delete course)\n", rollNo);
        response->status = 0;
        strcpy(response->message, buffer);        
    }
    else
    {
        fprintf(stderr, "error in the search case.(delete course)");
    }
}
