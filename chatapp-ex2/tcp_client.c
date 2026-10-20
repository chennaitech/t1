
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 8080

int main()
{
    int c, n;
    struct sockaddr_in server = {0};
    char roll[20], result[300] = {0};
    struct timespec start, end;
    double rtt, throughput;

    c = socket(AF_INET, SOCK_STREAM, 0);

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    connect(c, (struct sockaddr *)&server, sizeof(server));

    printf("Enter Roll Number: ");
    scanf("%19s", roll);

    clock_gettime(CLOCK_MONOTONIC, &start);

    send(c, roll, strlen(roll) + 1, 0);
    n = recv(c, result, sizeof(result) - 1, 0);

    clock_gettime(CLOCK_MONOTONIC, &end);

    if (n > 0) {
        result[n] = '\0';

        rtt = (end.tv_sec - start.tv_sec) * 1000.0
            + (end.tv_nsec - start.tv_nsec) / 1000000.0;

        throughput = n * 8.0 / (rtt / 1000.0);

        printf("\n%s\n", result);
        printf("RTT: %.3f ms\n", rtt);
        printf("Estimated Latency: %.3f ms\n", rtt / 2);

        if (rtt > 0)
            printf("Throughput: %.2f bps\n", throughput);
    } else {
        printf("No response received!\n");
    }

    close(c);
    return 0;
}