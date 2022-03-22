#include <string.h>
#include <signal.h>
#include "mongoose.h"
#include "logger.h"
#include "server.h"

static int running = 1;
static void signal_handler(int signo) {
    running = 0;
    lprintf(LOG_WARNING, "SIG %d received, terminating...\n", signo);
}


static void cb(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
  if (ev == MG_EV_HTTP_MSG) {
    struct mg_http_message *hm = ev_data, tmp = {0};
    lprintf(LOG_INFO, "%s %s %s %s", hm->method.ptr, hm->uri.ptr, tmp.uri.ptr);
  }
}


void start_fantom_server(fantom_config_t *config)
{
	  lprintf(LOG_INFO, "Starting server on %s with the database as %s.\n", config->bind_url, config->db_file);

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
 
    struct mg_mgr mgr;
  
    mg_log_set(MG_DEBUG);
    mg_mgr_init(&mgr);

    struct mg_connection *c = mg_http_listen(&mgr, config->bind_url, cb, &mgr);
    if (c == NULL) {
    		lprintf(LOG_ERROR, "Cannot start server on %s\n", config->bind_url);
    		return;
    }
  
	  while (running) {
        mg_mgr_poll(&mgr, 1000);
	  }
	
		lprintf(LOG_INFO, "Stopping server...\n");
    mg_mgr_free(&mgr);
}

