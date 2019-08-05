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
#include "ringer_call.hpp"
#include "ringer_endpoint.hpp"
#include "ringer_logger.hpp"

int main() {
    try {
        auto ep = ringer::RingerEndpoint::getInstance(5061, "localhost");
        ep->hangupAllCalls();

        // ep.audDevManager().setNullDev();
        std::cout << "*** PJSUA2 STARTED ***" << std::endl;

        // Add account
        ringer::RingerAccount acc{"test1", "pjsip.org"};

        pj_thread_sleep(2000);

        // Make outgoing call
        auto *call = new ringer::RingerCall(true, acc);
        acc.calls.push_back(call);

        pj::CallOpParam prm(true);
        prm.opt.audioCount = 1;
        prm.opt.videoCount = 0;
        call->makeCall("sip:test2@localhost:5060", prm);

        // AudioMediaRecorder amr;
        // amr.createRecorder("./recorder_test_output.wav");

        // Hangup all calls

        pj_thread_sleep(6000);
        // AudioMediaPlayer amp;
        // amp.createPlayer(testWavFile);
        // amp.startTransmit(call->getAudioMedia(-1));
        pj_thread_sleep(2000);

        // amp.startTransmit(amr);
        // if (auddev2.isOpened()) amp.startTransmit(auddev2);

        // Destroy library
        std::cout << "*** PJSUA2 SHUTTING DOWN ***" << std::endl;

        std::cout << "Success" << std::endl;
        return 0;
    } catch (pj::Error &err) {
        std::cout << "Error Found" << std::endl;
        std::cout << "Exception: " << err.info() << std::endl;
        return 1;
    }
}
