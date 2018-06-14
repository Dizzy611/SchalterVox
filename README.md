[![Build status](https://ci.appveyor.com/api/projects/status/w251kvnbnka3h00o/branch/master?svg=true)](https://ci.appveyor.com/project/Insidious611/schaltervox/branch/master)

If the build is passing, see "Releases" for the latest release. Please be aware SchalterVox is still Alpha code and is lacking a proper UI or transport controls.

# SchalterVox
A (WORKING, but horribly unfinished) media player for the Nintendo Switch

# What can it do?
It can currently play multiple Ogg Vorbis files from the "media" directory (so e.g. `/switch/schaltervox/media/myogg.ogg`, which have to be 2 channels. SchalterVox can now handle samplerates other than 48000Hz, but will do only basic linear resampling. Cubic and eventually sinc and catmull rom are coming. Work on supporting mono OGGs is underway. Higher channel counts will not be supported until we have the GUI, as I'll be relying on SDL_mixer to help with downmixing.

# SchalterVox?
"Schalter" is "switch" in german. "Vox" is "voice" in latin. Thus, "switch voice": A voice for your switch.

# Known Bugs
* Occasionally freezes during OGG load (issue has been rare lately).

# Immediate TODO
* Play other file types (MP3, FLAC, all modplug formats, WAV intended) (infrastructure in place)
* Ability to skip backwards/forwards in tracks (first by track, then eventually scrubbing within a track) (infrastructure in place for Vorbis)
* GUI (mocked up, private SDL prototype)

# Much Later TODO
* Video playback

# How to test
* *DOES NOT WORK ON YUZU* (Yuzu is lacking audio support atm)
* Compile .nro, put it in a folder in switch/, then within that folder create "media" folder and put any number of 2 channel Ogg Vorbis files (any quality) in it. It will play them in "filesystem order" (usually order that they were moved into the folder). Playlists and selecting individual files will come later.

Example directory tree:
```
/switch/schaltervox/schaltervox.nro
/switch/schaltervox/media/Studiopolis.ogg
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

My thanks to Lancer-X (again) and Madbrain for helping me crack the resampling problem.

My thanks to my family, friends, and wonderful fiancee, without whom none of this would be possible or worth it.

# Licensing
SchalterVox is licensed under the GPLv3. See LICENSE for details. 
