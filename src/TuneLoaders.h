/**
 * TuneLoaders.h
 * Helpers classes for TunePlayer that help with loading of notes from required
 * sources.
 * 
 * Each loader should inherrit BaseTuneLoader and have the loadNote method.
 * 
 * Written by Jotham Gates, 12/06/2021
 */

#pragma once
#include <Arduino.h>

/**
 * Abstract class to load a note from memory.
 * Each memory / tune source should inherit this class.
 */
class BaseTuneLoader {
    public:
        BaseTuneLoader() {}

        virtual void begin() {}

        /** Returns the raw note data (16 bits) at the given address */
        virtual uint16_t loadNote(uint16_t address) {}
};

/**
 * Class for loading notes from flash
 */
class FlashTuneLoader : public BaseTuneLoader {
    public:
        FlashTuneLoader() {}

        /** Sets the array in program memory to load from. */
        void setTune(uint16_t *tune) {
            m_tune = tune;
        }

        /** Returns the raw note data at the given address. */
        uint16_t loadNote(uint16_t address) {
            uint16_t rawNote = pgm_read_word_near(m_tune + address);
            return rawNote;
        }

    private:
        uint16_t *m_tune; // Pointer to the start of the array in flash
};

/* TODO: EEPROM Tune Loader */