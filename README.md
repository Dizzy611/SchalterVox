# SchalterVox
A (WORKING, but horribly unfinished) media player for the Nintendo Switch


# Known Bugs
* Occasionally freezes during OGG load.
* Vorbis decode thread sometimes (but not always) disobeys signal to quit. (keep hitting the "+" button if it doesn't seem to be stopping! I need to put input on its own thread)

# Immediate TODO
* Make pressing + cause an immediate stop instead of waiting for buffers to flush.
* Play more than one song.
* Fix resampling.
* Tweaks to buffer sizes to improve responsiveness/decrease initial track load time.
* Code cleanup

# Later TODO
* Play other file types (MP3, FLAC, all modplug formats, WAV intended)
* Ability to skip backwards/forwards in tracks (first by track, then eventually scrubbing within a track)
* GUI

# Much Later TODO
* Video playback

# How to test
* *DOES NOT WORK ON YUZU* (Yuzu is lacking audio support atm)
* Compile .nro, put it in a folder in switch/, then within that folder create an "oggs" folder and put a single 48000Hz, 2 channel Ogg Vorbis file (any quality) in it.

Example directory tree:
```
/switch/schaltervox/schaltervox.nro
/switch/schaltervox/oggs/Studiopolis.ogg
```


* Run SchalterVox. It may crash after "Loading first ogg...". Run it again and it will usually work the second time.
* When done testing, press "+" to exit the decode loop, then "+" to exit the main application loop. The playback should (but does not always, currently :/) stop immediately.
