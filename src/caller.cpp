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

        std::cout << "*** caller Incoming Call: " << ci.remoteUri << " [" << ci.stateText << "]"
                  << std::endl;

        calls.push_back(call);
        prm.statusCode = (pjsip_status_code)200;
        call->answer(prm);
    }
};

void MyCall::onCallState(OnCallStateParam &prm) {
    PJ_UNUSED_ARG(prm);

    CallInfo ci = getInfo();
    std::cout << "*** Caller Call: " << ci.remoteUri << " [" << ci.stateText << "]" << std::endl;

    if (ci.state == PJSIP_INV_STATE_DISCONNECTED) {
        // myAcc->removeCall(this);
        /* Delete the call */
        // delete this;
    }
    std::cout << "*** Finnish Caller Call State" << std::endl;
}

void MyCall::onCallMediaState(OnCallMediaStateParam &prm) {
    std::cout << "Caller Media state change" << std::endl;
    PJ_UNUSED_ARG(prm);

    unsigned i;
    CallInfo ci = getInfo();
    Endpoint::instance().audDevManager().setCaptureDev(-1);
    auto &captureDev = Endpoint::instance().audDevManager().getCaptureDevMedia();

    //     auto             callerMedia = call->getAudioMedia(-1);
    // AudioMediaPlayer callerPlayer;
    // try {
    //     callerPlayer.createPlayer(testWavFile, 0);
    // } catch (...) { std::cout << "Failed opening caller wav file" << std::endl; }
    // callerPlayer.startTransmit(callerMedia);
    // std::cout << "*** SAY SOMETHING ***" << std::endl;

    AudioMedia aud_med;
    try {
        // Get the first audio media
        aud_med = getAudioMedia(-1);
    } catch (...) {
        std::cout << "Failed to get audio media" << std::endl;
        return;
    }

    if (!wav_player) {
        wav_player = new AudioMediaPlayer();
        try {
            wav_player->createPlayer(testWavFile, 0);
        } catch (...) {
            std::cout << "Failed opening wav file" << std::endl;
            delete wav_player;
            wav_player = NULL;
        }
    }

    // This will connect the wav file to the call audio media
    // AudioMediaRecorder amr;
    // amr.createRecorder("./answerrer.wav");
    // aud_med.startTransmit(amr);
    // if (wav_player) wav_player->startTransmit(aud_med);

    wav_player->startTransmit(aud_med);

    std::cout << "Say something NOW" << std::endl;
    pj_thread_sleep(60000);
}

void MyCall::onCallTransferRequest(OnCallTransferRequestParam &prm) {
    std::cout << "Caller tx req" << std::endl;
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
    ep.audDevManager().setNullDev();

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
        // ep_cfg.logConfig.level = 5;
        ep_cfg.logConfig.level = 2;
        ep.libInit(ep_cfg);

        // Transport
        TransportConfig tcfg;
        tcfg.port          = 5061;
        tcfg.publicAddress = "localhost";
        ep.transportCreate(PJSIP_TRANSPORT_UDP, tcfg);

        // Start library
        ep.libStart();
        // ep.audDevManager().setNullDev();
        std::cout << "*** PJSUA2 STARTED ***" << std::endl;

        // Add account
        AccountConfig acc_cfg;
        // acc_cfg.idUri                  = "sip:test1@pjsip.org";
        acc_cfg.idUri                  = "sip:test1@pjsip.org";
        acc_cfg.regConfig.registrarUri = "sip:sip.pjsip.org";
        acc_cfg.sipConfig.authCreds.push_back(AuthCredInfo("digest", "*", "test1", 0, "test1"));
        MyAccount *acc(new MyAccount);
        try {
            acc->create(acc_cfg);
        } catch (...) { std::cout << "Adding account failed" << std::endl; }

        pj_thread_sleep(2000);

        // Make outgoing call
        Call *call = new MyCall(*acc);
        acc->calls.push_back(call);
        CallOpParam prm(true);
        prm.opt.audioCount = 1;
        prm.opt.videoCount = 0;
        // sip:178237@192.168.10.10:5090
        // 162.194.129.207:5060
        call->makeCall("sip:test2@localhost:5060", prm);

        // AudioMediaRecorder amr;
        // amr.createRecorder("./recorder_test_output.wav");

        // Hangup all calls

        pj_thread_sleep(6000);
        // AudioMediaPlayer amp;
        // amp.createPlayer(testWavFile);
        // amp.startTransmit(call->getAudioMedia(-1));
        pj_thread_sleep(2000);

        std::cout << "*** HANGING UP NOW ***" << std::endl;
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
        std::cout << "Error Found" << std::endl;
        std::cout << "Exception: " << err.info() << std::endl;
        return 1;
    }
}
