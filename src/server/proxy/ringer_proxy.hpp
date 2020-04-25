#pragma once

extern "C" {
#include "pjlib-util.h"
#include "pjlib.h"
#include "pjsip.h"
}

namespace ringer {

/* Options */
struct global_struct {
    pj_caching_pool cp;
    pjsip_endpoint *endpt;
    int             port;
    pj_pool_t *     pool;

    pj_thread_t *thread;
    pj_bool_t    quit_flag;

    pj_bool_t record_route;

    unsigned        name_cnt;
    pjsip_host_port name[16];
};

extern global_struct global;
extern pjsip_module  mod_msg_logger;

void app_perror(const char *msg, pj_status_t status);

pj_status_t init_options(int argc, char *argv[]);

/*****************************************************************************
 * This is a very simple PJSIP module, whose sole purpose is to display
 * incoming and outgoing messages to log. This module will have priority
 * higher than transport layer, which means:
 *
 *  - incoming messages will come to this module first before reaching
 *    transaction layer.
 *
 *  - outgoing messages will come to this module last, after the message
 *    has been 'printed' to contiguous buffer by transport layer and
 *    appropriate transport instance has been decided for this message.
 *
 */

/* Notification on incoming messages */
pj_bool_t logging_on_rx_msg(pjsip_rx_data *rdata);

/* Notification on outgoing messages */
pj_status_t logging_on_tx_msg(pjsip_tx_data *tdata);

pj_status_t init_stack(void);

pj_status_t init_proxy(void);

#if PJ_HAS_THREADS
int worker_thread(void *p);
#endif

/* Utility to determine if URI is local to this host. */
pj_bool_t is_uri_local(const pjsip_sip_uri *uri);

/* Proxy utility to verify incoming requests.
 * Return non-zero if verification failed.
 */
pj_status_t proxy_verify_request(pjsip_rx_data *rdata);

/* Process route information in the reqeust */
pj_status_t proxy_process_routing(pjsip_tx_data *tdata);

pj_status_t proxy_calculate_target(pjsip_rx_data *rdata, pjsip_tx_data *tdata);

/* Destroy stack */
void destroy_stack(void);
}  // namespace ringer