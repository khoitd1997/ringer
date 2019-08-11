#include "ringer_endpoint.hpp"

namespace ringer {
RingerEndpoint& RingerEndpoint::getInstance(const int port, const std::string& publicAddr) {
    static RingerEndpoint singleTon{port, publicAddr};
    return singleTon;
}

pj::Endpoint* RingerEndpoint::get() { return &ep_; }
pj::Endpoint* RingerEndpoint::operator->() { return &ep_; }

RingerEndpoint::RingerEndpoint(const int port, const std::string& publicAddr) {
    ep_.libCreate();

    pj::EpConfig epCfg;
    epCfg.logConfig.level = 2;
    ep_.libInit(epCfg);

    pj::TransportConfig tcfg;
    tcfg.port          = port;
    tcfg.publicAddress = publicAddr;
    ep_.transportCreate(PJSIP_TRANSPORT_UDP, tcfg);

    ep_.libStart();
}

RingerEndpoint::~RingerEndpoint() {
    ep_.hangupAllCalls();
    pj_thread_sleep(2000);
    ep_.libDestroy();
}
}  // namespace ringer