/*
 * telnet.c
 *
 *  Created on: Jul 22, 2019
 *      Author: Brian
 */

/* Includes ------------------------------------------------------------------*/
#include "lwip/tcp.h"
#include <string.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define PROMPT "Sender Telnet Shell"
#define MAX_CMD_LINE 64

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

struct cmd
{
	int 	length;
	char	bytes[MAX_CMD_LINE];
};


static uint8_t rload_ptr=0;
static uint8_t rx_index=0;
static char rx_buf[MAX_CMD_LINE];
static uint8_t RxNumberChars;

/* Private function prototypes -----------------------------------------------*/

static err_t Telnet_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err);
static err_t Telnet_accept(void *arg, struct tcp_pcb *pcb, err_t err);
static void  Telnet_conn_err(void *arg, err_t err);

// rx support functions
static uint8_t GetRxChar(char* c);
static void BufferChar(uint8_t c);

/* Private functions ---------------------------------------------------------*/

static err_t Telnet_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
	struct 	pbuf *q;
	struct 	cmd *cmd = (struct cmd *) arg;
	int 	done;
	char	*c;
	int		i;

	if (p != NULL)
	{
		// tell lwip we processed the data
		tcp_recved(pcb,p->tot_len);

		if (!cmd)
		{
			pbuf_free(p);
			return ERR_ARG;
		}

		done = 0;
		for (q = p; q!= NULL; q = q->next)
		{
			c = q->payload;
			for (i = 0; i < q->len && !done; i++)
			{
				done = (c[i] == '\r') || (c[i] == '\n');
				if (cmd->length < MAX_CMD_LINE)
				{
					cmd->bytes[cmd->length++] = c[i];
				}
			}
		}

		if (done)
		{
			for(int i = 0; i < cmd->length; i++)
			{
				BufferChar(cmd->bytes[i]);
			}
			cmd->length = 0;
		}

		// end of processing, free buffer
		pbuf_free(p);
	} // if p != NULL
	else if (err == ERR_OK)
	{
		// when pbuf is null, and err is ERR_OK, the remote end is closing the connection.
		mem_free(cmd);
		return (tcp_close(pcb));
	} // end of if err == ERR_OK

	return(ERR_OK);

}  // end of Cmd_recv

static err_t Telnet_accept(void *arg, struct tcp_pcb *pcb, err_t err)
{                                                     
    /* Tell LwIP to associate This structure with this connection. */
    tcp_arg(pcb, mem_calloc(sizeof(struct cmd), 1));

    /* Configure LwIP to use our call back functions. */
    tcp_err(pcb, Telnet_conn_err);
    tcp_recv(pcb, Telnet_recv);

    /* Send out the first message */
    tcp_write(pcb, PROMPT, strlen(PROMPT), 1);
    return ERR_OK;
}

void Telnet_init(void)
{
    struct tcp_pcb *pcb;
	
	/* Create a new TCP control block */
    pcb = tcp_new(); 
	
	/* Assign to the new pcb a local IP address and a port number */ /* Using IP_ADDR_ANY allow the pcb to be used by any local interface */
    tcp_bind(pcb, IP_ADDR_ANY, 23);                                                   
	
	/* Set the connection to the LISTEN state */
    pcb = tcp_listen(pcb);                                                            
	
	/* Specify the function to be called when a connection is established */
    tcp_accept(pcb, Telnet_accept);
}

static void Telnet_conn_err(void *arg, err_t err)
{
    struct cmd *cmnd;

    cmnd = (struct cmd *)arg;
    mem_free(cmnd);
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



/* end of telnet.c -----------------------------------------------------------*/
