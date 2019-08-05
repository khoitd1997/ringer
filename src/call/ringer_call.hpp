#pragma once

#include <pjsua2.hpp>

#include <string>

#include "ringer_account.hpp"

namespace ringer {
class RingerCall : public pj::Call {
   public:
    RingerCall(const bool isCaller, pj::Account &acc, int call_id = PJSUA_INVALID_ID);
    ~RingerCall();

    virtual void onCallState(pj::OnCallStateParam &prm);
    virtual void onCallTransferRequest(pj::OnCallTransferRequestParam &prm);
    virtual void onCallReplaced(pj::OnCallReplacedParam &prm);
    virtual void onCallMediaState(pj::OnCallMediaStateParam &prm);

    const bool isCaller;

   private:
    const std::string roleName_;

    RingerAccount *       myAcc;
    pj::AudioMediaPlayer *wav_player;
};
}  // namespace ringer