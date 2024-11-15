# COL788A5_Embedded

1. I have adopted pairwise file configuation for code generation in STM32, hence you would find differnt source file for i2c, uart, freertos and not in main, This is to just improve the readability of code
2. I have compiled all the issues faced during assignment in COL788A5.pdf, there were so many bugs I noticed while doing it and I keep noting all the issues. 
However the major problem I faced was, the FREERTOS tasks getting halted in deadlock. This I could not resolved but I have shared my fincdings with the TA. 
3. In addition to that, I have included STM32Setup.jpeg. The handwritten connection diagram I have uploaded in seprate file circuit.jpeg
4. The techniques I tried in FreeRTOS while doing the assignment were co-operative schedulling, premptive schedulling, diffrent and equal priority levels , semaphores for synchronization, queues for communication between tasks (threads). In all the efforts, I could still face deadlock situation.
5. In interrupt, I oberserved while we were printing the normal messages , without temperature values, the interrupt worked propely however as soon as I generated the uart message with float value of temperature, it was not getting genrated , infact Interrupt handler was not iniitaiting calls itself. This was a big which I later tried fixing with printing the value after typecasting it in INT from FLOAT. However it still did not work, while writing directly to SD card was working fine.
