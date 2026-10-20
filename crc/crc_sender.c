
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 8080
#define POLY "10011"

void divide(char *data)
{
    int i,j;

    for(i=0; i<=strlen(data)-strlen(POLY); i++)
        if(data[i]=='1')
            for(j=0; j<strlen(POLY); j++)
                data[i+j]=(data[i+j]==POLY[j])?'0':'1';
}

int main()
{
    int c,i,j,k=0,choice;
    struct sockaddr_in server;
    char data[50],binary[400]="",temp[500],rem[10],frame[500],ack[100];
    struct timespec start,end;
    double rtt,latency,throughput,ptime;

    printf("Enter data: ");
    scanf("%49s",data);

    /* Convert data to binary */
    for(i=0; i<strlen(data); i++)
        for(j=7; j>=0; j--)
            binary[k++]=((data[i]>>j)&1)+'0';

    binary[k]='\0';

    printf("Binary data: %s\n",binary);

    /* Calculate CRC */
    strcpy(temp,binary);
    for(i=0; i<strlen(POLY)-1; i++)
        strcat(temp,"0");

    clock_gettime(CLOCK_MONOTONIC,&start);
    divide(temp);
    clock_gettime(CLOCK_MONOTONIC,&end);

    ptime=(end.tv_sec-start.tv_sec)*1000.0+
          (end.tv_nsec-start.tv_nsec)/1000000.0;

    strcpy(rem,temp+strlen(binary));
    printf("CRC bits: %s\n",rem);

    strcpy(frame,binary);
    strcat(frame,rem);

    printf("Transmitted frame: %s\n",frame);
    printf("CRC processing time: %.6f ms\n",ptime);

    /* Manually introduce error */
    printf("Inject error? (1-Yes, 0-No): ");
    scanf("%d",&choice);

    if(choice==1)
    {
        frame[0]=(frame[0]=='0')?'1':'0';
        printf("Corrupted frame: %s\n",frame);
    }

    /* TCP connection */
    c=socket(AF_INET,SOCK_STREAM,0);

    server.sin_family=AF_INET;
    server.sin_port=htons(PORT);
    server.sin_addr.s_addr=inet_addr("127.0.0.1");

    connect(c,(struct sockaddr *)&server,sizeof(server));

    clock_gettime(CLOCK_MONOTONIC,&start);

    send(c,frame,strlen(frame)+1,0);
    i=recv(c,ack,sizeof(ack)-1,0);

    clock_gettime(CLOCK_MONOTONIC,&end);

    if(i>0)
    {
        ack[i]='\0';
        printf("Receiver: %s\n",ack);

        rtt=(end.tv_sec-start.tv_sec)*1000.0+
            (end.tv_nsec-start.tv_nsec)/1000000.0;

        latency=rtt/2;

        printf("RTT: %.3f ms\n",rtt);
        printf("Estimated latency: %.3f ms\n",latency);

        if(rtt>0)
        {
            throughput=strlen(frame)*8.0/(rtt/1000.0);
            printf("Throughput: %.2f bps\n",throughput);
        }
    }

    close(c);
    return 0;
}
