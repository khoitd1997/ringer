#include "ringer_account.hpp"

#include <pj/file_access.h>
#include <pjmedia_audiodev.h>

#include <algorithm>
#include <iostream>
#include <string>

#include "ringer_call.hpp"
#include "ringer_logger.hpp"

namespace ringer {
RingerAccount::RingerAccount(const std::string &name, const std::string &address) {
    pj::AccountConfig accConfig;
    accConfig.idUri                  = "sip:" + name + "@" + address;
    accConfig.regConfig.registrarUri = "sip:sip.pjsip.org";
    accConfig.sipConfig.authCreds.push_back(pj::AuthCredInfo("digest", "*", name, 0, name));

    try {
        this->create(accConfig);
    } catch (...) {
        logger::error("Adding account failed");
        exit(1);
    }
}
RingerAccount::~RingerAccount() {
    logger::info("*** Account is being deleted: No of calls=" + std::to_string(calls.size()));
    for (auto &call : calls) { delete (call); }
}

void RingerAccount::removeCall(pj::Call *callToRemove) {
    calls.erase(std::remove_if(calls.begin(),
                               calls.end(),
                               [callToRemove](pj::Call *call) { return callToRemove == call; }),
                calls.end());
}

void RingerAccount::onRegState(pj::OnRegStateParam &prm) {
    const auto ai = this->getInfo();
    logger::info((ai.regIsActive ? "*** Register: code=" : "*** Unregister: code=") +
                 std::to_string(ai.regStatus));
}

void RingerAccount::onIncomingCall(pj::OnIncomingCallParam &iprm) {
    auto       call = new RingerCall(false, *this, iprm.callId);
    const auto ci   = call->getInfo();

    logger::info("*** Incoming Call: " + ci.remoteUri + " [" + ci.stateText + "]");

    calls.push_back(call);

    pj::CallOpParam prm;
    prm.statusCode = (pjsip_status_code)200;
    call->answer(prm);
}
}  // namespace ringer