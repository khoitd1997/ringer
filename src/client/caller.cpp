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
    auto &ep = ringer::RingerEndpoint::getInstance(5061, "localhost");
    try {
        ringer::logger::init("caller");

        // Add account
        ringer::RingerAccount acc{"caller", "pjsip.org"};
        pj_thread_sleep(2000);

        // Make outgoing call
        auto *call = new ringer::RingerCall(true, acc);
        acc.calls.push_back(call);

        pj::CallOpParam prm(true);
        prm.opt.audioCount = 1;
        prm.opt.videoCount = 0;
        // call->makeCall("sip:answerrer@ringer_sip:5062", prm);
        call->makeCall("sip:answerrer@something:5062", prm);

        // AudioMediaRecorder amr;
        // amr.createRecorder("./recorder_test_output.wav");

        pj_thread_sleep(33000);
        // AudioMediaPlayer amp;
        // amp.createPlayer(testWavFile);
        // amp.startTransmit(call->getAudioMedia(-1));

        // amp.startTransmit(amr);
        // if (auddev2.isOpened()) amp.startTransmit(auddev2);

        // Destroy library

        std::cout << "Caller exitting" << std::endl;
    } catch (pj::Error &err) {
        std::cout << "Error Found" << std::endl;
        std::cout << "Exception: " << err.info() << std::endl;
        return 1;
    }
    return 0;
}
