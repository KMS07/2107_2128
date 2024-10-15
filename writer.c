#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "writer.h"


void writer()
// Writes the database to the output file
{

    struct student *tempStudentNode = (struct student *)malloc(sizeof(struct student));
    struct course *tempCourseNode = (struct course *)malloc(sizeof(struct course));
    tempStudentNode = head;
    FILE *outputFile = fopen("2107_2128.out", "w");
    if (outputFile == NULL)
    {
        printf("Error opening the file.");
        return;
    }
    if (tempStudentNode == NULL)
    {
        fprintf(stderr, "Empty database\n");
    }
    else
    {
        while (tempStudentNode->nextStudent != NULL)
        {
            tempStudentNode = tempStudentNode->nextStudent;
            fprintf(outputFile, "%d, %s, %f, %d\n", tempStudentNode->rollNo, tempStudentNode->name, tempStudentNode->CGPA, tempStudentNode->noOfSubjects);
            tempCourseNode = tempStudentNode->course;
            while (tempCourseNode != NULL)
            {
                fprintf(outputFile, "%d, %d\n", tempCourseNode->courseCode, tempCourseNode->marks);
                tempCourseNode = tempCourseNode->nextCourse;
            }
        }
        printf("Database updated\n");
    }
    fclose(outputFile);
}