#!/usr/bin/env python3
"""MidiProcessor.py
A small script for merging tracks and channels in a midi file and ensuring only
one note at a time is played.

Written for the TunePlayer library (https://github.com/jgOhYeah/TunePlayer)

Written by Jotham Gates
Created 22/11/2022
Modified 08/12/2022
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
    out = []
    carry = 0
    for msg in track:
        if not has_channel(msg) or  msg.channel in channels:
            # Allow through
            out.append(msg.copy(time=msg.time+carry))
            carry = 0
        else:
            carry += msg.time
    
    return out

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

def merge(groups:List[List[mido.Message]], selector:Callable) -> mido.MidiTrack:
    """Merges messages into one monotone track."""
    out_track = mido.MidiTrack()
    carry = 0
    stack = []
    for group in groups:
        timedelta = group[0].time + carry
        # A group exists, so len(group) >= 1
        group[0] = group[0].copy(time=0) # Remove the time delta

        # Add non-note messages
        not_notes = list(filter(invert(is_note_message), group))
        if not_notes:
            not_notes[0] = not_notes[0].copy(time=timedelta) # Add the time delay
            out_track.extend(not_notes)
        
        # Get all note on events.
        notes = list(filter(is_note_message, group))
        new_notes = []
        if notes:
            # Select notes to add
            new_notes = select_notes_helper(notes, stack, selector)
            # Add the time if needed
            if new_notes and not not_notes:
                new_notes[0] = new_notes[0].copy(time=timedelta) # Add the time delay if not 

            out_track.extend(new_notes)

        # Carry time over if needed
        if not_notes or new_notes:
            # Added at least one note, so can clear the carry over time
            carry = 0

    return out_track

def select_notes_helper(notes:List[mido.Message], stack:List[mido.Message], selector:Callable, forgetful:bool=False) -> List[mido.Message]:
    """Makes sure only one note at a time is played.

    Starts on the first call to start a message, only stops on the last
    matching finish.
    """
    out = []

    # Split into ons and offs
    note_ons = list(filter(lambda note: note.type == MIDI_ON, notes))
    note_offs = list(filter(lambda note: note.type == MIDI_OFF, notes))

    restart_note = False
    # Stop the currently playing note if needed
    if note_offs and stack:
        # Stop playing if the current note should be stopped
        look_for = stack[-1]
        for note in note_offs:
            if note.note == look_for.note:
                out.append(note)
                
                if not forgetful:
                    # Flag to restart the last note
                    restart_note = True
                
                break
        
        # Remove all stopped notes from the right end of the stack.
        for note in note_offs:
            for i, stack_note in enumerate(reversed(stack)):
                if stack_note.note == note.note:
                    stack.pop(len(stack)-1-i)
                    break

    if note_ons:
        # Stop the old not if playing and add the new notes to the stack
        if stack:
            # Already playing
            current = stack[-1]
            # out.append(current.copy(type=MIDI_OFF))
            out.append(mido.Message(
                MIDI_OFF,
                channel = current.channel,
                note = current.note,
                velocity = 0,
                time = current.time
            ))
        
        # Select and play the chosen note
        selector(note_ons) # Mutate note_ons to have most desirable at the end, least at the start.
        stack.extend(note_ons)
        out.append(note_ons[-1])
    elif restart_note and stack:
        out.append(stack[-1])

    return out

def selector_highest(notes:List[mido.Message]):
    """Mutates the notes list to preference higher notes."""
    notes.sort(key=lambda note: note.note)

def selector_lowest(notes:List[mido.Message]):
    """Mutates the notes list to preference lower notes."""
    notes.sort(key=lambda note: note.note, reverse=True)

def selector_latest(notes:List[mido.Message]):
    """Preference the latest notes by not changing the list."""
    pass

def selector_earliest(notes:List[mido.Message]):
    """Preferences the earliest notes."""
    notes.reverse()

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
        tracks_include = list(range(len(midi_file.tracks)))
    
    for i in tracks_exclude:
        tracks_include.remove(i)

    print("Including tracks:", tracks_include)
        
    # Work out what channels to include
    for i in channels_exclude:
        channels_include.remove(i)
    
    print("Including channels:", channels_include)

    # The actual processing
    merged_track = merge_to_track(midi_file.tracks, tracks_include)
    filtered_channels = filter_channels(merged_track, channels_include)
    merged_channels = merge_to_channel(filtered_channels, target_channel)
    
    out_track = merge(group_by_time(merged_channels), selector_highest)

    # Double check
    verify(out_track, ignore_on_at_end=True)

    # # Save midi file
    midi_file.tracks = [out_track]
    midi_file.save(filename=out_filename)

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

def verify(track:mido.MidiTrack, ignore_on_at_end:bool=False):
    """Verifies that only one note is playing at a time and all notes are shut
    off correctly.
    
    If ignore_on_at_end is true, then a single note being left on at the end
    will be ignored rather than cause an error.
    """
    # Check that there is never more than one note on at a time
    notes = [0]*MIDI_NOTES
    for message in track:
        if message.type == MIDI_ON:
            notes[message.note] += 1
        elif message.type == MIDI_OFF:
            notes[message.note] = 0
        if sum(notes) > 1:
            raise ValueError("More than one note on at a time!!!")
    
    # Check that all notes have been turned off
    if sum(notes) == 1 and ignore_on_at_end:
        note = notes.index(1)
        print("A single note {} was left on in the end. This will be ignored for now.".format(note))
        
    elif sum(notes) != 0:
        print("Notes on at end:")
        print(notes)
        raise ValueError("At least one note has not been turned off at the end.")

    print("Verified successfully")

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