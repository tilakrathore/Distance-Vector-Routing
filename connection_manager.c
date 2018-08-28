/**
 /* @section DESCRIPTION
 *
 * Connection Manager listens for incoming connections/messages from the
 * controller and other routers and calls the desginated handlers.
 */

#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <unistd.h>
#include <string.h>
#include <ifaddrs.h>
#include <netdb.h>
#include<inttypes.h>


#include "../include/connection_manager.h"
#include "../include/global.h"
#include "../include/control_handler.h"
#include "../include/deserialize.h"
#include "../include/crash.h"

#define increment 0x0C 

fd_set master_list, watch_list;
int head_fd;
int fag=0;
struct timeval timer;

void main_loop()
{
    printf("MAIN LOOP STARTED \n");
    int selret, sock_index, fdaccept;

    while(TRUE){
        watch_list = master_list;
        if(fag == 0)
        {
		printf("FLAG = 0\n");
        	selret = select(head_fd+1, &watch_list, NULL, NULL, NULL);
        }
        else
        {	
		printf("FLAG = 1\n");
        	//timer.tv_sec = (long)period;  
                //timer.tv_usec = 0;     
		selret = select(head_fd+1, &watch_list, NULL, NULL, &timer);	
		if(selret == 0){
        		// timeout has occured
        		printf("If Time out send periodic updates\n");
               		send_periodic_updates();        
        		timer.tv_sec = (long)period;
        		timer.tv_usec = 0;
        	}
        }

        if(selret < 0)
            ERROR("select failed.\n");
            

        /* Loop through file descriptors to check which ones are ready */
        for(sock_index=0; sock_index<=head_fd; sock_index+=1){

            if(FD_ISSET(sock_index, &watch_list)){

                /* control_socket */
                if(sock_index == control_socket){
		    printf("CONTROL SOCKET HANDLER\n\n");
                    fdaccept = new_control_conn(sock_index);

                    /* Add to watched socket list */
                    FD_SET(fdaccept, &master_list);
                    if(fdaccept > head_fd) head_fd = fdaccept;
                }

                /* router_socket */
                else if(sock_index == router_socket){
                    //call handler that will call recvfrom() .....
		    printf("ROUTER SOCKET HANDLER\n");
                    struct sockaddr_in their_addr;
                    socklen_t addr_len;
                    addr_len = sizeof their_addr;
                    int numbytes;
                    char recvbuf[1000];
                    if ((numbytes = recvfrom(router_socket, recvbuf, 1000-1 , 0, (struct sockaddr *)&their_addr, &addr_len)) == -1) {
                    perror("recvfrom\n");
                    exit(1);
                    }
		    printf("ROUTER ALGO UPDATE\n\n");
                    deserialize_updates(recvbuf);
                           
                }

                /* data_socket */
                else if(sock_index == data_socket){
		    printf("Handle Data Socket\n");
                    //new_data_conn(sock_index);
                }

                /* Existing connection */
                else{
                    if(isControl(sock_index)){
			printf("HANDLE EXISTING CONNECTION\n");
                        if(!control_recv_hook(sock_index)) FD_CLR(sock_index, &master_list);
                    }
                    //else if isData(sock_index);
                    else ERROR("Unknown socket index\n");
                }
            }
        }
    }
}


void init()
{
    //getIP(myip);
    //printf("my ip address:%s\n",myip);
    control_socket = create_control_sock();

    //router_socket and data_socket will be initialized after INIT from controller

    FD_ZERO(&master_list);
    FD_ZERO(&watch_list);

    /* Register the control socket */
    FD_SET(control_socket, &master_list);
    if(control_socket>head_fd){
    	head_fd = control_socket;
    }
	
	/*intialize cost matrix*/
	int i=0;
	int j=0;
	printf("Cost Matrix\n");
	for(i=0;i<5;i++){
		for(j=0;j<5;j++){
			costmatrix[i][j]=65535;
			printf("%d\n  ", costmatrix[i][j]);
		}
		printf("\n");
	}
    main_loop();
}

void init_router_socket()
{
	//Initialize Router socket
	printf("ROUTER SOCKET INITIALIZED\n");
	router_socket = create_router_sock();
	
	/* Register the router socket */
	FD_SET(router_socket, &master_list);
	if(router_socket>head_fd){
    	head_fd = router_socket;
	printf("Router Socket no: %d\n",head_fd);
    }
	fag = 1;
	timer.tv_sec = (long)period;  
        timer.tv_usec = 0;     
        // main_loop();	
	//send_periodic_updates();
	return;	
}

void send_periodic_updates(){

	//send periodic distance vector updates
	printf("PERIODIC UPDATE 1\n");
	uint16_t a;
	int buffersize = 8+12*num_routers;
	char *buffer = (char *) malloc(sizeof(char)*buffersize);
	a = htons(num_routers);
	memcpy(buffer, &a, sizeof(num_routers));
	printf("num_routers:%d\t",num_routers);
	a = htons(serv.rport);
	memcpy(buffer+(0x02), &a, sizeof(serv.rport));
	printf("SERVER ROUTER PORT:%d\t",serv.rport);
	memcpy(buffer+(0x04), &RT[serv.index].ip, sizeof(serv.ip));
	printf("SERVER IP:%d\n\n",serv.ip);
	int i=0;
	for (i=0;i<num_routers;i++){
		memcpy(buffer+((0x08)+(i*increment)), &RT[i].ip, sizeof(RT[i].ip));
		a = htons(RT[i].rport);
		memcpy(buffer+((0x0C)+(i*increment)), &a, sizeof(RT[i].rport));
		//memcpy(buffer+((0x0E)+(i*increment)), 0x0000, sizeof(RT[i].rport));
		a = htons(RT[i].id);
		memcpy(buffer+((0x10)+(i*increment)), &a, sizeof(RT[i].id));
		a = htons(RT[i].cost);
		memcpy(buffer+((0x12)+(i*increment)), &a, sizeof(RT[i].cost));
		RT[i].update--;
			
		printf("\n ID:%d, rport:%d, cost:%d, ip:%d", RT[i].id, RT[i].rport, RT[i].cost, RT[i].ip);
	}
	/*int n = 0;
	int flg = 0;
	for (n=0;n<num_routers;n++){
		if(RT[n].update == 0){ //delete node after 3 consecutive timeouts
			printf("crash:%d cost:%d", RT[n].id, RT[n].cost);
			RT[n].id = 65535;
			RT[n].cost = 65535;
			RT[n].nbour = 0;
			RT[n].active = 0;
			//qsort(RT, num_routers, sizeof(struct routingtable), struct_cmp_by_id);
			num_routers--;
			flg = 1;
			break;			
		}	
	}*/
	
	printf("\nSERVER PAYLOAD UPDATE SENT%s\n",buffer);
	int k = 0;
	for(k=0;k<num_routers;k++){
		if(RT[k].nbour){
			
			struct sockaddr_in remoteaddr;
			bzero(&remoteaddr, sizeof(remoteaddr));
			remoteaddr.sin_family = AF_INET;
			a = htons(RT[k].rport);
			remoteaddr.sin_port = a;
			remoteaddr.sin_addr.s_addr = RT[k].ip;
			
			//memset(&(remoteaddr.sin_zero), ’\0’, 8);
			int skt = socket(AF_INET,SOCK_DGRAM,0);
			socklen_t addrlen = sizeof(remoteaddr);
			int ret = sendto(skt, buffer, buffersize, 0, (struct sockaddr*)&remoteaddr, addrlen);
			if(ret == -1) {
				perror("send periodic Failed");
			}
			printf("DV UPDATE SENT TO %d\n",RT[k].id);
			close(skt);
						
		}
		
	}
	printf("periodic 3 Update sent to all non-zero cost routers\n\n");
        //timer.tv_sec = period;
	//timer.tv_usec = 0;
	return;	
}

void crash_router(int sock_index){
	FD_CLR(sock_index, &master_list);


	return;
}

void printcostmatrix(){
	int i=0;
	int j=0;
	printf("Cost Matrix\n");
	for(i=0;i<5;i++){
		for(j=0;j<5;j++){
			//costmatrix[i][j]=65535;
			printf("%d  ", costmatrix[i][j]);
		}
		printf("\n");
	}
	return;
}
