/*
 * 00_Basic_Rx
 * 
 * The nrf24 is configured as Rx, it will print the received data via UART.
 */

#include "project.h"
#include "main.h"
#include <stdbool.h>
#include <stdio.h>

#define NUM(a) sizeof(a) / sizeof(a[0])

#define MOCK_DISPLAY false
#define IS_DEBUG false

// parking lot info
#define NUM_OF_DEVICES 4
#define DISPLAY_NUM_OF_DEVICES 3
#define MESSAGE_LENGTH 128
uint8_t taken[NUM_OF_DEVICES] = {0};
uint8_t devices_to_display[DISPLAY_NUM_OF_DEVICES];
extern uint16 pos;
char msg_to_display[MESSAGE_LENGTH];
uint8_t free_devices = NUM_OF_DEVICES;

volatile bool irq_flag = false;
volatile bool data_ready = false;

// here we will store the received data
int data[5];
char msg[128];

void process_taken(void);
CY_ISR_PROTO(IRQ_Handler);

int main(void)
{
    const uint8_t RX_ADDR[5]= {'R', 'x', 'A', 'A', 'A'};
    char msg[128];
    
    // Set the Handler for the IRQ interrupt
    isr_IRQ_StartEx(IRQ_Handler);
    
    StandardDisplayInit();
    
    CyGlobalIntEnable;
    
    UART_Start();
    UART_PutChar(0x0C);
    UART_PutString("Data from Mag, Rx\r\n");
    
    nRF24_start();
    nRF24_set_channel(5);
    nRF24_set_rx_pipe_address(NRF_ADDR_PIPE0, RX_ADDR, 5);
    nRF24_start_listening();
    
    sprintf(msg_to_display, "FREE: %i/%i ^^", free_devices, NUM_OF_DEVICES);
    DisplayMessage(msg_to_display, NUM(msg_to_display));
    
    while (1) {
        if (IS_DEBUG)
            UART_PutString("Waiting for data...\r\n");
     

        CyGlobalIntDisable;
        // Get and clear the flag that caused the IRQ interrupt,
        nrf_irq flag = nRF24_get_irq_flag();
        if (flag == NRF_RX_DR_IRQ) {
            data_ready = true;
        }
        nRF24_clear_irq_flag(flag);
        CyGlobalIntEnable;
        
        if (data_ready) {
            
            // RED_LED_Write(~RED_LED_Read());
            
            // get the data from the transmitter
            nRF24_get_rx_payload((uint8_t *) data, sizeof(data));
            // send data via UART
            if (IS_DEBUG) {
                UART_PutString("Received: ");
                sprintf(msg, "Device: %i; Readings: %i %i %i; Taken: %i\r\n",
                    data[0], data[1], data[2], data[3], data[4]
                );
            } else {
                sprintf(msg, "%i;%i;%i;%i;%i\r\n", data[0], data[1], data[2], data[3], data[4]);
            }
            UART_PutString(msg);
            
            if (MOCK_DISPLAY) { 
                free_devices = 3;
                sprintf(msg_to_display, "FREE: %i/%i ^^", free_devices, NUM_OF_DEVICES);
            } else {
                if (data[0] - 1 < NUM_OF_DEVICES) {
     
                    taken[data[0] - 1] = data[4];
                    process_taken();
                    sprintf(msg_to_display, "FREE: %i/%i: %i %i %i",
                        free_devices, NUM_OF_DEVICES, devices_to_display[0],
                        devices_to_display[1], devices_to_display[2]);
                }
            }
            data_ready = false;
        }
        
        if (pos == NUM(msg_to_display)) {
            DisplayMessage(msg_to_display, NUM(msg_to_display));
        }
    }
}


void process_taken(void) {
    int taken_places = 0;
    int j = 0;
    
    for (int i = 0; i < NUM_OF_DEVICES; i++)
        if (taken[i] != 0)
           taken_places += 1;
        else
            if (j < DISPLAY_NUM_OF_DEVICES)
                devices_to_display[j++] = i + 1; // as the indexes of nodes start from 1
    
    free_devices = NUM_OF_DEVICES - taken_places;
}

CY_ISR(IRQ_Handler)
{
    IRQ_ClearInterrupt();
}

/* [] END OF FILE */
