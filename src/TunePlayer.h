/**
 * TunePlayer.h
 * Class for playing tunes from a compressed 16 bit format.
 * Written by Jotham Gates
 * Created 12/06/2021
 * Modified 07/07/2022
 */

#pragma once
#include <Arduino.h>
#include <cppQueue.h>
#include "SoundGenerators.h"
#include "TuneLoaders.h"


// #define PRECISE_FREQS // If using tone and not timer 1, store the required frequency for each note rather than one octave and caculating it. Might be a bit more correct, but more memory.
// #define MANUAL_CUTOFF // Define this if the SoundGenerator cannot stop playing automatically (TimerOneSound for instance)

// Define defaults if not already specified.
#ifndef NOTES_QUEUE_MAX
    #define NOTES_QUEUE_MAX 4
#endif
#ifndef REPEATS_MAX_CONCURRENT
    #define REPEATS_MAX_CONCURRENT 4
#endif
#ifndef DEFAULT_TEMPO
    #define DEFAULT_TEMPO 120
#endif

// Notes
#define NOTE_C 0
#define NOTE_CS 1
#define NOTE_D 2
#define NOTE_DS 3
#define NOTE_E 4
#define NOTE_F 5
#define NOTE_FS 6
#define NOTE_G 7
#define NOTE_GS 8
#define NOTE_A 9
#define NOTE_AS 10
#define NOTE_B 11
#define NOTE_REST 12
#define NOTE_REPEAT 13
#define NOTE_SETTING 14
#define NOTE_END 15

// Effects
#define EFFECT_NONE 0
#define EFFECT_STACCATO 1
#define EFFECT_LEGATO 2
// Effect 3 is currently unused

// Repeats
#define REPEAT_ONCE 0
#define REPEAT_TWICE 1
#define REPEAT_THRICE 2
#define REPEAT_EVERY 3

#ifdef ENABLE_CALLBACKS
typedef void (*CallbackFunction)();
#endif
/**
 * Main tune playing class
 */
class TunePlayer {
    public:
        TunePlayer() {}
        /**
         * Initialises the library with the given parameters
         */
        void begin(BaseTuneLoader *newTuneLoader, SoundGenerator *newSoundGenerator) {
            tuneLoader = newTuneLoader;
            soundGenerator = newSoundGenerator;

            soundGenerator->begin();
            tuneLoader->begin();
        }

        void begin() {
            soundGenerator->begin();
            tuneLoader->begin();
        }

        /**
         * Fills up the queue.
         */
        void spool() {
            while(!m_notesQueue.isFull()) {
                m_loadNote();
            }
        }

        /**
         * Starts to play the tune
         */
        void play() {
            m_isPlaying = true;
        }

        /**
         * Performs all housekeeping operations necessary for tune loading and
         * playback.
         */
        void update() {
            updateLowPriority();
            updateHighPriority();
        }

        /**
         * Performs low priority tasks and housekeeping functions.
         */
        void updateLowPriority() {
            spool(); // Low priority
        }

        /**
         * Performs high priority tasks (playing at the right time).
         */
        void updateHighPriority() {
            m_makeNoise(); // High priority

            // High priority
#ifdef MANUAL_CUTOFF
            if(isPlaying() && m_curNoteStop && micros()-m_curNoteStart > m_curNoteStop) {
                soundGenerator->stopSound();
            }
#endif
        }

        /**
         * Pauses playback, skipping the end of the current note. If the play
         * is called again, the tune continures from the next note.
         * If holdNote is true, keeps playing the same note, otherwise stops
         * sound generation.
         */
        void pause(bool holdNote = false) {
            if(!holdNote) {
                soundGenerator->stopSound();
            }
            m_isPlaying = false;
#ifdef ENABLE_CALLBACKS
            m_callback();
#endif
        }

        /**
         * Stops tune playback, clears the queue and sets the note address back
         * to the start. This also resets the tempo to the default. If play is
         * called, the tune restarts from the beginning.
         */
        void stop() {
            soundGenerator->stopSound();
            m_isPlaying = false;
#ifdef ENABLE_CALLBACKS
            m_callback();
#endif
            m_notesQueue.flush();
            m_noteIndex = 0;
            m_nextNoteTime = 0; // So play will work immediately
            m_timebase = 2500000 / DEFAULT_TEMPO; // Set tempo back to the default
        }

        /**
         * Returns true if a tune is currently being played, otherwise false.
         */
        inline bool isPlaying() {
            return m_isPlaying;
        }

        BaseTuneLoader *tuneLoader;
        SoundGenerator *soundGenerator;

#ifdef ENABLE_CALLBACKS
        /**
         * Sets the function that should be called when the tune stops. This is
         * only enabled when ENABLE_CALLBACKS is defined.
         * 
         * In other words, this is when isPlaying() becomes false. i.e. on the
         * tune finishing or the stop or pause methods being called.
         * 
         * callbackFunction is the function to call (must not return anything
         * and must accept no parameters).
         * 
         * To disable the callbacks, use setCallOnStop(NULL);
         */
        inline void setCallOnStop(CallbackFunction callbackFunction) {
            m_callOnStop = callbackFunction;
        }
#endif

    private:
        /**
         * Struct for processed note data to be given to the player.
         */
        struct NoteData {
            uint8_t note;
            uint8_t octave;
            uint32_t playTime;
            uint32_t nextTime;
        };

        /**
         * Struct for processed repeat data
         */
        struct RepeatData {
            uint16_t  index;
            uint8_t count;
        };

        /**
         * Loads a note, processes it and adds it to the queue. Also handles
         * addressing and repeats.
         */
        void m_loadNote() {
            uint16_t rawNote = tuneLoader->loadNote(m_noteIndex);
            // Extract the note and decide what to do next.
            NoteData noteData;
            noteData.note = (rawNote >> 0x0C) & 0x0F;
            uint8_t repeatCounts; // Doesn't cope if initialised in the NOTE_REPEAT case
            switch(noteData.note) {
                case NOTE_REPEAT:
                    // Check what type of repeat
                    repeatCounts = (rawNote >> 0x0A) & 0x03;
                    if(repeatCounts != REPEAT_EVERY) {
                        // This is a repeat type that uses the stack
                        // Setup somewhere to store the repeat.
                        RepeatData topRepeat;
                        if(!m_repeatsStack.peek(&topRepeat) || topRepeat.index != m_noteIndex) {
                            // We have not seen this repeat before. Add it to the stack and go back.
                            RepeatData newRepeat;
                            newRepeat.index = m_noteIndex;
                            newRepeat.count = repeatCounts;
                            m_repeatsStack.push(&newRepeat);

                            m_executeRepeat(rawNote); // Jump back
                        } else {
                            // We have seen this repeat before. Either decrease its count by 1 or go skip it.
                            m_repeatsStack.pop(&topRepeat); // We have to remove the repeat from the stack to modify it.
                            if(topRepeat.count == 0) { // Done for this repeat
                                m_noteIndex++;
                            } else {
                                // Decrease count and repeat again
                                topRepeat.count--;
                                m_repeatsStack.push(&topRepeat);
                                m_executeRepeat(rawNote);
                            }
                        }
                    } else {
                        m_executeRepeat(rawNote); // Jump back every time
                    }
                    break;

                case NOTE_SETTING:
                    // Set the tempo
                    m_timebase = 2500000 / (rawNote & 0x3FF); // bpm to us/(1/24b).
                    m_noteIndex++; // Increase the noteIndex
                    break;

                case NOTE_END:
                    // Add a note of frequency 2 to stop playback when reached.
                    if(rawNote & 0x01) {
                        // Let's start from the very beginning.
                        m_noteIndex = 0;
                    } else {
                        // Stop playing when reached. noteData.note is already set
                        m_notesQueue.push(&noteData);
                    }
                    break;

                default: // Actual playable note or a rest
                    // Get the rest of the note settings
                    noteData.octave = (rawNote >> 0x09) & 0x07;
                    uint8_t length = ((rawNote >> 0x03) & 0x3F) + 1;
                    uint8_t effect = (rawNote >> 0x01) & 0x03;
                    uint8_t triplet = rawNote & 0x01;

                    // Calculate the time to the next note
                    if(triplet) {
                        noteData.nextTime = length * 2;
                    } else {
                        noteData.nextTime = length * 3;
                    }
                    noteData.nextTime *= m_timebase;

                    // Calculate the frequency or set to 0
                    if(noteData.note != NOTE_REST) {
                        // Octave and note are already set
                        // Also calculate the time the sound should be on
                        switch(effect) { //Claculate how much of the total time there should be sound - small gaps between notes.
                            case EFFECT_LEGATO:
                                noteData.playTime = 0;
                                break;
                            case EFFECT_STACCATO:
                                noteData.playTime = noteData.nextTime / 2;
                                break;
                            default: //Note plays 7/8 of the time, as suggested by the picaxe manual 2 tune command.
                                noteData.playTime = noteData.nextTime * 7 / 8;
                        }
                    }

                    // Add the note to the queue to play
                    m_notesQueue.push(&noteData);

                    // Increase the noteIndex
                    m_noteIndex++;
            }

        }

        /**
         * Plays the sound
         */
        void m_makeNoise() {
            // Check if it is the correct time to update the tune
            if(m_isPlaying && micros()-m_curNoteStart > m_nextNoteTime) {
                if(!m_notesQueue.isEmpty()) {
                    // Get the data and play it
                    NoteData noteData;
                    m_notesQueue.pop(&noteData);
                    switch(noteData.note) {
                        case NOTE_END:
                            m_isPlaying = false;
#ifdef ENABLE_CALLBACKS
                            m_callback();
#endif
                            /* fallthrough */
                        case NOTE_REST:
                            soundGenerator->stopSound();
                            break;
                        default:
                            // Note to play. Play it using the specified method
#ifdef MANUAL_CUTOFF
                            m_curNoteStop = noteData.playTime;
#endif
                            soundGenerator->playNote(noteData.note, noteData.octave, noteData.playTime);
                    }

                    // Set the next time to update
                    m_nextNoteTime = noteData.nextTime;
                } else {
                    // Set to check repeatedly
                    m_nextNoteTime = 0;
                }
                m_curNoteStart = micros();
            }
        }

        /**
         * Extracts the number of notes to jump back from a repeat and jumps there
         */
        void m_executeRepeat(uint16_t rawNote) {
            uint16_t goBackAmount = rawNote & 0x3FF;
            uint16_t newAddress = 0;
            if(goBackAmount < m_noteIndex) {
                // This won't take us back before the start
                newAddress = m_noteIndex - goBackAmount;
            }
            m_noteIndex = newAddress;
        }

#ifdef ENABLE_CALLBACKS
        /**
         * Calls the call on stop callback function if it is not NULL.
         */
        inline void m_callback() {
            if (m_callOnStop != NULL) {
                m_callOnStop();
            }
        }
#endif

        cppQueue m_notesQueue = cppQueue(sizeof(NoteData), NOTES_QUEUE_MAX, FIFO);
        cppQueue m_repeatsStack = cppQueue(sizeof(RepeatData), REPEATS_MAX_CONCURRENT, LIFO);
        uint16_t m_noteIndex = 0;
        uint32_t m_timebase = 2500000 / DEFAULT_TEMPO; // us per 1/24 beats
        uint32_t m_nextNoteTime = 0;
        uint32_t m_curNoteStart = 0;

#ifdef ENABLE_CALLBACKS
        CallbackFunction m_callOnStop = NULL;
#endif
        bool m_isPlaying = false;

        // Only needed if the time has to be stopped manually
#ifdef MANUAL_CUTOFF
        uint32_t m_curNoteStop = 0;
#endif
};
