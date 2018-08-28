/**
 */

 /* @section DESCRIPTION
 *
 *  INIT [Control Code: 0x01]
 */

#include <string.h>

#include "../include/global.h"
#include "../include/control_header_lib.h"
#include "../include/network_util.h"


void init_reply(int sock_index)
{
	printf("Init Reply\n");
	uint16_t payload_len, response_len;
	char *cntrl_response_header, *cntrl_response;

	payload_len = 0; // Discount the NULL chararcter

	cntrl_response_header = create_response_header(sock_index, 1, 0, 0);

	response_len = CNTRL_RESP_HEADER_SIZE;
	cntrl_response = (char *) malloc(response_len);
	/* Copy Header */
	memcpy(cntrl_response, cntrl_response_header, CNTRL_RESP_HEADER_SIZE);
	free(cntrl_response_header);
	
	sendALL(sock_index, cntrl_response, response_len);

	free(cntrl_response);
}
