#!/usr/bin/env python3

import time
import subprocess

import mido


def ping(addr, interface=None):

    cmd = ["ping6", "-c", "1", "-i", "0.25"]
    if interface:
        cmd += ["-I", interface]

    cmd += [addr]

    subprocess.run(cmd)


def send_note(prefix, note):

    # Encode note (::1 - 128)
    channel = 23 # (maybe we need blinkenlights...)
    payload = "{}::{}:{}".format(prefix,
                                 channel,
                                 hex(note+1)[2:])

    ping(payload, "lo0")


def send_file(prefix, filename):
    """Generate packets"""
    song = mido.MidiFile(filename)

    for msg in song:
        time.sleep(msg.time)
        if msg.is_meta:
            continue

        if msg.type == "note_off":
            send_note(prefix, 0)
            continue

        if msg.type == "note_on":
            # Transpose to better utilize our ouput dev
            transposed_note = msg.note - (32 - 4)
            if transposed_note < 0:
                transposed_note = 0

            send_note(prefix, transposed_note)

