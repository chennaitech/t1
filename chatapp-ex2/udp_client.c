#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<arpa/inet.h>

#define PORT 8080

int main()
{
    int c;
    struct sockaddr_in server;

    //extra 
    socklen_t len=sizeof(server);
    
    char roll[20],result[200];

    c=socket(AF_INET,SOCK_DGRAM,0);


    server.sin_family=AF_INET;
    server.sin_port=htons(PORT);
    server.sin_addr.s_addr=inet_addr("127.0.0.1");

    printf("Enter the roll: ");
    scanf("%19s",roll);

    sendto(c,roll,strlen(roll)+1,0,(struct sockaddr *)&server,len);

    recvfrom(c,result,sizeof(result)-1,0,(struct sockaddr *)&server,&len);

    printf("%s\n",result);

    close(c);

    return 0;


    
}