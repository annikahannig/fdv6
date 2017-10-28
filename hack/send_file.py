#!/usr/bin/env python3

import time

import mido


def send_note(prefix, note):
    print("PING :: ... <- {}".format(hz))


def send_file(prefix, filename):
    """Generate packets"""
    song = mido.MidiFile(filename)

    for msg in song:
        time.sleep(msg.time)
        if msg.is_meta:
            continue

        if msg.type == "note_off":
            send_freq(prefix, 0)
            continue

        if msg.type == "note_on":
            # Transpose to better utilize our ouput dev
            transposed_note = msg.note - (32 - 4)
            if transposed_note < 0:
                transposed_note = 0

            freq = note_to_hz(transposed_note)

            send_freq(prefix, freq)

