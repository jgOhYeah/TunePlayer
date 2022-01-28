/** midiSynthDualChannel.ino
 * Example for the Arduino TunePlayer library.
 * Demonstrates using multiple SoundGenerators for something other than with
 * TunePlayer.
 * 
 * Designed for something like hairless midi serial or ttymidi to convert
 * between a midi device and a serial port on a computer - Or have a proper
 * midi shield or trickery to make the Arduino appear as an actual midi device.
 * 
 * It might be helpful to use a filtering / routing program like midi ox
 * (Windows) or qmidirouter (Linux) to map all channels into the one this is
 * listening to - or modify this demo to listen to all channels when trying midi
 * files.
 * 
 * Written by Jotham Gates
 * Created some time in 2016
 * Modified 03/11/2021
 */
 #define MANUAL_CUTOFF
#include <TunePlayer.h>
#define PIEZO_TONE_PIN 2
#define PIEZO_TIMER1_PIN 9
#define MIDI_CHANNEL_P0 0 // Zero indexed, so many software shows ch. 0 as ch. 1
#define MIDI_CHANNEL_P1 1 // Zero indexed, so many software shows ch. 0 as ch. 1

ToneSound piezo(PIEZO_TONE_PIN);
TimerOneSound piezo2;

// TimerOneSound piezo; // Limited to pin 9 only on atmega328p boards
uint8_t currentNote;

void setup() {
    Serial.begin(38400); // Set to 31250 for proper midi or set to 38400 for most usb - midi converter programs.
    pinMode(LED_BUILTIN, OUTPUT);
    piezo.begin();
    piezo2.begin();
}

void loop () {
    uint8_t incomingNote; // The next byte to be read off the serial buffer.
    uint8_t MIDIPitch; // The MIDI note number.
    uint8_t MIDIVelocity; //The velocity of the note.

    incomingNote = getByte();

    // Analyse the byte. The byte at this stage should be a status byte. If it isn't, the byte is ignored.
    switch (incomingNote) {
        case B10010000 | MIDI_CHANNEL_P0: // Note on
            MIDIPitch = getByte();
            MIDIVelocity = getByte();
            piezo.playMidiNote(MIDIPitch); // This will handle the conversion into note and octave.
            digitalWrite(LED_BUILTIN, HIGH);
            currentNote = MIDIPitch;
            break;

        case B10000000 | MIDI_CHANNEL_P0: // Note off
            MIDIPitch = getByte();
            if(MIDIPitch == currentNote) {
                piezo.stopSound();
                digitalWrite(LED_BUILTIN, LOW);
            }
            break;
        
        case B10010000 | MIDI_CHANNEL_P1: // Note on
            MIDIPitch = getByte();
            MIDIVelocity = getByte();
            piezo2.playMidiNote(MIDIPitch); // This will handle the conversion into note and octave.
            digitalWrite(LED_BUILTIN, HIGH);
            currentNote = MIDIPitch;
            break;

        case B10000000 | MIDI_CHANNEL_P1: // Note off
            MIDIPitch = getByte();
            if(MIDIPitch == currentNote) {
                piezo2.stopSound();
                digitalWrite(LED_BUILTIN, LOW);
            }
            break;
    }
}

/** Wait for an incoming note and return the note. */
byte getByte() {
    while (Serial.available() == 0) {} //Wait while there are no bytes to read in the buffer.
    return Serial.read();
}
