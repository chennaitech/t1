
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>

int main()
{
    int s, n;
    char name[100], ip[100];
    struct sockaddr_in server;
    socklen_t len = sizeof(server);

    char cache_name[100] = "", cache_ip[100] = "";

    s = socket(AF_INET, SOCK_DGRAM, 0);

    server.sin_family = AF_INET;
    server.sin_port = htons(8080);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    struct timeval tv = {3, 0};
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    while (1)
    {
        printf("\nEnter domain (0 to exit): ");
        scanf("%99s", name);

        if (strcmp(name, "0") == 0)
            break;

        if (strcmp(name, cache_name) == 0)
        {
            printf("Cached IP: %s\n", cache_ip);
            continue;
        }

        if (strchr(name, '.') == NULL)
        {
            printf("Invalid domain format!\n");
            continue;
        }

        sendto(s, name, strlen(name)+1, 0,
               (struct sockaddr *)&server, len);

        n = recvfrom(s, ip, sizeof(ip)-1, 0,
                     (struct sockaddr *)&server, &len);

        if (n < 0)
        {
            printf("DNS Timeout!\n");
            continue;
        }

        ip[n] = '\0';

        if (strcmp(ip, "NXDOMAIN") == 0)
            printf("Domain not found!\n");
        else if (strcmp(ip, "INVALID") == 0)
            printf("Invalid domain!\n");
        else
        {
            printf("IP Address: %s\n", ip);
            strcpy(cache_name, name);
            strcpy(cache_ip, ip);
        }
    }

    close(s);
    return 0;
}
