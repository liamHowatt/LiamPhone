import asyncio
from typing import Iterator
import io
import time
import pygame as pg
import ctypes as ct

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

async def a_main() -> None:
    reader, writer = await asyncio.open_connection("localhost", 3000)
    pg.init()
    bg = pg.Surface((400, 240))
    window = pg.display.set_mode((400, 240))

    while True:

        assert await initTransaction(reader) == 1

        screen = await reader.readexactly(12482)

        # bg.fill((0, 0, 0))
        for event in pg.event.get():
            if event.type == pg.QUIT:
                exit()

        # t = time.time()
        draw_screen_fast(bg, screen)
        # print(time.time() - t)
        
        window.blit(bg, (0, 0))
        pg.display.flip()

        await asyncio.sleep(0.05)
        
        await finishTransaction(reader, writer, b'')
    

def main() -> None:
    asyncio.run(a_main())

if __name__ == "__main__":
    main()
