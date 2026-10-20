
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

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
    int s,c,n,i,error=0;
    struct sockaddr_in server,client;
    socklen_t len=sizeof(client);
    char frame[500],temp[500],ack[100];

    s=socket(AF_INET,SOCK_STREAM,0);

    server.sin_family=AF_INET;
    server.sin_port=htons(PORT);
    server.sin_addr.s_addr=INADDR_ANY;

    bind(s,(struct sockaddr *)&server,sizeof(server));
    listen(s,5);

    printf("Receiver running...\n");

    c=accept(s,(struct sockaddr *)&client,&len);

    n=recv(c,frame,sizeof(frame)-1,0);

    if(n>0)
    {
        frame[n]='\0';
        printf("Received frame: %s\n",frame);

        strcpy(temp,frame);
        divide(temp);

        for(i=strlen(temp)-strlen(POLY)+1; i<strlen(temp); i++)
            if(temp[i]!='0')
                error=1;

        if(error)
        {
            strcpy(ack,"ERROR DETECTED");
            printf("Frame contains errors!\n");
        }
        else
        {
            strcpy(ack,"Frame is error-free");
            printf("Frame is error-free!\n");
        }

        send(c,ack,strlen(ack)+1,0);
    }

    close(c);
    close(s);
    return 0;
}
