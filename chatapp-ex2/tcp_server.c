
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

int main()
{
    int s, c;
    struct sockaddr_in server = {0};
    char roll[20], line[100], result[200];

    s = socket(AF_INET, SOCK_STREAM, 0);

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = INADDR_ANY;

    bind(s, (struct sockaddr *)&server, sizeof(server));
    listen(s, 5);

    printf("TCP Server Running!!\n");
    fflush(stdout);

    while (1) {
        c = accept(s, NULL, NULL);

        memset(roll, 0, sizeof(roll));
        recv(c, roll, sizeof(roll) - 1, 0);

        strcpy(result, "Student not found!!");

        FILE *f = fopen("students.csv", "r");

        if (f != NULL) {
            while (fgets(line, sizeof(line), f)) {
                char r[20], n[20], d[20], sem[20], cgpa[20];

                if (sscanf(line, "%19[^,],%19[^,],%19[^,],%19[^,],%19s",
                           r, n, d, sem, cgpa) == 5) {
                    if (strcmp(r, roll) == 0) {
                        sprintf(result,
                                "Name:%s\nDept:%s\nSemester:%s\nCGPA:%s\n",
                                n, d, sem, cgpa);
                        break;
                    }
                }
            }
            fclose(f);
        }

        send(c, result, strlen(result) + 1, 0);
        close(c);
    }

    close(s);
    return 0;
}
