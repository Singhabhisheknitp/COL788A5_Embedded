#include "sdcard.h"
#include "fatfs.h"
#include <stdio.h>
#include <string.h>
#include "main.h"

FATFS FatFs;   // FatFs handle
FIL fil;       // File handle
FRESULT fres;  // File operation result

// Function to initialize and mount the SD card
FRESULT SDCard_Init(void) {


    fres = f_mount(&FatFs, "", 1);
    myprintf("f_mount result is (%d)\r\n", fres);
    if (fres != FR_OK) {
        myprintf("f_mount error (%d)\r\n", fres);
    }
    return fres;
}

// Function to get SD card statistics
void SDCard_GetStats(void) {
    DWORD free_clusters, free_sectors, total_sectors;
    FATFS *getFreeFs;

    if (f_getfree("", &free_clusters, &getFreeFs) == FR_OK) {
        total_sectors = (getFreeFs->n_fatent - 2) * getFreeFs->csize;
        free_sectors = free_clusters * getFreeFs->csize;
        myprintf("SD card stats:\r\n%10lu KiB total.\r\n%10lu KiB free.\r\n",
                 total_sectors / 2, free_sectors / 2);
    }
}

// Function to read from a file
FRESULT SDCard_ReadFile(char *filename, char *buffer, UINT bytesToRead) {
    fres = f_open(&fil, filename, FA_READ);
    if (fres == FR_OK) {
        f_gets(buffer, bytesToRead, &fil);
        f_close(&fil);
    } else {
        myprintf("f_open error (%d)\r\n", fres);
    }
    return fres;
}

// Function to write to a file
FRESULT SDCard_WriteFile(char *filename, char *data) {
    fres = f_open(&fil, filename, FA_WRITE | FA_OPEN_APPEND | FA_CREATE_ALWAYS);
    if (fres == FR_OK) {
        UINT bytesWritten;
        f_write(&fil, data, strlen(data), &bytesWritten);
        f_close(&fil);
    } else {
        myprintf("f_open error (%d)\r\n", fres);
    }
    return fres;
}

// Function to unmount the SD card
void SDCard_Deinit(void) {
    fres= f_mount(NULL, "", 0);
    if (fres == FR_OK) {
             myprintf("unmounting successfull (%d)\r\n", fres);
}
}

// Helper function to log heart rate to SD card
void logToSDCard(float heartRate) {
    // Open the log file in append mode
    fres = f_open(&fil, "log.txt", FA_OPEN_APPEND | FA_WRITE);
    if (fres == FR_OK) {
        // Prepare log message with time-stamp
        char logBuffer[64];
        int timestamp = HAL_GetTick(); // Get the current time-stamp in ms
        snprintf(logBuffer, sizeof(logBuffer), "Time-stamp: %d s, Heart Rate: %.2f BPM\r\n", (timestamp/1000), heartRate);

        // Write log message to the SD card
        UINT bytesWritten;
        f_write(&fil, logBuffer, strlen(logBuffer), &bytesWritten);
        myprintf("data logged\r\n");

        // Close the log file
        f_close(&fil);
    } else {
        myprintf("Error opening log file: %d\r\n", fres);
    }
}



