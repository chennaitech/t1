
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <time.h>

int main()
{
    int s, n;
    char name[100], ip[100], cname[100] = "", cip[100] = "";
    struct sockaddr_in server = {0};
    socklen_t len = sizeof(server);
    struct timespec start, end;
    struct timeval tv = {3, 0};
    double rtt, throughput, t;

    s = socket(AF_INET, SOCK_DGRAM, 0);
    server.sin_family = AF_INET;
    server.sin_port = htons(8080);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    while (1)
    {
        printf("\nEnter domain (0 to exit): ");
        scanf("%99s", name);
        if (strcmp(name, "0") == 0) break;

        clock_gettime(CLOCK_MONOTONIC, &start);

        if (strcmp(name, cname) == 0)
        {
            clock_gettime(CLOCK_MONOTONIC, &end);
            t = (end.tv_sec-start.tv_sec)*1000.0 +
                (end.tv_nsec-start.tv_nsec)/1e6;
            printf("Cached IP: %s\nCache Time: %.3f ms\n", cip, t);
            continue;
        }

        if (strchr(name, '.') == NULL ||
            name[0] == '.' || name[strlen(name)-1] == '.')
        {
            printf("Invalid domain format!\n");
            continue;
        }

        clock_gettime(CLOCK_MONOTONIC, &start);
        sendto(s, name, strlen(name)+1, 0,
               (struct sockaddr *)&server, len);

        n = recvfrom(s, ip, 99, 0, NULL, NULL);
        clock_gettime(CLOCK_MONOTONIC, &end);

        if (n < 0)
        {
            printf("DNS Timeout!\n");
            continue;
        }

        ip[n] = '\0';
        rtt = (end.tv_sec-start.tv_sec)*1000.0 +
              (end.tv_nsec-start.tv_nsec)/1e6;

        printf("RTT: %.3f ms\nLatency: %.3f ms\n", rtt, rtt/2);

        if (rtt > 0)
        {
            throughput = n*8.0/(rtt/1000.0);
            printf("Throughput: %.2f bps\n", throughput);
        }

        if (strcmp(ip, "NXDOMAIN") == 0)
            printf("Domain not found!\n");
        else if (strcmp(ip, "INVALID") == 0)
            printf("Invalid domain!\n");
        else
        {
            printf("IP Address: %s\n", ip);
            strcpy(cname, name);
            strcpy(cip, ip);
        }
    }

    close(s);
    return 0;
}
