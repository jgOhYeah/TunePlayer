/** TunePlayer_MusescorePlugin.qml
 * Plugin for converting Musescore files to a format that can be used by the
 * TunePlayer Arduino library.
 *
 * Written by Jotham Gates
 * Created 06/04/2020
 * Modified 22/06/2021
 */
import QtQuick 2.8
import MuseScore 3.0
import QtQuick.Controls 2.2
MuseScore { 
      // TODO: Text box to set the max number of notes per line
      menuPath: "Plugins.Generate TunePlayer Code"
      description: "Exports single notes into a 16 bit format for an Arduino microcontroller"
      version: "1.7.0"
      pluginType: "dialog"

      // Properties that can be changed
      width:  800
      height: 400
      property var defaultRepeat: false;
      property var displayBinary: false;
      property var maxNotesPerLine: 40;

      // Global variables
      property var noteString: "";
      property var errorMsgs: "";
      property var isError: false;
      property var isTied: false;
      property var tieTotalTick: 0;
      property var repeatBackToAddress: 0;
      property var curLineNotes: 0;
      onRun: {
            if (typeof curScore === 'undefined') {
                  Qt.quit();
            }
            runConverter(defaultRepeat);
      }
      
      function runConverter(repeat) {
            // Reset everything
            noteString = "// Converted from '" + curScore.scoreName + "' by TunePlayer Musescore plugin V" + version;
            noteString += "\nconst uint16_t " + curScore.scoreName.replace(" ", "_") + "[] PROGMEM = {\n    ";
            errorMsgs = "Errors encounted:<br>";
            isError = false;
            isTied = false;
            tieTotalTick = 0;
            repeatBackToAddress = 0;
            curLineNotes = 0;
            curScore.createPlayEvents(); // To make the play events be meaningful

            // Do the work
            applyToNotesInSelection(repeat);

            // Pretty and display
            noteString = noteString + '\n};';
            outputText.text = noteString;
            outputText.selectAll();

            if(isError) {
                  outputErrors.text = errorMsgs;
                  outputErrors.color = "red";
            } else {
                  outputErrors.text = "No errors encounterd";
                  outputErrors.color = "green";
            }
      }

      function applyToNotesInSelection(repeat) { // Originally based off one of the examples.
            var track = 0;
            var segment = curScore.firstSegment();
            while (segment) {
                  // console.log("Name: " + segment.subtypeName() + ", L: " + segment.annotations.length);
                  var element = segment.elementAt(track);
                  if (element) {
                        if (element._name() == "BarLine") {
                              switch(element.subtypeName()) {
                                    case "start-repeat": //||: - not exported as a note / setting
                                          repeatBackToAddress = 0; //Reset the number of notes to count back to.
                                          break;
                                    case "end-repeat": //:|| - exported as a setting
                                          exportRepeat();
                                          break;
                                    case "end-start-repeat": //Don't know what as :||: behaves as 2 repeats
                                          exportRepeat();
                                          repeatBackToAddress = 0;
                                          break;
                                    case "end": //End of song. Ignoring for now.
                                          exportEnd(repeat);
                                          return;
                              }
                        }
                        var tempoElement = findExistingTempoElement(segment);
                        if(tempoElement != undefined) { //we have a tempo change element
                              exportTempoChange(tempoElement.tempo);
                        } //Have before to set before the note
                        if (element.type === Element.CHORD) {
                              var note = highestNote(element.notes);
                              processForTies(note.pitch,noteDurationCalc(segment.next.tick-segment.tick),note);
                        } else if (element.type === Element.REST) {
                              exportNote(128,noteDurationCalc(segment.next.tick-segment.tick),0);
                        }
                  }
                  segment = segment.next;
            }
      }
      // Finds the highest note in case there are multiple notes
      function highestNote(notes) {
            if(notes.length < 1) {
                  return;
            }
            var highest = notes[0];
            for(var i = 1; i < notes.length; i++) {
                  if(notes[i].pitch > highest.pitch) {
                        highest = notes[i];
                  }
            }
            return highest;
      }
      function findExistingTempoElement(segment) //Copied from https://github.com/jeetee/MuseScore_TempoChanges/blob/master/TempoChanges.qml
      { //look in reverse order, there might be multiple TEMPO_TEXTs attached
            // in that case MuseScore uses the last one in the list
            for (var i = segment.annotations.length; i-- > 0; ) {
                  if (segment.annotations[i].type === Element.TEMPO_TEXT) {
                        return (segment.annotations[i]);
                  }
            }
            return undefined; //invalid - no tempo text found
      }
      //Function for selecting what format numbers should be easily
      function formatNumber(dec) {
            //return dec2bin(dec);
            if(displayBinary) {
                  return dec2bin(dec);
            } else {
                  return dec2hex(dec);
            }
      }

      function dec2bin(dec){ //From 
            return "0b" + (dec >>> 0).toString(2);
      }

      function dec2hex(dec){ //From 
            return "0x" + (dec >>> 0).toString(16);
      }

      function exportTempoChange(tempo) {
            var tempoBpm = tempo * 60;
            var note = 0xE;
            var noteCode = formatNumber((note << 12) | tempoBpm);

            // New line if required
            if(curLineNotes != 0) {
                  noteString += "\n    ";
            }
            noteString += noteCode + ", // Tempo change to " + tempoBpm + " BPM\n    ";
            curLineNotes = 0;
            repeatBackToAddress ++;
      }
      function processForTies(notePitchInput,noteLength,noteInfo) {
            if(isTied) { //Was previously
                  tieTotalTick += noteLength;
                  if(noteInfo.tieForward) { //Multiple notes tied
                        //isTied is already true
                  } else { //End of tie, export
                        isTied = false;
                        exportNote(notePitchInput,tieTotalTick,noteInfo);
                        tieTotalTick = 0;
                  }
            } else {
                  if(noteInfo.tieForward) { //Don't export now, export next note or at the end of the tie
                        tieTotalTick = noteLength;
                        isTied = true;
                  } else {
                        exportNote(notePitchInput,noteLength,noteInfo);
                        tieTotalTick = 0;
                  }
            }
                  
      }
      function exportRepeat() {
            var note = 13;
            var numberRepeats = 1;
            var noteCode = formatNumber((note << 12) | (numberRepeats << 10) | repeatBackToAddress);
            noteString += "\n    " + noteCode + ", // Repeat going back " + repeatBackToAddress + " notes\n    ";
            curLineNotes = 0;
            repeatBackToAddress++;
      }
      function noteDurationCalc(noteLength) { //Returns the note duration in 1/24 beats.
            //Separate function as this caluclation method might need / has been changed.
            //return (32 * noteLength.numerator) / noteLength.denominator;
            var arduinoTicks = (24 * noteLength) / division;
            return arduinoTicks;
      }

      /**
       * Adds an error message to the list to display.
       */
      function addError(msg) {
            errorMsgs += msg;
            isError = true;
      }

      function exportNote(notePitchInput,noteLength,note) {
            // Note effect (staccarto, slurred / legato)
            var noteEffect = 0;
            if(note.playEvents[0].len < 501) {
                  // Staccarto
                  noteEffect = 1;
            } else if (note.playEvents[0].len == 1000) {
                  // Legato
                  noteEffect = 2;
            }

            // Note pitch
            if (notePitchInput < 128) { //It makes sound
                  var notePitch = notePitchInput % 12;
                  var noteOctave = Math.floor(notePitchInput / 12) - 2; // 2 * 12 is midi note 24 start

            } else {
                  var notePitch = 12; //Rest
                  var noteOctave = 0;
            }

            // Timing and length
            var tripletTime = 0;
            if((noteLength % 2 == 0) && (noteLength % 3 == 0)) {
                  //This note can be expressed in 1/8 and 1/12 beats - use normal mode (1/8)
                  var noteDuration = noteLength / 3;
            } else if(noteLength % 2 == 0) {
                  //Triplet that cannot be expressed in 1/8 beats.
                  var noteDuration = noteLength / 2;
                  tripletTime = 1;
            } else {
                  //Can't be expressed as a triplet, so is either something weird or a 1/8 note. Use normal mode
                  addError("WARNING: A note with a weird / non supported length was encountered.<br>");
                  var noteDuration = noteLength / 3;
            }
            if(noteDuration <= 0) {
                  console.log("WARNING: Note encountered that is 0 or smaller length!");
                  addError("WARNING: Note encountered that is 0 or smaller length!<br>");
                  noteDuration = 1;
            }
            if(noteDuration > 64) {
                  console.log("WARNING: Note encountered that is longer than 8 beats!");
                  addError("WARNING: Note encountered that is over 64 in length!<br>");
                  noteDuration = 64;
            }
            noteDuration--; // Otherwise the 0 isn't used
            var noteCode = ((notePitch << 12) & 0xF000) | ((noteOctave << 9) & 0xE00) | ((noteDuration << 3) & 0x1F8) | ((noteEffect << 1) & 0x06) | (tripletTime & 0x01);
            var displayNote = formatNumber(noteCode);

            // New line if needed.
            curLineNotes++;
            if(curLineNotes > maxNotesPerLine) {
                  noteString += "\n    ";
                  curLineNotes = 1; // To include the current note
            }
            noteString += displayNote + ",";
            repeatBackToAddress ++;
      }
      
      function exportEnd(repeat) {outputErrors.color = "green";
            var note = 15;
            var noteCode = (note << 12) & 0xF000;
            if(repeat) {
                  noteCode |= 0x0001; // LSB is 1 to signal endless loop
            }
            var displayNote = formatNumber(noteCode);

            // Message about loop
            var repeatMsg = "Stop playing.";
            if(repeat) {
                  repeatMsg = "Restart from the beginning.";
            }

            // Add to the output
            noteString += "\n    " + displayNote + " // End of tune. " + repeatMsg;
      }
      Rectangle {
            id: leftSide
            color: "transparent"
            x: 180

            anchors.left: parent.left
            anchors.right: rightSide.left
            anchors.bottom: parent.bottom
            anchors.top: parent.top
            anchors.margins: 10

            Text {
                  id: optionsText
                  text: "Options"
            }

            Column {
                  anchors.top: optionsText.bottom;
                  anchors.topMargin: 5;
                  CheckBox {
                        id: loopEndlessly
                        checked: defaultRepeat
                        text: qsTr("Loop endlessly")
                        onClicked: {
                              runConverter(loopEndlessly.checked);
                        }
                  }
                  CheckBox {
                        id: binaryCheckbox
                        checked: displayBinary
                        text: qsTr("Display binary")
                        onClicked: {
                              displayBinary = binaryCheckbox.checked;
                              runConverter(loopEndlessly.checked);
                        }
                  }
            }

            Rectangle {
                  id: copyButton
                  width: 170
                  height: 40
                  color: "#0040FF"
                  anchors.bottom: parent.bottom
                  anchors.left: parent.left
                  anchors.margins: 0
                  
                  Text {
                        text: "Copy to clipboard"
                        color: "white"
                        anchors.fill: parent
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                  }

                  MouseArea {
                        anchors.fill: parent
                        onClicked: {
                              outputText.selectAll();
                              outputText.copy();
                              outputErrors.text += "<br><i>Copied to clipboard</i>";
                        }
                  }
            }

      }
      Rectangle {
            id: rightSide
            color: "transparent"
            anchors.left: leftSide.right
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.top: parent.top
            anchors.margins: 10

            Text {
                  id: outputInfoText
                  text: "Output <i>(Copy and paste this into your sketch)</i>"
            }

            ScrollView { // Based off https://stackoverflow.com/a/53967239
                  id: outputBackground
                  anchors.left: parent.left
                  anchors.right: parent.right
                  anchors.bottom: rightDivider.top
                  anchors.top: outputInfoText.bottom
                  anchors.topMargin: 5
                  anchors.bottomMargin: 5
                  clip: true

                  Flickable {
                        contentHeight: Math.max(outputText.contentHeight + 100, parent.height);
                        //width: parent.width
                        contentWidth: Math.max(outputText.contentWidth + 20, parent.width);
                        Rectangle {
                              id : rectangle
                              color: "white"
                              anchors.fill: parent
                              TextEdit {
                                    id: outputText
                                    anchors.fill: parent
                                    anchors.margins: 10
                                    color: "black"
                                    font.family: "Courier";
                                    text: qsTr("Something went wrong... Maybe run from the plugin creator window.")
                              }
                        }
                  }

                  MouseArea {
                        anchors.fill: parent
                        onClicked: {
                              if(outputText.selectedText) {
                                    // Deselect
                                    outputText.deselect();
                              } else {
                                    outputText.selectAll();
                                    outputText.copy(); // Can't really get ctrl c to work, so will just use this
                                    outputErrors.text += "<br><i>Copied to clipboard</i>";
                              }
                        }
                  }
            }
            Text {
                  id: rightDivider
                  anchors.left: parent.left
                  anchors.right: parent.right
                  anchors.bottom: outputErrorsRect.top
                  anchors.bottomMargin: 5
                  text: "Messages"
            }

            ScrollView { // Based off https://stackoverflow.com/a/53967239
                  id: outputErrorsRect
                  anchors.left: parent.left
                  anchors.right: parent.right
                  anchors.bottom: parent.bottom
                  height: 60
                  clip: true

                  Flickable {
                        contentHeight: Math.max(outputErrors.contentHeight + 20, parent.height);
                        contentWidth: Math.max(outputErrors.contentWidth + 20, parent.width);
                        Rectangle {
                              color: "white"
                              anchors.fill: parent
                              Text {
                                    id: outputErrors
                                    anchors.fill: parent
                                    anchors.margins: 10
                                    color: "red"
                                    text: "Error messages go here."
                              }
                        }
                  }
            }
      }
}
