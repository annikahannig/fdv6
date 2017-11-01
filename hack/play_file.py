#!/usr/bin/env python3

import time
import subprocess
import argparse

import serial
import mido


TONE_MAP_HZ = [
  16.35, 17.32,  18.35,  19.45,  20.60,  21.83,  23.12,  24.50,
  25.96,  27.50,  29.14,  30.87,  32.70,  34.65,  36.71,  38.89,
  41.20,  43.65,  46.25,  49.00,  51.91,  55.00,  58.27,  61.74,
  65.41,  69.30,  73.42,  77.78,  82.41,  87.31,  92.50,  98.00,
 103.83,  110.00,  116.54,  123.47,  130.81,  138.59,  146.83,  155.56,
 164.81,  174.61,  185.00,  196.00,  207.65,  220.00,  233.08,  246.94,
 261.63,  277.18,  293.66,  311.13,  329.63,  349.23,  369.99,  392.00
]

def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("-f", "--filename", required=True)
    parser.add_argument("-d", "--serial-device", required=True)
    parser.add_argument("-b", "--baudrate", default=19200, type=int)

    return parser.parse_args()


def note_to_hz(note):
    """Convert midi note to freq"""
    return TONE_MAP_HZ[note % len(TONE_MAP_HZ)]


def send_note(conn, note):

    freq = round(note_to_hz(note))
    if note == 0:
        freq = 0

    # Encode note (::1 - 128)
    payload = "023 {}\n".format(freq)

    conn.write(payload)


def play_file(conn, filename):
    """Generate serial packets"""
    song = mido.MidiFile(filename)

    for msg in song:
        time.sleep(msg.time)
        if msg.is_meta:
            continue

        if msg.type == "note_off":
            send_note(conn, 0)
            continue

        if msg.type == "note_on":
            # Transpose to better utilize our ouput dev
            transposed_note = msg.note - (32 - 4)
            if transposed_note < 0:
                transposed_note = 0

            send_note(conn, transposed_note)


if __name__ == "__main__":
    args = parse_args()

    conn = serial.Serial(args.serial_device, args.baudrate)

    play_file(conn, args.filename)

