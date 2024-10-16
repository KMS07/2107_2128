#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <strings.h>

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
int
main(int argc, char **argv)
{
	int					i, maxi, maxfd, listenfd, connfd, sockfd;
	int					nready, client[FD_SETSIZE];
	ssize_t				n;
	fd_set				rset, allset;
	char				buf[MAXLINE];
	socklen_t			clilen;
	struct sockaddr_in	cliaddr, servaddr;

	listenfd = socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(SERV_PORT);

	bind(listenfd, (SA *) &servaddr, sizeof(servaddr));

	listen(listenfd, LISTENQ);

	maxfd = listenfd;			/* initialize */
	maxi = -1;					/* index into client[] array */
	for (i = 0; i < FD_SETSIZE; i++)
		client[i] = -1;			/* -1 indicates available entry */
	FD_ZERO(&allset);
	FD_SET(listenfd, &allset);

	for ( ; ; ) {
		rset = allset;	
		nready = select(maxfd+1, &rset, NULL, NULL, NULL);

		if (FD_ISSET(listenfd, &rset)) {	/* new client connection */
			clilen = sizeof(cliaddr);
			connfd = accept(listenfd, (SA *) &cliaddr, &clilen);

			for (i = 0; i < FD_SETSIZE; i++)
				if (client[i] < 0) {
					client[i] = connfd;	/* save descriptor */
					break;
				}
			if (i == FD_SETSIZE)
				err_quit("too many clients");

			FD_SET(connfd, &allset);	/* add new descriptor to set */
			if (connfd > maxfd)
				maxfd = connfd;			/* for select */
			if (i > maxi)
				maxi = i;				/* max index in client[] array */

			if (--nready <= 0)
				continue;				/* no more readable descriptors */
		}

		for (i = 0; i <= maxi; i++) {	/* check all clients for data */
			if ( (sockfd = client[i]) < 0)
				continue;
			if (FD_ISSET(sockfd, &rset)) {
                struct API_Request message;
                struct API_Response response;
				if ( (n = read(sockfd,&message ,sizeof(message))) == 0) {
						/*4connection closed by client */
					close(sockfd);
					FD_CLR(sockfd, &allset);
					client[i] = -1;
				} else{
                    // Process the received message
                    switch (message.api_type) {
                        case 0:
                            if (addStudent(message.data.api_add_student.rNo, message.data.api_add_student.name,
                                           message.data.api_add_student.cgpa, message.data.api_add_student.noOfSubjects)) {
                                response.status = 0;
                                strcpy(response.message, "Student added successfully");
                            } else {
                                response.status = 1;
                                strcpy(response.message, "Failed to add student");
                            }
                            break;

                        case 1:
                            if (modifyStudent(message.data.api_modify_student.rNo, message.data.api_modify_student.cgpa)) {
                                response.status = 0;
                                strcpy(response.message, "Student modified successfully");
                            } else {
                                response.status = 1;
                                strcpy(response.message, "Failed to modify student");
                            }
                            break;

                        case 2:
                            if (deleteStudent(message.data.api_delete_student.rNo)) {
                                response.status = 0;
                                strcpy(response.message, "Student deleted successfully");
                            } else {
                                response.status = 1;
                                strcpy(response.message, "Failed to delete student");
                            }
                            break;

                        case 3:
                            if (addCourse(message.data.api_add_course.rNo, message.data.api_add_course.courseCode, 
                                          message.data.api_add_course.marks)) {
                                response.status = 0;
                                strcpy(response.message, "Course added successfully");
                            } else {
                                response.status = 1;
                                strcpy(response.message, "Failed to add course");
                            }
                            break;

                        case 4:
                            if (modifyCourse(message.data.api_modify_course.rNo, message.data.api_modify_course.courseCode, 
                                             message.data.api_modify_course.marks)) {
                                response.status = 0;
                                strcpy(response.message, "Course modified successfully");
                            } else {
                                response.status = 1;
                                strcpy(response.message, "Failed to modify course");
                            }
                            break;

                        case 5:
                            if (deleteCourse(message.data.api_delete_course.rNo, message.data.api_delete_course.courseCode)) {
                                response.status = 0;
                                strcpy(response.message, "Course deleted successfully");
                            } else {
                                response.status = 1;
                                strcpy(response.message, "Failed to delete course");
                            }
                            break;

                        default:
                            response.status = 1;
                            strcpy(response.message, "Unknown operation");
                            break;
                    }

                    // Send the response back to the client
                    send(sockfd, &response, sizeof(response), 0);
                }

				if (--nready <= 0)
					break;				/* no more readable descriptors */
			}
		}
	}
}