# API Reference
- [Tune Pipeline](#tune-pipeline)
- [Class Diagram](#class-diagram)
- [Constants and definitions](#constants-and-definitions)
- [`TunePlayer` Class](#tuneplayer-class)
- [`BaseTuneLoader` Abstract Class](#basetuneloader-abstract-class)
    - [`FlashTuneLoader` Derived Class](#flashtuneloader-derived-class)
    - [`EEPROMTuneLoader` Derived Class](#eepromtuneloader-derived-class)
    - [`RAMTuneLoader` Derived Class](#ramtuneloader-deried-class)
- [`SoundGenerator` Abstract Class](#soundgenerator-abstract-class)
    - [`ToneSound` Derived Class](#tonesound-derived-class)
    - [`TimerOneSound` Derived Class](#timeronesound-derived-class)

For more info, feel free to look at the class diagram, examples and [source code](../src), or create an issue.

## Tune Pipeline
This diagram may help to give an overview of how a note goes from data in memory to sound. Dashed lines represent an action being started, thin solid lines are addresses being transferred and thick solid lines are the notes in their various states being transferred between components.

<image src="images/Pipeline.svg" alt="Diagram of transformations from raw data to sound" width="650px">

While playing a tune, the `TuneLoader` is asked for a note at an address. It returns the raw data, which is processed. If the note is something that makes a noise, a rest or an end of song with no repeat message, it is added to a queue to be picked up when needed by another part that plays the notes on time.

If the note is a repeat, the current address of the note is compared to the top of a stack of repeats addresses. This keeps a record of it we have come accross it before. If the address is on the top of the stack, than it has been seen once already and should be skipped. Otherwise, the address counter is changed as required and the address of the repeat is added to the repeats stack.

End of song with repeats messages just reset the address counter back to zero.

## Class Diagram
Excuse the probably not proper arrows, but better than nothing.
![Class diagram](images/ClassDiagram.svg)

## Constants and definitions
If needed, define these before including `TunePlayer.h`.

|        Definition        | Comment                                                                                                                                                                                                                                                                                                                  | Default value |
| :----------------------: | :----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | :-----------: |
|     `MANUAL_CUTOFF`      | If defined, the `stopSound` method of the `SoundGenerator` object will be called whenever the note ends. This is not necessary for implementations that stop sound automatically like `ToneSound`, but is required for the likes of `TimerOnesound` that will keep playing until stopped.                                |   Undefined   |
|     `PRECISE_FREQS`      | If defined and using `ToneSound`, every single frequency of each note is stored as an array and loaded. This is still limited to integer values, but may be sometimes marginally more accurate than the default method of halving from an array containing only the highest notes, although a lot less memory efficient. |   Undefined   |
|    `ENABLE_CALLBACKS`    | If enabled, the `setCallOnStop` method in `TunePlayer` will be enabled. This will allow calling a function when the tune stops playing. An example use is to load and start playing the next tune.                                                                                                                       |   Undefined   |
|    `NOTES_QUEUE_MAX`     | The number of notes that are buffered in the queue of notes to play at a time. If loading notes and playing in different tasks with different priorities, increasing this will allow the loading of notes to happen less frequently, but will increase the delay from calling `play` to sound actually being produced    |       4       |
| `REPEATS_MAX_CONCURRENT` | The maximum number of repeats that will be expected at a time (levels of repeats inside repeats)                                                                                                                                                                                                                         |       4       |
|     `DEFAULT_TEMPO`      | The default tempo in beats per minute to play at before a set tempo note is received.                                                                                                                                                                                                                                    |      120      |
|    `TIMER_ONE_SOUND`     | If defined, will force the inclusion of the `TimerOneSound`. This is included if `AVR` or `TIMER_ONE_SOUND` is defined.                                                                                                                                                                                                  |   Undefined   |

## `TunePlayer` Class
This class does handles interpreting notes given by a `TuneLoader` class, timing and passing the correct note to play on to a `SoundGenerator` object.

#### Constructor
###### Example
No parameters given.
```c++
TunePlayer tune;
```

#### Attributes
|       Name       |     Data Type     | Comments                                                    |   Default   |
| :--------------: | :---------------: | :---------------------------------------------------------- | :---------: |
|   `tuneLoader`   | `BaseTuneLoader*` | Pointer to an object that can load and return notes to play | unspecified |
| `soundGenerator` | `SoundGenerator*` | Pointer to an object that can turn notes into sound.        | unspecified |

#### Methods
##### `void begin(BaseTuneLoader *newTuneLoader, SoundGenerator *newSoundGenerator)`
Initialises the library with the given parameters. Sets `tuneLoader` and `soundGenerator`.
Also initialises `soundGenerator` and `tuneLoader` by calling their `begin` methods.

##### `void begin()`
Initialises `soundGenerator` and `tuneLoader` by calling their `begin` methods. `soundGenerator` and `tuneLoader` need to be set before this method is called, or can be passed as parameters (see above).

##### `void spool()`
Fills up the queue of notes for playback, ready for a fast response from calling `play()` to sound actually coming out.

##### `void play()`
Enables playback of the tune. Playback will start on the next call to `update()`.

##### `void update()`
Performs all housekeeping operations necessary for tune loading and playback. This should be called as often as possible.

##### `void updateLowPriority()`
Performs low priority tasks and housekeeping functions. This can be used for example with some form of multitasking.

##### `void updateHighPriority()`
Performs high priority tasks (playing at the right time). This can be used for example with some form of multitasking.

##### `void pause(bool holdNote = false)`
Stops making noise, but keeps the queue of notes and pointers to the next note. This means that `play` can be called and the tune will keep playing from the next note after the one on which the tune was paused.

If `holdNote` is `true`, the note will be played for its normal time (not cut off immediately), or for `SoundGenerators` that do not stop automatically like `TimerOneSound`, will keep playing forever.

##### `void stop()`
Stops making noise, clears the queue of notes and resets the notes address back to the start of the song. This also resets the tempo to the default (`DEFAULT_TEMPO` bpm). This means on calling `play` next time, the tune will restart from the beginning.

Calling `stop` can also be used when changing tunes - see the [jukebox example](../examples/jukebox/jukebox.ino).

##### `inline bool isPlaying()`
Returns true if a tune is currently being played, otherwise false.

##### `inline void setCallOnStop(CallbackFunction callbackFunction)`
Sets the function that should be called when the tune stops. This is only enabled when `ENABLE_CALLBACKS` is defined.

In other words, this is when `isPlaying()` becomes false. i.e. on the tune finishing or the `stop` or `pause` methods being called.

`callbackFunction` is the function to call (must not return anything and must accept no parameters).

To disable the callbacks, use `setCallOnStop(NULL);`

###### Example
```c++
#define ENABLE_CALLBACKS
// ...

void myCallbackFunction() {
    Serial.println(F("Tune has stopped playing"));
}

// To register
tune.setCallOnStop(myCallbackFunction());

// To deregister
tune.setCallOnStop(NULL);
```

## `BaseTuneLoader` Abstract Class
Helper classes for `TunePlayer` that help with loading of notes from required sources. Each loader should inherrit `BaseTuneLoader` and have the `loadNote` method at least.

#### Attributes
None

#### Methods
##### `virtual void begin()`
Placeholder for if there is any initialisation required.

##### `virtual uint16_t loadNote(uint16_t address)`
Placeholder for the method that loads a 2 byte integer at address `address` in the song and returns it. The first note is address *0*, the last note is *number of notes - 1*.

### `FlashTuneLoader` Derived Class
Uses the `PROGMEM` feature of AVR chips to load tunes stored in the program memory.

#### Attributes
None

#### Methods
##### `void setTune(uint16_t *tune)`
Sets the array in program memory to load from.

##### `uint16_t* getTune()`
Returns the array in program memory that the tune is being loaded from.

###### Example
```c++
const uint16_t FucikEntryoftheGladiatorsPNO[] PROGMEM = {
    0xe118, // Tempo change to 280.0002 BPM
    0x3a38,0x2a38,0x1a18,0x2a18,0x1a18,0xa18,
    //...
    0xf000 // End of tune. Stop playing.
};
flashLoader.setTune(FucikEntryoftheGladiatorsPNO);
```

### `EEPROMTuneLoader` Derived Class
TODO
<br>**NOTE:** NOT IMPLEMENTED YET!!!

### `RAMTuneLoader` Derived Class
TODO
<br>**NOTE:** NOT IMPLEMENTED YET!!!

## `SoundGenerator` Abstract Class
A set of classes that can output sound at the right times.

#### Constructor
No parameters required.
#### Attributes
None

#### Methods
##### `virtual void begin()`
Sets up the class. Called by `TunePlayer.begin`.

##### `virtual void playNote(uint8_t note, uint8_t octave, uint32_t playTime=0)`
Plays a note. `playTime` is the time in microseconds that the note should be played for. If this is not supported or `playNote` is 0, then the note should play forever.

##### `virtual void stopSound()`
Stops making noises. If `MANUAL_CUTOFF` is defined, `TunePlayer` will call this method to cut off each note.

##### `void playMidiNote(uint8_t midiNote)`
Takes a midi note (*0-127*) and calls `playNote` with the correct note and octave.

### `ToneSound` Derived Class
Uses the built in Arduino [`tone`](https://www.arduino.cc/reference/en/language/functions/advanced-io/tone/) function.

### `TimerOneSound` Derived Class
Uses Timer One in avr based boards (atmega328p specifically). This offers a low more control over things such as the duty cycle. This class can be inherited by another class that overrides the `uint16_t m_compareValue(uint16_t counter)` method that returns the value at which the pin should go low when using fast pwm mode. `counter` is the value that the timer will reset at.

See [this code](https://github.com/jgOhYeah/BikeHorn/blob/main/BikeHorn/soundGeneration.h) for an example (and original target) for the Timer One method.

### `MIDISound` Derived Class
TODO: Not implemented yet.
Outputs a midi instruction to play the note.

### `TeeSound` Derived Class
TODO: Not implemented yet.
Splits sound inputs into multiple outputs to allow many SoundGenerator objects to be controlled at a time (in unison).