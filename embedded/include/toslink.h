/** 
**************************************************************
* @file embedded/lib/toslink.h
* @author Shane Baptist - 47484164
* @date 17/05/2025
* @brief Bidirectional communication protocol driver
*************************************************************** 
*/

#ifndef TOSLINK
#define TOSLINK

// Size of the payload to be transmitted
#define PAYLOAD_SIZE    32

// Size of the payload being received
#define INCOMING_PAYLOAD_SIZE 20

// COMMUNICATION BYTES
#define SENSOR_HEADER   0x77
#define CONTROL_HEADER  0x99

#define SENSOR_STOP		0x55
#define CONTROL_STOP    0xFD

// Thread stack size
#define TX_STACK_SIZE 2048
#define RX_STACK_SIZE 2048

// Thread priority
#define TX_THREAD_PRIORITY 5
#define RX_THREAD_PRIORITY 5

// delay between packets in microseconds
#define TX_SAMPLING_RATE 50

// delay between transmissions in microseconds
#define TX_DELAY      50

// Time until reinitiate comms
#define RX_TIMEOUT  5000

// tx buffer mutex
extern struct k_mutex tx_mutex;
extern struct k_mutex rx_mutex;

// Global message queues for high and low alarm states
extern struct k_msgq high_msgq;
extern struct k_msgq low_msgq;

// Global range message queue for range switching
extern struct k_msgq range_msgq;

// Global tx buffer for toslink transmission
extern uint8_t txBuf[PAYLOAD_SIZE];

// Function prototypes
void uart_callback(const struct device *dev, void *user_data);
void print_uart(uint8_t* buf);
void tx_thread();
void rx_thread();

#endif