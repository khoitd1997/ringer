#define THIS_FILE "proxy_server.cpp"

#include <chrono>
#include <iostream>

#include "ringer_logger.hpp"

/* Common proxy functions */
#define STATEFUL 0
#include "ringer_proxy.hpp"

/* Callback to be called to handle outgoing requests. */
static pj_bool_t on_tx_request(pjsip_tx_data *rdata) { ringer::logger::info("Got tx req"); }

/* Callback to be called to handle outgoing response. */
static pj_bool_t on_tx_response(pjsip_tx_data *rdata) { ringer::logger::info("Got tx resp"); }

/* Callback to be called to handle incoming requests. */
static pj_bool_t on_rx_request(pjsip_rx_data *rdata);

/* Callback to be called to handle incoming response. */
static pj_bool_t on_rx_response(pjsip_rx_data *rdata);

static pj_status_t init_stateless_proxy(void) {
    static pjsip_module mod_stateless_proxy = {
        NULL,
        NULL,                              /* prev, next.	*/
        {"mod-stateless-proxy", 19},       /* Name.		*/
        -1,                                /* Id		*/
        PJSIP_MOD_PRIORITY_UA_PROXY_LAYER, /* Priority		*/
        NULL,                              /* load()		*/
        NULL,                              /* start()		*/
        NULL,                              /* stop()		*/
        NULL,                              /* unload()		*/
        &on_rx_request,                    /* on_rx_request()	*/
        &on_rx_response,                   /* on_rx_response()	*/

        NULL,
        NULL,
        // &on_tx_request,                    /* on_tx_request.	*/
        // &on_tx_response,                   /* on_tx_response()	*/

        NULL, /* on_tsx_state()	*/
    };

    pj_status_t status;

    /* Register our module to receive incoming requests. */
    status = pjsip_endpt_register_module(ringer::global.endpt, &mod_stateless_proxy);
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);

    return PJ_SUCCESS;
}

/* Callback to be called to handle incoming requests. */
static pj_bool_t on_rx_request(pjsip_rx_data *rdata) {
    pjsip_tx_data *tdata;
    pj_status_t    status;

    /* Verify incoming request */
    status = ringer::proxy_verify_request(rdata);
    if (status != PJ_SUCCESS) {
        ringer::app_perror("RX invalid request", status);
        return PJ_TRUE;
    }

    /*
     * Request looks sane, next clone the request to create transmit data.
     */
    status = pjsip_endpt_create_request_fwd(ringer::global.endpt, rdata, NULL, NULL, 0, &tdata);
    if (status != PJ_SUCCESS) {
        pjsip_endpt_respond_stateless(
            ringer::global.endpt, rdata, PJSIP_SC_INTERNAL_SERVER_ERROR, NULL, NULL, NULL);
        return PJ_TRUE;
    }

    /* Process routing */
    status = ringer::proxy_process_routing(tdata);
    if (status != PJ_SUCCESS) {
        ringer::app_perror("Error processing route", status);
        return PJ_TRUE;
    }

    /* Calculate target */
    status = ringer::proxy_calculate_target(rdata, tdata);
    if (status != PJ_SUCCESS) {
        ringer::app_perror("Error calculating target", status);
        return PJ_TRUE;
    }

    /* Target is set, forward the request */
    status = pjsip_endpt_send_request_stateless(ringer::global.endpt, tdata, NULL, NULL);
    if (status != PJ_SUCCESS) {
        ringer::app_perror("Error forwarding request", status);
        return PJ_TRUE;
    }

    return PJ_TRUE;
}

/* Callback to be called to handle incoming response. */
static pj_bool_t on_rx_response(pjsip_rx_data *rdata) {
    pjsip_tx_data *     tdata;
    pjsip_response_addr res_addr;
    pjsip_via_hdr *     hvia;
    pj_status_t         status;

    /* Create response to be forwarded upstream (Via will be stripped here) */
    status = pjsip_endpt_create_response_fwd(ringer::global.endpt, rdata, 0, &tdata);
    if (status != PJ_SUCCESS) {
        ringer::app_perror("Error creating response", status);
        return PJ_TRUE;
    }

    /* Get topmost Via header */
    hvia = (pjsip_via_hdr *)pjsip_msg_find_hdr(tdata->msg, PJSIP_H_VIA, NULL);
    if (hvia == NULL) {
        /* Invalid response! Just drop it */
        pjsip_tx_data_dec_ref(tdata);
        return PJ_TRUE;
    }
    // ringer::logger::info("rx resp: " + std::to_string((hvia->sent_by).port));

    /* Calculate the address to forward the response */
    pj_bzero(&res_addr, sizeof(res_addr));
    res_addr.dst_host.type = PJSIP_TRANSPORT_UDP;
    res_addr.dst_host.flag = pjsip_transport_get_flag_from_type(PJSIP_TRANSPORT_UDP);

    /* Destination address is Via's received param */
    res_addr.dst_host.addr.host = hvia->recvd_param;
    if (res_addr.dst_host.addr.host.slen == 0) {
        /* Someone has messed up our Via header! */
        res_addr.dst_host.addr.host = hvia->sent_by.host;
    }

    /* Destination port is the rpot */
    if (hvia->rport_param != 0 && hvia->rport_param != -1)
        res_addr.dst_host.addr.port = hvia->rport_param;

    if (res_addr.dst_host.addr.port == 0) {
        /* Ugh, original sender didn't put rport!
         * At best, can only send the response to the port in Via.
         */
        res_addr.dst_host.addr.port = hvia->sent_by.port;
    }

    /* Forward response */
    status = pjsip_endpt_send_response(ringer::global.endpt, &res_addr, tdata, NULL, NULL);
    if (status != PJ_SUCCESS) {
        ringer::app_perror("Error forwarding response", status);
        return PJ_TRUE;
    }

    return PJ_TRUE;
}

/*
 * main()
 */
int main(int argc, char *argv[]) {
    ringer::logger::init("server");
    pj_status_t status;

    ringer::global.port = 5060;
    pj_log_set_level(4);

    status = ringer::init_options(argc, argv);
    if (status != PJ_SUCCESS) return 1;

    status = ringer::init_stack();
    if (status != PJ_SUCCESS) {
        ringer::app_perror("Error initializing stack", status);
        return 1;
    }

    status = ringer::init_proxy();
    if (status != PJ_SUCCESS) {
        ringer::app_perror("Error initializing proxy", status);
        return 1;
    }

    status = init_stateless_proxy();
    if (status != PJ_SUCCESS) {
        ringer::app_perror("Error initializing stateless proxy", status);
        return 1;
    }

#if PJ_HAS_THREADS
    status = pj_thread_create(
        ringer::global.pool, "sproxy", &ringer::worker_thread, NULL, 0, 0, &ringer::global.thread);
    if (status != PJ_SUCCESS) {
        ringer::app_perror("Error creating thread", status);
        return 1;
    }

    using clock = std::chrono::system_clock;
    using sec   = std::chrono::duration<double>;
    // for milliseconds, use using ms = std::chrono::duration<double, std::milli>;

    const auto before = clock::now();

    while (!ringer::global.quit_flag) {
        char line[10];

        const auto dur =
            (std::chrono::duration_cast<std::chrono::seconds>(clock::now() - before)).count();
        if (dur > 30) { ringer::global.quit_flag = PJ_TRUE; }
    }

    pj_thread_join(ringer::global.thread);

#else
    puts("\nPress Ctrl-C to quit\n");
    for (;;) {
        pj_time_val delay = {0, 0};
        pjsip_endpt_handle_events(ringer::global.endpt, &delay);
    }
#endif

    ringer::destroy_stack();

    ringer::logger::info("done");
    return 0;
}
