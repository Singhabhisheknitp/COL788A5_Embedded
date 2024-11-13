#ifndef SDCARD_H
#define SDCARD_H

#include "fatfs.h"


// Initialize and mount the SD card
FRESULT SDCard_Init(void);

// Get SD card statistics
void SDCard_GetStats(void);

// Read from a file
FRESULT SDCard_ReadFile(char *filename, char *buffer, UINT bytesToRead);

// Write to a file
FRESULT SDCard_WriteFile(char *filename, char *data);

// Unmount the SD card
void SDCard_Deinit(void);

// Helper function to log heart rate to SD card
void logToSDCard(float heartRate);


#endif // SDCARD_H
