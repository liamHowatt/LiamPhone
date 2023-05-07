import json
import shutil
import os
import struct
import random

CONTENTS_FILE = "disk_contents.json"
DISK_DIR = "disk\\"

with open(CONTENTS_FILE) as f:
    cont = json.load(f)

shutil.rmtree(DISK_DIR, ignore_errors=True)
assert not os.path.exists(DISK_DIR)
os.mkdir(DISK_DIR)

with open(DISK_DIR + "contacts", "wb") as f:
    f.write(struct.pack("<h", len(cont["contacts"])))
    for name, number in cont["contacts"].items():
        f.write(struct.pack("<B", len(name)))
        f.write(name.encode())
        f.write(struct.pack("<B", len(number)))
        f.write(number.encode())

os.mkdir(DISK_DIR + "texts")

for number, convo in cont["texts"].items():
    with open(DISK_DIR + "texts\\" + number, "wb") as f:
        for rxtx, timestamp, text in convo:
            assert rxtx in {"rx", "tx"}
            f.write(struct.pack("<B", int(rxtx == "tx")))
            assert len(timestamp) == 14
            f.write(timestamp.encode())
            f.write(struct.pack("<h", len(text)))
            f.write(text.encode())
            f.write(struct.pack("<h", len(text)))

index = [
    (number, random.randint(0, 2), convo[-1][1], convo[-1][2])
    for number, convo
    in cont["texts"].items()
]
random.shuffle(index)
index_index = sorted(
    range(len(index)),
    key=(lambda i: index[i][2]),
    reverse=True,
)
for i in index_index:
    print(index[i])
with open(DISK_DIR + "text_index", "wb") as f:
    f.write(struct.pack("<h", len(index)))
    for (number, unreads, stamp, message), i in zip(index, index_index):
        f.write(struct.pack("<h", i))
        f.write(struct.pack("<B", len(number)))
        f.write(number.encode())
        f.write(struct.pack("<B", unreads))
        assert len(stamp) == 14
        f.write(stamp.encode())
        message = message.encode()
        if len(message) <= 50:
            f.write(struct.pack("<B", len(message)))
            f.write(message)
        else:
            f.write(struct.pack("<B", 50))
            f.write(message[:47] + b"...")

