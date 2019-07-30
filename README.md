# ringer

voice-over-ip local phone system

## Facts

Audio transmitted over phones are from 300Hz to 3.4kHz(4kHz including guard band)

## Terms

- Wide-band codec: Wider frequency range of recorded voice, resulting in HD voice quality, the G722 is in this category

## Codec choice

https://blog.flowroute.com/2013/07/16/the-case-against-g-722/ (why g722 may not be chosen)

http://opus-codec.org/comparison/ (opus comparision)

https://blog.simwood.com/2017/02/opus/ (opus indepth comparision)

https://en.wikipedia.org/wiki/Xiph.Org_Foundation (check the projects section for list of codec)

```Speex``` was considered but its front page straight up advises using ```opus```(which have been shown to be possible on TM4C). ```G711``` and ```G722``` was recommended as well, they are proprietary but *their licenses have expired*

- G722: Encodes at 8kHz, make it easier to distinguish stuffs like 'f' and 's'
- OPUS: Designed to make all lossy compression format obsolete, should really use this one. Designed originally for voice-over-ip has good packet loss and full-band support

## pjsip notes

### Dependencies

These are for testing on a Fedora computer

```shell
sudo dnf install -y alsa-lib-devel uuid-devel
```

### Inner Working notes

The ```sound.h``` struct has been deperacated in favor of ```https://trac.pjsip.org/repos/wiki/Audio_Dev_API```, the new API is more extensible and allows for better control of the device, this new one is the ```pjmedia-audiodev``` library

### porting to other OS

people have done it for embedded OS

https://www.pjsip.org/testing/0.5.6.5/footprint-arm-rtems.htm

most efforts will be in ```pjlib``` and ```pjmedia```, use ```pjtest``` to evaluate whether port is completed. Efforts 
