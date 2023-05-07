import asyncio
from typing import Iterator
import io
import time
import pygame as pg
import ctypes as ct
import struct
import os

Re = asyncio.StreamReader
Wr = asyncio.StreamWriter

async def initTransaction(reader: Re) -> int:
    return (await reader.readexactly(1))[0]

async def finishTransaction(reader: Re, writer: Wr, data: bytes) -> None:
    ptr_size = (await reader.readexactly(1))[0]
    ptrs = await reader.readexactly(ptr_size * 2)
    writer.write(ptrs)
    writer.write(bytes((len(data), )))
    writer.write(data)
    await writer.drain()

def bitterator(b: io.BytesIO) -> Iterator[int]:
    while 1:
        b.read(2)
        for _ in range(400 // 8):
            a = b.read(1)[0]
            for i in range(8):
                yield (a << i) & 0x80

def draw_screen(surf: pg.Surface, screen: bytes) -> None:
    bitt = bitterator(io.BytesIO(screen))
    for y in range(240):
        for x in range(400):
            surf.set_at((x, y), (255, 255, 255) if next(bitt) else (0, 0, 0))

fast_draw_dll = ct.CDLL(r"C:\Users\liamj\Documents\diy_phone\mock_host\fast_draw.dll")
fast_draw_dll.fast_draw.argtypes = (ct.POINTER(ct.c_uint32), ct.POINTER(ct.c_uint8))
fast_draw_dll.fast_draw.restype = None

def draw_screen_fast(pgBuff: pg.Surface, spiBuff: bytes):
    rawPgBuff = pg.surfarray.pixels2d(pgBuff).ctypes.data_as(ct.POINTER(ct.c_uint32))
    rawSpiBuff = (ct.c_uint8 * len(spiBuff)).from_buffer_copy(spiBuff)
    fast_draw_dll.fast_draw(rawPgBuff, rawSpiBuff)


# encoder = 0
# async def encoder_incrementer():
#     global encoder
#     while True:
#         await asyncio.sleep(1)
#         encoder += 1

async def a_main() -> None:
    # asyncio.create_task(encoder_incrementer())
    encoder = 0

    button_presses = []

    files = {}

    reader, writer = await asyncio.open_connection("localhost", 3000)
    pg.init()
    bg = pg.Surface((400, 240))
    window = pg.display.set_mode((400, 240))

    while True:

        code = await initTransaction(reader)

        for event in pg.event.get():
            if event.type == pg.QUIT:
                exit()
            elif event.type == pg.KEYDOWN:
                if event.key == pg.K_UP:
                    encoder -= 1
                elif event.key == pg.K_DOWN:
                    encoder += 1
                elif event.key == pg.K_RETURN:
                    button_presses.insert(0, 0)
                elif event.key == pg.K_BACKSPACE:
                    button_presses.insert(0, 1)

        if code == 1: # display

            screen = await reader.readexactly(12482)

            # t = time.time()
            draw_screen_fast(bg, screen)
            # print(time.time() - t)
            
            window.blit(bg, (0, 0))
            pg.display.flip()

            # await asyncio.sleep(0.05)
            
            await finishTransaction(reader, writer, b'')
        
        elif code == 2: # encoder
            await finishTransaction(reader, writer, struct.pack("<i", encoder))

        elif code == 3: # button
            await finishTransaction(reader, writer, struct.pack("<b",
                button_presses.pop()
                if button_presses
                else -1
            ))
        
        elif code == 4: # file open
            path_len = struct.unpack("<B", await reader.readexactly(1))[0]
            print(f"{path_len=}")
            path = await reader.readexactly(path_len)
            print(f"{path=}")
            f = open(b"disk\\" + path, "rb")
            fd = f.fileno()
            print(f"open {fd=}")
            await finishTransaction(reader, writer, struct.pack("<i", fd))
            files[fd] = f

        elif code == 5: # file close
            fd = struct.unpack("<i", await reader.readexactly(4))[0]
            print(f"close {fd=}")
            files.pop(fd).close()
            await finishTransaction(reader, writer, b'')

        elif code == 6: # file size
            fd = struct.unpack("<i", await reader.readexactly(4))[0]
            print(f"size {fd=}")
            f = files[fd]
            offset = f.tell()
            size = f.seek(0, 2)
            f.seek(offset, 0)
            print(f"{size=}")
            await finishTransaction(reader, writer, struct.pack("<i", size))

        elif code == 7: # file read
            fd = struct.unpack("<i", await reader.readexactly(4))[0]
            print(f"read {fd=}")
            n = struct.unpack("<i", await reader.readexactly(4))[0]
            print(f"{n=}")
            f = files[fd]
            read_bytes = f.read(n)
            print(f"{read_bytes=}")
            assert len(read_bytes) == n
            await finishTransaction(reader, writer, read_bytes)

        elif code == 8: # file seek
            fd = struct.unpack("<i", await reader.readexactly(4))[0]
            print(f"seek {fd=}")
            n = struct.unpack("<i", await reader.readexactly(4))[0]
            print(f"{n=}")
            whence = struct.unpack("<B", await reader.readexactly(1))[0]
            print(f"{whence=}")
            files[fd].seek(n, whence)
            await finishTransaction(reader, writer, b'')

        else:
            raise Exception
    

def main() -> None:
    asyncio.run(a_main())

if __name__ == "__main__":
    main()
