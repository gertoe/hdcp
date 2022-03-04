/* 
 * Date: Apr 15, 2019
 * Modified by: cplus.shen@advantech.com.tw
 * Description: a client program for hdcpd
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include "hdcpapi.h"

#define HDCP_DEFAULT_LEVEL HDCP_LEVEL1

static uint32_t hdcpHandle;

static char *printhdcpStatus(HDCP_STATUS ret)
{
    char *msg = "";

    switch (ret) {
    case HDCP_STATUS_SUCCESSFUL:
	msg = "HDCP_STATUS_SUCCESSFUL";
	break;
    case HDCP_STATUS_ERROR_ALREADY_CREATED:
	msg = "HDCP_STATUS_ERROR_ALREADY_CREATED";
	break;
    case HDCP_STATUS_ERROR_INVALID_PARAMETER:
	msg = "HDCP_STATUS_ERROR_INVALID_PARAMETER";
	break;
    case HDCP_STATUS_ERROR_NO_DISPLAY:
	msg = "HDCP_STATUS_ERROR_NO_DISPLAY";
	break;
    case HDCP_STATUS_ERROR_MAX_DEVICES_EXCEEDED:
	msg = "HDCP_STATUS_ERROR_MAX_DEVICES_EXCEEDED";
	break;
    case HDCP_STATUS_ERROR_INTERNAL:
	msg = "HDCP_STATUS_ERROR_INTERNAL";
	break;
    case HDCP_STATUS_ERROR_SRM_INVALID:
	msg = "HDCP_STATUS_ERROR_SRM_INVALID";
	break;
    case HDCP_STATUS_ERROR_INSUFFICIENT_MEMORY:
	msg = "HDCP_STATUS_ERROR_INSUFFICIENT_MEMORY";
	break;
    case HDCP_STATUS_ERROR_SRM_NOT_RECENT:
	msg = "HDCP_STATUS_ERROR_SRM_NOT_RECENT";
	break;
    case HDCP_STATUS_ERROR_SRM_FILE_STORAGE:
	msg = "HDCP_STATUS_ERROR_SRM_FILE_STORAGE";
	break;
    case HDCP_STATUS_ERROR_MSG_TRANSACTION:
	msg = "HDCP_STATUS_ERROR_MSG_TRANSACTION";
	break;
    default:
	msg = "Some unhandled error";
	break;
    }

    return msg;
}

static void onhdcpEvent(uint32_t hdcpHandle, uint32_t port, PORT_EVENT pEvent, void *context)
{
    HDCP_STATUS pl_ret;

    switch (pEvent) {
    case PORT_EVENT_PLUG_IN:
	printf("[Port %d] HDMI plugged In....\n", port);
	pl_ret = HDCPSetProtectionLevel(hdcpHandle, port, HDCP_DEFAULT_LEVEL);
	printf("*****Set protection level %d on port %d: %s\n", HDCP_DEFAULT_LEVEL, port, printhdcpStatus(pl_ret));
	break;
    case PORT_EVENT_PLUG_OUT:
	/*STEP 6: The HDCP daemon will notify the App by PORT_EVNET_PLUG_OUT if a hotplug-out event is detected. 
                  App will call HDCPSetProtectionLevel with HDCP_LEVEL0 to disable link in callback function. */
	printf("[Port %d] HDMI plugged Out....\n", port);
	pl_ret = HDCPSetProtectionLevel(hdcpHandle, port, HDCP_LEVEL0);
	printf("*****Set protection level %d on port %d: %s\n", HDCP_LEVEL0, port, printhdcpStatus(pl_ret));
	break;
    case PORT_EVENT_LINK_LOST:
	printf("[Port %d] HDMI Link lost....\n", port);
	break;
    default:
	break;
    }
}


void my_handler(int s)
{
    HDCPDestroy(hdcpHandle);
    printf("HDCP session finished.\n");
    exit(1);
}

int main(int argc, char *argv[])
{
    char context[1024];
    struct sigaction sigIntHandler;
    HDCP_STATUS ret;

    sigIntHandler.sa_handler = my_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);


    ret = HDCPCreate(&hdcpHandle, &onhdcpEvent, context);

    printf("*****HDCP creation status: %s\n", printhdcpStatus(ret));

    if (ret == HDCP_STATUS_SUCCESSFUL) {
	PortList allportlist;
	HDCP_STATUS port_ret = HDCPEnumerateDisplay(hdcpHandle, &allportlist);

	printf("*****Enumerate display status: %s\n", printhdcpStatus(port_ret));

	if (port_ret == HDCP_STATUS_SUCCESSFUL) {
	    printf("Ports list: %d\n", allportlist.PortCount);

	    for (int i = 0; i < allportlist.PortCount; i++) {
		HDCP_STATUS pl_ret;

		printf("Port id: %d\n", allportlist.Ports[i].Id);
		printf("Port status: %d\n", allportlist.Ports[i].status);

		pl_ret = HDCPSetProtectionLevel(hdcpHandle, allportlist.Ports[i].Id, HDCP_DEFAULT_LEVEL);

		printf("*****Set protection level status on port %d: %s\n", i, printhdcpStatus(pl_ret));
	    }
	}

    }

    pause();

    return 0;
}
