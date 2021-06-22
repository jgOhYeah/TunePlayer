# TunePlayer
An Arduino library to decode and play simple tunes.

Each note is stored as a 16 bit integer and the method of loading tunes and how each note is played is reconfigurable. The current anticipated method of tune playing revolves around a pwm square wave on a piezo siren or speaker.

## Quick links
- [Musescore Plugin](extras/MusescorePlugin.md)
- [API Reference](extras/API.md)
- [Note structure](extras/NoteStructure.md)
- [Examples](examples)

This library is inspired (but not compatible with) the [PICAXE tune command](https://picaxe.com/basic-commands/digital-inputoutput/tune/)

## Getting started
See the [simple example](examples/simple) for this in one file.

### Hardware
The simplest way is to connect a piezo buzzer / siren (e.g. smoke alarm) to the output pin (pin 9 in the examples). For makimum loudness, you will probably need a driver circuit and tune things to resonate.
<!-- TODO: Link to [Bike Horn] repository -->

### Include the library
```c++
#include <TunePlayer.h>
```

### Add the tune
See the [Musescore Plugin](extras/MusescorePlugin.md) page for a fairly straightforward way to generate the tunes.
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