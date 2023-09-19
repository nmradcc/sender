
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

#include "ff.h"
#include "GetLine.h"

extern char* strsep(char **stringp, const char *delim);

//const int CONNECTED_BIT = BIT0;

const static char http_html_hdr[] =
    "HTTP/1.1 200 OK\r\nContent-type: text/html\r\n\r\n";

#ifdef NOT_USED
const static char http_index_hml[] ="<!DOCTYPE html>"
		"<html>"
		"<body>"
		"<form action=\"/action_page.php\">"
		"Select a file: <input type=\"file\" name=\"myFile\"><br><br>"
		"<input type=\"submit\">"
		"</form>"
		"</body>"
		"</html>";
#endif

const static char http_fnf_hml[] ="<!DOCTYPE html>"
		"<html>"
		"<body>"
		"<form action=\"/action_page.php\">"
		"Select a file: <input type=\"file\" name=\"myFile\"><br><br>"
		"<input type=\"submit\">"
		"</form>"
		"<title>404 (Page Not Found) Error - Ever feel like you're in the wrong place?</title>"
		"</body>"
		"<div class=\"body-404\">"
		"<div class=\"info-container\">"
		"<div class=\"inner-border clear-fix\">"
		"<h2 class=\"info-top\">"
		"404 (Page Not Found) Error"
		"</h2>"
		"<div class=\"site-visitor-404\">"
		"<h3>If you're a <strong>visitor</strong> and not sure what happened:</h3>"
		"<ol>"
		"<li>"
		"	1) You entered or copied the URL incorrectly or"
		"</li>"
		"<li>"
		"	2) The link you used to get here is faulty."
		"</li>"
		"<li class=\"last\">"
		"	(It's an excellent idea to let the link owner know.)"
		"</li>"
		"</ol>"
		"</div>"
		"</html>";


#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/api.h"

char line[500];

static void
http_server_netconn_serve(struct netconn *conn)
{
  struct netbuf *inbuf;
  char *buf;
  u16_t buflen;
  err_t err;
  char* pfn;

  /* Read the data from the port, blocking if nothing yet there.
   We assume the request (the part we care about) is in one netbuf */
  err = netconn_recv(conn, &inbuf);

  if (err == ERR_OK)
  {
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
//        printf("buf[5] = %c\n", &buf[5]);
      /* Send the HTML header
             * subtract 1 from the size, since we dont send the \0 in the string
             * NETCONN_NOCOPY: our data is const static, so no need to copy it
       */

#ifdef USE_NEW
      // Open file for reading
             //ESP_LOGI(TAG, "Reading file");
             FILE* f = fopen("/spiflash/hello.txt", "rb");
             if (f == NULL) {
                 //ESP_LOGE(TAG, "Failed to open file for reading");
                 return;
             }
             char line[500];
             fgets(line, sizeof(line), f);
             fclose(f);
             // strip newline
             char *pos = strchr(line, '\n');
             if (pos) {
                 *pos = '\0';
             }
             //ESP_LOGI(TAG, "Read from file: '%s'", line);

      netconn_write(conn, http_html_hdr, sizeof(http_html_hdr)-1, NETCONN_NOCOPY); // Open renamed file for reading
      netconn_write(conn, line, strlen(line)-1, NETCONN_NOCOPY);
#else
		FRESULT ret;
		unsigned int bc;
		FIL fp;

		pfn = &buf[5];
		pfn = strsep(&pfn, " ");

		// open file
		ret = f_open(&fp, pfn, FA_READ);
		if(ret != FR_OK)
		{
			ret = f_open(&fp, "index.html", FA_READ);
		}

		if(ret == FR_OK)
		{
			netconn_write(conn, http_html_hdr, sizeof(http_html_hdr)-1, NETCONN_NOCOPY); // Open renamed file for reading
			while(1)
			{
				bc = getLine(&fp, line, sizeof(line));
				if(bc == 0)
				{
					break;
				}

				netconn_write(conn, line, strlen(line)-1, NETCONN_NOCOPY);
			}
			f_close(&fp);
		}
		else
		{
			// "File not found.\r\n"
			netconn_write(conn, http_fnf_hml, sizeof(http_fnf_hml)-1, NETCONN_NOCOPY);
		}
	}
#endif
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
