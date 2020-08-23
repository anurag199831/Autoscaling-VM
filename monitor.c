#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <libvirt/libvirt.h>
#include <netdb.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#define MAX 80 
#define PORT 8100
#define SA struct sockaddr 

int vm_limit=3;

double avg_cpu_util(virConnectPtr conn)
{
    virDomainPtr dom;
    int num_of_dom;
    num_of_dom=virConnectNumOfDomains(conn);
    // printf("%d",num_of_dom);
    char str[3]="vm1";
    double util,avg_util=0;
    for(int i=1;i<=num_of_dom;i++)
    {
        str[2]=i+48;
        dom= virDomainLookupByName(conn,str);
        char *s=virDomainGetName(dom);
        int nparams;
        nparams=virDomainGetCPUStats(dom, NULL, 0, -1, 1, 0);
        virTypedParameterPtr params;
        params = calloc(nparams, sizeof(virTypedParameter));
        unsigned long long start,end;
        virDomainGetCPUStats(dom, params, nparams, -1, 1, 0);
        start=params->value.ul;
        sleep(2);
        virDomainGetCPUStats(dom, params, nparams, -1, 1, 0);
        end=params->value.ul;
        util=(double)(end-start)/(double)(2*10000000);
        printf("%s -> %.2lf % \n",s,util);
        avg_util+=util;
    }
    avg_util/=num_of_dom;
    if(num_of_dom==0)
    return 0;
    return avg_util;
}

void increase_vm(virConnectPtr conn)
{
    virDomainPtr dom;
    int num_of_dom;
    num_of_dom=virConnectNumOfDomains(conn);
    if(num_of_dom==vm_limit)
    return ;
    char str[3]="vm1";
    str[2]=num_of_dom+48+1;
    dom=virDomainLookupByName(conn,str);
    virDomainCreate(dom);

}

void decrease_vm(virConnectPtr conn)
{
    virDomainPtr dom;
    int num_of_dom;
    num_of_dom=virConnectNumOfDomains(conn);
    if(num_of_dom==1)
    return ;
    char str[3]="vm1";
    str[2]=num_of_dom+48;
    dom=virDomainLookupByName(conn,str);
    virDomainShutdown(dom);
}

// Function designed for chat between client and server. 
void func(int sockfd) 
{ 
    virConnectPtr conn;

    conn = virConnectOpen("qemu:///system");
    if (conn == NULL) {
        fprintf(stderr, "Failed to open connection to qemu:///system\n");
        return ;
    }
    double avg_util;//=avg_cpu_util(conn);
	char buff[MAX]; 
	int n; 
	// infinite loop for chat 
	for (;;) { 
		bzero(buff, MAX); 

		// read the message from client and copy it in buffer 
		read(sockfd, buff, sizeof(buff)); 
		// print buffer which contains the client contents 
		// printf("From client: %s\nTo client : ", buff); 
		bzero(buff, MAX); 
        avg_util=avg_cpu_util(conn);
        if(avg_util>90)
        {
            increase_vm(conn);
            sleep(2);
            buff[0]='+';
            buff[1]='1';
            buff[2]='\n';
        }
        else if(avg_util<30)
        {
            buff[0]='-';
            buff[1]='1';
            buff[2]='\n';
        }
        else 
        {
            buff[0]='0';
            buff[1]='0';
            buff[2]='\n';
        }

		// and send that buffer to client 
		write(sockfd, buff, sizeof(buff)); 
        printf("%s\n",buff);
        if(buff[0]=='-' && buff[1]=='1')
        {
            read(sockfd, buff, sizeof(buff));
            decrease_vm(conn);
            sleep(2);

        }
        else read(sockfd, buff, sizeof(buff));
        printf("%s\n",buff);

	} 
    virConnectClose(conn);
}

int main(int argc, char *argv[])
{
    int sockfd, connfd, len; 
	struct sockaddr_in servaddr, cli; 

	// socket create and verification 
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1) { 
		printf("socket creation failed...\n"); 
		exit(0); 
	} 
	else
		printf("Socket successfully created..\n"); 
	bzero(&servaddr, sizeof(servaddr)); 

	// assign IP, PORT 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
	servaddr.sin_port = htons(PORT); 

	// Binding newly created socket to given IP and verification 
	if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) { 
		printf("socket bind failed...\n"); 
		exit(0); 
	} 
	else
		printf("Socket successfully binded..\n"); 

	// Now server is ready to listen and verification 
	if ((listen(sockfd, 5)) != 0) { 
		printf("Listen failed...\n"); 
		exit(0); 
	} 
	else
		printf("Server listening..\n"); 
	len = sizeof(cli); 

	// Accept the data packet from client and verification 
	connfd = accept(sockfd, (SA*)&cli, &len); 
	if (connfd < 0) { 
		printf("server acccept failed...\n"); 
		exit(0); 
	} 
	else
		printf("server acccept the client...\n"); 

	// Function for chatting between client and server 
	func(connfd); 

	// After chatting close the socket 
	close(sockfd); 



    // virConnectPtr conn;

    // conn = virConnectOpen("qemu:///system");
    // if (conn == NULL) {
    //     fprintf(stderr, "Failed to open connection to qemu:///system\n");
    //     return 1;
    // }
    // double avg_util=avg_cpu_util(conn);
    // printf("%.2lf\n",avg_util);
    
    return 0;
}