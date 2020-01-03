#include "store_to_spi_flash.h"
#include "m25p16.h"

#include <memory.h>

//Пожалуй формат записи аналогичный store_to_stm32_flash слишком много данных требует прочитать.
//Поэтому будет другой вариант, оптимизирующий скорость поиска.
//Первые два байта сектора - размер структуры.
//Потом последующие биты постепенно заполняются нулями, на каждую записанную структуру - один нолик.

//Размер поля, в котором хранится размер структуры
#define STRUCT_SS 2
//Смещение структуры с таким индексом
#define PARTS_OFFSET(structure_index) (STRUCT_SS + flags_size + (structure_index)*(uint32_t)struct_size)

//Количество байт, которые занимает служебная область
//В которой битиками отмеченна уже использованная часть flash
//Кратно 4 байтам для ускорения чтения
static uint16_t partsFlagsSize(uint16_t struct_size)
{
    uint16_t count = (m25p16_sector_size()-STRUCT_SS)*8/(struct_size*8+1);
    return (count+31)/32*4;
}

//Сколько раз можно записать структуру в этом секторе
//учитывая служебные данные
static uint16_t partsAvailable(uint16_t struct_size)
{
    uint16_t flags_size = partsFlagsSize(struct_size);
    uint16_t count = (m25p16_sector_size()-STRUCT_SS-flags_size)/struct_size;
    return count;
}

static bool SpiCheckEqual(uint32_t flash_offset, uint16_t struct_size, void* data_to_check)
{
    static uint8_t buf[64];
    int buf_size = sizeof(buf);
    uint8_t* data_to_check8 = (uint8_t*)data_to_check;

    while(struct_size>0)
    {
        int size = struct_size;
        if(size > buf_size)
            size = buf_size;

        m25p16_read(flash_offset, size, buf);
        if(memcmp(buf, data_to_check8, size)!=0)
            return false;

        flash_offset += size;
        data_to_check8 += size;
        struct_size -= size;
    }

    return true;
}

bool SpiFlashWriteToFlash(uint16_t flash_sector, uint16_t struct_size, void* void_data)
{
    if(struct_size==0)
        return false;
    uint32_t flash_offset = flash_sector*m25p16_sector_size();

    uint16_t flags_size = partsFlagsSize(struct_size);
    uint16_t parts = partsAvailable(struct_size);
    if(parts==0)
        return false;

    uint16_t struct_size_flash = 0;
    m25p16_read(flash_offset+0, STRUCT_SS, &struct_size_flash);

    if(struct_size_flash!=struct_size)
    {
        m25p16_ram_erase64k(flash_offset);
        m25p16_write(flash_offset+0, STRUCT_SS, &struct_size);
    }

    uint32_t summary_size = STRUCT_SS+flags_size+parts*struct_size;
    if(summary_size>m25p16_sector_size())
        return false;

    uint16_t struct_idx = 0xFFFF;
    for(uint16_t fpos=0; fpos<flags_size; fpos+=4)
    {
        uint32_t flags;
        m25p16_read(flash_offset+fpos+STRUCT_SS, sizeof(flags), &flags);

        if(flags==0)//Все текущие структуры уже заняты
            continue;

        for(int i=0; i<32; i++)
        {
            if((flags&1)==1)
            {
                struct_idx = fpos*8 + i;
                break;
            }

            flags >>= 1;
        }
        break;
    }

    if(struct_idx >= parts)
    {
        m25p16_ram_erase64k(flash_offset);
        m25p16_write(flash_offset+0, STRUCT_SS, &struct_size);
        struct_idx = 0;
    }

    uint32_t parts_offset;
    if(struct_idx>0)
    {
        //Проверяем, если данные не изменились, то ничего не пишем.
        parts_offset = PARTS_OFFSET(struct_idx-1);

        if(SpiCheckEqual(flash_offset+parts_offset, struct_size, void_data))
        {
            return true;
        }
    }

    parts_offset = PARTS_OFFSET(struct_idx);
    if(parts_offset+struct_size > m25p16_sector_size())
        return false;

    //Пишем битик, что этот struct_idx занят.
    uint8_t flag8;
    m25p16_read(flash_offset+STRUCT_SS + struct_idx/8, 1, &flag8);
    flag8 &= ~(uint8_t)(1<<(struct_idx%8));
    m25p16_write(flash_offset+STRUCT_SS + struct_idx/8, 1, &flag8);

    //Пишем структуру
    m25p16_write(flash_offset+parts_offset, struct_size, void_data);
    return true;
}

bool SpiFlashReadFromFlash(uint16_t flash_sector, uint16_t struct_size, void *void_data)
{
    if(struct_size==0)
        return false;

    uint32_t flash_offset = flash_sector*m25p16_sector_size();

    uint16_t struct_size_flash = 0;
    m25p16_read(flash_offset+0, STRUCT_SS, &struct_size_flash);

    if(struct_size_flash!=struct_size)
        return false;

    uint16_t flags_size = partsFlagsSize(struct_size);
    uint16_t parts = partsAvailable(struct_size);
    if(parts==0)
        return false;

    uint16_t struct_idx = 0xFFFF;
    for(uint16_t fpos=0; fpos<flags_size; fpos+=4)
    {
        uint32_t flags;
        m25p16_read(flash_offset+fpos+STRUCT_SS, sizeof(flags), &flags);

        if(flags==0)//Все текущие структуры уже использованны, берем последнюю
        {
            struct_idx = fpos*8+31;
            continue;
        }

        for(int i=0; i<32; i++)
        {
            if((flags&1)==1)
            {
                break;
            } else
            {
                struct_idx = fpos*8 + i;
            }

            flags >>= 1;
        }

        break;
    }

    if(struct_idx >= parts)
        return false;

    uint32_t parts_offset = PARTS_OFFSET(struct_idx);
    if(parts_offset+struct_size > m25p16_sector_size())
        return false;

    m25p16_read(flash_offset+parts_offset, struct_size, void_data);

    return true;
}
