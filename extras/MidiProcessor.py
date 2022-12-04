#!/usr/bin/env python3
"""MidiProcessor.py
A small script for merging tracks and channels in a midi file and ensuring only
one note at a time is played.

Written for the TunePlayer library (https://github.com/jgOhYeah/TunePlayer)

Written by Jotham Gates
Created 22/11/2022
Modified 24/11/2022
"""
from __future__ import annotations
import mido
import sys
import argparse
from typing import List, TypeVar, Union, Callable
T = TypeVar('T')

MIDI_ON = "note_on"
MIDI_OFF = "note_off"
MIDI_NOTES = 128

def select_by_index(src:List[T], indices:List[int]) -> List[T]:
    """Filters a list to only include elements whose index is in the index
    list."""
    return [src[i] for i in indices]

def merge_to_track(tracks:List[mido.MidiTrack], indices:Union[List[int], None]) -> mido.MidiTrack:
    """Takes the tracks whose indices appear in indices and merges them into
    one track."""
    filtered_tracks = select_by_index(tracks, indices) if indices else tracks
    return mido.merge_tracks(filtered_tracks)

def has_channel(message:mido.Message) -> bool:
    """Returns True if a message has a channel, False otherwise."""
    return hasattr(message, "channel")

def merge_to_channel(track:mido.MidiTrack, channel:int) -> mido.MidiTrack:
    """Maps all events in a track to one channel."""
    return mido.MidiTrack([message.copy(channel=channel) if has_channel(message) else message for message in track])

def filter_channels(track:mido.MidiTrack, channels:List[int]) -> mido.MidiTrack:
    """Filters a track to only include messages whose channel appears in the
    channels list."""
    def allow_through(msg:mido.Message) -> bool:
        # if msg
        return not has_channel(msg) or  msg.channel in channels
    
    return mido.MidiTrack(filter(allow_through, track))

# def remove_multiple_on(track:mido.MidiTrack) -> mido.MidiTrack:
#     """Makes sure that no note is turned on multiples times at once across all
#     channels."""
#     # NOTE: Will end up with lots of 0 length notes if more than 2 notes start together.
#     notes = [False] * MIDI_NOTES
#     out_track = mido.MidiTrack()
#     message:mido.Message
#     for message in track:
#         if message.type == MIDI_OFF:
#             # Cancel the record of the note being on
#             notes[message.note] = False
#         elif message.type == MIDI_ON:
#             if notes[message.note]:
#                 # Currently on, been told to turn on again.
#                 # Stop the current note playing and restart it.
#                 out_track.append(message.copy(type=MIDI_OFF, velocity=0))
#                 out_track.append(message.copy(time=0))
#             else:
#                 # Was off, can turn on as normal
#                 out_track.append(message)
#             notes[message.note] = True
#         else:
#             out_track.append(message)
    
#     return out_track


def group_by_time(track:mido.MidiTrack) -> List[List[mido.Message]]:
    """Groups messages in a track into messages that are sent at the same time.
    The time of the first element of each group is the time delta from the
    previous. All other times are 0 as they happen at the same time as the
    first, so 0 time delta.
    """
    track_iter = iter(track)
    messages = []
    try:
        group = [next(track_iter)] # First message
        while True:
            # Until we run out of messages
            cur = next(track_iter)
            while cur.time == 0:
                # While we still have messages sent at the same time (time = 0)
                group.append(cur)
                cur = next(track_iter)

            # Jump in time. Start next group
            messages.append(group)
            group = [cur]
    except StopIteration:
        # Final save if needed
        if group:
            messages.append(group)
    
    return messages

def is_note_message(message:mido.Message) -> bool:
    """Returns True if a message is a note_on or note_off instruction."""
    return message.type == MIDI_OFF or message.type == MIDI_ON

def invert(func):
    def inverted_func(*args, **kwargs):
        return not func(*args, **kwargs)
    
    return inverted_func

def merge(groups:List[List[mido.Message]],
    notes_selector:Callable[[List[mido.Message]], List[mido.Message]]
    ) -> mido.MidiTrack:
    """Merges messages into one monotone track."""
    out_track = mido.MidiTrack()
    for group in groups:
        timedelta = group[0].time
        group[0] = group[0].copy(time=0) # Remove the time delta

        # Add non-note messages
        not_notes = list(filter(invert(is_note_message), group))
        if not_notes:
            not_notes[0] = not_notes[0].copy(time=timedelta) # Add the time delay
            out_track.extend(not_notes)
        
        # Get all note on events.
        notes = list(filter(is_note_message, group))
        if notes:
            notes = notes_selector(notes)
            if not notes:
                raise NotImplementedError("Selecting no note is not implemented yet")
    
            # Add the time if needed
            if not not_notes:
                notes[0] = notes[0].copy(time=timedelta) # Add the time delay if not 

            out_track.extend(notes)
    
    return out_track

def select_notes(notes:List[mido.Message], keys:List[bool]) -> List[mido.Message]:
    out = []
    for note in notes:
        if keys[note.channel]:
            out.append(mido.Message(
                MIDI_OFF,
                channel = note.channel,
                note = note.note,
                velocity = 0,
                time = 0
            ))
            
            out.append(mido.)

def merge_to_monotone(track:mido.MidiTrack) -> mido.MidiTrack:
    """Merges a track to a monotone (only one note active at a time).

    Metadata and control messages are passed through

    Plays only the latest note. i.e.
      A#     ---      ---
      A  ---   ------------
    Will be converted to:
      A#     --       ---
      A  ---   -------
    Args:
        track (mido.MidiTrack): The input track

    Returns:
        mido.MidiTrack: The output track with only one note at a time.
    """
    current_note:mido.Message = None
    out_track = mido.MidiTrack()
    message:mido.Message
    for message in track:
        # print(repr(message))
        if message.type == MIDI_OFF and (not current_note or current_note.note == message.note):
            # Stop the current note
            out_track.append(message)
            current_note = None
        elif message.type == MIDI_ON:
            # print(message)
            if current_note:
                # Need to stop current note before starting a new one.
                out_track.append(
                    mido.Message(
                        MIDI_OFF,
                        channel = current_note.channel,
                        note = current_note.note,
                        velocity = 0,
                        time = message.time
                    )
                )
                out_track.append(message.copy(time=0))
            else:
                # Nothing currently playing.
                out_track.append(message)
            current_note = message
        else:
            # Metadata, control, pitch wheel...
            out_track.append(message)
    
    # Done processing
    return out_track

def process(in_filename:str, out_filename:str, tracks_include:List[int],
    tracks_exclude:List[int], channels_include:List[int], channels_exclude:List[int],
    target_channel:int) -> None:
    """Reads a midi file, filters and merges the tracks, filters the channels,
    makes it monotone, maps all messages to one channel and saves the output in
    a file."""
    # Load the file
    midi_file = mido.MidiFile(in_filename)

    # Work out what tracks to include
    if not tracks_include:
        tracks_include = range(len(midi_file.tracks))
    
    for i in tracks_exclude:
        tracks_include.remove(i)

    print("Including tracks:", tracks_include)
        
    # Work out what channels to include
    for i in channels_exclude:
        channels_include.remove(i)
    
    print("Including channels:", channels_include)

    # The actual processing
    merged_track = merge_to_track(midi_file.tracks, tracks_include)
    # filtered = filter_channels(merged_track, channels_include)
    # monotone_track = merge_to_monotone(filtered)
    # out_track = merge_to_channel(monotone_track, target_channel)
    
    # # Save midi file
    # midi_file.tracks = [out_track]
    # midi_file.save(filename=out_filename)

    group_by_time(merged_track)

def parse_args() -> argparse.Namespace:
    """Parser for the command line interface arguments."""
    # Create the parser
    parser = argparse.ArgumentParser(
        description = "A small script for merging tracks and channels in a midi\
 file and ensuring only one note at a time is played.",
        epilog = "Written for the TunePlayer library \
(https://github.com/jgOhYeah/TunePlayer) by Jotham Gates."
    )

    # Add arguments
    parser.add_argument(
        "-a", "--add-channels",
        action="extend", nargs="*", type=int,
        default=list(range(16)),
        help="""Channels that should be included in processing.
If not given, all channels will be included."""
    )
    parser.add_argument(
        "-r", "--remove-channels",
        action="extend", nargs="*", type=int,
        default=[],
        help="""Channels that should not be included in processing.
If not given, no channels will be removed from those included."""
    )
    parser.add_argument(
        "-A", "--add-tracks",
        action="extend", nargs="*", type=int,
        default=None,
        help="""Tracks that should be included in processing.
If not given, all tracks will be included."""
    )
    parser.add_argument(
        "-R", "--remove-tracks",
        action="extend", nargs="*", type=int,
        default=[],
        help="""Tracks that should not be included in processing.
If not given, no tracks will be removed from those included."""
    )
    parser.add_argument(
        "-i", "--input",
        action='store', nargs=1, required=True,
        help="Input file."
    )
    parser.add_argument(
        "-o", "--output",
        action='store', nargs=1,
        default=["output.mid"],
        help="Output file. Defaults to 'output.mid'"
    )

    # Parse
    return parser.parse_args()
    
if __name__ == "__main__":
    namespace = parse_args()
    # print(namespace)
    process(
        in_filename=namespace.input[0],
        out_filename=namespace.output[0],
        tracks_include=namespace.add_tracks,
        tracks_exclude=namespace.remove_tracks,
        channels_include=namespace.add_channels,
        channels_exclude=namespace.remove_channels,
        target_channel=0
    )