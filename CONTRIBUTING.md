# How to contribute to SchalterVox!

## Bug Reporting

Before reporting issues, please attempt to reliably reproduce them first if possible. Specifially, it is probably not particularly helpful to me if your issue only shows up when you've run other homebrew recently without at least sleeping and waking your switch first. Clean reboots are preferred. It's early days in the homebrew scene, many of these apps are very experimental in nature and I have seen firsthand filesystem and memory corruption attributable to other apps causing havoc in SchalterVox.

When reporting an issue, please state first the version of the Switch firmware you are running, and then your homebrew setup. For example `My switch is on 2.0.0 and I'm using PegaSwitch` or `I am using Hekate/Atmosphere on 4.1.0`. Please note that as of this writing, the primary author is using Hekate/Atmosphere and 4.1.0.

Pull requests for suggested fixes are not required but appreciated if you have the ability.


## Pull Requests

There is no particular style guideline for the code at this time. This would be patently unfair, as I've not really followed a consistent style myself. New functionality that does not modify existing code should go in its own pair of .cpp and .hpp files, ie if you were to write an MP3 decoder it should go in mp3dec.cpp and mp3dec.hpp, with only hooks to use it added to audiofile.cpp. 

Pull requests should clearly state what you are attempting to implement or fix, and any issues you've encountered along the way that you suspect may still be present. I (Insidious611) may give corrections and suggestions, which should be followed up on within a reasonable timeframe. I reserve the right to refuse to merge a pull request without comment if suggestions/corrections are not addressed. Contributors should be familiar with the GPLv3 before contributing, as all accepted contributions will implicitly be licensed under it.
