# ALSA & SDL C Synthesizer
Small project for a C synthesizer written with ALSA C Library and SDL2.  
Only runs on Linux, tested on a Debian machine, works on WSL but only using keyboard input, not MIDI input.  

# Features
- 3 oscillators synthesizer
- ADSR envelope
- Low pass filter
- Detune
- Keyboard input
- MIDI input
- SDL graphical interface

# Dependencies
pulseaudio  
libasound-dev  
libsdl2-dev  
libsdl2-ttf-dev  

# Compile and run
To compile the projet : `make`  
To run the synth with keyboard input, the layout then defaults to QWERTY : `./bin/synth -kb`  
To run the synth with keyboard input and a specific keyboard layout : `./bin/synth -kb <QWERTY/AZERTY>`  
To run the synth with MIDI input : `./bin/synth -midi midi device hardware id>`  
The MIDI input was only tested with an Arturia Keylab Essential 61, the keyboard itself should work, but knobs CC values may be off for your keyboard, you can change them in midi.h.  

# Keyboard input
You can set the keyboard layout to be either QWERTY or  AZERTY.  
The keyboard simulates a "real" piano keyboard, starting from `a` (`q` in AZERTY) to `j`.  
The up arrow key move the keys up an octave and the down arrow key move the keys down an octave.  
The `1` key change the first oscillator waveform, the `2` key change the second oscillator waveform and the `3` key change the third oscillator waveform.  
The `z` (`w` in AZERTY) key increment the envelope attack by `0.05`, reseting to `0.0` when going past `1.0`.  
The `x` key increment the envelope decay by `0.05`, reseting to `0.0` when going past `1.0`.  
The `c` key increment the envelope sustain by `0.05`, reseting to `0.0` when going past `1.0`.  
The `v` key increment the envelope release by `0.05`, reseting to `0.0` when going past `1.0`.  
  
Don't hesitate to give feedback and contribute to the project!
