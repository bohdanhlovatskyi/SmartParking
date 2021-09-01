#include <project.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#define dbgprintf(...) sprintf((char *)buff,__VA_ARGS__); UART_PutString((const char *)buff)
#define SETUP_DEBUG false

// just some temporary buffers to hold formatting print strings
char buff[128];
char msg[20];

volatile uint8_t NODE_ID = 0;

// allocates one row of flash and initialises it with zeroes
const uint8_t flash_row[CY_FLASH_SIZEOF_ROW] CY_ALIGN(CY_FLASH_SIZEOF_ROW) = {0};
static uint8_t config[CY_FLASH_SIZEOF_ROW] = {0};
volatile bool setup_verbose = false;
int flash_row_num = 0;
 
void sys_init();
void process_setup(void);
void write_id_to_flash(uint8_t node_id);
uint8_t get_config_id(uint8_t *array, int size);
void dump(uint8_t *array, int size);
CY_ISR_PROTO(Setup_Handler);

 
int main(void) {
    CyGlobalIntEnable;
    
    sys_init();
    setup_isr_StartEx(Setup_Handler);
    
    
    flash_row_num = ((int)flash_row - CY_FLASH_BASE) / CY_FLASH_SIZEOF_ROW;

    
    UART_PutString("Setup Test Project\r\n"); // print test_string
    CySysPmSleep();

    for(;;) // endless loop
    {
        if(setup_verbose) { // make sure data is non-zero
            process_setup();
            CySysPmSleep();
        }
    }
}

CY_ISR (Setup_Handler)
{
    setup_verbose = true;
    SW_ClearInterrupt();   
}


void sys_init (void) {
    UART_Start();
}


uint8_t get_config_id(uint8_t *array, int size) {
   return array[0];
}

void process_setup(void) {
    UART_PutString("Setup Mode\r\n"); // print test_string
    UART_PutString("To setup node id, print 1 in the terminal\r\n"
                   "To exit from the setup mode, print 2\r\n");
            
    // waits for the character from the user
    while(UART_GetRxBufferSize() == 0);
    uint32 rxData = UART_GetChar();
            
    switch (rxData) {
        case '1':
            NODE_ID = get_config_id((uint8_t *)flash_row, CY_FLASH_SIZEOF_ROW);
            sprintf(msg, "Set up the node id. Old value: %d. Please enter a new value\r\n", NODE_ID);
            UART_PutString(msg);
           
            UART_PutString("If you want to save the old one, please enter 0 to the terminal\r\n");

            if (SETUP_DEBUG) 
                dump((uint8_t *)flash_row, CY_FLASH_SIZEOF_ROW);

            // waits for the character from the user
            while(UART_GetRxBufferSize() == 0);
            char rxData = UART_GetChar();
                    
            if (rxData != '0') {
                // gets the new id from the user via UART
                NODE_ID = atoi((const char *)&rxData);
                write_id_to_flash(NODE_ID);
            }
            
            break;
        case '2':
            break;
        default:
            UART_PutString("Invalid command is inputed\r\n");
            break;
    }
            
    UART_PutString("Exiting from the setup mode\r\n");

    // clear the irq variable
    setup_verbose = 0;
}

void write_id_to_flash(uint8_t node_id) {
    config[0] = node_id; // writes it to the config array that will be
                         // written to the flash
    uint32 rval = CySysFlashWriteRow(flash_row_num, config);                                    
    
    // checks, how the writing to FLASH went
    if(rval == CY_SYS_FLASH_SUCCESS )
    {
        sprintf(msg, "Node's id is successfully changed. Now it is: %i\r\n", NODE_ID);
        UART_PutString(msg);
        if (SETUP_DEBUG)
            dump((uint8_t *)flash_row, CY_FLASH_SIZEOF_ROW);
        return;
    }
    else if (rval == CY_SYS_FLASH_INVALID_ADDR)
    {
        UART_PutString("Flash Write Failed: Invalid Address\r\n");
    }
    else if (rval == CY_SYS_FLASH_PROTECTED)
    {
        UART_PutString("Flash Write Failed: Flash Protected\r\n");
    }
    else {
        UART_PutString("Flash Write Failed: Unknown\r\n");                                     
    }
    UART_PutString("Try one more time to setup the node\r\n");
}


// This function dumps a block of memory into rows of hex ...16 bytes long in hex
void dump(uint8_t *array, int size)
{
    int i,j;
    j=0;
    for(i=0;i<size;i++)
    {
        // dbgprintf("%2X (%i)",array[i], i);
        dbgprintf("%2X  ", array[i]);
        j=j+1;
        if(j==16) // 16 values per line
        {
            j = 0;
            dbgprintf("\r\n");
        }
    }
    UART_PutString("\r\n");
}
