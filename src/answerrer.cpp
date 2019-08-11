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
#include "ringer_endpoint.hpp"
#include "ringer_logger.hpp"

int main() {
    try {
        auto& ep = ringer::RingerEndpoint::getInstance(5060, "localhost");

        ringer::logger::info("answerrer started");

        ringer::RingerAccount acc{"answerrer", "localhost"};
        ringer::logger::info("*** Answerrer Waiting ***");

        pj_thread_sleep(10000);
        ringer::logger::info("*** Answerrer hanging up ***");
        ringer::logger::info("*** Answerrer exitting***");
        // AudioMediaPlayer amp;
        // amp.createPlayer(testWavFile);
        // amp.startTransmit(call->getAudioMedia(-1));
        ep->hangupAllCalls();
        pj_thread_sleep(70000);
    } catch (pj::Error& err) {
        std::cout << "Answerrer Error Found" << std::endl;
        std::cout << "Exception: " << err.info() << std::endl;
        return 1;
    }
    return 0;
}
