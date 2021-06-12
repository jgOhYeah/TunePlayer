/** simple.ino
 * Basic example for the Arduino TunePlayer library
 * 
 * Written by Jotham Gates, 12/06/2021
 */
// #define MANUAL_CUTOFF
#include <TunePlayer.h>
#define PIEZO_PIN 9

// Converted from 'Never_Gonna_Give_You_Up' by the To Arduino Tune V1.5
const uint16_t Never_Gonna_Give_You_Up[] PROGMEM = {
    0xe072, // Tempo change to 114 BPM
    0x5a58,0x7a58,0xa38,0x7a58,0x9c58,0xc08,0xac08,0x9c18,0x5a58,0x7a58,0xa98,0xc038,0xa08,0xa08,0x2a08,0x5a18,0x5a08,
    0xd412, // Repeat going back 18 notes
    0xc038,0x2a18,0x4a18,0x5a18,0x5a18,0x7a18,0x4a28,0x2a08,0xa98,0xc038,0xc018,0x2a18,0x2a18,0x4a18,0x5a18,0x2a38,0xa18,0xc18,0xc018,0xc18,0x7a58,0xc038,0x2a18,0x2a18,0x4a18,0x5a18,0x2a18,0x5a18,0x7a18,0xc018,0xc018,0x4a18,0x2a18,0xa58,0xc038,0xc018,0x2a18,0x2a18,0x4a18,0x5a18,
    0x2a18,0xa38,0x7a18,0x7a18,0x7a18,0x9c18,0x7a38,0xc038,0x5a98,0x7a18,0x9c18,0x5a18,0x7a18,0x7a18,0x7a18,0x9c18,0x7a38,0xa38,0xc078,0x2a18,0x4a18,0x5a18,0x2a18,0xc018,0x7a18,0x9c18,0x7a58,0xa08,0x2a08,0x5a08,0x2a08,0x9c28,0x9c28,0x7a58,0xa08,0x2a08,0x5a08,0x2a08,0x7a28,0x7a28,
    0x5a28,0x4a08,0x2a18,0xa08,0x2a08,0x5a08,0x2a08,0x5a38,0x7a18,0x4a28,0x2a08,0xa18,0xa18,0xa18,0x7a38,0x5a78,0xa08,0x2a08,0x5a08,0x2a08,0x9c28,0x9c28,0x7a58,0xa08,0x2a08,0x5a08,0x2a08,0xc38,0x4a18,0x5a28,0x4a08,0x2a18,0xa08,0x2a08,0x5a08,0x2a08,0x5a38,0x7a18,0x4a28,0x2a08,
    0xa38,0xa18,0x7a38,0x5a78,0xc038,0xc018,0x2a18,0x5a18,0x2a18,0x5a18,0x7a38,0xc018,0xc018,0x4a18,0x2a18,0xa58,0xc038,0xc018,0x2a18,0x2a18,0x4a18,0x5a18,0x2a18,0xa38,0xc018,0xc18,0xc18,0x7a38,0x9c18,0x7a18,0x5a18,0xc018,0x2a18,0x5a18,0x2a18,0x5a18,0x2a18,0x5a18,0x7a18,0xc018,
    0x4a18,0x2a18,0xa58,0xc038,0x2a18,0x2a18,0x4a18,0x5a18,0x2a18,0xa38,0xc018,0xc018,0x7a18,0x7a18,0x9c38,0x7a58,0x5a98,0x7a18,0x9c18,0x7a38,0x7a18,0x7a18,0x9c18,0x7a18,0xa18,0xa38,0xc058,0xa18,0x2a18,0x4a18,0x5a18,0x2a18,0xc018,0x7a18,0x9c18,0x7a58,0xa08,0x2a08,0x5a08,0x2a08,
    0x9c28,0x9c28,0x7a58,0xa08,0x2a08,0x5a08,0x2a08,0x7a28,0x7a28,0x5a28,0x4a08,0x2a18,0xa08,0x2a08,0x5a08,0x2a08,0x5a38,0x7a18,0x4a28,0x2a08,0xa38,0xa18,0x7a38,0x5a78,0xa08,0x2a08,0x5a08,0x2a08,0x9c28,0x9c28,0x7a58,0xa08,0x2a08,0x5a08,0x2a08,0xc38,0x4a18,0x5a28,0x4a08,0x2a18,
    0xa08,0x2a08,0x5a08,0x2a08,0x5a38,0x7a18,0x4a28,0x2a08,0xa38,0xa18,0x7a38,0x5a78,0xa08,0x2a08,0x5a08,0x2a08,0x9c28,0x9c28,0x7a58,0xa08,0x2a08,0x5a08,0x2a08,0x7a28,0x7a28,0x5a28,0x4a08,0x2a18,0xa08,0x2a08,0x5a08,0x2a08,0x5a38,0x7a18,0x4a28,0x2a08,0xa18,0xa18,0xa18,0x7a38,
    0x5a78,0xa08,0x2a08,0x5a08,0x2a08,0x9c28,0x9c28,0x7a58,0xa08,0x2a08,0x5a08,0x2a08,0x7a28,0x7a28,0x5a28,0x4a08,0x2a18,0xa08,0x2a08,0x5a08,0x2a08,0x5a38,0x7a18,0x4a28,0x2a08,0xa38,0xa18,0x7a38,0x5a78,0xc038,
    0xf000 // End of tune. Stop playing.
};

FlashTuneLoader flashLoader; // Where the notes come from
ToneSound piezo(PIEZO_PIN); // What plays the notes
// TimerOneSound piezo; // Limited to pin 9 only on atmega328p boards
TunePlayer tune; // Coordinates everything and does things at the right times.

void setup() {
    Serial.begin(38400);
    Serial.print(F("Simple TunePlayer example started on pin "));
    Serial.println(PIEZO_PIN);

    flashLoader.setTune(Never_Gonna_Give_You_Up);
    tune.begin(&flashLoader, &piezo);
    tune.play();
}

void loop() {
    tune.update();
}