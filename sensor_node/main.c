/*
 * End-Node code
 * 
 * Handles reading the data from the magnetometer and
 * nRF24l01 transmission via the ACK to the gateway.
 */

#include "project.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define dbgprintf(...) sprintf((char *)buff,__VA_ARGS__); UART_PutString((const char *)buff)
#define IS_DEBUG true
#define MOCK false

// used to determine whether the place is taken or not
#define THRESHOLD 3
#define TAKEN_ITERS_THRESHOLD 5
#define MEASUREMENTS_TO_DETERMINE_MEAN 5

typedef struct {
    int x, y, z;
} MagData;

// variables for occupacy determination
uint8_t is_taken = 0;
uint8_t taken_iters = 0;
uint16_t cur_mean_magnitude = 0;
uint16_t mean_magnitude = 0;
uint32_t iterations = 0;

// I2C address of magnetometer
#define HMC5883L 0x1E
// magnetometer registers' addresses and its gain
#define SENSORS_GAUSS_TO_MICROTESLA (100)
#define HMC5883_REGISTER_MAG_MR_REG_M 0x02
#define HMC5883_REGISTER_MAG_OUT_X_H_M 0x03
#define _hmc5883_Gauss_LSB_XY 1100.0F
#define _hmc5883_Gauss_LSB_Z 980.0F

// IRQ vars
volatile bool irq_flag = false;
volatile bool setup_verbose = false;

// allocates one row of flash and initialises it with zeroes
static uint8_t config[CY_SFLASH_SIZEOF_USERROW] = {0};
uint8_t NODE_ID;

// just some temporary buffers to hold formatting print strings
char buff[128];
char msg[128];

const uint8_t gateway[5]= {'R', 'x', 'A', 'A', 'A'};
int data[5] = {0};

// the IRQ pin triggered an interrupt
CY_ISR_PROTO(IRQ_Handler);
CY_ISR_PROTO(Timer_INT_Handler);
CY_ISR_PROTO(Setup_Handler);

void process_setup(void);
void write_id_to_flash(uint8_t node_id);
uint8_t get_config_id(uint8_t *sflash);
void dump(uint8_t *array, int size);
void print_status(void);
void init_HMC5883();
int Read_Mag(int *magData);
uint16_t get_magnitude(MagData magData);
void process_mag_data(int *mag_data);

int main(void)
{

    CyGlobalIntEnable;
    
    isr_IRQ_StartEx(IRQ_Handler);
    setup_isr_StartEx(Setup_Handler);
    Timer_INT_StartEx(Timer_INT_Handler);
 
    if (!MOCK) {
        I2C_Start();
        init_HMC5883();
    }
    
    UART_Start();
    UART_PutString("\r\nTx\r\n");
    
    nRF24_start();
    nRF24_set_channel(5);
    nRF24_set_rx_pipe_address(NRF_ADDR_PIPE0, gateway, 5);
    // set tx pipe address to match the receiver address
    nRF24_set_tx_address(gateway, 5);

    // reads the node's id that is stored in the flash. If it is
    // the first time the node is powered, the id will be equal to 0
    // which won't be processed by the main node
    NODE_ID = get_config_id((uint8_t *)CY_SFLASH_USERBASE);

    // Set the interrupts handlers

    CySysWdtEnable(CY_SYS_WDT_COUNTER0);
    
    for (;;) {
        if (setup_verbose == true) {
            process_setup();
            UART_PutString("Setup is finished. Reseting...\r\n\r\n");
            // clear the irq variable
            setup_verbose = false;
            CySoftwareReset();
        }

        // nRF24_wakeup();
        // nRF24_init();
        uint8 data_ready;
        if (!MOCK) {
            data_ready = Read_Mag(data); // this function implicitly writes the id to the payload
        } else {
            data_ready = 0;
            data[0] = NODE_ID;
            data[1] = 3;
            data[2] = 3;
            data[3] = 3;
        }
 
        // if data is not ready, restart the loop
        if (data_ready != 0) {
            CySysPmDeepSleep();
            continue;
        }
        
        if (!MOCK)
            process_mag_data(data);
        data[4] = is_taken;
        
        if (IS_DEBUG) {
            sprintf(msg, "Sending data | Device: %i; Readings: %i %i %i; | Is taken: %i\r\n", \
                    data[0], data[1], data[2], data[3], is_taken);
            UART_PutString(msg);
        }
        nRF24_transmit((const uint8_t *) data, sizeof(data));
        memset(data, 0, sizeof(data));
        uint8 lost_packets = nRF24_get_lost_packets_count();
        uint8 retransmissions = nRF24_get_retransmissions_count();
        
        if (IS_DEBUG) {
            char msg[18];
            sprintf(msg, "LP: %i, RT: %ii\r\n", lost_packets, retransmissions);
            UART_PutString(msg);
        }

        
        // print_status();
        while(false == irq_flag);
            
        // Get and clear the flag that caused the IRQ interrupt,
        // in this project the only IRQ cause is the caused by
        // transmitting data (NRF_STATUS_TX_DS_MASK) or timeout
        nrf_irq flag = nRF24_get_irq_flag();

        switch (flag) {
        case NRF_TX_DS_IRQ:
            // turn on the Green LED if the transmit was successfull
            LED_Write(0);
            break;
        case NRF_MAX_RT_IRQ:
            // turn off the Green LED if the transmit was not successfull
            LED_Write(1);
            break;
        default:
            break;
        }
        
        nRF24_clear_irq_flag(flag);
        
        irq_flag = false;

        // nRF24_flush_tx();
        // nRF24_sleep();
        CySysPmDeepSleep();  
        
        if (IS_DEBUG)
            UART_PutString("Woke up!\r\n"); 
    }
}


/* Processes the readings from the magnetometer and
   determines whether the place is taken or not.
   
   Args: uint8_t *mag_data - pointer to the array of
         three elements that stores the magnetometer readings

         uint8_t *is_taken - verbose that will be checked,
         when the delivery of packets will be conducted.
         It is of type uint8_t to make the nRF transmission
         slighlty more convenient */
void process_mag_data(int *mag_data) {
    // initialisation of magdata struct to make the
    // proccessment of it slighlty more readable
    MagData mag = {mag_data[1], mag_data[2], mag_data[3]};
    uint16_t magnitude = 0;
    int change = 0;
    char debug_msg[128];
    
    magnitude = get_magnitude(mag);
    
    if (IS_DEBUG) {
        sprintf(debug_msg, "Current magnitude: %u\r\n", magnitude);
        UART_PutString(debug_msg);
    }
    
    // this block determines the mean magnitude each time
    // after reset
    if (iterations < MEASUREMENTS_TO_DETERMINE_MEAN) {
        if (iterations == 0) {
            cur_mean_magnitude = magnitude;
        } else {
            cur_mean_magnitude += magnitude;
            cur_mean_magnitude = cur_mean_magnitude / 2;
        }
        
        if (IS_DEBUG) {
            sprintf(debug_msg, "Current mean magnitude: %u ; Calibrating...\r\n", cur_mean_magnitude);
            UART_PutString(debug_msg);
        }
        
        iterations++;

    // sets the mean magnitue variable after the calibration
    } else if (mean_magnitude == 0 && iterations >= MEASUREMENTS_TO_DETERMINE_MEAN) {
        mean_magnitude = cur_mean_magnitude;

        if (IS_DEBUG) {
            sprintf(debug_msg, "Mean magnitude: %u\r\n", mean_magnitude);
            UART_PutString(debug_msg);
        }
    } else {
    
        change = abs(mean_magnitude - magnitude);
   
        if (IS_DEBUG) {
            sprintf(debug_msg, "Current state: %u\r\n\r\n", change);
            UART_PutString(debug_msg);
        }
    }

    if (change > THRESHOLD) {  
        if (taken_iters < 10) {
           taken_iters += 1;   
        }
    } else {
        // change is below threshold, so reduce the number
        // of iteration that the system might be in the 
        // is_taken state
        if (taken_iters != 0) {
            taken_iters -= 1;
        }
    }

    // if the system seems to be taken for a certain time,
    // determined by the TAKEN_ITERS_THRESHOLD macros,
    // then put it in is_taken state. If it won't be
    // taken anymore, it will require some time to
    // went back into is not taken state, as it will
    // wait for the taken_iters to stabilise.
    is_taken = 0;
    if (taken_iters > TAKEN_ITERS_THRESHOLD)
        is_taken = 1;

}


/* Counts the magnitude of the magnetic
   induction vector. Used to determine
   whether the parking lot is empty or not */
uint16_t get_magnitude(MagData magData) {
    double squared_sum = 0;
    
    squared_sum = pow(magData.x, 2) + \
                  pow(magData.y, 2) + \
                  pow(magData.z, 2);
    
    // sqrt from squared sum is up to 442
    return (uint16_t)sqrt(squared_sum); 
}


void print_status(void)
{
    char status_str[20];
    uint8_t status = nRF24_get_status();
    sprintf(status_str, "0x%02X\r\n", status);
    UART_PutString(status_str);
}


/* Initialising the HMC5883 magnetometer.
   ! Use carefully as it has not got yet any
   success write checks for the I2C commands */
void init_HMC5883() {
    I2C_I2CMasterSendStart(HMC5883L, I2C_I2C_WRITE_XFER_MODE, I2C_I2C_MASTER_TIMEOUT);
    I2C_I2CMasterWriteByte(HMC5883_REGISTER_MAG_MR_REG_M, I2C_I2C_MASTER_TIMEOUT);
    // required bits in the register
    I2C_I2CMasterWriteByte(0x00u, I2C_I2C_MASTER_TIMEOUT);
    I2C_I2CMasterSendStop(I2C_I2C_MASTER_TIMEOUT);
}


/* Gets the magnetometers readings.

   ARGS: uint8_t *magData: 4 elements array that will store
         the magnetometers' readings and the node id

   Side Effects: gets the globally available node id and puts
        it into the packet with magnetometer readings

   Returns: uint8 status verbose. It is 0 if the function call is
        successful, -1 otherwise. */
int Read_Mag(int *magData) {
    uint8 data[6];
    uint32 err_status = 0;
    
    I2C_I2CMasterSendStart(HMC5883L, I2C_I2C_WRITE_XFER_MODE, I2C_I2C_MASTER_TIMEOUT);
    if ((err_status = I2C_I2CMasterWriteByte(HMC5883_REGISTER_MAG_OUT_X_H_M, I2C_I2C_MASTER_TIMEOUT))\
                    != I2C_I2C_MSTR_NO_ERROR) {
        if (IS_DEBUG)
            UART_PutString("Read mag failed\r\n");  
        RED_LED_Write(0);
        CyDelay(1000);
        RED_LED_Write(1);
        return -1;
    }
    I2C_I2CMasterSendStop(50);
    
    I2C_I2CMasterSendStart(HMC5883L, I2C_I2C_READ_XFER_MODE, I2C_I2C_MASTER_TIMEOUT);
    
    int i = 0;
    for (; i < 5; i++) {
        I2C_I2CMasterReadByte(I2C_I2C_ACK_DATA, &data[i], I2C_I2C_MASTER_TIMEOUT); 
    }
    I2C_I2CMasterReadByte(I2C_I2C_NAK_DATA, &data[++i], I2C_I2C_MASTER_TIMEOUT);
    I2C_I2CMasterSendStop(50);
    
    int x = (data[1] | (data[0] << 8));
    int y = (data[5] | (data[4] << 8));
    int z = (data[3] | (data[2] << 8));

    // prepares the data packet
    magData[0] = NODE_ID;
    magData[1] = x / _hmc5883_Gauss_LSB_XY * SENSORS_GAUSS_TO_MICROTESLA;
    magData[2] = y / _hmc5883_Gauss_LSB_XY * SENSORS_GAUSS_TO_MICROTESLA;
    magData[3] = z / _hmc5883_Gauss_LSB_Z * SENSORS_GAUSS_TO_MICROTESLA;

    return 0;
}


/* High-level API to get the id from the
SFLASH first memory chank, that is given as pointer to it
(Pointer passed usually should be the pre-defined macross)
(REWRITE)*/
uint8_t get_config_id(uint8_t *sflash) {
   return sflash[0];
}


/* Terminal UI for the setup process. 
   Allows the user to set the node id via UART. 

   Possible bug: though node id is set here, it has little effect,
                 as the system is soft reset after the setup. Then
                 the node id is set up in the main function.

   ! if SETUP DEBUG verbose is set, will dump the SFLASH config array
   into the terminal */
void process_setup(void) {
    UART_PutString("Setup Mode\r\n"); // print test_string
    UART_PutString("To setup node id, print 1 in the terminal\r\n"
                   "To exit from the setup mode, print 2\r\n\r\n");
            
    // waits for the character from the user
    while(UART_GetRxBufferSize() == 0);
    uint32 rxData = UART_GetChar();
            
    switch (rxData) {
        case '1':
            // NODE_ID = get_config_id((uint8_t *)flash_row, CY_FLASH_SIZEOF_ROW);
            NODE_ID = get_config_id((uint8_t *)CY_SFLASH_USERBASE);
            sprintf(msg, "Set up the node id. Old value: %d. Please enter a new value\r\n\r\n", NODE_ID);
            UART_PutString(msg);
           
            UART_PutString("If you want to save the old one, please enter 0 to the terminal\r\n");

            // dumps the data from sflash into terminal
            dump((uint8_t *)CY_SFLASH_USERBASE, CY_SFLASH_SIZEOF_USERROW);

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
}


/* Writes the modified config array (row in the SFLASH) into
   the 0 address of available for the user SFLASH memory
   location. 

   ARGS: uint8_t node_id - node id that will be written as the
         first element of the config array

   Returns: None; */
void write_id_to_flash(uint8_t node_id) {
    config[0] = node_id; // writes it to the config array that will be
                         // written to the flash
    
    // wrotes to the first row of the user flash
    uint32 rval = CySysSFlashWriteUserRow(0, config);     
    
    // checks, how the writing to FLASH went
    if(rval == CY_SYS_FLASH_SUCCESS )
    {
        sprintf(msg, "Node's id is successfully changed. Now it is: %i\r\n\r\n", NODE_ID);
        UART_PutString(msg);
        dump((uint8_t *)CY_SFLASH_USERBASE, CY_FLASH_SIZEOF_ROW);
        
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


/* This function dumps a block of memory into rows of hex
   (it is 16 bytes long in hex)
   Used for debug puproses during the setup process
   FLASH read and write)

   ARGS: uint8_t *array - pointer to the memory address
         in SFLASH
         int size - size of an array to dump on the screen
         via UART

   Returns: None;

*/
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


/* isr for nRF24l01 data transmission. 
   Sets the irq flag true, when the
   acknowledgement packet arrives from the
   master node (gateway) */
CY_ISR(IRQ_Handler)
{
    irq_flag = true;
    IRQ_ClearInterrupt();
}


/* Handles periodical interrupt from the timer */
CY_ISR(Timer_INT_Handler)
{
    CySysWdtClearInterrupt(CY_SYS_WDT_COUNTER0_INT);
}


/* Inerrupt handler for the sw2 switch
   (2.7 pin on the Cy8CKIT042-BLE-A)
   Sets the verbose true, it will be checked
   in the main loop of the program */
CY_ISR (Setup_Handler)
{
    setup_verbose = true;
    SW_ClearInterrupt();   
}

/* [] END OF FILE */
