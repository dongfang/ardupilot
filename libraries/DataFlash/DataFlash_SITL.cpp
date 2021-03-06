/// -*- tab-width: 4; Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*-
/*
  hacked up DataFlash library for Desktop support
*/

#include <AP_HAL.h>

#if CONFIG_HAL_BOARD == HAL_BOARD_AVR_SITL

#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include "DataFlash.h"

#define DF_PAGE_SIZE 512
#define DF_NUM_PAGES 4096

extern const AP_HAL::HAL& hal;

static int flash_fd;
static uint8_t buffer[2][DF_PAGE_SIZE];

// Public Methods //////////////////////////////////////////////////////////////
void DataFlash_SITL::Init(const struct LogStructure *structure, uint8_t num_types)
{
    DataFlash_Class::Init(structure, num_types);
	if (flash_fd == 0) {
		flash_fd = open("dataflash.bin", O_RDWR, 0777);
		if (flash_fd == -1) {
			uint8_t *fill;
			fill = (uint8_t *)malloc(DF_PAGE_SIZE*DF_NUM_PAGES);
			flash_fd = open("dataflash.bin", O_RDWR | O_CREAT, 0777);
			memset(fill, 0xFF, DF_PAGE_SIZE*DF_NUM_PAGES);
			write(flash_fd, fill, DF_PAGE_SIZE*DF_NUM_PAGES);
			free(fill);
		}
	}
	df_PageSize = DF_PAGE_SIZE;

    // reserve last page for config information
    df_NumPages   = DF_NUM_PAGES - 1;
}

// This function is mainly to test the device
void DataFlash_SITL::ReadManufacturerID()
{
	df_manufacturer = 1;
	df_device = 0x0203;
}

bool DataFlash_SITL::CardInserted(void)
{
    return true;
}

// Read the status register
uint8_t DataFlash_SITL::ReadStatusReg()
{
	return 0;
}

// Read the status of the DataFlash
inline
uint8_t DataFlash_SITL::ReadStatus()
{
	return 1;
}


inline
uint16_t DataFlash_SITL::PageSize()
{
	return df_PageSize;
}


// Wait until DataFlash is in ready state...
void DataFlash_SITL::WaitReady()
{
	while(!ReadStatus());
}

void DataFlash_SITL::PageToBuffer(unsigned char BufferNum, uint16_t PageAdr)
{
	pread(flash_fd, buffer[BufferNum], DF_PAGE_SIZE, PageAdr*DF_PAGE_SIZE);
}

void DataFlash_SITL::BufferToPage (unsigned char BufferNum, uint16_t PageAdr, unsigned char wait)
{
	pwrite(flash_fd, buffer[BufferNum], DF_PAGE_SIZE, PageAdr*DF_PAGE_SIZE);
}

void DataFlash_SITL::BufferWrite (unsigned char BufferNum, uint16_t IntPageAdr, unsigned char Data)
{
	buffer[BufferNum][IntPageAdr] = (uint8_t)Data;
}

void DataFlash_SITL::BlockWrite(uint8_t BufferNum, uint16_t IntPageAdr, 
                                const void *pHeader, uint8_t hdr_size,
                                const void *pBuffer, uint16_t size)
{
    if (!_writes_enabled) {
        return;
    }
    if (hdr_size) {
        memcpy(&buffer[BufferNum][IntPageAdr],
               pHeader,
               hdr_size);
    }
    memcpy(&buffer[BufferNum][IntPageAdr+hdr_size],
           pBuffer,
           size);
}

unsigned char DataFlash_SITL::BufferRead (unsigned char BufferNum, uint16_t IntPageAdr)
{
	return (unsigned char)buffer[BufferNum][IntPageAdr];
}

// read size bytes of data to a page. The caller must ensure that
// the data fits within the page, otherwise it will wrap to the
// start of the page
bool DataFlash_SITL::BlockRead(uint8_t BufferNum, uint16_t IntPageAdr, void *pBuffer, uint16_t size)
{
	memcpy(pBuffer, &buffer[BufferNum][IntPageAdr], size);
    return true;
}


// *** END OF INTERNAL FUNCTIONS ***

void DataFlash_SITL::PageErase (uint16_t PageAdr)
{
	uint8_t fill[DF_PAGE_SIZE];
	memset(fill, 0xFF, sizeof(fill));
	pwrite(flash_fd, fill, DF_PAGE_SIZE, PageAdr*DF_PAGE_SIZE);
}

void DataFlash_SITL::BlockErase (uint16_t BlockAdr)
{
	uint8_t fill[DF_PAGE_SIZE*8];
	memset(fill, 0xFF, sizeof(fill));
	pwrite(flash_fd, fill, DF_PAGE_SIZE*8, BlockAdr*DF_PAGE_SIZE*8);
}


void DataFlash_SITL::ChipErase()
{
	for (int i=0; i<DF_NUM_PAGES; i++) {
		PageErase(i);
        hal.scheduler->delay(1);
	}
}


#endif


