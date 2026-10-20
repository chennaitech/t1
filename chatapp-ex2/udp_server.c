#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<arpa/inet.h>

#define PORT 8080

int main(){
    int s;
    struct sockaddr_in server,client;

    socklen_t len=sizeof(client);

    char roll[20],line[100],result[300];

    s=socket(AF_INET,SOCK_DGRAM,0);


    server.sin_family=AF_INET;
    server.sin_port=htons(PORT);
    server.sin_addr.s_addr=INADDR_ANY;

    bind(s,(struct sockaddr *)&server,sizeof(server));
    

    printf("Server running");
    fflush(stdout);

    while(1){

        memset(roll,0,sizeof(roll));

        recvfrom(s,roll,sizeof(roll)-1,0,(struct sockaddr *)&client,&len);

        strcpy(result,"Student not found");

        FILE *f=fopen("students.csv","r");

        if(f!=NULL)
         {
            while(fgets(line,sizeof(line),f))
            {
                char r[20],n[20],d[20],sem[20],cgpa[20];

                if(sscanf(line,"%19[^,],%19[^,],%19[^,],%19[^,],%19s",r,n,d,sem,cgpa)==5)
                {
                    if(strcmp(r,roll) ==0)
                {
                    sprintf(result,"roll:%s\nName:%s\n",r,n);
                    break;
                }
                }     
            }

            fclose(f);
        }

        sendto(s,result,strlen(result)+1,0,(struct sockaddr *)&client,len);
    }

    close(s);
    return 0;

    
}