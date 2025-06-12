/*
 * sdcard.c
 *
 *  Created on: Apr 12, 2025
 *      Author: ching
 */
#include "sdcard.h"
#include "getrtc.h"
#include <stdio.h>
#include <string.h>
#include "u8g2.h"
#include "spi.h"

extern u8g2_t u8g2;
extern Disk_drvTypeDef  disk;

FATFS FatFs;
FIL file;
FRESULT fres;
char filename[128];

uint8_t SD_SPI_Transmit(uint8_t data)
{
    uint8_t response = 0xFF;
    HAL_SPI_TransmitReceive(&hspi2, &data, &response, 1, HAL_MAX_DELAY);
    return response;
}

void init_csv(void) {
	unsigned int bytesWritten;
	char buffer[128];
	char timestamp2[100];

	get_time_csv(timestamp2);

	sprintf(filename, "%s.csv", timestamp2);
	disk.is_initialized[0] = 0;

	fres = f_mount(&FatFs, "", 1);
	if (fres != FR_OK) {
		Error_Handler();
//		f_err_str(fres);
	}

	// Create file
	if (f_open(&file, filename, FA_WRITE | FA_CREATE_ALWAYS) != FR_OK) {
//	if (f_open(&file, "tp2.txt", FA_WRITE | FA_CREATE_ALWAYS) != FR_OK) {
		Error_Handler();
	}

	sprintf(buffer, "Timestamp, CH1, CH2, CH3, CH4, CH5, CH6, CH7, CH8\n");

	f_write(&file, buffer, strlen(buffer), &bytesWritten);

	f_close(&file);
}

void write_csv(float ch1, float ch2, float ch3, float ch4, float ch5, float ch6, float ch7, float ch8) {
	unsigned int bytesWritten;
	char buffer[128];
	char timestamp[20];

	get_time_print(timestamp);

	if (f_open(&file, filename, FA_WRITE | FA_OPEN_APPEND) != FR_OK) {
		return;
	}

	// Write the data entry
	sprintf(buffer, "%s,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f\n", timestamp, ch1, ch2, ch3, ch4, ch5, ch6, ch7, ch8);
	f_write(&file, buffer, strlen(buffer), &bytesWritten);

	f_close(&file);
}

void init_sdcard() {
	if (disk_initialize(0) != RES_OK) {
		Error_Handler();
	}
}

void f_err_str(FRESULT res) {
    switch (res) {
        case FR_OK:
    		u8g2_DrawStr(&u8g2, 30, 37, "OK");
    		u8g2_SendBuffer(&u8g2);
    		break;
        case FR_DISK_ERR:
    		u8g2_DrawStr(&u8g2, 30, 37, "disk error");
    		u8g2_SendBuffer(&u8g2);
        	break;
        case FR_INT_ERR:
    		u8g2_DrawStr(&u8g2, 30, 37, "int error");
    		u8g2_SendBuffer(&u8g2);
        	break;
        case FR_NOT_READY:
    		u8g2_DrawStr(&u8g2, 30, 37, "not ready");
    		u8g2_SendBuffer(&u8g2);
        	break;
        case FR_NO_FILE:
    		u8g2_DrawStr(&u8g2, 30, 37, "no file");
    		u8g2_SendBuffer(&u8g2);
        	break;
        case FR_NO_PATH:
    		u8g2_DrawStr(&u8g2, 30, 37, "no path");
    		u8g2_SendBuffer(&u8g2);
        	break;
        case FR_INVALID_NAME:
    		u8g2_DrawStr(&u8g2, 30, 37, "invalid name");
    		u8g2_SendBuffer(&u8g2);
        	break;
        case FR_DENIED:
    		u8g2_DrawStr(&u8g2, 30, 37, "denied");
    		u8g2_SendBuffer(&u8g2);
        	break;
        case FR_EXIST:
    		u8g2_DrawStr(&u8g2, 30, 37, "exist");
    		u8g2_SendBuffer(&u8g2);
        	break;
        case FR_INVALID_OBJECT:
    		u8g2_DrawStr(&u8g2, 30, 37, "invalid object");
    		u8g2_SendBuffer(&u8g2);
        	break;
        case FR_WRITE_PROTECTED:
    		u8g2_DrawStr(&u8g2, 30, 37, "write protected");
    		u8g2_SendBuffer(&u8g2);
        	break;
        case FR_INVALID_DRIVE:
    		u8g2_DrawStr(&u8g2, 30, 37, "invalid drive");
    		u8g2_SendBuffer(&u8g2);
        	break;
        case FR_NOT_ENABLED:
    		u8g2_DrawStr(&u8g2, 30, 37, "not enabled");
    		u8g2_SendBuffer(&u8g2);
        	break;
        case FR_NO_FILESYSTEM:
    		u8g2_DrawStr(&u8g2, 30, 37, "no file system");
    		u8g2_SendBuffer(&u8g2);
        	break;
        case FR_MKFS_ABORTED:
    		u8g2_DrawStr(&u8g2, 30, 37, "mkfs aborted");
    		u8g2_SendBuffer(&u8g2);
        	break;
        case FR_TIMEOUT:
    		u8g2_DrawStr(&u8g2, 30, 37, "timeout");
    		u8g2_SendBuffer(&u8g2);
        	break;
        case FR_LOCKED:
    		u8g2_DrawStr(&u8g2, 30, 37, "locked");
    		u8g2_SendBuffer(&u8g2);
        	break;
        case FR_NOT_ENOUGH_CORE:
    		u8g2_DrawStr(&u8g2, 30, 37, "no enough core");
    		u8g2_SendBuffer(&u8g2);
        	break;
        case FR_TOO_MANY_OPEN_FILES:
    		u8g2_DrawStr(&u8g2, 30, 37, "too many open files");
    		u8g2_SendBuffer(&u8g2);
        	break;
        case FR_INVALID_PARAMETER:
    		u8g2_DrawStr(&u8g2, 30, 37, "invalid parameter");
    		u8g2_SendBuffer(&u8g2);
        	break;
        default:
    		u8g2_DrawStr(&u8g2, 30, 37, "unknown");
    		u8g2_SendBuffer(&u8g2);
        	break;
    }
}

