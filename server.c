#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include "fifo.h"
#include "student.h"
#include "writer.h"

int main(void)
{
   if ((mkfifo (FIFO, FILE_MODE) < 0) && (errno != EEXIST)) {
       if (errno == EACCES) {
           perror("Error: Permission denied");
       } else if (errno == EEXIST) {
           perror("Error: File already exists");
       } else if (errno == ENOENT) {
           perror("Error: Directory does not exist");
       } else {
           perror("Error: mkfifo failed");
       }
       exit(EXIT_FAILURE);
  
      fprintf(stderr, "can't create %s\n",FIFO);
      exit(1);
   }


   int readfd = open(FIFO, O_RDONLY);


   if (readfd == -1)
   {
       perror("open");
       exit(EXIT_FAILURE);
   }   
  
   struct API_Request message;

    int retries = 0;
   while(1)
   {
        ssize_t dataRead = read(readfd, &message, sizeof(message));

       if (dataRead == -1) {
           if (errno == EAGAIN || errno == EWOULDBLOCK)
           {
               retries++;
               printf("FIFO is empty, retrying... (%d)\n", retries);
           }
           else
           {
               perror("read");
               close(readfd);
               exit(EXIT_FAILURE);
           }
       }
       else if (dataRead > 0)
       {
           switch (message.api_type)
           {
           case 0:
               addStudent(message.data.api_add_student.rNo, message.data.api_add_student.name, message.data.api_add_student.cgpa, message.data.api_add_student.noOfSubjects);
               break;


           case 1:
               modifyStudent(message.data.api_modify_student.rNo, message.data.api_modify_student.cgpa);
               break;


           case 2:
               deleteStudent(message.data.api_delete_student.rNo);
               break;


           case 3:
               addCourse(message.data.api_add_course.rNo, message.data.api_add_course.courseCode, message.data.api_add_course.marks);
               break;


           case 4:
               modifyCourse(message.data.api_modify_course.rNo, message.data.api_modify_course.courseCode, message.data.api_modify_course.marks);
               break;


           case 5:
               deleteCourse(message.data.api_delete_course.rNo, message.data.api_delete_course.courseCode);
               break;


           default:
               printf("Unknown operation found");
               exit(1);
           }
           retries = 0;
       }
       else if (dataRead == 0)
       {
           // End-of-file (EOF) detected
           printf("FIFO writer closed, exiting.\n");
           break;
       }
   }

   // write to file
   writer();
   close(readfd);
   unlink(FIFO);


}
