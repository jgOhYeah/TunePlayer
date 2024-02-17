/** simple.ino
 * Basic example for the Arduino TunePlayer library
 * 
 * Written by Jotham Gates, 12/06/2021
 */
// #define USE_TIMER_ONE
#ifdef USE_TIMER_ONE
#define MANUAL_CUTOFF // Required for TimerOneSound
#endif
#include <TunePlayer.h>

#define PIEZO_PIN 9 // Can be just about any pin if using ToneSound, needs to be pin D9 on a Uno, Nano, Micro, etc for TimerOneSound.

// Adapted from this midi file: https://www.8notes.com/scores/16380.asp?ftype=midi
// See FucikEntryoftheGladiatorsPNO.mscz in extras/songs for an example of a suitable file to convert
// Converted from 'FucikEntryoftheGladiatorsPNO' by TunePlayer Musescore plugin V1.6.1
const uint16_t FucikEntryoftheGladiatorsPNO[] PROGMEM = {
    0xe118, // Tempo change to 280.0002 BPM
    0x3a38,0x2a38,0x1a18,0x2a18,0x1a18,0xa18,0xb838,0xa838,0x9838,0xa838,0xa38,0xb838,0xa818,0xb818,0xa818,0x9818,0x8838,0x7838,0x6838,0x7838,0xa838,0x8818,0x8818,0x4838,0x5838,0xa838,0x8818,0x8818,0x4838,0x5838,0x2818,0x3818,0x4818,0x5818,0x6818,0x7818,0x8818,0x9818,0xa818,0xb818,
    0xa18,0x1a18,0xa38,0xa838,0x3a38,0x2a38,0x1a18,0x2a18,0x1a18,0xa18,0xb838,0xa838,0x9838,0xa838,0xa38,0xb838,0xa818,0xb818,0xa818,0x9818,0x8838,0x7838,0x6838,0x7838,0x6838,0x6818,0x6818,0x9838,0x2838,0xa818,0xa18,0xa818,0x9818,0x7838,0xa838,0x2a38,0x2a18,0x2a18,0x2a38,0x2a38,
    0x2a18,0x2a18,0x2a18,0x2a18,0x2a18,0x2a18,0x2a38,0x3a38,0x2a38,0x1a18,0x2a18,0x1a18,0xa18,0xb838,0xa838,0x9838,0xa838,0xa38,0xb838,0xa818,0xb818,0xa818,0x9818,0x8838,0x7838,0x6838,0x7838,0xa838,0x8818,0x8818,0x4838,0x5838,0xa838,0x8818,0x8818,0x4838,0x5838,0x2818,0x3818,0x4818,
    0x5818,0x6818,0x7818,0x8818,0x9818,0xa818,0xb818,0xa18,0x1a18,0xa38,0xa838,0x3a38,0x2a38,0x1a18,0x2a18,0x1a18,0xa18,0xb838,0xa838,0x9838,0xa838,0xa38,0xb838,0xa818,0xb818,0xa818,0x9818,0x8838,0x7838,0x6838,0x7838,0xa838,0xa818,0xa818,0xa838,0x5838,0xb838,0xb818,0xb818,0x6878,
    0x7818,0xa818,0x9818,0x8818,0x7838,0x5838,0x3838,0xc038,0x3a38,0xc038,
    0xf000 // End of tune. Stop playing.
};

FlashTuneLoader flashLoader; // Where the notes come from
#ifdef USE_TIMER_ONE
TimerOneSound piezo; // Limited to pin 9 only on atmega328p boards
#else
ToneSound piezo(PIEZO_PIN); // What plays the notes
#endif
TunePlayer tune; // Coordinates everything and does things at the right times.

void setup() {
    Serial.begin(38400);
    Serial.print(F("Simple TunePlayer example started on pin "));
    Serial.println(PIEZO_PIN);

    flashLoader.setTune(FucikEntryoftheGladiatorsPNO);
    tune.begin(&flashLoader, &piezo);
    tune.play();
}

void loop() {
    tune.update();
}
