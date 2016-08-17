#ifndef _MODTRONIX_CONFIG_H
#define _MODTRONIX_CONFIG_H
#define DEBUG                           1

#define RF_FREQUENCY                    868000000           // 868KHz
#define TX_OUTPUT_POWER                 14                  // 14 dBm for inAir9
#define LORA_BANDWIDTH                  7                   // 0: 7.8 kHz,  1: 10.4 kHz, 2: 15.6kHz, 3: 20.8kHz,
// 4: 31.25kHz, 5: 41.7 kHz, 6: 62.5 kHz,
// 7: 125 kHz,  8: 250 kHz,  9: 500 kHz
#define LORA_SPREADING_FACTOR           12                  // SF7..SF12
#define LORA_CODINGRATE                 1                   // 1=4/5, 2=4/6, 3=4/7, 4=4/8
#define LORA_PREAMBLE_LENGTH            8                   // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT             5                   // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON      false
#define LORA_FHSS_ENABLED               false
#define LORA_NB_SYMB_HOP                4
#define LORA_IQ_INVERSION_ON            false
#define LORA_CRC_ENABLED                true

#define TX_TIMEOUT_VALUE                2000000
#define RX_TIMEOUT_VALUE                3500000     // in us
#define BUFFER_SIZE                     32          // Define the payload size here
#endif
