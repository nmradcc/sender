
#include "lwip/init.h"
#include "lwip/debug.h"
#include "lwip/stats.h"
//#include "lwip/apps/httpd.h"
//#include "lwip/apps/fs.h"
//#include "httpd_structs.h"
#include "lwip/def.h"
#include "lwip/ip.h"
#include "lwip/tcp.h"

#include <string.h>
#include <stdio.h>

//#include "ff.h"
//#include "GetLine.h"

//extern char* strsep(char **stringp, const char *delim);

#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/api.h"

#define MAX_CMD_LINE 64

// rx support data
static uint8_t rload_ptr=0;
static uint8_t rx_index=0;
static char rx_buf[MAX_CMD_LINE];
static uint8_t RxNumberChars;

// rx support functions
static uint8_t GetRxChar(char* c);
static void BufferChar(uint8_t c);

static void telnet_server_netconn_serve(struct netconn *conn)
{
	struct netbuf *inbuf;
	char *buf;
	u16_t buflen;
	err_t err;

	/* Read the data from the port, blocking if nothing yet there.
	We assume the request (the part we care about) is in one netbuf */
	err = netconn_recv(conn, &inbuf);

	if (err == ERR_OK)
	{
		netbuf_data(inbuf, (void**)&buf, &buflen);

		for(int i = 0; i < buflen; i++)
		{
			BufferChar(buf[i]);
		}
	}

  /* Close the connection (server closes in HTTP) */
  netconn_close(conn);

  /* Delete the buffer (netconn_recv gives us ownership,
   so we have to make sure to deallocate the buffer) */
  netbuf_delete(inbuf);
}


static void telnet_server(void *pvParameters)
{
  struct netconn *conn, *newconn;
  err_t err;
  conn = netconn_new(NETCONN_TCP);
  netconn_bind(conn, NULL, 23);
  netconn_listen(conn);
  do {
     err = netconn_accept(conn, &newconn);
     if (err == ERR_OK) {
       telnet_server_netconn_serve(newconn);
       netconn_delete(newconn);
     }
   } while(err == ERR_OK);
   netconn_close(conn);
   netconn_delete(conn);
}


#ifdef NOT_YET
static void telnet_server2(void *pvParameters)
{
	struct netconn *conn, *newconn;
	err_t err;

	//LWIP_UNUSED_ARG(arg);

	/* Create a new connection identifier. */
	conn = netconn_new(NETCONN_TCP);
	if (conn!=NULL)
	{
		/* Bind connection to well known port number 7. */
		err = netconn_bind(conn, NULL, 7);
		if (err == ERR_OK)
		{
			/* Tell connection to go into listening mode. */
			netconn_listen(conn);
			while (1)
			{
				/* Grab new connection. */
				//newconn = netconn_accept(conn);
			     err = netconn_accept(conn, &newconn);

				/* Process the new connection. */
				if (newconn)
				{
					struct netbuf *buf;
					void *data;
					u16_t len;
					while ((buf = netconn_recv(newconn)) != NULL)
					{


						netbuf_data(inbuf, (void**)&buf, &buflen);

						for(int i = 0; i < buflen; i++)
						{
							BufferChar(buf[i]);
						}


						do
						{
							netbuf_data(buf, &data, &len);
							netconn_write(newconn, data, len, NETCONN_COPY);
						}
						while (netbuf_next(buf) >= 0);
						netbuf_delete(buf);
					}

					/* Close connection and discard connection identifier. */
					netconn_close(newconn);
					netconn_delete(newconn);
				}
			}
		}
		else
		{
			printf(" can not bind TCP netconn");
		}
	}
	else
	{
		printf("can not create TCP netconn");
	}
}
#endif


void telnet_server_init(void)
{

	const osThreadAttr_t telnetTask_attributes = {
		.name = "telnetTask",
		.priority = (osPriority_t) osPriorityBelowNormal,
		.stack_size = 512
	};
	osThreadNew(telnet_server, NULL, &telnetTask_attributes);
}


void Telnet_write(struct tcp_pcb *pcb, char* buf, int len)
{

	// add cr lf to the command
	//buf[len] = '\r';
	//len++;
	//buf[len] = '\n';
	//len++;

	tcp_write(pcb, buf, len, TCP_WRITE_FLAG_COPY);
}


char Telnet_getc(void)
{
	char c;

	if(GetRxChar(&c))
	{
		return c;
	}
	return 0;
}


/**********************************************************************
*
* FUNCTION:		GetRxChar
*
* ARGUMENTS:
*
* RETURNS:
*
* DESCRIPTION:
*
* RESTRICTIONS:
*
**********************************************************************///
static uint8_t GetRxChar(char* c)
{
	if(rload_ptr == rx_index)
	{
		return 0;
	}

	*c = rx_buf[rload_ptr++];
	if(rload_ptr > MAX_CMD_LINE - 1)
	{
		rload_ptr = 0;
	}
	return 1;
}


/**********************************************************************
*
* FUNCTION:		BufferChar
*
* ARGUMENTS:
*
* RETURNS:
*
* DESCRIPTION:
*
* RESTRICTIONS:
*
**********************************************************************///
static void BufferChar(uint8_t c)
{
	rx_buf[rx_index++] = c; // store data in buffer
	// check if we are on the buffer wrap
	if( rx_index > MAX_CMD_LINE - 1 )
	{
		rx_index = 0;
	}
	RxNumberChars++;
}

