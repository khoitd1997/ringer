# pjsip notes

## References

[SIP URI](https://en.wikipedia.org/wiki/SIP_URI_scheme)

[Conference Call for multiple user](https://stackoverflow.com/questions/27501442/how-to-call-group-using-pjsip)

[Call transfer](https://www.pjsip.org/pjsip/docs/html/group__PJSUA__XFER.htm)

[SIP protocol](https://tools.ietf.org/html/rfc3261#section-16.4)

## Design

Since the location of the exact user is determined dynamically, the address isn't fixed. Have the client do a DNS lookup on where the user is and then call that number. When user moves to different room, do a call transfer(reference link above)

## Terms

- SIP Proxy: Equivalent to SIP server, job is to find the target to initiate the session and then drop out to allow point to point communication
- Stateless vs stateful proxy: One just forward request the other one saves state and may do things like resend messages in case of error
- STUN server: Used when wanting to reach voiceIP server outside of local network, used only for establishing connections, not when actually exchanging media
- TURN server: More involved protocols, use STUN to establish and then all media must pass through TURN

## Customize Routing

The step that would allow customization of routing is during the [Target Determination](https://tools.ietf.org/html/rfc3261#section-16.5), the ```Location Service``` defined in the paper is the thing that we can customize. In the ```stateless_proxy.c``` example provided by pjsip, the customized part if left emtpy in ```proxy.h```, although the ```proxy_postprocess``` function is a good example of how to set the target.

Use the ```DHCP hostname``` of devices to identify them(some devices may not support them but the esp32 does)

## Dependencies

These are for testing on a Fedora computer

```shell
sudo dnf install -y alsa-lib-devel uuid-devel
```

## Inner Working notes

The ```sound.h``` struct has been deperacated in favor of ```https://trac.pjsip.org/repos/wiki/Audio_Dev_API```, the new API is more extensible and allows for better control of the device, this new one is the ```pjmedia-audiodev``` library

## porting to other OS

people have done it for embedded OS

https://www.pjsip.org/testing/0.5.6.5/footprint-arm-rtems.htm

most efforts will be in ```pjlib``` and ```pjmedia```, use ```pjtest``` to evaluate whether port is completed. Efforts 
