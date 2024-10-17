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

#include "socket.h"
#include "student.h"
#include "writer.h"

int main()
{
	int	i, maxi, maxfd, listenfd, connfd, sockfd;
	int	nready, client[FD_SETSIZE];
	ssize_t	n;
	fd_set rset, allset;
	socklen_t clilen;
	struct sockaddr_in	cliaddr, servaddr;

	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if (listenfd < 0) {
        perror("socket error");
        exit(EXIT_FAILURE);
    }

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(SERV_PORT);
    
	if (bind(listenfd, (const struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        perror("bind error");
        close(listenfd);
        exit(EXIT_FAILURE);
    }

	if (listen(listenfd, LISTENQ) < 0) {
        perror("listen error");
        close(listenfd);
        exit(EXIT_FAILURE);
    }

	maxfd = listenfd;			/* initialize */
	maxi = -1;					/* index into client[] array */
	for (i = 0; i < FD_SETSIZE; i++)
		client[i] = -1;			/* -1 indicates available entry */
	FD_ZERO(&allset);
	FD_SET(listenfd, &allset);

	for ( ; ; ) {
		rset = allset;	
		nready = select(maxfd+1, &rset, NULL, NULL, NULL);
        if (nready < 0) {
            perror("select error");
            exit(1);
        }

		if (FD_ISSET(listenfd, &rset)) {	/* new client connection */
			clilen = sizeof(cliaddr);
			connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen);
			printf("Server ip: %s\n",inet_ntoa(servaddr.sin_addr));

            if (connfd < 0) {
                perror("accept error");
                continue;
            }
			
			for (i = 0; i < FD_SETSIZE; i++)
				if (client[i] < 0) {
					client[i] = connfd;	/* save descriptor */
					break;
				}
			if (i == FD_SETSIZE) {
                fprintf(stderr, "Too many clients\n");
                close(connfd); // close the new connection 
                continue;
            }

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
				n = recv(sockfd, &message, sizeof(message), MSG_WAITALL);
				if ( n == 0) {
					/*connection closed by client */
					printf("Client disconnected\n");
					close(sockfd); // close conenction
					FD_CLR(sockfd, &allset); // Remove the client from the set
					client[i] = -1;
				} else if(n < 0){
                    if (errno == EAGAIN || errno == EWOULDBLOCK) {
                        // non-blocking mode and no data available
                        continue;
                    } else if (errno == EINTR) {
                        // retry when interrupted by signal
                        continue;
                    } else if (errno == ECONNRESET) {
                        printf("Connection reset by client\n");
                        close(sockfd);
                        FD_CLR(sockfd, &allset); // Remove the client from the set
                        client[i] = -1;
                        continue;
                    } else {
                        perror("read error");
                        close(sockfd);
                        FD_CLR(sockfd, &allset); // Remove the client from the set
                        client[i] = -1;
                        continue;
                    }
				}else{
                    // Process the received message
                    switch (message.api_type) {
                        case 0:
                            addStudent(message.data.api_add_student.rNo, message.data.api_add_student.name, message.data.api_add_student.cgpa, message.data.api_add_student.noOfSubjects, (struct API_Response *)&response);
                            break;

                        case 1:
                            modifyStudent(message.data.api_modify_student.rNo, message.data.api_modify_student.cgpa, (struct API_Response *)&response);
                            break;

                        case 2:
                            deleteStudent(message.data.api_delete_student.rNo, (struct API_Response *)&response);
                            break;

                        case 3:
                            addCourse(message.data.api_add_course.rNo, message.data.api_add_course.courseCode, 
                                          message.data.api_add_course.marks, (struct API_Response *)&response);
                            break;

                        case 4:
                            modifyCourse(message.data.api_modify_course.rNo, message.data.api_modify_course.courseCode, 
                                             message.data.api_modify_course.marks, (struct API_Response *)&response);
                            break;

                        case 5:
                            deleteCourse(message.data.api_delete_course.rNo, message.data.api_delete_course.courseCode, (struct API_Response *)&response);
                            break;

                        default:
                            response.status = 1;
                            strcpy(response.message, "Unknown operation");
                            break;
                    }
                    
                    writer();
                    // Send the response back to the client
                    if (send(sockfd, &response, sizeof(response), 0) < 0) {
                        perror("send error");
                        close(sockfd);
                        FD_CLR(sockfd, &allset);
                        client[i] = -1;
                    }
                }

				if (--nready <= 0)
					break;				/* no more readable descriptors */
			}
		}
	}
}