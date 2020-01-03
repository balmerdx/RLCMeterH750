#include "main.h"
#include "test_store_to_flash.h"
#include "hardware/m25p16.h"
#include "hardware/store_to_spi_flash.h"

#include <stdio.h>

void TestFlash()
{
    m25p16_ram_erase64k(0x10000);

    uint32_t offset = 0x10000;
    uint16_t size = 128;
    static char buffer[128];
    for(int i=0; i<size; i++)
        buffer[i] = i;

    m25p16_write(offset, size, buffer);

    sprintf(buffer, "Flash write complete");
    UTFT_print(buffer, 20, 50);

    m25p16_read(offset, size, buffer);

    bool ok = true;
    for(int i=0; i<size; i++)
        if(buffer[i] != i)
            ok = false;

    if(ok)
        sprintf(buffer, "Flash read OK      ");
    else
        sprintf(buffer, "Flash read FAIL      ");
    UTFT_print(buffer, 20, 50);
    while(1);
}

typedef struct
{
    uint32_t data0;
    uint32_t data1;
    uint32_t data2;
    uint16_t data3;
    uint16_t data4;
} TestWriteStruct;

#define BIG_STRUCT_WORDS 1234

void TestStoreToFlash()
{
    int x = 0, y = 0;
    UTFT_clrScr();
    UTF_SetFont(font_condensed30);
    UTFT_setColorW(VGA_WHITE);
    UTFT_setBackColorW(VGA_BLACK);

    x = UTF_DrawString(x,y, "TestStoreToSpi init");
    x = 0; y+= UTF_Height();

    int num_width = UTF_StringWidth("0000");

    uint8_t flash_sector = 2;
    bool ok;

    //Test big struct
    static uint32_t big_to_write[BIG_STRUCT_WORDS];
    static uint32_t big_to_read[BIG_STRUCT_WORDS];

    x = UTF_DrawString(x,y, "Write big struct"); x = 0; y+= UTF_Height();

    for(int i=0; i<50; i++)
    {
        UTF_printNumI(i, x, y, num_width, UTF_LEFT);

        for(int j=0; j<BIG_STRUCT_WORDS; j++)
            big_to_write[j] = (i>>1)+j;

        ok = SpiFlashWriteToFlash(flash_sector, sizeof(big_to_write), big_to_write);
        if(!ok)
        {
            x = 0; y+= UTF_Height();
            x = UTF_DrawString(x,y, ok?"write ok":"write fail");
        }

        ok = SpiFlashReadFromFlash(flash_sector, sizeof(big_to_read), &big_to_read);
        if(!ok)
        {
            x = 0; y+= UTF_Height();
            x = UTF_DrawString(x,y, ok?"read ok":"read fail");
        }

        ok = memcmp(big_to_write, big_to_read, sizeof(big_to_write))==0;
        if(!ok)
        {
            x = 0; y+= UTF_Height();
            x = UTF_DrawString(x,y, ok?"cmp ok":"cmp fail");
        }
    }


    //Test small struct
    TestWriteStruct d = {0x12345678, 0x22334455, 0x77221100, 0xFF00};
    TestWriteStruct dr;

    x = UTF_DrawString(x,y, "Write small struct"); x = 0; y+= UTF_Height();

    for(int i=0; i<5000; i++)
    {
        UTF_printNumI(i, x, y, num_width, UTF_LEFT);

        ok = SpiFlashWriteToFlash(flash_sector, sizeof(d), &d);
        if(!ok)
        {
            x = 0; y+= UTF_Height();
            x = UTF_DrawString(x,y, ok?"write ok":"write fail");
        }

        ok = SpiFlashReadFromFlash(flash_sector, sizeof(dr), &dr);
        if(!ok)
        {
            x = 0; y+= UTF_Height();
            x = UTF_DrawString(x,y, ok?"read ok":"read fail");
        }

        ok = memcmp(&d, &dr, sizeof(d))==0;
        if(!ok)
        {
            x = 0; y+= UTF_Height();
            x = UTF_DrawString(x,y, ok?"cmp ok":"cmp fail");
        }

        d.data4+=77;
    }

    while(1);
}
