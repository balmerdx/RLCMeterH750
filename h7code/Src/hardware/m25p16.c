#include "main.h"
#include "m25p16.h"

/*
 * Initial rev 0 - only m25p16 micron support
 * Balmer rev 1 - add W25Q32 winbond support
*/

/* Support for M25P16 2Mbyte flash RAM 


BEWARE  Programming can only set bits to zero. IT CANNOT CHANGE A ZERO TO A ONE
You must use the Bulk or sector erase to set bits back to a one


	RAM_PORT		PORT for the RAM CS signal
	RAM_CS


and RAM registers

	RAM_RDID
	RAM_WREN
	RAM_BE
	RAM_PP
	RAM_RDSR

Additionally, RAM_CS must be defined as an output on the appropriate DDR

*/

// Register definitions for M25P16 flash ram
#define RAM_WREN 0x06	// write enable
#define RAM_WRDI 0x04	// write disable
#define RAM_RDID 0x9F	// read id
#define RAM_RDSR 0x05	// read status
#define RAM_RDSR2 0x35	// read status
#define RAM_RDSR3 0x15	// read status

#define RAM_WRSR 0x01	// write status
#define RAM_READ 0x03	// read data
#define RAM_FASTREAD 0x0B
#define RAM_PP 0x02		// page program
#define RAM_ERASE64K 0xD8	// erase 64 kilobyte data
#define RAM_ERASE4K 0x20	// erase 64 kilobyte data
#define RAM_BE 0xC7		// bulk erase
#define RAM_DP 0xB9		// deep power down
#define RAM_RES 0xAB	// release from power down, read electronic signature

#include "flash_spi.h"

static uint16_t sectors_count = 32;

#define ID0_WINBOND	0xEF
#define ID0_MICRON	0x20

void m25p16_init()
{
    init_spi();
}

uint16_t m25p16_sectors_count()
{
    return sectors_count;
}

static uint32_t m25p16_capacity(const uint8_t *id)
{
    uint32_t n = 1048576; // unknown chips, default to 1 MByte

    if (id[2] >= 16 && id[2] <= 31) {
        n = 1ul << id[2];
    } else
    if (id[2] >= 32 && id[2] <= 37) {
        n = 1ul << (id[2] - 6);
    } else
    if ((id[0]==0 && id[1]==0 && id[2]==0) ||
        (id[0]==255 && id[1]==255 && id[2]==255)) {
        n = 0;
    }
    //Serial.printf("capacity %lu\n", n);
    return n;
}


void m25p16_read_ram_id(uint8_t* mem_ptr) {

    CS_ZERO
	xmit_spi(RAM_RDID);
	mem_ptr[0] = recv_spi();
	mem_ptr[1] = recv_spi();
	mem_ptr[2] = recv_spi();
    CS_ONE
}

bool m25p16_read_ram_id_and_check()
{
    uint8_t id[3];
    m25p16_read_ram_id(id);

    sectors_count = m25p16_capacity(id) >> m25p16_sector_shift();

    if(!(id[0]==ID0_WINBOND || id[0]==ID0_MICRON))
        return false;

    return (id[1]==0x20 || id[1]==0x40) &&
           (id[2]>=16 && id[2]<=37);
}


uint8_t m25p16_read_ram_status(void) {
	uint8_t status;
    CS_ZERO
	xmit_spi(RAM_RDSR);
	status = recv_spi();
    CS_ONE
	return status;
}

uint8_t m25p16_read_ram_status2(void) {
    uint8_t status;
    CS_ZERO
    xmit_spi(RAM_RDSR2);
    status = recv_spi();
    CS_ONE
    return status;
}

uint8_t m25p16_read_ram_status3(void) {
    uint8_t status;
    CS_ZERO
    xmit_spi(RAM_RDSR3);
    status = recv_spi();
    CS_ONE
    return status;
}

bool m25p16_write_in_progress(void) {

    uint8_t status;

    CS_ZERO
    xmit_spi(RAM_RDSR);
    status = recv_spi();
    CS_ONE
    return (status&1)?true:false;
}

void m25p16_ram_bulk_erase(void) {
		
    CS_ZERO
	xmit_spi(RAM_WREN);					// write enable instruction 	
    CS_ONE
    DelayUs(2);
    CS_ZERO
	xmit_spi(RAM_BE);					// bulk erase instruction 
    CS_ONE
    while (m25p16_write_in_progress())
		;
}


static void m25p16_ram_eraseXk(uint32_t addr, uint8_t command)
{
    CS_ZERO
    xmit_spi(RAM_WREN);					// write enable instruction
    CS_ONE
    DelayUs(2);
    CS_ZERO
    xmit_spi(command);
    xmit_spi((uint8_t)(addr>>16));
    xmit_spi((uint8_t)(addr>>8));
    xmit_spi((uint8_t)(addr));
    CS_ONE

    DelayUs(10000);
    while (m25p16_write_in_progress())
        DelayUs(100);
}

void m25p16_ram_erase64k(uint32_t addr)
{
    m25p16_ram_eraseXk(addr, RAM_ERASE64K);
}

void m25p16_ram_erase4k(uint32_t addr)
{
    m25p16_ram_eraseXk(addr, RAM_ERASE4K);
}

void m25p16_read_write_flash_ram(M25P16_RW_TYPE rw_type, uint16_t bytes_to_readwrite, uint8_t flash_sector, uint8_t flash_page, uint8_t offset,
                                 uint8_t* mem_ptr)
{

// NB CAUTION page writes which cross page boundaries will wrap 

    if (rw_type==M25P16_RW_WRITE)
    {
        CS_ZERO
        xmit_spi(RAM_WREN);				// write enable instruction
        CS_ONE
        DelayUs(2);
	}

    CS_ZERO
    uint8_t pos[4] = {rw_type?RAM_READ:RAM_PP, flash_sector, flash_page, offset};
    xmit_spi_buffer(pos, 4);

    if (rw_type)
        recv_spi_buffer(mem_ptr, bytes_to_readwrite);
    else
        xmit_spi_buffer(mem_ptr, bytes_to_readwrite);
    CS_ONE

    if(rw_type==M25P16_RW_WRITE)
    {
        DelayUs(2000);

        while (m25p16_write_in_progress()) {
            DelayUs(2000);
        }
    } else
    {
        DelayUs(2);
    }

}


// write to the RAM status byte. 0 in bottom bit position = ready 
void m25p16_write_ram_status(uint8_t status) {

    CS_ZERO
	xmit_spi(RAM_WREN);					// write enable instruction 	
    CS_ONE
    DelayUs(2);
    CS_ZERO
	xmit_spi(RAM_WRSR);
	xmit_spi(status);
    CS_ONE
    DelayUs(2);
    while (m25p16_read_ram_status() & 0x01)
		;	
}


void m25p16_power_up_flash_ram(void) {

    CS_ZERO
	xmit_spi(RAM_RES);
    CS_ONE
    DelayUs(30);
}


void m25p16_power_down_flash_ram(void) {

    CS_ZERO
	xmit_spi(RAM_DP);
    CS_ONE
}

static void m25p16_read_or_write_flash(uint8_t one_read_zero_write, uint8_t flash_sector, uint16_t offset, uint16_t size,
                                       uint8_t* mem_ptr)
{
    if(size==0)
        return;
    int page_size = 256;
    uint8_t flash_page = offset/page_size;
    uint8_t offset_first = offset%page_size;
    uint16_t bytes_to_readwrite;

    //Первая страница может быть как неполной вначале, так и в конце
    bytes_to_readwrite = page_size-offset_first;
    if(bytes_to_readwrite>size)
        bytes_to_readwrite = size;
    m25p16_read_write_flash_ram(one_read_zero_write, bytes_to_readwrite, flash_sector, flash_page, offset_first, mem_ptr);
    size -= bytes_to_readwrite;
    mem_ptr+= bytes_to_readwrite;
    flash_page++;

    //Остальные страницы всегда начинаются сначала
    while(size>0)
    {
        bytes_to_readwrite = page_size;
        if(bytes_to_readwrite>size)
            bytes_to_readwrite = size;
        m25p16_read_write_flash_ram(one_read_zero_write, bytes_to_readwrite, flash_sector, flash_page, 0, mem_ptr);
        size -= bytes_to_readwrite;
        mem_ptr+= bytes_to_readwrite;
        flash_page++;
    }

}

static void m25p16_offset_to_sector(uint32_t offset, uint8_t* flash_sector, uint16_t* flash_sector_offset)
{
    *flash_sector = offset >> m25p16_sector_shift();
    *flash_sector_offset = offset & ((1u<<m25p16_sector_shift())-1);
}

void m25p16_read(uint32_t offset, uint16_t size, void* mem_ptr)
{
    uint8_t flash_sector;
    uint16_t flash_sector_offset;
    m25p16_offset_to_sector(offset, &flash_sector, &flash_sector_offset);
    m25p16_read_or_write_flash(M25P16_RW_READ, flash_sector, flash_sector_offset, size, (uint8_t*)mem_ptr);
}

void m25p16_write(uint32_t offset, uint16_t size, const void* mem_ptr)
{
    uint8_t flash_sector;
    uint16_t flash_sector_offset;
    m25p16_offset_to_sector(offset, &flash_sector, &flash_sector_offset);
    m25p16_read_or_write_flash(M25P16_RW_WRITE, flash_sector, flash_sector_offset, size, (uint8_t*)mem_ptr);
}
