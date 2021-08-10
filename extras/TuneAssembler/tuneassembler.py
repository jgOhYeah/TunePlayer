#!/usr/bin/env python3
"""tuneassembler.py

An assembler for generating tunes for the Arduino TunePlayer library from
assembly language like files. For a more graphical and versatile method, check
out the Musescore plugin
(https://github.com/jgOhYeah/TunePlayer/blob/main/extras/MusescorePlugin.md)
"""
import sys, getopt

use_colour = True
visible_error = False

def print_help():
    print("""USAGE:
tuneassembler.py [OPTIONS] [INPUT FILE] [[OUTPUT FILE]]
""")

# TODO: Load from stdin, file and export to stdout, file?
# TODO: vscode syntax highlighting for tuneasm files?

def load_options():
    global use_colour
    try:
        opts, _ = getopt.getopt(sys.argv,"h", ["help", "nocolor"])
    except getopt.GetoptError:
        print_help()
        sys.exit(2)
    for opt, arg in opts:
        if opt == "help" or opt == "h":
            print_help()
            exit()
        elif opt == "--nocolor":
            use_colour = False

def assemble(fileobj) -> list:
    """ Takes a file in tuneasm format and assembles a list of notes. """
    fileobj = open("example.tuneasm")
    line = fileobj.readline()
    while line:
        # TODO: Process line
        line = fileobj.readline()

def output_progmem(assembled: list) -> str:
    """ Takes a list of notes and gives a string representing an array to save
    in progmem. """
    pass

def preprocessor_error(msg):
    """ Prints an error message that may be coloured if there is an issue preprocessing.
    Will also stop the script executing. If visible_error is False, will raise
    an exception instead """
    if visible_error:
        if use_colour:
            print("\u001b[1m\u001b[31m", end="") # Bold Red
        print("TUNE ASSEMBLER ERROR")
        if use_colour:
            print("\u001b[0m", end="") # Reset
        
        # Body
        print(msg)

        # Note to say giving up
        if use_colour:
            print("\u001b[33m", end="") # Yellow
        print("Processing halted. Please try again.")
        if use_colour:
            print("\u001b[0m", end="") # Reset
        exit()
    else:
        raise TuneAssemblerError(msg)

    
def preprocessor_warning(msg):
    """ Prints a warning message. Similar to error, but will not stop. """
    if visible_error:
        if use_colour:
            print("\u001b[1m\u001b[33m", end="") # Bold Yellow
        print("TUNE ASSEMBLER WARNING")
        if use_colour:
            print("\u001b[0m", end="") # Reset
        print(msg)
        print()

class TuneAssemblerError(Exception):
    """ An exception for critical issues assmebling the tune. """
    def __init__(self, message=""):
        self.message = message

if __name__ == "__main__":
    visible_error = True
    if len(sys.argv) == 1:
        print_help()
    
    # TODO: Things other than help