#include "ringer_proxy.hpp"

#include <iostream>

#include "ringer_logger.hpp"
#include "ringer_pjsip_type_util.hpp"

namespace ringer {
#define THIS_FILE "proxy.cpp"

global_struct global;
pjsip_module  mod_msg_logger = {
    NULL,
    NULL,                                   /* prev, next.		*/
    {"mod-msg-logger", 14},                 /* Name.		*/
    -1,                                     /* Id			*/
    PJSIP_MOD_PRIORITY_TRANSPORT_LAYER - 1, /* Priority	        */
    NULL,                                   /* load()		*/
    NULL,                                   /* start()		*/
    NULL,                                   /* stop()		*/
    NULL,                                   /* unload()		*/
    &logging_on_rx_msg,                     /* on_rx_request()	*/
    &logging_on_rx_msg,                     /* on_rx_response()	*/
    &logging_on_tx_msg,                     /* on_tx_request.	*/
    &logging_on_tx_msg,                     /* on_tx_response()	*/
    NULL,                                   /* on_tsx_state()	*/

};

void app_perror(const char *msg, pj_status_t status) {
    char errmsg[PJ_ERR_MSG_SIZE];

    pj_strerror(status, errmsg, sizeof(errmsg));
    PJ_LOG(1, (THIS_FILE, "%s: %s", msg, errmsg));
}

pj_status_t init_options(int argc, char *argv[]) {
    struct pj_getopt_option long_opt[] = {{"port", 1, 0, 'p'},
                                          {"rr", 0, 0, 'R'},
                                          {"log-level", 1, 0, 'L'},
                                          {"help", 0, 0, 'h'},
                                          {NULL, 0, 0, 0}};
    int                     c;
    int                     opt_ind;

    pj_optind = 0;
    while ((c = pj_getopt_long(argc, argv, "p:L:Rh", long_opt, &opt_ind)) != -1) {
        switch (c) {
            case 'p':
                global.port = atoi(pj_optarg);
                printf("Port is set to %d\n", global.port);
                break;

            case 'R':
                global.record_route = PJ_TRUE;
                printf("Using record route mode\n");
                break;

            case 'L':
                pj_log_set_level(atoi(pj_optarg));
                break;

            case 'h':
                return -1;

            default:
                puts("Unknown option. Run with --help for help.");
                return -1;
        }
    }

    return PJ_SUCCESS;
}

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
pj_bool_t logging_on_rx_msg(pjsip_rx_data *rdata) {
    PJ_LOG(5,
           (THIS_FILE,
            "RX %d bytes %s from %s %s:%d:\n"
            "%.*s\n"
            "--end msg--",
            rdata->msg_info.len,
            pjsip_rx_data_get_info(rdata),
            rdata->tp_info.transport->type_name,
            rdata->pkt_info.src_name,
            rdata->pkt_info.src_port,
            (int)rdata->msg_info.len,
            rdata->msg_info.msg_buf));

    /* Always return false, otherwise messages will not get processed! */
    return PJ_FALSE;
}

/* Notification on outgoing messages */
pj_status_t logging_on_tx_msg(pjsip_tx_data *tdata) {
    /* Important note:
     *	tp_info field is only valid after outgoing messages has passed
     *	transport layer. So don't try to access tp_info when the module
     *	has lower priority than transport layer.
     */

    PJ_LOG(5,
           (THIS_FILE,
            "TX %d bytes %s to %s %s:%d:\n"
            "%.*s\n"
            "--end msg--",
            (tdata->buf.cur - tdata->buf.start),
            pjsip_tx_data_get_info(tdata),
            tdata->tp_info.transport->type_name,
            tdata->tp_info.dst_name,
            tdata->tp_info.dst_port,
            (int)(tdata->buf.cur - tdata->buf.start),
            tdata->buf.start));

    /* Always return success, otherwise message will not get sent! */
    return PJ_SUCCESS;
}

pj_status_t init_stack(void) {
    pj_status_t status;

    /* Must init PJLIB first: */
    status = pj_init();
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, status);

    /* Then init PJLIB-UTIL: */
    status = pjlib_util_init();
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, status);

    /* Must create a pool factory before we can allocate any memory. */
    pj_caching_pool_init(&global.cp, &pj_pool_factory_default_policy, 0);

    /* Create the endpoint: */
    status = pjsip_endpt_create(&global.cp.factory, NULL, &global.endpt);
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, status);

    /* Init transaction layer for stateful proxy only */
#if STATEFUL
    status = pjsip_tsx_layer_init_module(global.endpt);
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, status);
#endif

    /* Create listening transport */
    {
        pj_sockaddr_in addr;

        addr.sin_family      = pj_AF_INET();
        addr.sin_addr.s_addr = 0;
        addr.sin_port        = pj_htons((pj_uint16_t)global.port);

        status = pjsip_udp_transport_start(global.endpt, &addr, NULL, 1, NULL);
        if (status != PJ_SUCCESS) return status;
    }

    /* Create pool for the application */
    global.pool = pj_pool_create(&global.cp.factory, "proxyapp", 4000, 4000, NULL);

    /* Register the logger module */
    pjsip_endpt_register_module(global.endpt, &mod_msg_logger);

    return PJ_SUCCESS;
}

pj_status_t init_proxy(void) {
    pj_sockaddr pri_addr;
    pj_sockaddr addr_list[16];
    unsigned    addr_cnt = PJ_ARRAY_SIZE(addr_list);
    unsigned    i;

    /* List all names matching local endpoint.
     * Note that PJLIB version 0.6 and newer has a function to
     * enumerate local IP interface (pj_enum_ip_interface()), so
     * by using it would be possible to list all IP interfaces in
     * this host.
     */

    /* The first address is important since this would be the one
     * to be added in Record-Route.
     */
    if (pj_gethostip(pj_AF_INET(), &pri_addr) == PJ_SUCCESS) {
        char addr[PJ_INET_ADDRSTRLEN];
        pj_inet_ntop(pj_AF_INET(), &pri_addr.ipv4.sin_addr, addr, sizeof(addr));
        pj_strdup2(global.pool, &global.name[global.name_cnt].host, addr);
        global.name[global.name_cnt].port = global.port;
        global.name_cnt++;
    }

    /* Get the rest of IP interfaces */
    if (pj_enum_ip_interface(pj_AF_INET(), &addr_cnt, addr_list) == PJ_SUCCESS) {
        for (i = 0; i < addr_cnt; ++i) {
            char addr[PJ_INET_ADDRSTRLEN];

            if (addr_list[i].ipv4.sin_addr.s_addr == pri_addr.ipv4.sin_addr.s_addr) continue;

            pj_inet_ntop(pj_AF_INET(), &addr_list[i].ipv4.sin_addr, addr, sizeof(addr));
            pj_strdup2(global.pool, &global.name[global.name_cnt].host, addr);
            global.name[global.name_cnt].port = global.port;
            global.name_cnt++;
        }
    }

    /* Add loopback address. */
#if PJ_IP_HELPER_IGNORE_LOOPBACK_IF
    global.name[global.name_cnt].host = pj_str("127.0.0.1");
    global.name[global.name_cnt].port = global.port;
    global.name_cnt++;
#endif

    global.name[global.name_cnt].host = *pj_gethostname();
    global.name[global.name_cnt].port = global.port;
    global.name_cnt++;

    global.name[global.name_cnt].host = pj_str("localhost");
    global.name[global.name_cnt].port = global.port;
    global.name_cnt++;

    PJ_LOG(3, (THIS_FILE, "Proxy started, listening on port %d", global.port));
    PJ_LOG(3, (THIS_FILE, "Local host aliases:"));
    for (i = 0; i < global.name_cnt; ++i) {
        PJ_LOG(3,
               (THIS_FILE,
                " %.*s:%d",
                (int)global.name[i].host.slen,
                global.name[i].host.ptr,
                global.name[i].port));
    }

    if (global.record_route) { PJ_LOG(3, (THIS_FILE, "Using Record-Route mode")); }

    return PJ_SUCCESS;
}

#if PJ_HAS_THREADS
int worker_thread(void *p) {
    pj_time_val delay = {0, 10};

    PJ_UNUSED_ARG(p);

    while (!global.quit_flag) { pjsip_endpt_handle_events(global.endpt, &delay); }

    return 0;
}
#endif

/* Utility to determine if URI is local to this host. */
pj_bool_t is_uri_local(const pjsip_sip_uri *uri) {
    unsigned i;
    for (i = 0; i < global.name_cnt; ++i) {
        if ((uri->port == global.name[i].port || (uri->port == 0 && global.name[i].port == 5060)) &&
            pj_stricmp(&uri->host, &global.name[i].host) == 0) {
            /* Match */
            return PJ_TRUE;
        }
    }

    /* Doesn't match */
    return PJ_FALSE;
}

/* Proxy utility to verify incoming requests.
 * Return non-zero if verification failed.
 */
pj_status_t proxy_verify_request(pjsip_rx_data *rdata) {
    const pj_str_t STR_PROXY_REQUIRE = {"Proxy-Require", 13};

    /* RFC 3261 Section 16.3 Request Validation */

    /* Before an element can proxy a request, it MUST verify the message's
     * validity.  A valid message must pass the following checks:
     *
     * 1. Reasonable Syntax
     * 2. URI scheme
     * 3. Max-Forwards
     * 4. (Optional) Loop Detection
     * 5. Proxy-Require
     * 6. Proxy-Authorization
     */

    /* 1. Reasonable Syntax.
     * This would have been checked by transport layer.
     */

    /* 2. URI scheme.
     * We only want to support "sip:"/"sips:" URI scheme for this simple proxy.
     */
    if (!PJSIP_URI_SCHEME_IS_SIP(rdata->msg_info.msg->line.req.uri) &&
        !PJSIP_URI_SCHEME_IS_SIPS(rdata->msg_info.msg->line.req.uri)) {
        pjsip_endpt_respond_stateless(
            global.endpt, rdata, PJSIP_SC_UNSUPPORTED_URI_SCHEME, NULL, NULL, NULL);
        return PJSIP_ERRNO_FROM_SIP_STATUS(PJSIP_SC_UNSUPPORTED_URI_SCHEME);
    }

    /* 3. Max-Forwards.
     * Send error if Max-Forwards is 1 or lower.
     */
    if (rdata->msg_info.max_fwd && rdata->msg_info.max_fwd->ivalue <= 1) {
        pjsip_endpt_respond_stateless(
            global.endpt, rdata, PJSIP_SC_TOO_MANY_HOPS, NULL, NULL, NULL);
        return PJSIP_ERRNO_FROM_SIP_STATUS(PJSIP_SC_TOO_MANY_HOPS);
    }

    /* 4. (Optional) Loop Detection.
     * Nah, we don't do that with this simple proxy.
     */

    /* 5. Proxy-Require */
    if (pjsip_msg_find_hdr_by_name(rdata->msg_info.msg, &STR_PROXY_REQUIRE, NULL) != NULL) {
        pjsip_endpt_respond_stateless(
            global.endpt, rdata, PJSIP_SC_BAD_EXTENSION, NULL, NULL, NULL);
        return PJSIP_ERRNO_FROM_SIP_STATUS(PJSIP_SC_BAD_EXTENSION);
    }

    /* 6. Proxy-Authorization.
     * Nah, we don't require any authorization with this sample.
     */

    return PJ_SUCCESS;
}

/* Process route information in the reqeust */
pj_status_t proxy_process_routing(pjsip_tx_data *tdata) {
    pjsip_sip_uri *  target;
    pjsip_route_hdr *hroute;

    /* RFC 3261 Section 16.4 Route Information Preprocessing */

    target = (pjsip_sip_uri *)tdata->msg->line.req.uri;

    /* The proxy MUST inspect the Request-URI of the request.  If the
     * Request-URI of the request contains a value this proxy previously
     * placed into a Record-Route header field (see Section 16.6 item 4),
     * the proxy MUST replace the Request-URI in the request with the last
     * value from the Route header field, and remove that value from the
     * Route header field.  The proxy MUST then proceed as if it received
     * this modified request.
     */
    if (is_uri_local(target)) {
        pjsip_route_hdr *r;
        pjsip_sip_uri *  uri;

        /* Find the first Route header */
        r = hroute = (pjsip_route_hdr *)pjsip_msg_find_hdr(tdata->msg, PJSIP_H_ROUTE, NULL);
        if (r == NULL) {
            /* No Route header. This request is destined for this proxy. */
            return PJ_SUCCESS;
        }

        /* Find the last Route header */
        while ((r = (pjsip_route_hdr *)pjsip_msg_find_hdr(tdata->msg, PJSIP_H_ROUTE, r->next)) !=
               NULL) {
            hroute = r;
        }

        /* If the last Route header doesn't have ";lr" parameter, then
         * this is a strict-routed request indeed, and we follow the steps
         * in processing strict-route requests above.
         *
         * But if it does contain ";lr" parameter, skip the strict-route
         * processing.
         */
        uri = (pjsip_sip_uri *)pjsip_uri_get_uri(&hroute->name_addr);
        if (uri->lr_param == 0) {
            ringer::logger::info("this is strict route");
            /* Yes this is strict route, so:
             * - replace req URI with the URI in Route header,
             * - remove the Route header,
             * - proceed as if it received this modified request.
             */
            tdata->msg->line.req.uri = hroute->name_addr.uri;
            target                   = (pjsip_sip_uri *)tdata->msg->line.req.uri;
            pj_list_erase(hroute);
        }
    }

    /* If the Request-URI contains a maddr parameter, the proxy MUST check
     * to see if its value is in the set of addresses or domains the proxy
     * is configured to be responsible for.  If the Request-URI has a maddr
     * parameter with a value the proxy is responsible for, and the request
     * was received using the port and transport indicated (explicitly or by
     * default) in the Request-URI, the proxy MUST strip the maddr and any
     * non-default port or transport parameter and continue processing as if
     * those values had not been present in the request.
     */
    if (target->maddr_param.slen != 0) {
        pjsip_sip_uri maddr_uri;

        maddr_uri.host = target->maddr_param;
        maddr_uri.port = global.port;

        if (is_uri_local(&maddr_uri)) {
            target->maddr_param.slen     = 0;
            target->port                 = 0;
            target->transport_param.slen = 0;
        }
    }

    /* If the first value in the Route header field indicates this proxy,
     * the proxy MUST remove that value from the request.
     */
    hroute = (pjsip_route_hdr *)pjsip_msg_find_hdr(tdata->msg, PJSIP_H_ROUTE, NULL);
    if (hroute && is_uri_local((pjsip_sip_uri *)hroute->name_addr.uri)) { pj_list_erase(hroute); }

    return PJ_SUCCESS;
}

/* Calculate new target for the request */
pj_status_t proxy_calculate_target(pjsip_rx_data *rdata, pjsip_tx_data *tdata) {
    /* RFC 3261 Section 16.5 Determining Request Targets */
    auto target = (pjsip_sip_uri *)tdata->msg->line.req.uri;
    if (pjStrToStr(target->user) == "answerrer") { target->host = {"192.168.1.169", 13}; }
    ringer::logger::info("target is " + pjStrToStr(target->user));
    return PJ_SUCCESS;
}

/* Destroy stack */
void destroy_stack(void) {
    pjsip_endpt_destroy(global.endpt);
    pj_pool_release(global.pool);
    pj_caching_pool_destroy(&global.cp);

    pj_shutdown();
}
}  // namespace ringer