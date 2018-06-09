# SchalterVox
A (currently broken as hell) media player for the Nintendo Switch


# Known Bugs
* Unlistenable, stuttering playback
* Occasionally freezes during OGG load.
* Vorbis decode thread sometimes (but not always) disobeys signal to quit.


# Things that have been tried already to fix the stuttering issue, to no real difference.
* Increasing/decreasing the decode buffer size (and thus the amount of bytes ov_read attempts to decode at once)
* Increasing/decreasing the individual audio buffer size (the amount of PCM bytes pushed to the Switch at once)
* Increasing/decreasing the audio transfer buffer size (a multiple of the audio buffer size, where data is stored before being pushed to the Switch buffers)

# How to test
* *DOES NOT WORK ON YUZU* (Yuzu is lacking audio support atm)
* Compile .nro, put it in a folder in switch/, then within that folder create an "oggs" folder and put a single 48000Hz, 2 channel Ogg Vorbis file (any quality) in it.

Example directory tree:
```
/switch/schaltervox/schaltervox.nro
/switch/schaltervox/oggs/Studiopolis.ogg```

