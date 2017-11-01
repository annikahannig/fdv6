#!/usr/bin/env python3

import subprocess

#import serial

# Index 0 -> C0
TONE_MAP_HZ = [
  16.35, 17.32,  18.35,  19.45,  20.60,  21.83,  23.12,  24.50,
  25.96,  27.50,  29.14,  30.87,  32.70,  34.65,  36.71,  38.89,
  41.20,  43.65,  46.25,  49.00,  51.91,  55.00,  58.27,  61.74,
  65.41,  69.30,  73.42,  77.78,  82.41,  87.31,  92.50,  98.00,
 103.83,  110.00,  116.54,  123.47,  130.81,  138.59,  146.83,  155.56,
 164.81,  174.61,  185.00,  196.00,  207.65,  220.00,  233.08,  246.94,
 261.63,  277.18,  293.66,  311.13,  329.63,  349.23,  369.99,  392.00
]


def note_to_hz(note):
    """Convert midi note to freq"""
    return TONE_MAP_HZ[note % len(TONE_MAP_HZ)]

def set_note(conn, note):
    freq = round(note_to_hz(note))

    payload = "023 {}\n".format(freq)
    print(payload)
  
    # conn.write(payload)


def doggo_sniff(serial_conn):
    """Open libpcap-doggo and parse destination"""
    cmd = ["./doggo"]

    proc = subprocess.Popen(cmd, stdout=subprocess.PIPE)
    for line in iter(proc.stdout.readline,''):
        line = str(line, "utf-8")
        tokens = line.strip().split(" ");
        if len(tokens) != 2:
            continue

        src, dst = tokens
        print("Ping to: {}".format(dst))

        # Extract paylaod channel and note
        tokens = dst.replace("::", ":").split(":")
        
        note = int(tokens[-1], 16) - 1
        channel = int(tokens[-2], 16)

        set_note(serial_conn, note)

        print("RECV note {} @ channel {}".format(note, channel))


if __name__ == "__main__":
  conn = None
  # conn = serial.Serial("/dev/ttyUSB1", 9600)

  doggo_sniff(conn)


