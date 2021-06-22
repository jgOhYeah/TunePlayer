/** jukebox.ino
 * Example for the Arduino TunePlayer library.
 * Demonstrates the ability to play multiple tunes. Changes tune whenever
 * something is received from the Serial port.
 * 
 * Written by Jotham Gates
 * Created 22/06/2021
 * Modified 22/06/2021
 */
// #define MANUAL_CUTOFF // Required for TimerOneSound
#include <TunePlayer.h>
#define PIEZO_PIN 9

#include "tunes.h" // All the tunes are in another file for clarity.
const uint16_t *const tunes[] PROGMEM = {TakeOnMeIntroLoop, FucikEntryoftheGladiatorsPNO, Scale, Final_Countdown};
const uint8_t tuneCount = sizeof(tunes) / sizeof(uint16_t); // The length of the tunes array

FlashTuneLoader flashLoader; // Where the notes come from
ToneSound piezo(PIEZO_PIN); // What plays the notes
// TimerOneSound piezo; // Limited to pin 9 only on atmega328p boards
TunePlayer tune; // Coordinates everything and does things at the right times.

uint8_t current = 0;

void setup() {
    Serial.begin(38400);
    Serial.print(F("Jukebox TunePlayer example started on pin "));
    Serial.println(PIEZO_PIN);

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    tune.begin(&flashLoader, &piezo);
    changeTune(current);

}

void loop() {
    // Change tune if something sent to the serial port
    if(Serial.available()) {
        digitalWrite(LED_BUILTIN, HIGH);
        current++;
        if(current == tuneCount) {
            current = 0;
        }
        changeTune(current);

        // Clear ther serial buffer so it changes only once per time
        while(Serial.available()) {
            tune.update();
            Serial.read();
        }
        digitalWrite(LED_BUILTIN, LOW);
    }

    // Update often
    tune.update();
}

/** Changes to the tune in the array at index newTune */
void changeTune(uint8_t newTune) {
    Serial.print(F("Changing tune to "));
    Serial.println(newTune);
    flashLoader.setTune(pgm_read_word(&(tunes[newTune])));
    tune.stop(); // Reset to start
    tune.play(); // Start playing
}