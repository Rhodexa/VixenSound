# VixenSound / CTS7
### A versatile stereo polyphonic wavetable synthesizer for ATMEGAxx8P MCUs

Formerly known as Chiptune Studio v7 (CTS7). This code now entered the Vixen family. As with all other Vixen programs, this is a platform-specific piece of software. In this case designed for the ATMEGA328P running at 16MHz. 
It could work on other systems too, but I didn't take them into account. 

VixenSound CTS7 was made to prove myself a point: You can make fancy chiptune with an Arduino Nano.
The first version of CTS (Chiptune Studio 1) was made a long time ago during a robotics hangout with friends. I barely knew much about Arduino or C++ at the time, but I had some intuition. One of my best friends asked me if I could make a puece of code to play two different notes on two different speakers ( he's first thought was using 'tone()' of course ). I accepted the challenge with joy, and managed to make that work at the time (CTS2); a simple synth that played two square waves simultaneously using timers 1 and 2. But I thought 'We can do better than this!', and the project sat abandoned for a while; Afterall, my friend already had what he wanted. Later I went back to it, and made a few not so great revisions CTS3-5.Then I discovered the concept of the Phase Accumulator and Wave Tables, used in many of the most classic chiptune chips, like the OPNA, OPNB, OPL2, OPL3, OPLL, SID, and many others. < Realisation Moment! > I sat down to work: A a few weeks later I had CTS6, which is still around in some repository. CTS6 had multiple channels, with panning, and volume control, FM synthessis and what not.

Later, I retook the project once more to launch it's final version: CTS7, simpler, elegant, compact, fast. It was inspired by the mechanisms used by SPC700 (SNES) combined with other lesser known WT samplers.

This repository is just a new home for it, as it was originally stored alone in a public Google Drive folder. 
I will make a few more changes later to make it more elegant. But this project is in fact pretty much abandoned now, since the ATMEGA328P was pretty much swept away by modern MCUs with 10+ the power at the same price. I see no use for this creation at the present moment. Nonetheless is a simple, not really groundbreaking, project I'm proud of, and it's a nice curiosity too!

Have fun! Hugs!
P.S: I'm aware I possibly made more than one typo. Welp.
