#pragma once

#include <pjsua2.hpp>

#include <vector>

namespace ringer {
class RingerAccount : public pj::Account {
   public:
    RingerAccount(const std::string &name, const std::string &address);
    ~RingerAccount();

    void removeCall(pj::Call *call);

    virtual void onRegState(pj::OnRegStateParam &prm);
    virtual void onIncomingCall(pj::OnIncomingCallParam &iprm);

    std::vector<pj::Call *> calls;
};
}  // namespace ringer