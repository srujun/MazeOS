/*
 * vga.h - Declares functions for VGA/Bochs Extensions
 */

#ifndef VGA_H
#define VGA_H


/* BGA Constants */
#define VBE_DISPI_BANK_ADDRESS        0xA0000

#define VBE_DISPI_IOPORT_INDEX        0x01CE
#define VBE_DISPI_IOPORT_DATA         0x01CF

/* BGA Register Indices */
#define VBE_DISPI_INDEX_ID            0
#define VBE_DISPI_INDEX_XRES          1
#define VBE_DISPI_INDEX_YRES          2
#define VBE_DISPI_INDEX_BPP           3
#define VBE_DISPI_INDEX_ENABLE        4
#define VBE_DISPI_INDEX_BANK          5
#define VBE_DISPI_INDEX_VIRT_WIDTH    6
#define VBE_DISPI_INDEX_VIRT_HEIGHT   7
#define VBE_DISPI_INDEX_X_OFFSET      8
#define VBE_DISPI_INDEX_Y_OFFSET      9

#define VBE_DISPI_ID0                 0xB0C0
#define VBE_DISPI_ID1                 0xB0C1
#define VBE_DISPI_ID2                 0xB0C2
#define VBE_DISPI_ID3                 0xB0C3
#define VBE_DISPI_ID4                 0xB0C4
#define VBE_DISPI_ID5                 0xB0C5

#define VBE_DISPI_BPP_4               0x04
#define VBE_DISPI_BPP_8               0x08
#define VBE_DISPI_BPP_15              0x0F
#define VBE_DISPI_BPP_16              0x10
#define VBE_DISPI_BPP_24              0x18
#define VBE_DISPI_BPP_32              0x20

#define VBE_DISPI_DISABLED            0x00
#define VBE_DISPI_ENABLED             0x01
#define VBE_DISPI_GETCAPS             0x02
#define VBE_DISPI_8BIT_DAC            0x20
#define VBE_DISPI_LFB_ENABLED         0x40
#define VBE_DISPI_NOCLEARMEM          0x80


/* MazeOS definitions */
#define VBE_BANK_SIZE_BYTES           65536
#define VBE_BANK_NUM_SHIFT            16
#define SCREEN_WIDTH                  800
#define SCREEN_HEIGHT                 600


/* Externally visible functions */
void vga_init(void);

#endif
