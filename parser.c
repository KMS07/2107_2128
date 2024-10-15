#include "parser.h"
#include "client.h"
#include "fifo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>


int emptyLineCheck(char *line)
{
    // Empty line check
    if (*line == '\0')
        return 1;
    while (*line)
    {
        if (!isspace((unsigned char)*line))
        {
            return 0;
        }
        line++;
    }
    return 1;
}

void removeExtraAtEnd(char *str)
{ // Extra buffer at end of line is removed
    int length = strlen(str);
    while (length > 0 && isspace(str[length - 1]))
    {
        str[length - 1] = '\0';
        length--;
    }
}

void read_data(char *input_file)
{
    FILE *input_data;
    char line[400];

    if(openClientFIFO() == -1){
        printf("Server didnt open the fifo");
        return;
    } //openin the fifo here once to write till the end of program.

    input_data = fopen(input_file, "r");
    if (input_data == NULL)
    {
        perror("Error opening file");
        return;
    }

    int lineNo = 1;
    while(fgets(line, sizeof(line), input_data) && emptyLineCheck(line))
        lineNo++;
    int rNo, noOfSub;
    char name[nameSize];
    float cgpa;
    int numOfParams;

    int courseCode, marks;

    if (strncmp(line, "# initial database", strlen("# initial database")) == 0)
    {
        while (!feof(input_data))
        {
            if (strncmp(line, "# initial database", strlen("# initial database")) == 0)
            {
                fgets(line, sizeof(line), input_data);
                lineNo++;

                removeExtraAtEnd(line);
                while (strchr(line, '#') == NULL && !feof(input_data))
                {
                    removeExtraAtEnd(line);
                    if (emptyLineCheck(line))
                    {
                        if (!fgets(line, sizeof(line), input_data))
                            break;
                        lineNo++;
                        continue;
                    }
                    else if (sscanf(line, "%d,%199[^,],%f,%d%n", &rNo, name, &cgpa, &noOfSub, &numOfParams) == 4 && line[numOfParams] == '\0')
                    {
                        addStudent(rNo, name, cgpa, 0);
                        if (!fgets(line, sizeof(line), input_data))
                            break;
                        lineNo++;

                        int countCourses = 0;
                        int whiteSpaces = 0;
                        while ((sscanf(line, "%d,%d%n", &courseCode, &marks, &numOfParams) == 2 && countCourses <= noOfSub) || emptyLineCheck(line))
                        {
                            if (emptyLineCheck(line))
                            {
                                if (!fgets(line, sizeof(line), input_data))
                                    break;
                                lineNo++;
                                whiteSpaces++;
                                continue;
                            }
                            removeExtraAtEnd(line);
                            if (line[numOfParams] != '\0')
                            {
                                fprintf(stderr, "Wrong number of parameters of add course at line %d\n", lineNo);
                            }
                            addCourse(rNo, courseCode, marks);
                            countCourses++;

                            if (!fgets(line, sizeof(line), input_data))
                                break;

                            lineNo++;
                        }

                        if (countCourses < noOfSub)
                        {
                            fprintf(stderr, "Number of courses are less than provided for the student at line %d\n", lineNo - (countCourses + whiteSpaces + 1));
                        }
                    }
                    else
                    {

                        fprintf(stderr, "Wrong format of values in initial database for add student at line %d\n", lineNo);
                        fgets(line, sizeof(line), input_data);
                        lineNo++;
                    }
                }
            }
            else if (strncmp(line, "# add student", strlen("# add student")) == 0)
            {

                while (fgets(line, sizeof(line), input_data) != NULL)
                {
                    if (emptyLineCheck(line))
                    {
                        lineNo++;
                        continue;
                    }
                    if (strchr(line, '#') != NULL)
                    {
                        lineNo++;
                        break;
                    }

                    lineNo++;
                    removeExtraAtEnd(line);

                    if (sscanf(line, "%d,%199[^,],%f,%d%n", &rNo, name, &cgpa, &noOfSub, &numOfParams) == 4 && line[numOfParams] == '\0')
                    {
                        // Call AddStudent API
                        addStudent(rNo, name, cgpa, 0);
                    }
                    else
                    {
                        fprintf(stderr, "Wrong format of values for add student at line %d\n", lineNo);
                    }
                }
            }
            else if (strncmp(line, "# add course", strlen("# add course")) == 0)
            {
                while (fgets(line, sizeof(line), input_data) != NULL)
                {
                    if (emptyLineCheck(line))
                    {
                        lineNo++;
                        continue;
                    }
                    if (strchr(line, '#') != NULL)
                    {
                        lineNo++;
                        break;
                    }
                    lineNo++;
                    removeExtraAtEnd(line);

                    if (sscanf(line, "%d,%d,%d%n", &rNo, &courseCode, &marks, &numOfParams) == 3 && line[numOfParams] == '\0')
                    {
                        // Call AddCourse API
                        addCourse(rNo, courseCode, marks);
                    }
                    else
                    {
                        fprintf(stderr, "Wrong format of values for add course at line %d\n", lineNo);
                    }
                }
            }
            else if (strncmp(line, "# modify student", strlen("# modify student")) == 0)
            {
                while (fgets(line, sizeof(line), input_data) != NULL)
                {
                    if (emptyLineCheck(line))
                    {
                        lineNo++;
                        continue;
                    }
                    if (strchr(line, '#') != NULL)
                    {
                        lineNo++;
                        break;
                    }
                    lineNo++;
                    removeExtraAtEnd(line);

                    if (sscanf(line, "%d,%f%n", &rNo, &cgpa, &numOfParams) == 2 && line[numOfParams] == '\0')
                    {
                        // Call ModifyStudent API
                        modifyStudent(rNo, cgpa);
                    }
                    else
                    {
                        fprintf(stderr, "Wrong format of values for modify student at line %d\n", lineNo);
                    }
                }
            }
            else if (strncmp(line, "# modify course", strlen("# modify course")) == 0)
            {
                while (fgets(line, sizeof(line), input_data) != NULL)
                {
                    if (emptyLineCheck(line))
                    {
                        lineNo++;
                        continue;
                    }
                    if (strchr(line, '#') != NULL)
                    {
                        lineNo++;
                        break;
                    }

                    lineNo++;
                    removeExtraAtEnd(line);

                    if (sscanf(line, "%d,%d,%d%n", &rNo, &courseCode, &marks, &numOfParams) == 3 && line[numOfParams] == '\0')
                    {
                        // Call ModifyCourse API
                        modifyCourse(rNo, courseCode, marks);
                    }
                    else
                    {
                        fprintf(stderr, "Wrong format of values for modify course at line %d\n", lineNo);
                    }
                }
            }
            else if (strncmp(line, "# delete student", strlen("# delete student")) == 0)
            {
                while (fgets(line, sizeof(line), input_data) != NULL )
                {
                    if (emptyLineCheck(line))
                    {
                        lineNo++;
                        continue;
                    }
                    if (strchr(line, '#') != NULL)
                    {
                        lineNo++;
                        break;
                    }
                    lineNo++;
                    removeExtraAtEnd(line);
                    if (sscanf(line, "%d%n", &rNo, &numOfParams) == 1 && line[numOfParams] == '\0')
                    {
                        // Call DeleteStudent API
                        deleteStudent(rNo);
                    }
                    else
                    {
                        fprintf(stderr, "Wrong format of values for delete student at line %d\n", lineNo);
                    }
                }
            }
            else if (strncmp(line, "# delete course", strlen("# delete course")) == 0)
            {
                while (fgets(line, sizeof(line), input_data) != NULL)
                {
                    if (emptyLineCheck(line))
                    {
                        lineNo++;
                        continue;
                    }
                    if (strchr(line, '#') != NULL)
                    {
                        lineNo++;
                        break;
                    }
                    lineNo++;
                    removeExtraAtEnd(line);
                    if (sscanf(line, "%d,%d%n", &rNo, &courseCode, &numOfParams) == 2 && line[numOfParams] == '\0')
                    {
                        // Call DeleteStudent API
                        deleteCourse(rNo, courseCode);
                    }
                    else
                    {
                        fprintf(stderr, "Wrong format of values for delete course at line %d\n", lineNo);
                    }
                }
            }
            else
            {
                fprintf(stderr, "Invalid operation\n");
            }
        }
        printf("File reading done\n");
    }
    else
    {
        fprintf(stderr, "File incorrect syntax\n");
    }
    close(writefd);// closing client side write file descriptor
    fclose(input_data);
}