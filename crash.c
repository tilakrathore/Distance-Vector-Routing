/**
 *     *
 *      * @section LICENSE
 *       *
 *        * This program is free software; you can redistribute it and/or
 *         * modify it under the terms of the GNU General Public License as
 *          * published by the Free Software Foundation; either version 2 of
 *           * the License, or (at your option) any later version.
 *            *
 *             * This program is distributed in the hope that it will be useful, but
 *              * WITHOUT ANY WARRANTY; without even the implied warranty of
 *               * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *                * General Public License for more details at
 *                 * http://www.gnu.org/copyleft/gpl.html
 *                  *
 *                   */
 /* @section DESCRIPTION
 *  *
 *   * CRASH command [Control Code: 0x04]
 *    */

#include <string.h>
 
#include "../include/global.h"
#include "../include/control_header_lib.h"
#include "../include/network_util.h"
#include "../include/connection_manager.h"
 

void crash_response(int sock_index){
	printf("Crash Response Sent\n");
	char *cntrl_response_header, *cntrl_response;
	uint16_t payload_len, response_len;
	payload_len = 0;
	cntrl_response_header = create_response_header(sock_index, 4, 0, 0);
	
	response_len = CNTRL_RESP_HEADER_SIZE;
	cntrl_response = (char *) malloc(response_len);
	
	/* Copy Header */
	memcpy(cntrl_response, cntrl_response_header, CNTRL_RESP_HEADER_SIZE);
	free(cntrl_response_header);
	
	sendALL(sock_index, cntrl_response, response_len);
	
	free(cntrl_response);
	
	crash_router(sock_index);
	exit(0);	
	//return;
}
