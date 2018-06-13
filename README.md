# SchalterVox
A (WORKING, but horribly unfinished) media player for the Nintendo Switch

# What can it do?
It can currently play a single Ogg Vorbis file from the "oggs" directory (so e.g. `/switch/schaltervox/oggs/myogg.ogg`, which has to be 2 channels at 48000Hz (resampling is in the works but I've had some crash issues with libsamplerate). 

# SchalterVox?
"Schalter" is "switch" in german. "Vox" is "voice" in latin. Thus, "switch voice": A voice for your switch.

# Known Bugs
* Occasionally freezes during OGG load (issue has been rare lately).

# Immediate TODO
* Play more than one song. (in progress, see multiple-files-take2 branch)
* Fix resampling. (stalled due to problems with libsamplerate, see resampling branch, hopefully somebody who knows libsamplerate better can help)

# Later TODO
* Play other file types (MP3, FLAC, all modplug formats, WAV intended) (infrastructure in place)
* Ability to skip backwards/forwards in tracks (first by track, then eventually scrubbing within a track) (infrastructure in place for Vorbis)
* GUI (mocked up, private SDL prototype)

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

# Credits and special thanks.
## Credits
SchalterVox is coded by Dylan J "Insidious611" Morrison. Portions are based on code from the libnx examples and vba-next-switch, used under the MIT and GPLv2 licenses respectively. playback.cpp in particular owes an enormous debt to vba-next-switch.

## Thanks
My thanks to the users of the ReSwitched discord for providing guidance and patience to someone who is new to Switch homebrew, modern homebrew development in general, and hasn't touched C++ in so long he's practically new to that again too. Specific thanks go to
 * misson20000
 * natinsula
 * Retr0id

My thanks to Lancer-X and Lachesis of the MegaZeux development team for their help with threading, thread safety, and general C++ questions.

My thanks to my family, friends, and wonderful fiancee, without whom none of this would be possible or worth it.

# Licensing
SchalterVox is licensed under the GPLv3. See LICENSE for details. 
