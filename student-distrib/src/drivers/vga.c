/*
 * vga.c
 */

#include "drivers/vga.h"
#include "lib.h"

void
fill_palette()
{
    /* 6-bit RGB (red, green, blue) values for first 64 colors */
    /* these are coded for 2 bits red, 2 bits green, 2 bits blue */
    static unsigned char palette_RGB[64][3] = {
        {0x00, 0x00, 0x00}, {0x00, 0x00, 0x15},
        {0x00, 0x00, 0x2A}, {0x00, 0x00, 0x3F},
        {0x00, 0x15, 0x00}, {0x00, 0x15, 0x15},
        {0x00, 0x15, 0x2A}, {0x00, 0x15, 0x3F},
        {0x00, 0x2A, 0x00}, {0x00, 0x2A, 0x15},
        {0x00, 0x2A, 0x2A}, {0x00, 0x2A, 0x3F},
        {0x00, 0x3F, 0x00}, {0x00, 0x3F, 0x15},
        {0x00, 0x3F, 0x2A}, {0x00, 0x3F, 0x3F},
        {0x15, 0x00, 0x00}, {0x15, 0x00, 0x15},
        {0x15, 0x00, 0x2A}, {0x15, 0x00, 0x3F},
        {0x15, 0x15, 0x00}, {0x15, 0x15, 0x15},
        {0x15, 0x15, 0x2A}, {0x15, 0x15, 0x3F},
        {0x15, 0x2A, 0x00}, {0x15, 0x2A, 0x15},
        {0x15, 0x2A, 0x2A}, {0x15, 0x2A, 0x3F},
        {0x15, 0x3F, 0x00}, {0x15, 0x3F, 0x15},
        {0x15, 0x3F, 0x2A}, {0x15, 0x3F, 0x3F},
        {0x2A, 0x00, 0x00}, {0x2A, 0x00, 0x15},
        {0x2A, 0x00, 0x2A}, {0x2A, 0x00, 0x3F},
        {0x2A, 0x15, 0x00}, {0x2A, 0x15, 0x15},
        {0x2A, 0x15, 0x2A}, {0x2A, 0x15, 0x3F},
        {0x2A, 0x2A, 0x00}, {0x2A, 0x2A, 0x15},
        {0x2A, 0x2A, 0x2A}, {0x2A, 0x2A, 0x3F},
        {0x2A, 0x3F, 0x00}, {0x2A, 0x3F, 0x15},
        {0x2A, 0x3F, 0x2A}, {0x2A, 0x3F, 0x3F},
        {0x3F, 0x00, 0x00}, {0x3F, 0x00, 0x15},
        {0x3F, 0x00, 0x2A}, {0x3F, 0x00, 0x3F},
        {0x3F, 0x15, 0x00}, {0x3F, 0x15, 0x15},
        {0x3F, 0x15, 0x2A}, {0x3F, 0x15, 0x3F},
        {0x3F, 0x2A, 0x00}, {0x3F, 0x2A, 0x15},
        {0x3F, 0x2A, 0x2A}, {0x3F, 0x2A, 0x3F},
        {0x3F, 0x3F, 0x00}, {0x3F, 0x3F, 0x15},
        {0x3F, 0x3F, 0x2A}, {0x3F, 0x3F, 0x3F}
    };

    /* Start writing at color 0. */
    outb(0x00, 0x03C8);

    /* Write all 64 colors from array. */
    int i;
    for (i = 0; i < 64; i++)
    {
        outb(palette_RGB[i][0], 0x03C9);
        outb(palette_RGB[i][1], 0x03C9);
        outb(palette_RGB[i][2], 0x03C9);
    }
}


void 
BGA_write(uint32_t index, uint32_t value)
{
    outw(index, VBE_DISPI_IOPORT_INDEX);
    outw(value, VBE_DISPI_IOPORT_DATA);
}


uint32_t
BGA_read(uint32_t index)
{
    outw(index, VBE_DISPI_IOPORT_INDEX);
    return inw(VBE_DISPI_IOPORT_DATA);
}


int32_t
write_pixel_palette(uint32_t x, uint32_t y, uint32_t palette_num)
{
    if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y > SCREEN_HEIGHT)
        return -1;
    if (palette_num >= 256)
        return -1;

    static int32_t last_bank_num = -1;

    uint32_t offset = y * SCREEN_WIDTH + x;
    uint32_t bank_num = offset >> VBE_BANK_NUM_SHIFT;

    if (bank_num != last_bank_num)
    {
        BGA_write(VBE_DISPI_INDEX_BANK, bank_num);
        last_bank_num = bank_num;
    }

    offset &= (VBE_BANK_SIZE_BYTES - 1);
    memset((void*)VBE_DISPI_BANK_ADDRESS + offset, palette_num, 1);

    return 0;
}


int32_t
write_pixel_rgb(uint32_t x, uint32_t y, uint32_t r, uint32_t g, uint32_t b)
{
    if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y > SCREEN_HEIGHT)
        return -1;

    static int32_t last_bank_num = -1;

    uint32_t offset = (y * SCREEN_WIDTH + x) * 2;
    uint32_t bank_num = offset >> VBE_BANK_NUM_SHIFT;
    r = (r >> 3) & 0x1F;
    g = (g >> 3) & 0x1F;
    b = (b >> 3) & 0x1F;
    uint8_t byte1 = (r << 2) | ((g & 0x18) >> 3);
    uint8_t byte2 = ((g & 0x7) << 5) | b;

    if (bank_num != last_bank_num)
    {
        BGA_write(VBE_DISPI_INDEX_BANK, bank_num);
        last_bank_num = bank_num;
    }

    offset &= (VBE_BANK_SIZE_BYTES - 1);
    memset((void*)VBE_DISPI_BANK_ADDRESS + offset, byte2, 1);
    memset((void*)VBE_DISPI_BANK_ADDRESS + offset + 1, byte1, 1);

    return 0;
}


void
vga_init(void)
{
    BGA_write(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_DISABLED);
    BGA_write(VBE_DISPI_INDEX_ID, VBE_DISPI_ID4);
    BGA_write(VBE_DISPI_INDEX_XRES, SCREEN_WIDTH);
    BGA_write(VBE_DISPI_INDEX_YRES, SCREEN_HEIGHT);
    BGA_write(VBE_DISPI_INDEX_BPP, VBE_DISPI_BPP_15);
    BGA_write(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_ENABLED);
}
