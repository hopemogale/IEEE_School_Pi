// server-temp.c - Web server for senshat temperature sensor

#include "mongoose.h"
#include "humidity.h"

static struct mg_serve_http_opts s_http_server_opts;
static char *s_http_port = "8080";

static void ev_handler(struct mg_connection *nc, int ev, void *ev_data)
{
   char str[100];
   int  index;
   double temp, hum;
   struct http_message *hm = (struct http_message *)ev_data;

   if (ev == MG_EV_HTTP_REQUEST) {
      if (mg_vcmp(&hm->uri, "/temp") == 0) {

         // retrieve index from URL
         mg_get_http_var(&hm->query_string, "index", str, sizeof(str));
         index = atoi(str);

         // read temperature and humidity
         read_temp_hum(&temp, &hum);

         // send reply
         mg_printf(nc, "%s", "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n");
         mg_printf_http_chunk(nc, "{\r\n");
         mg_printf_http_chunk(nc, "  \"index\": %d,\r\n", index);
         mg_printf_http_chunk(nc, "  \"temp\": %lg\r\n", temp);
         mg_printf_http_chunk(nc, "}\r\n");
         mg_send_http_chunk(nc, "", 0); // send empty chunk as end of response
      } else {
         // serve static content
         mg_serve_http(nc, hm, s_http_server_opts);
      }
   }
}

int main(int argc, char *argv[])
{
   struct mg_mgr mgr;
   struct mg_connection *nc;

   mg_mgr_init(&mgr, NULL);
   nc = mg_bind(&mgr, s_http_port, ev_handler);
   if (nc == NULL) {
      fprintf(stderr, "Error starting server on port %s\n", s_http_port);
      exit(1);
   }

   mg_set_protocol_http_websocket(nc);
   s_http_server_opts.document_root = ".";
   s_http_server_opts.enable_directory_listing = "yes";

   printf("Starting server on port %s\n", s_http_port);

   while(1) {
      mg_mgr_poll(&mgr, 1000);
   }

   return 0;
}
