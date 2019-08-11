#include "ringer_call.hpp"

#include <pj/file_access.h>
#include <pjmedia_audiodev.h>

#include "ringer_logger.hpp"

namespace ringer {
const auto testWavFile = "sine_test.wav";

RingerCall::RingerCall(const bool isCaller, pj::Account &acc, int call_id)
    : Call(acc, call_id), isCaller{isCaller}, roleName_{(isCaller ? "caller" : "answerrer")} {
    wav_player = NULL;
    myAcc      = (RingerAccount *)&acc;
}

RingerCall::~RingerCall() {
    if (wav_player) delete wav_player;
}

void RingerCall::onCallState(pj::OnCallStateParam &prm) {
    PJ_UNUSED_ARG(prm);

    const auto ci = this->getInfo();
    logger::info(roleName_, "*** Caller Call: " + ci.remoteUri + " [" + ci.stateText + "]");

    if (ci.state == PJSIP_INV_STATE_DISCONNECTED) {
        // myAcc->removeCall(this);
        /* Delete the call */
        // delete this;
    }
}

void RingerCall::onCallMediaState(pj::OnCallMediaStateParam &prm) {
    logger::info(roleName_, "media state change");
    PJ_UNUSED_ARG(prm);

    pj::CallInfo ci = getInfo();
    pj::Endpoint::instance().audDevManager().setCaptureDev(-1);
    auto &captureDev = pj::Endpoint::instance().audDevManager().getCaptureDevMedia();

    pj::AudioMedia audMed;
    try {
        // Get the first audio media
        audMed = this->getAudioMedia(-1);
        // const auto medInfo = audMed.getPortInfo();
        // std::cout << "med info: " << medInfo.name << " " << medInfo.portId << std::endl;
    } catch (...) {
        logger::error(roleName_, "failed to get audio media");
        return;
    }

    if (isCaller) {
        if (!wav_player) {
            wav_player = new pj::AudioMediaPlayer();
            try {
                wav_player->createPlayer(testWavFile, 0);
            } catch (...) {
                logger::error(roleName_, "failed opening wav file");
                delete wav_player;
                wav_player = NULL;
            }
        }

        // This will connect the wav file to the call audio media
        // AudioMediaRecorder amr;
        // amr.createRecorder("./answerrer.wav");
        // audMed.startTransmit(amr);
        // if (wav_player) wav_player->startTransmit(audMed);

        wav_player->startTransmit(audMed);

        logger::info(roleName_, "say something NOW");

        pj_thread_sleep(30000);
    } else {
        audMed.startTransmit(pj::Endpoint::instance().audDevManager().getPlaybackDevMedia());
    }
    logger::info(roleName_, "finishing onCallMediaState");
}

void RingerCall::onCallTransferRequest(pj::OnCallTransferRequestParam &prm) {
    /* Create new Call for call transfer */
    prm.newCall = new RingerCall(isCaller, *myAcc);
}

void RingerCall::onCallReplaced(pj::OnCallReplacedParam &prm) {
    /* Create new Call for call replace */
    prm.newCall = new RingerCall(isCaller, *myAcc, prm.newCallId);
}
}  // namespace ringer