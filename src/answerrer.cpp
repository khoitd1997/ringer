// #include <pjlib-util.h>
// #include <pjlib.h>
// #include <pjmedia-codec.h>
// #include <pjmedia.h>
// #include <pjsip.h>
// #include <pjsip_simple.h>
// #include <pjsip_ua.h>
// #include <pjsua-lib/pjsua.h>

#include <pj/file_access.h>
#include <pjmedia_audiodev.h>
#include <iostream>
#include <pjsua2.hpp>

#include "ringer_account.hpp"
#include "ringer_logger.hpp"

int main() {
    try {
        pj::Endpoint ep;
        ep.libCreate();

        // Init library
        pj::EpConfig ep_cfg;
        ep_cfg.logConfig.level = 2;
        ep.libInit(ep_cfg);

        // Transport
        pj::TransportConfig tcfg;
        tcfg.port          = 5060;
        tcfg.publicAddress = "localhost";
        ep.transportCreate(PJSIP_TRANSPORT_UDP, tcfg);

        // Start library
        ep.libStart();
        // ep.audDevManager().setNullDev();
        std::cout << "*** PJSUA2 STARTED ***" << std::endl;

        // Add account
        ringer::RingerAccount acc{"test2", "localhost"};
        ringer::logger::info("*** Answerrer Waiting ***");

        // Hangup all calls
        pj_thread_sleep(10000);
        // AudioMediaPlayer amp;
        // amp.createPlayer(testWavFile);
        // amp.startTransmit(call->getAudioMedia(-1));

        ringer::logger::info("*** Answerrer hanging up ***");
        ep.hangupAllCalls();
        pj_thread_sleep(2000);

        // amp.startTransmit(amr);
        // if (auddev2.isOpened()) amp.startTransmit(auddev2);

        ep.libDestroy();
        std::cout << "Success" << std::endl;
        return 0;
    } catch (pj::Error &err) {
        std::cout << "Answerrer Error Found" << std::endl;
        std::cout << "Exception: " << err.info() << std::endl;
        return 1;
    }
}
