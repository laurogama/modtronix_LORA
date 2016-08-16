// This is a very simple demo program for the Modtronix NZ32-Sxx range of boards, like the
// NZ32-SC151, NZ32-SB072, NZ32-SE411.... It blinks the system LED. If the system button
// is pressed, the LED stays on permanently. Additionally it outputs a "Hello World" message
// to the default serial port. Available via USB Com port when using PGM-NUCLEO programmer.
//
// This demo contains the full mbed API, enabling very easy software development. For detail, see:
// https://developer.mbed.org/handbook/Homepage
//
// For details on the NZ32-SC151 board, see:
// http://wiki.modtronix.com/doku.php?id=products:nz-stm32:nz32-sc151
//
// This demo can be used with the free CoIDE(from coocox) or SW4STM32(System Workbench for STM32)
// IDEs. Both are free, and use the free GNU ARM GCC C/C++ compilers.
//
// For details how to build this project, debug and program the target board using CoIDE, see:
// http://wiki.modtronix.com/doku.php?id=tutorials:coide-with-nz32-boards
//
// For details how to build this project, debug and program the target board using SW4STM32, see:
// http://wiki.modtronix.com/doku.php?id=tutorials:sw4stm32-with-nz32-boards
#include "mbed.h"
#include "modtronix_config.h"
#include "sx1276Regs-LoRa.h"
#include "sx1276-inAir.h"

void OnTxDone(void);
void OnTxTimeout(void);
void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr);
void OnRxTimeout(void);
void OnRxError(void);

Serial pc(USBTX, USBRX); //Use default TX and RX. Available via USB Com port when using PGM-NUCLEO programmer
SX1276inAir radio(OnTxDone, OnTxTimeout, OnRxDone, OnRxTimeout, OnRxError, NULL,
		NULL);
DigitalOut led(LED1);

uint8_t Buffer[BUFFER_SIZE];
uint16_t BufferSize = BUFFER_SIZE;
int16_t LoRaRssi;
int8_t LoRaSNR;
volatile RadioState State = LOWPOWER;

const uint8_t PingMsg[] = "PING";
const uint8_t PongMsg[] = "PONG";

void OnTxDone(void) {
	radio.Sleep();
	State = TX_DONE;

#if DEBUG == 1
	pc.printf("OnTxDone\r\n");
#endif
}

void OnTxTimeout(void) {
	radio.Sleep();
	State = TX_TIMEOUT;

#if DEBUG == 1
	pc.printf("OnTxTimeout\r\n");
#endif
}

void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr) {
	radio.Sleep();
	BufferSize = size;
	memcpy(Buffer, payload, BufferSize);
	LoRaRssi = rssi;
	LoRaSNR = snr;
	State = RX_DONE;

#if DEBUG == 1
	pc.printf("OnRxDone\r\n");
#endif
}

void OnRxTimeout(void) {
	radio.Sleep();
	Buffer[BufferSize] = 0;
	State = RX_TIMEOUT;

#if DEBUG == 1
	pc.printf("OnRxTimeout\r\n");
#endif
}

void OnRxError(void) {
	radio.Sleep();
	State = RX_ERROR;

#if DEBUG == 1
	pc.printf("OnRxError\r\n");
#endif
}
DigitalOut myled(LED1);
DigitalIn myButton(USER_BUTTON);

int main() {
	wait_ms(500); // start delay

	// configure radio
	radio.SetBoardType(BOARD_INAIR9); // the correct hardware for our own board

	led = 0;
	while (radio.Read(REG_VERSION) == 0x00) {
		pc.printf("Trying to connect to radio device\r\n");
		wait_ms(200);
	}
	led = 1;
	// set radio frequency
	radio.SetChannel(RF_FREQUENCY);

	// setup the modern
	radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
			LORA_SPREADING_FACTOR, LORA_CODINGRATE, LORA_PREAMBLE_LENGTH,
			LORA_FIX_LENGTH_PAYLOAD_ON, LORA_CRC_ENABLED, LORA_FHSS_ENABLED,
			LORA_NB_SYMB_HOP, LORA_IQ_INVERSION_ON, TX_TIMEOUT_VALUE);
	radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
			LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH, LORA_SYMBOL_TIMEOUT,
			LORA_FIX_LENGTH_PAYLOAD_ON, 0, LORA_CRC_ENABLED, LORA_FHSS_ENABLED,
			LORA_NB_SYMB_HOP, LORA_IQ_INVERSION_ON, true);

	uint8_t i;
	bool isMaster = true;

	radio.Rx(RX_TIMEOUT_VALUE);

	while (1) {
		// Check for connection to radio module
		while (radio.Read(REG_VERSION) == 0x00) {
			led = !led;
			pc.printf("Reconnecting...\r\n");
			wait_ms(200);
		}
		led = 1;

		switch (State) {
		case RX_DONE:
			if (isMaster) {
				if (BufferSize > 0) {
					if (strncmp((const char *) Buffer, (const char *) PongMsg,
							4) == 0) {
						pc.printf("...Pong\r\n");
						// send next ping frame
						strcpy((char *) Buffer, (char *) PingMsg);
						// fill the buffer with numbers for the payload
						for (i = 4; i < BufferSize; i++) {
							Buffer[i] = i - 4;
						}
						wait_ms(10);
						radio.Send(Buffer, BufferSize);
					} else if (strncmp((const char*) Buffer,
							(const char*) PingMsg, 4) == 0) {
						// A master already exists then become a slave
						pc.printf("...Ping\r\n");
						isMaster = false;
						// send the next pong frame
						strcpy((char*) Buffer, (char*) PongMsg);
						// We fill the buffer with numbers for the payload
						for (i = 4; i < BufferSize; i++) {
							Buffer[i] = i - 4;
						}
						wait_ms(10);
						radio.Send(Buffer, BufferSize);
					} else {
						isMaster = true;
						radio.Rx(RX_TIMEOUT_VALUE);
					}
				}
			} else {
				if (BufferSize > 0) {
					if (strncmp((const char*) Buffer, (const char*) PingMsg, 4)
							== 0) {
						pc.printf("...Ping\r\n");
						// Send the reply to the PING string
						strcpy((char*) Buffer, (char*) PongMsg);
						// We fill the buffer with numbers for the payload
						for (i = 4; i < BufferSize; i++) {
							Buffer[i] = i - 4;
						}
						wait_ms(10);
						radio.Send(Buffer, BufferSize);
					} else // valid reception but not a PING as expected
					{ // Set device as master and start again
						isMaster = true;
						radio.Rx(RX_TIMEOUT_VALUE);
					}
				}
			}
			State = LOWPOWER;
			break;

		case TX_DONE:
			if (isMaster) {
				pc.printf("Ping...\r\n");
			} else {
				pc.printf("Pong...\r\n");
			}
			radio.Rx(RX_TIMEOUT_VALUE);
			State = LOWPOWER;
			break;

		case RX_TIMEOUT:
			if (isMaster == true) {
				// Send the next PING frame
				strcpy((char*) Buffer, (char*) PingMsg);
				for (i = 4; i < BufferSize; i++) {
					Buffer[i] = i - 4;
				}
				wait_ms(10);
				radio.Send(Buffer, BufferSize);
			} else {
				radio.Rx(RX_TIMEOUT_VALUE);
			}
			State = LOWPOWER;
			break;

		case TX_TIMEOUT:
			radio.Rx(RX_TIMEOUT_VALUE);
			State = LOWPOWER;
			break;

		case RX_ERROR:
			// We have received a Packet with a CRC error, send reply as if packet was correct
			if (isMaster == true) {
				// Send the next PING frame
				strcpy((char*) Buffer, (char*) PingMsg);
				for (i = 4; i < BufferSize; i++) {
					Buffer[i] = i - 4;
				}
				wait_ms(10);
				radio.Send(Buffer, BufferSize);
			} else {
				// Send the next PONG frame
				strcpy((char*) Buffer, (char*) PongMsg);
				for (i = 4; i < BufferSize; i++) {
					Buffer[i] = i - 4;
				}
				wait_ms(10);
				radio.Send(Buffer, BufferSize);
			}
			State = LOWPOWER;
			break;

		case LOWPOWER:
			led = !led;
			wait_ms(500);
			break;

		default:
			State = LOWPOWER;
			break;
		}
	}
}
