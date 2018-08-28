/**
 *     */

 /* @section DESCRIPTION
 *  *
 *   *  Recv updates from neighbours and update Routing table
 *    */
#include <netinet/in.h>
#include <string.h>
#include "../include/global.h"
#include "../include/connection_manager.h"
#include "../include/deserialize.h"

#define increment 0x0C

uint16_t nrouters, source_rport;
uint32_t source_ip;

 void deserialize_updates(char *recvbuf){
 	uint16_t d;
 	printf("DESERIALIZE\n");
	memcpy(&d, recvbuf, sizeof(nrouters));
 	nrouters = ntohs(d);
 	printf("Deserialize:nrouters:%d\t",nrouters);
 	memcpy(&d, recvbuf+0x02, sizeof(source_rport));
 	source_rport = ntohs(d);
 	printf("Deserialize Source Rport:%d\t",source_rport);
 	memcpy(&source_ip, recvbuf+0x04, sizeof(source_ip));
	
	int j = 0;
 		for(j=0;j<nrouters;j++){
 			if(RT[j].rport == source_rport){
 				RT[j].update = 3;
				printf("J: %d:, source_rport index: %d\n\n", j, source_rport);
 				break;
 			}
 		}
	
 	if(num_routers==nrouters){
 
 		int i=0;
		for (i=0;i<nrouters;i++){
			memcpy(&d, recvbuf+((0x10)+(i*increment)), sizeof(d));
			memcpy(&d, recvbuf+((0x12)+(i*increment)), sizeof(d));
			costmatrix[j][i] = ntohs(d);	
		}

		//run Bellman-Ford equation
		int k = 0;
		int leastcost;
		for(k=0;k<nrouters;k++){
			if(costmatrix[j][k]==65535){
				continue;
			}
			leastcost = costmatrix[serv.index][j]+costmatrix[j][k];
			printf("Server Index: %d\t", serv.index);
			printf("J Value: %d\n", j);
			
			printf("Least Cost: %d\t k value: %d\t RT[k] Cost: %d\n\n",leastcost, k, RT[k].cost);
			if(RT[k].cost>leastcost){
				RT[k].cost = leastcost;
				RT[k].nextid = RT[j].id;
				printf("RT[k] cost:%d, Next id:%d\n\n", RT[k].cost, RT[k].nextid);
			}
			
		}
		printcostmatrix();
		printf("\n--------------------------------\n");
		int m = 0;
		for(m=0;m<num_routers;m++){
			costmatrix[serv.index][m] = RT[m].cost;
		}
		printcostmatrix();
	}
	return;	
}
