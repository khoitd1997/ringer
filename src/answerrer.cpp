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

#define THIS_FILE (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

const auto testWavFile = "sine_test.wav";

using namespace pj;

class MyAccount;

class MyCall : public Call {
   private:
    MyAccount *       myAcc;
    AudioMediaPlayer *wav_player;

   public:
    MyCall(Account &acc, int call_id = PJSUA_INVALID_ID) : Call(acc, call_id) {
        wav_player = NULL;
        myAcc      = (MyAccount *)&acc;
    }

    ~MyCall() {
        if (wav_player) delete wav_player;
    }

    virtual void onCallState(OnCallStateParam &prm);
    virtual void onCallTransferRequest(OnCallTransferRequestParam &prm);
    virtual void onCallReplaced(OnCallReplacedParam &prm);
    virtual void onCallMediaState(OnCallMediaStateParam &prm);
};

class MyAccount : public Account {
   public:
    std::vector<Call *> calls;

   public:
    MyAccount() {}

    ~MyAccount() {
        std::cout << "*** Account is being deleted: No of calls=" << calls.size() << std::endl;

        for (std::vector<Call *>::iterator it = calls.begin(); it != calls.end();) {
            delete (*it);
            it = calls.erase(it);
        }
    }

    void removeCall(Call *call) {
        for (std::vector<Call *>::iterator it = calls.begin(); it != calls.end(); ++it) {
            if (*it == call) {
                calls.erase(it);
                break;
            }
        }
    }

    virtual void onRegState(OnRegStateParam &prm) {
        AccountInfo ai = getInfo();
        std::cout << (ai.regIsActive ? "*** Register: code=" : "*** Unregister: code=") << prm.code
                  << std::endl;
    }

    virtual void onIncomingCall(OnIncomingCallParam &iprm) {
        Call *      call = new MyCall(*this, iprm.callId);
        CallInfo    ci   = call->getInfo();
        CallOpParam prm;

        std::cout << "*** answerrer Incoming Call: " << ci.remoteUri << " [" << ci.stateText << "]"
                  << std::endl;

        calls.push_back(call);
        prm.opt        = PJSUA_CALL_REINIT_MEDIA | PJSUA_CALL_INCLUDE_DISABLED_MEDIA;
        prm.statusCode = (pjsip_status_code)200;
        std::cout << "answerring call" << std::endl;
        call->answer(prm);
        std::cout << "done answerring call" << std::endl;
    }
};

void MyCall::onCallState(OnCallStateParam &prm) {
    PJ_UNUSED_ARG(prm);

    CallInfo ci = getInfo();
    std::cout << "*** Answerrer Call State: " << ci.remoteUri << " [" << ci.stateText << "]"
              << std::endl;

    if (ci.state == PJSIP_INV_STATE_DISCONNECTED) {
        // myAcc->removeCall(this);
        /* Delete the call */
        // delete this;
    }
}

void MyCall::onCallMediaState(OnCallMediaStateParam &prm) {
    PJ_UNUSED_ARG(prm);

    unsigned i;
    // CallInfo   ci = getInfo();
    AudioMedia aud_med;
    Endpoint::instance().audDevManager().setPlaybackDev(14);
    // Endpoint::instance().audDevManager().setSndDevMode(PJSUA_SND_DEV_SPEAKER_ONLY);
    auto &play_dev_med = Endpoint::instance().audDevManager().getPlaybackDevMedia();

    pjmedia_aud_dev_index dev_idx;
    pj_status_t           status;
    auto                  dev_count = pjmedia_aud_dev_count();
    printf("Got %d audio devices\n", dev_count);
    for (dev_idx = 0; dev_idx < dev_count; ++dev_idx) {
        pjmedia_aud_dev_info info;
        status = pjmedia_aud_dev_get_info(dev_idx, &info);
        printf("%d. %s (in=%d, out=%d)\n", dev_idx, info.name, info.input_count, info.output_count);
    }
    std::cout << "answerrer Media state change" << std::endl;
    try {
        // Get the first audio media
        aud_med            = this->getAudioMedia(-1);
        const auto medInfo = aud_med.getPortInfo();
        std::cout << "med info: " << medInfo.name << " " << medInfo.portId << std::endl;
    } catch (...) {
        std::cout << "Failed to get audio media" << std::endl;
        return;
    }
    std::cout << "finishing onCallMediaState" << std::endl;

    // if (!wav_player) {
    //     wav_player = new AudioMediaPlayer();
    //     try {
    //         wav_player->createPlayer(testWavFile, 0);
    //     } catch (...) {
    //         std::cout << "Failed opening wav file" << std::endl;
    //         delete wav_player;
    //         wav_player = NULL;
    //     }
    // }

    // This will connect the wav file to the call audio media
    // std::cout << "starting to tx from wav to audio_media" << std::endl;
    // if (wav_player) wav_player->startTransmit(play_dev_med);

    // And this will connect the call audio media to the sound device/speaker
    // std::cout << "starting to transmit to speaker" << std::endl;
    aud_med.startTransmit(play_dev_med);
}

void MyCall::onCallTransferRequest(OnCallTransferRequestParam &prm) {
    /* Create new Call for call transfer */
    prm.newCall = new MyCall(*myAcc);
}

void MyCall::onCallReplaced(OnCallReplacedParam &prm) {
    /* Create new Call for call replace */
    prm.newCall = new MyCall(*myAcc, prm.newCallId);
}

static void mainProg3(Endpoint &ep) {
    const char *paths[] = {testWavFile};
    unsigned    i;
    const char *filename = NULL;

    // Init library
    EpConfig ep_cfg;
    ep.libInit(ep_cfg);

    for (i = 0; i < PJ_ARRAY_SIZE(paths); ++i) {
        if (pj_file_exists(paths[i])) {
            filename = paths[i];
            break;
        }
    }

    if (!filename) {
        PJSUA2_RAISE_ERROR3(PJ_ENOTFOUND, "mainProg3()", "Could not locate sine_test.wav");
    }

    // Start library
    ep.libStart();
    std::cout << "*** PJSUA2 STARTED ***" << std::endl;

    /* Use Null Audio Device as main media clock. This is useful for improving
     * media clock (see also https://trac.pjsip.org/repos/wiki/FAQ#tx-timing)
     * especially when sound device clock is jittery.
     */
    // ep.audDevManager().setNullDev();

    /* And install sound device using Extra Audio Device */
    ExtraAudioDevice auddev2(-1, -1);
    try {
        auddev2.open();
    } catch (...) { std::cout << "Extra sound device failed" << std::endl; }

    // Create player and recorder
    {
        AudioMediaPlayer amp;
        amp.createPlayer(filename);

        AudioMediaRecorder amr;
        amr.createRecorder("./recorder_test_output.wav");

        amp.startTransmit(amr);
        if (auddev2.isOpened()) amp.startTransmit(auddev2);

        pj_thread_sleep(5000);
    }
}

static void mainProg() {
    string json_str;

    {
        JsonDocument  jdoc;
        AccountConfig accCfg;

        accCfg.idUri                  = "\"Just Test\" <sip:test@pjsip.org>";
        accCfg.regConfig.registrarUri = "sip:sip.pjsip.org";
        SipHeader h;
        h.hName  = "X-Header";
        h.hValue = "User header";
        accCfg.regConfig.headers.push_back(h);

        accCfg.sipConfig.proxies.push_back("<sip:sip.pjsip.org;transport=tcp>");
        accCfg.sipConfig.proxies.push_back("<sip:sip.pjsip.org;transport=tls>");

        accCfg.mediaConfig.transportConfig.tlsConfig.ciphers.push_back(1);
        accCfg.mediaConfig.transportConfig.tlsConfig.ciphers.push_back(2);
        accCfg.mediaConfig.transportConfig.tlsConfig.ciphers.push_back(3);

        AuthCredInfo aci;
        aci.scheme   = "digest";
        aci.username = "test";
        aci.data     = "passwd";
        aci.realm    = "*";
        accCfg.sipConfig.authCreds.push_back(aci);

        jdoc.writeObject(accCfg);
        json_str = jdoc.saveString();
        std::cout << "Original:" << std::endl;
        std::cout << json_str << std::endl << std::endl;
    }

    {
        JsonDocument rdoc;

        rdoc.loadString(json_str);
        AccountConfig accCfg;
        rdoc.readObject(accCfg);

        JsonDocument wdoc;
        wdoc.writeObject(accCfg);
        json_str = wdoc.saveString();

        std::cout << "Parsed:" << std::endl;
        std::cout << json_str << std::endl << std::endl;
    }
}

int main() {
    try {
        Endpoint ep;
        ep.libCreate();

        // Init library
        EpConfig ep_cfg;
        ep_cfg.logConfig.level = 2;
        ep.libInit(ep_cfg);

        // Transport
        TransportConfig tcfg;
        tcfg.port          = 5060;
        tcfg.publicAddress = "localhost";
        ep.transportCreate(PJSIP_TRANSPORT_UDP, tcfg);

        // Start library
        ep.libStart();
        // ep.audDevManager().setNullDev();
        std::cout << "*** PJSUA2 STARTED ***" << std::endl;

        // Add account
        AccountConfig acc_cfg;
        // acc_cfg.idUri                  = "sip:test2@pjsip.org";
        acc_cfg.idUri                  = "sip:test2@localhost";
        acc_cfg.regConfig.registrarUri = "sip:sip.pjsip.org";
        acc_cfg.sipConfig.authCreds.push_back(AuthCredInfo("digest", "*", "test2", 0, "test2"));
        MyAccount *acc(new MyAccount);
        try {
            acc->create(acc_cfg);
        } catch (...) { std::cout << "Adding account failed" << std::endl; }

        std::cout << "*** Answerrer Waiting ***" << std::endl;
        // while (1) {}

        // Hangup all calls
        pj_thread_sleep(10000);
        // AudioMediaPlayer amp;
        // amp.createPlayer(testWavFile);
        // amp.startTransmit(call->getAudioMedia(-1));

        std::cout << "*** Answerrer hanging up ***" << std::endl;
        ep.hangupAllCalls();
        pj_thread_sleep(2000);

        // amp.startTransmit(amr);
        // if (auddev2.isOpened()) amp.startTransmit(auddev2);

        // Destroy library
        std::cout << "*** PJSUA2 SHUTTING DOWN ***" << std::endl;
        delete acc; /* Will delete all calls too */

        ep.libDestroy();
        std::cout << "Success" << std::endl;
        return 0;
    } catch (Error &err) {
        std::cout << "Answerrer Error Found" << std::endl;
        std::cout << "Exception: " << err.info() << std::endl;
        return 1;
    }
}
