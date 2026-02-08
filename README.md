# ALSA & Raygui C Synthesizer 🎹
Small project for a C synthesizer written with ALSA C Library and Raygui.  
Only runs on Linux and tested on a Debian 13 machine, works on WSL but only using keyboard input, not MIDI input.  
Not tested, but MIDI can maybe work in a Linux VM using USB passthrough, although there may be latency induced by the VM.

# Features 🎵
- 3 oscillator synthesizer
- Up to 6 note polyphony
- ADSR envelope
- Low pass filter with ADSR envelope
- Detune
- Keyboard input
- MIDI input
- Raygui graphical user interface

# GUI 🖼️
The GUI shows all of the informations about the synth and let the user configure its parameters graphically :
- Waveform of the sound output
- ADSR envelope parameters level
- Filter ADSR envelope parameters level
- Filter cutoff level
- Amplification level
- Detune level
- Oscillators selected waveforms
- Piano keyboard showing which keys are being pressed

# MIDI Input 🎹
The MIDI input should work with all USB MIDI keyboards, to use the synth with your keyboard just follow these two steps :
- Using the `amidi -l` command, get your MIDI device hardware id (should look something similar to `hw:1,0,0`)
- Then run the synth : `./bin/synth -midi <hardware id>`

# Keyboard input ⌨️
You can set the keyboard layout to be either QWERTY or  AZERTY.  
The keyboard simulates a "real" piano keyboard, starting from `a` (`q` in AZERTY) to `j`.  
The up arrow key move the keys up an octave and the down arrow key move the keys down an octave.  

# Dependencies 💻
pulseaudio  
libasound-dev  
raylib  
raygui  
zenity  

# Compile and run 🛠️
To compile the projet : `make`  
To run the synth with keyboard input, the layout then defaults to QWERTY : `./bin/synth -kb`  
To run the synth with keyboard input and a specific keyboard layout : `./bin/synth -kb <QWERTY/AZERTY>`  
To run the synth with MIDI input : `./bin/synth -midi midi device hardware id>` (you can get this id by using the amidi -l command)
The MIDI input was only tested with an Arturia Keylab Essential 61, the keyboard itself should work, but knobs CC values may be off for your keyboard, you can change them in midi.h. 
  
# Contribute & feedback
Don't hesitate to give feedback and contribute to the project!  
And if you encounter any bug (and you probably will), please report them to me if you have the time!
