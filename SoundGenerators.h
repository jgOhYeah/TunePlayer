/**
 * TuneLoaders.h
 * Helpers classes for TunePlayer that help with playing the sound.
 * 
 * Each class should inherit SoundGenerator and implement at least the playNote
 * and stopSound methods.
 * 
 * Written by Jotham Gates, 12/06/2021
 */

/**
 * Abstract class to play a sound
 */
class SoundGenerator {
    public:
        SoundGenerator() {}

        /**
         * Plays a note. If playTime is non zero, may automatically stop playing
         * if supported.
         */
        virtual void playNote(uint8_t note, uint8_t octave, uint32_t playTime=0) {}
        virtual void stopSound() {}
};

/**
 * Play using the built in tone function
 */
class ToneSound : public SoundGenerator {
    public:
        ToneSound(uint8_t pin) {
            m_pin = pin;
        }

        /**
         * Plays a given note for a given (optional) time
         */
        void playNote(uint8_t note, uint8_t octave, uint32_t playTime=0) {
            uint16_t freq = m_frequency(note, octave)
            if(playTime) {
                // Time given
                tone(m_pin, freq, playTime);
            } else {
                // No time given
                tone(m_pin, freq);
            }
        }

        /** Stops making noises */
        void stopNote() {
            noTone(m_pin);
        }

    private:
        /** Calculates the frequency a note in a certain octave should be. */
        uint16_t m_frequency(uint8_t note, uint8_t octave) {
#ifdef PRECISE_FREQS
            const uint16_t noteFrequencies[8][12] { // TODO: Could store in progmem
                {33, 35, 37, 39, 41, 44, 46, 49, 52, 55, 58, 62},                         //C1 - B1
                {65, 69, 73, 78, 82, 87, 93, 98, 104, 110, 117, 123},                     //C2 - B2
                {131, 139, 147, 156, 165, 175, 185, 196, 208, 220, 233, 247},             //C3 - B3
                {262, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466, 494},             //C4 - B4
                {523, 554, 587, 622, 659, 698, 740, 784, 831, 880, 932, 988},             //C5 - B5
                {1047, 1109, 1175, 1245, 1319, 1397, 1480, 1568, 1661, 1760, 1865, 1976}, //C6 - B6
                {2093, 2217, 2349, 2489, 2637, 2794, 2960, 3136, 3322, 3520, 3729, 3951}, //C7 - B7
                {4186, 4435, 4699, 4978, 5274, 5588, 5920, 6272, 6645, 7040, 7459, 7902}  //C8 - B8
            };
            return noteFrequencies[octave][note];
#else
            // Notes for octave 8 - highest this can play. Divide to get lower. Error is a bit worse, but less space.
            const uint16_t notesOctave8[12] = {4186, 4435, 4699, 4978, 5274, 5588, 5920, 6272, 6645, 7040, 7459, 7902};
            return  notesOctave8[note] >> (7 - octave);
#endif
        }

        uint8_t m_pin;
};

/**
 * Play using Timer One (more control, but limited to pin 9)
 */
class TimerOneSound : public SoundGenerator {
    public:
        TimerOneSound() {}

        /** Sets the pin as an output */
        void begin() {
            DDRB |= (1 << PB1); // Set PB1 to be an output (Pin9 Arduino UNO)
        }

        /** Plays a given note. playTime is ignored. */
        void playNote(uint8_t note, uint8_t octave, uint32_t playTime=0) {
            // Setup non inverting mode (duty cycle is sensible), fast pwm mode 14 on PB1 (Pin 9)
            TCCR1A = (1 << COM1A1) | (1 << WGM11);
            TCCR1B = (1 << WGM12) | (1 << WGM13) | (1 << CS11); // With prescalar 8 (with a clock frequency of 16MHz, can get all notes required)
            ICR1 = m_counterTop(); // Maximum to count to
            OCR1A = m_compareValue(ICR1); // Duty cycle
        }

        /** Stops making a noise */
        void stopSound() {
            // Shutdown timer
            TCCR1A = 0;
            TCCR1B = 0;
            PORTD &= ~(1 << PB1); // Set pin low just in case it is left high (not sure if needed)
        }

    private:
        /** Returns the maximum value for timer 1 to count to for each note. */
        uint16_t m_counterTop(uint8_t note, uint8_t octave) {
            // Notes for octave 0 (real life 2 I think) - divide by 2 for each octave higher
            const uint16_t noteCounts = {61155, 57723, 54483, 51425, 48539, 45814, 43243, 40816, 38525, 36363, 34322, 32395};
            return noteCounts[note] >> octave;
        }

        /** Returns the value at which the pin should go low each time */
        uint16_t m_compareValue(uint16_t counter) {
            // For a simple example, set the duty cycle to be 50%
            return counter >> 1;
        }
};