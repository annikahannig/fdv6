#!/usr/bin/env python3

import time
import subprocess
import argparse

import mido


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("-i", "--interface", required=True)
    parser.add_argument("-p", "--prefix", required=True)
    parser.add_argument("-f", "--filename", required=True)

    return parser.parse_args()


def ping(addr, interface=None):

    cmd = ["ping6", "-c", "1", "-i", "0.25"]
    if interface:
        cmd += ["-I", interface]

    cmd += [addr]

    print(" ".join(cmd))

    subprocess.run(cmd)


def send_note(interface, prefix, note):

    # Encode note (::1 - 128)
    channel = 23 # (maybe we need blinkenlights...)
    payload = "{}:{:X}:{:X}".format(prefix,
                                    channel,
                                    note+1)

    ping(payload, interface)


def send_file(interface, prefix, filename):
    """Generate packets"""
    song = mido.MidiFile(filename)

    for msg in song:
        time.sleep(msg.time)
        if msg.is_meta:
            continue

        if msg.type == "note_off":
            send_note(interface, prefix, 0)
            continue

        if msg.type == "note_on":
            # Transpose to better utilize our ouput dev
            transposed_note = msg.note - (32 - 4)
            if transposed_note < 0:
                transposed_note = 0

            send_note(interface, prefix, transposed_note)


if __name__ == "__main__":
    args = parse_args()

    send_file(args.interface, args.prefix, args.filename)

