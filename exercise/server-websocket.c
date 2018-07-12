// server-json.c - Simple JSON server

#include "mongoose.h"
#include "sensehat.h"

static struct mg_serve_http_opts s_http_server_opts;
static char *s_http_port = "8080";

static void broadcast(struct mg_connection *nc, char *str) {
   struct mg_connection *c;

   for (c = mg_next(nc->mgr, NULL); c != NULL; c = mg_next(nc->mgr, c))
      mg_send_websocket_frame(c, WEBSOCKET_OP_TEXT, str, strlen(str));
}

static void ev_handler(struct mg_connection *nc, int ev, void *ev_data)
{
   struct http_message *hm = (struct http_message *) ev_data;
   struct websocket_message *wm = (struct websocket_message *)ev_data;
   char str[256];

   if (ev == MG_EV_HTTP_REQUEST)
      mg_serve_http(nc, hm, s_http_server_opts);
   if (ev == MG_EV_WEBSOCKET_FRAME) {
      strncpy(str, wm->data, wm->size);
      str[wm->size] = 0;
      printf("%s\n", str);
   }   
}

int main(int argc, char *argv[])
{
   struct mg_mgr mgr;
   struct mg_connection *nc;
   int key;
   char str[80];
   
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
      mg_mgr_poll(&mgr, 10);
      key = read_joystick();
      if (key) {
         sprintf(str, "{ \"key\":%d }", key);
         broadcast(nc, str);
      }
   }
   
   return 0;
}
