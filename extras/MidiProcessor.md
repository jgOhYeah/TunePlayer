# MIDI Processor
MidiProcessor is a small script for merging tracks and channels in a midi file and ensuring only one note at a time is played. This is useful for taking a MIDI file with many channels and tracks and attempting to automatically convert this into a single track and channel with only one note on at a time.

A useful MIDI editor I use is [MidiEditor](https://midieditor.org/) (on [Github](https://github.com/markusschwenk/midieditor) and has a few forks).

## Use with the TunePlayer library
The suggested workflow for converting a generic MIDI file into code for the TunePlayer library is:
1. Examine the original MIDI file and note down any tracks and channels that can be removed or which are the main tracks and channels to include. Note that this script uses zero indexing whilst many MIDI editors and devices use 1-indexing.
2. If the file does not have notes starting and finishing exactly at the same time, then it may be worth 'quantising' the notes into discrete time steps.
3. Run MidiProcessor on the MIDI file. Some adjustment of what to include / remove and the processing options may be required for best results.
4. View and play back the output to see if it is ok.
5. Import the MIDI file into MuseScore to generte sheet music.
6. Use the [MuseScore plugin](./MusescorePlugin.md) to generate code that can be used by the TunePlayer library.

## Usage instructions
    usage: MidiProcessor.py [-h] [-a [ADD_CHANNELS ...]] [-r [REMOVE_CHANNELS ...]] [-A [ADD_TRACKS ...]] [-R [REMOVE_TRACKS ...]] [-m {highest,lowest,latest,earliest}]
                            [-t TARGET_CHANNEL] [-f] -i INPUT [-o OUTPUT]

    A small script for merging tracks and channels in a midi file and ensuring only one note at a time is played.

    options:
    -h, --help            show this help message and exit

    MIDI Channels:
    Filter out and select MIDI channels from the input file. Channel 9 is usually a drum track.

    -a [ADD_CHANNELS ...], --add-channels [ADD_CHANNELS ...]
                            Channels that should be included in processing. If not given, all channels will be included.
    -r [REMOVE_CHANNELS ...], --remove-channels [REMOVE_CHANNELS ...]
                            Channels that should not be included in processing. If not given, no channels will be removed from those included.

    MIDI Tracks:
    Filter out and select MIDI tracks from the input file

    -A [ADD_TRACKS ...], --add-tracks [ADD_TRACKS ...]
                            Tracks that should be included in processing. If not given, all tracks will be included.
    -R [REMOVE_TRACKS ...], --remove-tracks [REMOVE_TRACKS ...]
                            Tracks that should not be included in processing. If not given, no tracks will be removed from those included.

    Processing options:
    How the MIDI file will be processed. Different options will work best with different files.

    -m {highest,lowest,latest,earliest}, --method {highest,lowest,latest,earliest}
                            What method to use when prioritising several notes that start at the same time.
    -t TARGET_CHANNEL, --target-channel TARGET_CHANNEL
                            The channel that the output will be placed in.
    -f, --forgetful       Do not return to previously playing notes upon the end of the current note. If not provided, a stack is used to return to previously playing
                            notes when the current one ends.

    Files:
    The input and output files to use

    -i INPUT, --input INPUT
                            Input file.
    -o OUTPUT, --output OUTPUT
                            Output file. Defaults to 'output.mid'

    Written for the TunePlayer library (https://github.com/jgOhYeah/TunePlayer) by Jotham Gates.
