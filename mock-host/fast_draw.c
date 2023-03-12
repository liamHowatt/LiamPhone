#include <stdint.h>

__declspec(dllexport) void fast_draw(uint32_t *pgBuff, const uint8_t *spiBuff) {
    for (int yline=0; yline<240; yline++) {
        spiBuff += 2;
        for (int xby=0; xby<(400 / 8); xby++) {
            uint8_t b = *(spiBuff++);
            for (int xbi=0; xbi<8; xbi++) {
                *(pgBuff++) = ((b << xbi) & 0x80) ? 0xffffff : 0x0;
            }
        }
    }
}
