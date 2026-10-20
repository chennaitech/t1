
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main()
{
    int s;
    char name[100], ip[100];
    struct sockaddr_in server, client;
    socklen_t len = sizeof(client);

    char *domain[] = {"google.com", "youtube.com", "example.com"};
    char *addr[] = {"142.250.0.1", "142.250.0.2", "93.184.216.34"};

    s = socket(AF_INET, SOCK_DGRAM, 0);

    server.sin_family = AF_INET;
    server.sin_port = htons(8080);
    server.sin_addr.s_addr = INADDR_ANY;

    bind(s, (struct sockaddr *)&server, sizeof(server));
    printf("DNS Server Running\n");

    while (1)
    {
        memset(name, 0, sizeof(name));
        len = sizeof(client);

        recvfrom(s, name, sizeof(name)-1, 0,
                 (struct sockaddr *)&client, &len);

        strcpy(ip, "NXDOMAIN");

        if (strchr(name, '.') == NULL)
            strcpy(ip, "INVALID");
        else
            for (int i = 0; i < 3; i++)
                if (strcmp(name, domain[i]) == 0)
                    strcpy(ip, addr[i]);

        sendto(s, ip, strlen(ip)+1, 0,
               (struct sockaddr *)&client, len);
    }

    close(s);
    return 0;
}
