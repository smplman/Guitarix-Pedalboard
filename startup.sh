# Kill running processes
pkill jackd
pkill guitarix

# Start jackd
JACK_NO_AUDIO_RESERVATION=1 /usr/bin/jackd -d alsa -d hw:1 -r 192000 -p 512 -n 2 &

# Start guitarix
guitarix -i system:capture_1 -o system:playback_1 -o system:playback_2 -b A:0 -N &

# Start a2jmidid for midi controller
a2jmidid -e &

# configure jackd ports
# list ports
# jack_lsp
# list connections
# jack_lsp -c

jack_connect a2j:SparkFun\ SAMD21\ [16]\ \(capture\):\ SparkFun\ SAMD21\ MIDI\ 1 gx_head_amp:midi_in_1
# these should already be connected with the options passed to jackd
# jack_connect system:capture_1 gx_head_amp:in_0
# jack_connect system:playback_1 gx_head_amp:out_0
# jack_connect system:playback_2 gx_head_amp:out_0


# MIDI Debug
# aseqdump -l
# aseqdump -p port