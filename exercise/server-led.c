// server-json.c - Simple JSON server

#include "mongoose.h"
#include "sensehat.h"

static struct mg_serve_http_opts s_http_server_opts;
static char *s_http_port = "8080";

unsigned char led_array[8][8][3]; // 8 rows, 8 cols, RGB color

static int rpc_getled(char *buf, int len, struct mg_rpc_request *req)
{
   int r, c;

   // return led_array in JSON encoding
   sprintf(buf, "{\"jsonrpc\":\"2.0\",\"id\":%d,\"result\":[", atoi(req->id->ptr));
   for (r=0 ; r<8 ; r++) {
      sprintf(buf+strlen(buf), "[");
      for (c=0 ; c<8 ; c++) {
         sprintf(buf+strlen(buf), "[%d,%d,%d]",
                 led_array[r][c][0], led_array[r][c][1], led_array[r][c][2]);
         if (c<7)
            sprintf(buf+strlen(buf), ",");
      }
      if (r<7)
         sprintf(buf+strlen(buf), "],\n");
      else
         sprintf(buf+strlen(buf), "]");
   }
   sprintf(buf+strlen(buf), "]}\n");
   return strlen(buf);
}

static int rpc_setled(char *buf, int len, struct mg_rpc_request *req)
{
   int row, col, r, g, b;


   row = atoi(req->params[1].ptr);
   col = atoi(req->params[2].ptr);
   r = atoi(req->params[3].ptr);
   g = atoi(req->params[4].ptr);
   b = atoi(req->params[5].ptr);

   led_array[row][col][0] = r;
   led_array[row][col][1] = g;
   led_array[row][col][2] = b;

   set_pixel(row, col, r, g, b);

   return mg_rpc_create_reply(buf, len, req, "i", 1);
}

static void ev_handler(struct mg_connection *nc, int ev, void *ev_data)
{
   struct http_message *hm = (struct http_message *) ev_data;
   static const char *methods[] = { "getled", "setled", NULL };
   static mg_rpc_handler_t handlers[] = { rpc_getled, rpc_setled, NULL };
   char buf[1000];

   if (ev == MG_EV_HTTP_REQUEST) {
      if (mg_vcmp(&hm->uri, "/json-rpc") == 0) {
         mg_rpc_dispatch(hm->body.p, hm->body.len, buf, sizeof(buf),
                         methods, handlers);
         mg_printf(nc, "HTTP/1.0 200 OK\r\nContent-Length: %d\r\n"
                   "Content-Type: application/json\r\n\r\n%s",
                   (int) strlen(buf), buf);
         nc->flags |= MG_F_SEND_AND_CLOSE;
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

   led_clear();
   memset(led_array, 0, sizeof(led_array));

   while(1) {
      mg_mgr_poll(&mgr, 1000);
   }

   return 0;
}
