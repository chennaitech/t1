
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

int main()
{
    int c;
    struct sockaddr_in server = {0};
    char roll[20], result[300] = {0};

    c = socket(AF_INET, SOCK_STREAM, 0);

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    connect(c, (struct sockaddr *)&server, sizeof(server));

    printf("Enter Roll Number: ");
    scanf("%19s", roll);

    send(c, roll, strlen(roll) + 1, 0);

    recv(c, result, sizeof(result) -1, 0);

    printf("\n%s\n", result);

    close(c);

    return 0;
}
