
#include "lwip/init.h"
#include "lwip/apps/httpd.h"
#include "lwip/debug.h"
#include "lwip/stats.h"
//#include "lwip/apps/fs.h"
#include "httpd_structs.h"
#include "lwip/def.h"
#include "lwip/ip.h"
#include "lwip/tcp.h"

#include <string.h>
#include <stdio.h>

//const int CONNECTED_BIT = BIT0;

const static char http_html_hdr[] =
    "HTTP/1.1 200 OK\r\nContent-type: text/html\r\n\r\n";
const static char http_index_hml[] ="<!DOCTYPE html>"
		"<html>"
		"<body>"
		"<form action=\"/action_page.php\">"
		"Select a file: <input type=\"file\" name=\"myFile\"><br><br>"
		"<input type=\"submit\">"
		"</form>"
		"</body>"
		"</html>";

#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/api.h"

static void
http_server_netconn_serve(struct netconn *conn)
{
  struct netbuf *inbuf;
  char *buf;
  u16_t buflen;
  err_t err;

  /* Read the data from the port, blocking if nothing yet there.
   We assume the request (the part we care about) is in one netbuf */
  err = netconn_recv(conn, &inbuf);

  if (err == ERR_OK) {
    netbuf_data(inbuf, (void**)&buf, &buflen);

    // strncpy(_mBuffer, buf, buflen);

    /* Is this an HTTP GET command? (only check the first 5 chars, since
    there are other formats for GET, and we're keeping it very simple )*/
    //printf("buffer = %s \n", buf);
    if (buflen>=5 &&
        buf[0]=='G' &&
        buf[1]=='E' &&
        buf[2]=='T' &&
        buf[3]==' ' &&
        buf[4]=='/' )
	{
    	FILE* f = fopen(&buf[5], "r");

    	if(f == NULL)
    	{
    		return;
    	}

		while(1)
		{
            char line[100];
//			bc = getLine(f, line, sizeof(line));
//			if(bc == 0)
			{
				break;
			}

             // strip newline
             char *pos = strchr(line, '\n');
             if (pos)
             {
                 *pos = '\0';
             }
			netconn_write(conn, http_html_hdr, sizeof(http_html_hdr)-1, NETCONN_NOCOPY);
		}


        fclose(f);

//        printf("buf[5] = %c\n", buf[5]);
      /* Send the HTML header
             * subtract 1 from the size, since we dont send the \0 in the string
             * NETCONN_NOCOPY: our data is const static, so no need to copy it
       */

      // Open file for reading
             //ESP_LOGI(TAG, "Reading file");
//             FILE* f = fopen("/spiflash/hello.txt", "rb");
//             if (f == NULL) {
//                 //ESP_LOGE(TAG, "Failed to open file for reading");
//                 return;
//             }
//             char line[500];
//             fgets(line, sizeof(line), f);
//             fclose(f);
//             // strip newline
//             char *pos = strchr(line, '\n');
//             if (pos) {
//                 *pos = '\0';
//             }
             //ESP_LOGI(TAG, "Read from file: '%s'", line);

      netconn_write(conn, http_html_hdr, sizeof(http_html_hdr)-1, NETCONN_NOCOPY); // Open renamed file for reading
//      netconn_write(conn, line, strlen(line)-1, NETCONN_NOCOPY);
    }

  }
  /* Close the connection (server closes in HTTP) */
  netconn_close(conn);

  /* Delete the buffer (netconn_recv gives us ownership,
   so we have to make sure to deallocate the buffer) */
  netbuf_delete(inbuf);
}

static void http_server(void *pvParameters)
{
  struct netconn *conn, *newconn;
  err_t err;
  conn = netconn_new(NETCONN_TCP);
  netconn_bind(conn, NULL, 80);
  netconn_listen(conn);
  do {
     err = netconn_accept(conn, &newconn);
     if (err == ERR_OK) {
       http_server_netconn_serve(newconn);
       netconn_delete(newconn);
     }
   } while(err == ERR_OK);
   netconn_close(conn);
   netconn_delete(conn);
}


//    xTaskCreate(&http_server, "http_server", 1024*4, NULL, 5, NULL);
void http_server_init(void)
{

	const osThreadAttr_t httpdTask_attributes = {
		.name = "httpdTask",
		.priority = (osPriority_t) osPriorityNormal,
		.stack_size = 512
	};
	osThreadNew(http_server, NULL, &httpdTask_attributes);
}
