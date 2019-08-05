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
        auto ep  = ringer::RingerEndpoint::getInstance(5060, "localhost");
        auto ep2 = ringer::RingerEndpoint::getInstance(12321, "jklas");

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
        return 0;
    } catch (pj::Error &err) {
        std::cout << "Answerrer Error Found" << std::endl;
        std::cout << "Exception: " << err.info() << std::endl;
        return 1;
    }
}
