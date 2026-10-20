
#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<time.h>

#define PORT 8080

int main()
{
    int c,n;
    struct sockaddr_in server;
    socklen_t len=sizeof(server);

    char roll[20],result[200];
    struct timespec start,end;
    double rtt,latency,throughput;

    c=socket(AF_INET,SOCK_DGRAM,0);

    server.sin_family=AF_INET;
    server.sin_port=htons(PORT);
    server.sin_addr.s_addr=inet_addr("127.0.0.1");

    printf("Enter the roll: ");
    scanf("%19s",roll);

    clock_gettime(CLOCK_MONOTONIC,&start);

    sendto(c,roll,strlen(roll)+1,0,
           (struct sockaddr *)&server,len);

    n=recvfrom(c,result,sizeof(result)-1,0,
               (struct sockaddr *)&server,&len);

    clock_gettime(CLOCK_MONOTONIC,&end);

    if(n > 0)
    {
        result[n]='\0';

        rtt=(end.tv_sec-start.tv_sec)*1000.0+
            (end.tv_nsec-start.tv_nsec)/1000000.0;

        latency=rtt/2;

        printf("\n%s\n",result);
        printf("RTT: %.3f ms\n",rtt);
        printf("Latency: %.3f ms\n",latency);

        if(rtt > 0)
        {
            throughput=n*8.0/(rtt/1000.0);
            printf("Throughput: %.2f bps\n",throughput);
        }
    }
    else
        printf("No response received\n");

    close(c);
    return 0;
}
