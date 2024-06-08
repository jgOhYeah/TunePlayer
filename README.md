# TunePlayer ![Arduino Lint Actions Status](https://github.com/jgOhYeah/TunePlayer/actions/workflows/arduino-lint.yml/badge.svg)
An Arduino library to decode and play simple tunes.
<img alt="TunePlayer Logo" src="./extras/TunePlayer_MusescorePlugin/tuneplayer_logo.svg" width="200px" align="right">

Each note is stored as a 16-bit integer and the method of loading tunes and how each note is played is reconfigurable. The current anticipated method of tune playing revolves around a PWM square wave on a piezo siren or speaker.

## Quick links
- [MuseScore Plugin](extras/TunePlayer_MusescorePlugin/MusescorePlugin.md)
- [API Reference](extras/API.md)
- [Note structure](extras/NoteStructure.md)
- [MIDI Processor](extras/MidiProcessor.md)
- [Examples](examples)

This library is inspired (but not compatible with) the [PICAXE tune command](https://picaxe.com/basic-commands/digital-inputoutput/tune/)

This [cppQueue](https://github.com/SMFSW/Queue) library is required to be installed.

## Getting started
See the [simple example](examples/simple) for this in one file.

### Installation
Search for *TunePlayer* in the Arduino library manager or download this repository and copy it to the libraries folder (for me, *~/Documents/Arduino/libraries/*).
If not installed already, the [cppQueue](https://github.com/SMFSW/Queue) library should be installed as well.

### Hardware
The simplest way is to connect a piezo buzzer / siren (e.g. smoke alarm) to the output pin (pin 9 in the examples). For maximum loudness, you will probably need a driver circuit and tune things to resonate.
<!-- TODO: Link to [Bike Horn] repository -->

### Include the library
```c++
#include <TunePlayer.h>
```

### Add the tune
See the [MuseScore Plugin](extras/TunePlayer_MusescorePlugin/MusescorePlugin.md) page for a fairly straightforward way to generate the tunes.
```c++
// Converted from 'FucikEntryoftheGladiatorsPNO' by TunePlayer Musescore plugin V1.6
const uint16_t FucikEntryoftheGladiatorsPNO[] PROGMEM = {
    0xe118, // Tempo change to 280.0002 BPM
    0x3a38,0x2a38,0x1a18,0x2a18,0x1a18,0xa18,0xb838,
    // ...
    0xf000 // End of tune. Stop playing.
};
```

### Create the required objects
```c++
FlashTuneLoader flashLoader; // Where the notes come from
#define PIEZO_PIN 9
ToneSound piezo(PIEZO_PIN); // What plays the notes
TunePlayer tune; // Coordinates everything and does things at the right times.
```

### Set up to play
```c++
flashLoader.setTune(FucikEntryoftheGladiatorsPNO);
tune.begin(&flashLoader, &piezo);
```

### Play
```c++
tune.play();
```

### Regularly update
This needs to be called as often as possible while the tune is playing.
```c++
tune.update();
```

### Other functionality
See the [API reference](extras/API.md).