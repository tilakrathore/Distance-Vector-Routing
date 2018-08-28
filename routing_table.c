
 /* @section DESCRIPTION
 *
 * INIT [Control Code: 0x01]
 */
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>

#include "../include/global.h"
#include "../include/control_header_lib.h"
#include "../include/network_util.h"
#include "../include/connection_manager.h"
#include "../include/control_handler.h"
#include "../include/init.h"

#define increment 0x0C

void print_ip(uint32_t ip, char *IP)
{
    unsigned char bytes[4];
    bytes[0] = ip & 0xFF;
    bytes[1] = (ip >> 8) & 0xFF;
    bytes[2] = (ip >> 16) & 0xFF;
    bytes[3] = (ip >> 24) & 0xFF;
	strcpy(IP,bytes);	
    //printf("In function:%d.%d.%d.%d\n", bytes[3], bytes[2], bytes[1], bytes[0]);        
}	

/*int struct_cmp_by_id(const void *a, const void *b) 
{ 
	struct routingtable *ia = (struct routingtable *)a;
	struct routingtable *ib = (struct routingtable *)b;
	return (ia->id - ib->id);
	integer comparison: returns negative if b > a 
	and positive if a > b }	*/



void init_response(int sock_index,char *cntrl_payload)
{	
	printf("Init Response\n");
	uint16_t b;
	uint32_t ip;
	memcpy(&num_routers, cntrl_payload, sizeof(num_routers));
	num_routers = ntohs(num_routers);
	printf("num_routers:%d\n",num_routers);
	memcpy(&period, cntrl_payload+(0x02), sizeof(period));
	period = ntohs(period);
	printf("period:%d\n",period);
	int i=0;
	for (i=0;i<num_routers;i++)
	{
		memcpy(&b, cntrl_payload+((0x04)+(i*increment)), sizeof(RT[i].id));
		RT[i].id = ntohs(b);
		memcpy(&b, cntrl_payload+((0x06)+(i*increment)), sizeof(RT[i].rport));
		printf("\nrport:%d", RT[i].rport);
		RT[i].rport = ntohs(b);
		printf("\nrport:%d", RT[i].rport);
		memcpy(&b, cntrl_payload+((0x08)+(i*increment)), sizeof(RT[i].dport));
		RT[i].dport = ntohs(b);
		memcpy(&b, cntrl_payload+((0x0A)+(i*increment)), sizeof(RT[i].cost));
		RT[i].cost = ntohs(b);
		memcpy(&ip, cntrl_payload+((0x0C)+(i*increment)), sizeof(RT[i].ip));
		//printf("IP before:%d\n",RT[i].ip);
		RT[i].ip = ip;
		print_ip(ntohl(ip),RT[i].IP);
		//printf("routing table: %d.%d.%d.%d\n", RT[i].IP[3], RT[i].IP[2], RT[i].IP[1], RT[i].IP[0]);
		//printf("after function call:%d\n",RT[i].ip);
		//RT[i].ip = ntohl(RT[i].ip);
		//print_ip(RT[i].ip,RT[i].IP);
		//printf("%d.%d.%d.%d\n", RT[i].IP[3], RT[i].IP[2], RT[i].IP[1], RT[i].IP[0]);
		RT[i].update = 65535;
		RT[i].active = 1;
		/*if(RT[i].cost == 0)
		{
			serv.id = RT[i].id;
			serv.ip = RT[i].ip;
			serv.rport = RT[i].rport;
			serv.dport = RT[i].dport;
			serv.index = i;
			printf("\n ID:%d, rport:%d, dport:%d, ip:%d, index:%d\n", serv.id, serv.rport, serv.dport, serv.ip, serv.index);
		}*/
		if(RT[i].cost==0)
		{
			RT[i].nbour = 0;
			RT[i].nextid = RT[i].id;
		}		
		
		if(RT[i].cost>0 && RT[i].cost<65535)
		{
			RT[i].nbour = 1;
			RT[i].nextid = RT[i].id;
		}
		else if(RT[i].cost == 65535)
		{
			RT[i].nbour = 0;
			RT[i].nextid = INF;
		}	
		printf("\n ID:%d, rport:%d, dport:%d, cost:%d, ip:%d, update:%d, nbour:%d, nextid:%d\n", RT[i].id, RT[i].rport, RT[i].dport, RT[i].cost, RT[i].ip, RT[i].update, RT[i].nbour, RT[i].nextid);
	}
	
	/*qsort(RT, num_routers, sizeof(struct routingtable), struct_cmp_by_id);
	
	*/
	int k = 0;
	for(k = 0;k<num_routers;k++){
		if(RT[k].cost == 0)
		{
			serv.id = RT[k].id;
			serv.ip = RT[k].ip;
			serv.rport = RT[k].rport;
			serv.dport = RT[k].dport;
			serv.index = k;
			RT[k].nextid = RT[k].id;
			printf("\n ID:%d, rport:%d, dport:%d, ip:%d, index:%d, RT[k].nextid: %d\n\n", serv.id, serv.rport, serv.dport, serv.ip, serv.index, RT[k].nextid);
		}
	}
	
	int m = 0;
	for(m=0;m<num_routers;m++){
		if(m==serv.index){
			costmatrix[serv.index][m] = 0;
		}
		else{
			costmatrix[serv.index][m] = RT[m].cost;
		}
	}
	printcostmatrix();
	printf("\n----------------------------");
	printf("INIT RESPONSE SENT\n");
	init_router_socket();
	init_reply(sock_index);
	return;
		
}

/* Create payload for DV updates*/
char* create_routing_update(){

	uint16_t a;
	int buffersize = 8+12*num_routers;
	char *buffer = (char *) malloc(sizeof(char)*buffersize);

	a = htons(num_routers);
	memcpy(buffer, &a, sizeof(num_routers));
	printf("num_routers:%d\n",num_routers);
	a = htons(serv.rport);
	memcpy(buffer+(0x02), &a, sizeof(serv.rport));
	printf("Router port:%d\n",serv.rport);
	memcpy(buffer+(0x04), &RT[serv.index].ip, sizeof(serv.ip));
	printf("Source IP:%d\n",serv.ip);
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
			
		
	}
	int n = 0;
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
	}
	
	/*int m;
	if(flg){
		for(m=n;m<num_routers;m++){
			costmatrix[serv.index][m] = costmatrix[serv.index][m+1];
		}
	}*/
	
	printcostmatrix();
	return buffer;	
}

/* Create payload for Routing Table request from Controller*/
void routing_reply(char *payload){
	int i=0;
	for(i=0;i<num_routers;i++){
		uint16_t a;
		a = htons(RT[i].id);
		memcpy(payload+(i*0x08), &a, sizeof(RT[i].id));
		a = htons(RT[i].nextid);
		memcpy(payload+(0x04+(i*0x08)), &a, sizeof(RT[i].nextid));
		a = htons(RT[i].cost);
		memcpy(payload+(0x06+(i*0x08)), &a, sizeof(RT[i].cost));
		printf("\n ID:%d, nextid:%d,  cost:%d\n", RT[i].id,  RT[i].nextid, RT[i].cost);
	}
	
	return;
}
