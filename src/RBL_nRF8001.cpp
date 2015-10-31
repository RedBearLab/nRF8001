
/*

Copyright (c) 2012, 2013 RedBearLab

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#include <avr/pgmspace.h>

#include "RBL_nRF8001.h"

#ifdef SERVICES_PIPE_TYPE_MAPPING_CONTENT
    static services_pipe_type_mapping_t
        services_pipe_type_mapping[NUMBER_OF_PIPES] = SERVICES_PIPE_TYPE_MAPPING_CONTENT;
#else
    #define NUMBER_OF_PIPES 0
    static services_pipe_type_mapping_t * services_pipe_type_mapping = NULL;
#endif

/* Store the setup for the nRF8001 in the flash of the AVR to save on RAM */
#if ARDUINO < 150
static /*const*/ hal_aci_data_t setup_msgs[NB_SETUP_MESSAGES] PROGMEM = SETUP_MESSAGES_CONTENT;
#else
static const hal_aci_data_t setup_msgs[NB_SETUP_MESSAGES] PROGMEM = SETUP_MESSAGES_CONTENT;
#endif

#if defined(BLEND_MICRO)
static char device_name[11] = "BlendMicro";
#elif defined(BLEND)
static char device_name[11] = "Blend     ";
#else
static char device_name[11] = "BLE Shield";
#endif
	
static uint8_t  			  bd_addr_own[BTLE_DEVICE_ADDRESS_SIZE];
static aci_bd_addr_type_t  bd_addr_type;  
static uint8_t				  addr_get = 0;

static uint16_t Adv_Timeout = 0;	// Advertising all the time
static uint16_t Adv_Interval = 0x0050; /* advertising interval 50ms

/*aci_struct that will contain :
total initial credits
current credit
current state of the aci (setup/standby/active/sleep)
open remote pipe pending
close remote pipe pending
Current pipe available bitmap
Current pipe closed bitmap
Current connection interval, slave latency and link supervision timeout
Current State of the the GATT client (Service Discovery)
Status of the bond (R) Peer address*/
static struct aci_state_t aci_state;

/*Temporary buffers for sending ACI commands*/
static hal_aci_evt_t  aci_data;
//static hal_aci_data_t aci_cmd;

/*Timing change state variable*/
static bool timing_change_done = false;

/*Initialize the radio_ack. This is the ack received for every transmitted packet.*/
//static bool radio_ack_pending = false;

//The maximum size of a packet is 64 bytes.
#define MAX_TX_BUFF 64
static uint8_t tx_buff[MAX_TX_BUFF];
static uint8_t tx_buffer_len = 0;

#define MAX_RX_BUFF 64

static uint8_t rx_buff[MAX_RX_BUFF+1];
static uint8_t rx_buffer_len = 0;
static uint8_t *p_before = &rx_buff[0] ;
static uint8_t *p_back = &rx_buff[0];

static unsigned char is_connected = 0;

uint8_t reqn_pin = DEFAULT_REQN;
uint8_t rdyn_pin = DEFAULT_RDYN;

static unsigned char spi_old;

static void process_events();

/* Define how assert should function in the BLE library */
void __ble_assert(const char *file, uint16_t line)
{
    RBL_LOG("ERROR ");
    RBL_LOG(file);
    RBL_LOG(": ");
    RBL_LOG(line);
    RBL_LOG("\n");
    while(1);
}

void ble_set_pins(uint8_t reqn, uint8_t rdyn)
{
#if defined(BLEND_MICRO)
    return;
#else
	reqn_pin = reqn;
    rdyn_pin = rdyn;
#endif
}

void ble_begin()
{
#if ( !defined(__SAM3X8E__) && !defined(__PIC32MX__) )
    spi_old = SPCR;
    SPI.setBitOrder(LSBFIRST);
	SPI.setClockDivider(SPI_CLOCK_DIV8);
    SPI.setDataMode(SPI_MODE0);
#endif
	
	memset(bd_addr_own, 0x00, BTLE_DEVICE_ADDRESS_SIZE);

     /* Point ACI data structures to the the setup data that the nRFgo studio generated for the nRF8001 */
    if (NULL != services_pipe_type_mapping)
    {
        aci_state.aci_setup_info.services_pipe_type_mapping = &services_pipe_type_mapping[0];
    }
    else
    {
        aci_state.aci_setup_info.services_pipe_type_mapping = NULL;
    }
    aci_state.aci_setup_info.number_of_pipes    = NUMBER_OF_PIPES;
    aci_state.aci_setup_info.setup_msgs         = (hal_aci_data_t*)setup_msgs;
    aci_state.aci_setup_info.num_setup_msgs     = NB_SETUP_MESSAGES;

    /*
    Tell the ACI library, the MCU to nRF8001 pin connections.
    The Active pin is optional and can be marked UNUSED
    */
    aci_state.aci_pins.board_name = REDBEARLAB_SHIELD_V2; //See board.h for details
    aci_state.aci_pins.reqn_pin   = reqn_pin;
    aci_state.aci_pins.rdyn_pin   = rdyn_pin;
    aci_state.aci_pins.mosi_pin   = MOSI;
    aci_state.aci_pins.miso_pin   = MISO;
    aci_state.aci_pins.sck_pin    = SCK;

#if defined(__SAM3X8E__)
    aci_state.aci_pins.spi_clock_divider     = 84;
#else
    aci_state.aci_pins.spi_clock_divider     = SPI_CLOCK_DIV8;
#endif

    aci_state.aci_pins.reset_pin             = UNUSED;
    aci_state.aci_pins.active_pin            = UNUSED;
    aci_state.aci_pins.optional_chip_sel_pin = UNUSED;

    aci_state.aci_pins.interface_is_interrupt	  = false;
    aci_state.aci_pins.interrupt_number			  = 4;//1;

    //We reset the nRF8001 here by toggling the RESET line connected to the nRF8001
    //If the RESET line is not available we call the ACI Radio Reset to soft reset the nRF8001
    //then we initialize the data structures required to setup the nRF8001
    //The second parameter is for turning debug printing on for the ACI Commands and Events so they be printed on the Serial
    lib_aci_init(&aci_state, false);

#if ( !defined(__SAM3X8E__) && !defined(__PIC32MX__) )
#if (ARDUINO < 150)
    SPCR = spi_old;
    SPI.begin();
#endif
#endif
}

static volatile byte ack = 0;

void ble_write(unsigned char data)
{
    if(tx_buffer_len == MAX_TX_BUFF)
    {
            return;
    }
    tx_buff[tx_buffer_len] = data;
    tx_buffer_len++;
}

void ble_write_bytes(unsigned char *data, uint8_t len)
{
    for (int i = 0; i < len; i++)
        ble_write(data[i]);
}

int ble_read()
{
	int data;
	if(rx_buffer_len == 0) return -1;
	if(p_before == &rx_buff[MAX_RX_BUFF])
	{
        p_before = &rx_buff[0];
	}
	data = *p_before;
	p_before ++;
	rx_buffer_len--;
	return data;
}

unsigned char ble_available()
{
	return rx_buffer_len;
}

unsigned char ble_connected()
{
    return is_connected;
}

void ble_set_name(char *name)
{
    unsigned char len=0;

    len = strlen(name);
    if(len > 10)
    {
        RBL_LOG("the new name is too long");
    }
    else
    {
       strcpy(device_name, name);
    }
}

unsigned char ble_busy()
{
    if(digitalRead(reqn_pin) == HIGH)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

void ble_reset(uint8_t reset_pin)
{
	pinMode(reset_pin, OUTPUT);
	digitalWrite(reset_pin, HIGH);
	digitalWrite(reset_pin, LOW);
	digitalWrite(reset_pin, HIGH);
}

void ble_disconnect(void)
{
	lib_aci_disconnect(&aci_state, ACI_REASON_TERMINATE);
}

void ble_get_mac_addr(uint8_t *bd_addr)
{
	if(!addr_get)
	{
		lib_aci_get_address();	// Get device's MAC address and address type
		while(!addr_get)
		{
			process_events();
		}
	}

	memcpy(bd_addr, bd_addr_own, BTLE_DEVICE_ADDRESS_SIZE);
}

static void process_events()
{
    static bool setup_required = false;
    // We enter the if statement only when there is a ACI event available to be processed
    if (lib_aci_event_get(&aci_state, &aci_data))
    {
        aci_evt_t  *aci_evt;
        aci_evt = &aci_data.evt;
        switch(aci_evt->evt_opcode)
        {
            /* As soon as you reset the nRF8001 you will get an ACI Device Started Event */
            case ACI_EVT_DEVICE_STARTED:
                aci_state.data_credit_total = aci_evt->params.device_started.credit_available;
                switch(aci_evt->params.device_started.device_mode)
                {
                    case ACI_DEVICE_SETUP:
                        /* When the device is in the setup mode*/
                        RBL_LOGLN(F("Evt Device Started: Setup"));
                        setup_required = true;
                        break;
                    case ACI_DEVICE_STANDBY:
                        RBL_LOGLN(F("Evt Device Started: Standby"));
                        //Looking for an iPhone by sending radio advertisements
                        //When an iPhone connects to us we will get an ACI_EVT_CONNECTED event from the nRF8001
                        if (aci_evt->params.device_started.hw_error)
                        {
                            delay(20); //Magic number used to make sure the HW error event is handled correctly.
                        }
                        else
                        {
                            lib_aci_set_local_data(&aci_state, PIPE_GAP_DEVICE_NAME_SET , (uint8_t *)&device_name , strlen(device_name));
                            lib_aci_connect(Adv_Timeout/* in seconds */, Adv_Interval /* advertising interval 50ms*/);
                            RBL_LOGLN(F("Advertising started"));
                        }
                        break;
                }
                break; //ACI Device Started Event

            case ACI_EVT_CMD_RSP:
                //If an ACI command response event comes with an error -> stop
                if (ACI_STATUS_SUCCESS != aci_evt->params.cmd_rsp.cmd_status)
                {
                    //ACI ReadDynamicData and ACI WriteDynamicData will have status codes of
                    //TRANSACTION_CONTINUE and TRANSACTION_COMPLETE
                    //all other ACI commands will have status code of ACI_STATUS_SCUCCESS for a successful command
                    RBL_LOG(F("ACI Command "));
                    RBL_LOGLN(aci_evt->params.cmd_rsp.cmd_opcode, HEX);
                    RBL_LOG(F("Evt Cmd respone: Status "));
                    RBL_LOGLN(aci_evt->params.cmd_rsp.cmd_status, HEX);
                }
                if (ACI_CMD_GET_DEVICE_VERSION == aci_evt->params.cmd_rsp.cmd_opcode)
                {
                    //Store the version and configuration information of the nRF8001 in the Hardware Revision String Characteristic
                    lib_aci_set_local_data(&aci_state, PIPE_DEVICE_INFORMATION_HARDWARE_REVISION_STRING_SET,
                    (uint8_t *)&(aci_evt->params.cmd_rsp.params.get_device_version), sizeof(aci_evt_cmd_rsp_params_get_device_version_t));
                }
				else if (ACI_CMD_GET_DEVICE_ADDRESS == aci_evt->params.cmd_rsp.cmd_opcode)
                {
					memcpy(bd_addr_own, aci_evt->params.cmd_rsp.params.get_device_address.bd_addr_own, BTLE_DEVICE_ADDRESS_SIZE);
                    bd_addr_type = aci_evt->params.cmd_rsp.params.get_device_address.bd_addr_type;
					addr_get = 1;
					
					RBL_LOG(F("Device address: "));
					for(uint8_t i=0; i<BTLE_DEVICE_ADDRESS_SIZE-1; i++)
					{
						RBL_LOG(bd_addr_own[i], HEX);
						RBL_LOG(":");
					}
					RBL_LOGLN(bd_addr_own[BTLE_DEVICE_ADDRESS_SIZE-1], HEX);
					RBL_LOG(F("Device address type: "));
					switch(bd_addr_type)
					{
						case ACI_BD_ADDR_TYPE_PUBLIC: RBL_LOGLN(F("Public address")); break;
						case ACI_BD_ADDR_TYPE_RANDOM_STATIC: RBL_LOGLN(F("Random static address")); break;
						case ACI_BD_ADDR_TYPE_RANDOM_PRIVATE_RESOLVABLE: RBL_LOGLN(F("Private resolvable address")); break;
						case ACI_BD_ADDR_TYPE_RANDOM_PRIVATE_UNRESOLVABLE: RBL_LOGLN(F("Private unresolvable address")); break;
						default: RBL_LOGLN(F("Invalid address"));
					}
                }
                break;

            case ACI_EVT_CONNECTED:
                is_connected = 1;
                RBL_LOGLN(F("Evt Connected"));
                timing_change_done = false;
                aci_state.data_credit_available = aci_state.data_credit_total;
                /*Get the device version of the nRF8001 and store it in the Hardware Revision String*/
                lib_aci_device_version();
                break;

            case ACI_EVT_PIPE_STATUS:
                RBL_LOGLN(F("Evt Pipe Status"));
                if (lib_aci_is_pipe_available(&aci_state, PIPE_UART_OVER_BTLE_UART_TX_TX) && (false == timing_change_done))
                {
                    lib_aci_change_timing_GAP_PPCP(); // change the timing on the link as specified in the nRFgo studio -> nRF8001 conf. -> GAP.
                                                      // Used to increase or decrease bandwidth
                    timing_change_done = true;
                }
                break;

            case ACI_EVT_TIMING:
                RBL_LOGLN(F("Evt link connection interval changed"));
                break;

            case ACI_EVT_DISCONNECTED:
                is_connected = 0;
                ack = 1;
                RBL_LOGLN(F("Evt Disconnected/Advertising timed out"));
                lib_aci_connect(Adv_Timeout/* in seconds */, Adv_Interval /* advertising interval 50ms*/);
                RBL_LOGLN(F("Advertising started"));
                break;

            case ACI_EVT_DATA_RECEIVED:
                RBL_LOG(F("Pipe Number: "));
                RBL_LOGLN(aci_evt->params.data_received.rx_data.pipe_number, DEC);
                for(int i=0; i<aci_evt->len - 2; i++)
                {
                    if(rx_buffer_len == MAX_RX_BUFF)
                    {
                        break;
                    }
                    else
                    {
                        if(p_back == &rx_buff[MAX_RX_BUFF])
                        {
                            p_back = &rx_buff[0];
                        }
                        *p_back = aci_evt->params.data_received.rx_data.aci_data[i];
                        rx_buffer_len++;
                        p_back++;
                    }
                }
                break;

            case ACI_EVT_DATA_CREDIT:
                aci_state.data_credit_available = aci_state.data_credit_available + aci_evt->params.data_credit.credit;
                RBL_LOG("ACI_EVT_DATA_CREDIT     ");
                RBL_LOG("Data Credit available: ");
                RBL_LOGLN(aci_state.data_credit_available,DEC);
                ack=1;
                break;

            case ACI_EVT_PIPE_ERROR:
                //See the appendix in the nRF8001 Product Specication for details on the error codes
                RBL_LOG(F("ACI Evt Pipe Error: Pipe #:"));
                RBL_LOG(aci_evt->params.pipe_error.pipe_number, DEC);
                RBL_LOG(F("  Pipe Error Code: 0x"));
                RBL_LOGLN(aci_evt->params.pipe_error.error_code, HEX);

                //Increment the credit available as the data packet was not sent.
                //The pipe error also represents the Attribute protocol Error Response sent from the peer and that should not be counted
                //for the credit.
                if (ACI_STATUS_ERROR_PEER_ATT_ERROR != aci_evt->params.pipe_error.error_code)
                {
                    aci_state.data_credit_available++;
                }
                RBL_LOG("Data Credit available: ");
                RBL_LOGLN(aci_state.data_credit_available,DEC);
                break;

            case ACI_EVT_HW_ERROR:
                RBL_LOG(F("HW error: "));
                RBL_LOGLN(aci_evt->params.hw_error.line_num, DEC);

                for(uint8_t counter = 0; counter <= (aci_evt->len - 3); counter++)
                {
                  Serial.write(aci_evt->params.hw_error.file_name[counter]); //uint8_t file_name[20];
                }
                RBL_LOGLN();
                lib_aci_connect(Adv_Timeout/* in seconds */, Adv_Interval /* advertising interval 50ms*/);
                RBL_LOGLN(F("Advertising started"));
                break;
        }
    }
    else
    {
        //RBL_LOGLN(F("No ACI Events available"));
        // No event in the ACI Event queue and if there is no event in the ACI command queue the arduino can go to sleep
        // Arduino can go to sleep now
        // Wakeup from sleep from the RDYN line
    }
    /* setup_required is set to true when the device starts up and enters setup mode.
    * It indicates that do_aci_setup() should be called. The flag should be cleared if
    * do_aci_setup() returns ACI_STATUS_TRANSACTION_COMPLETE.
    */
    if(setup_required)
    {
        if (SETUP_SUCCESS == do_aci_setup(&aci_state))
        {
            setup_required = false;
        }
    }
}

void ble_do_events()
{
#if ( !defined(__SAM3X8E__) && !defined(__PIC32MX__) )
    spi_old = SPCR;
    SPI.setBitOrder(LSBFIRST);
		SPI.setClockDivider(SPI_CLOCK_DIV8);
    SPI.setDataMode(SPI_MODE0);
#endif

    if (lib_aci_is_pipe_available(&aci_state, PIPE_UART_OVER_BTLE_UART_TX_TX))
    {
        if(tx_buffer_len > 0)
        {
            unsigned char Index = 0;
            while(tx_buffer_len > 20)
            {
                if(true == lib_aci_send_data(PIPE_UART_OVER_BTLE_UART_TX_TX, &tx_buff[Index], 20))
                {
                    RBL_LOG("data transmmit success!  Length: ");
                    RBL_LOG(20, DEC);
                    RBL_LOG("    ");
                }
                else
                {
                    RBL_LOGLN("data transmmit fail !");
                }
                tx_buffer_len -= 20;
                Index += 20;
                aci_state.data_credit_available--;
                RBL_LOG("Data Credit available: ");
                RBL_LOGLN(aci_state.data_credit_available,DEC);
                ack = 0;
                while (!ack)
                    process_events();
            }

            if(true == lib_aci_send_data(PIPE_UART_OVER_BTLE_UART_TX_TX,& tx_buff[Index], tx_buffer_len))
            {
                RBL_LOG("data transmmit success!  Length: ");
                RBL_LOG(tx_buffer_len, DEC);
                RBL_LOG("    ");
            }
            else
            {
                RBL_LOGLN("data transmmit fail !");
            }
            tx_buffer_len = 0;
            aci_state.data_credit_available--;
            RBL_LOG("Data Credit available: ");
            RBL_LOGLN(aci_state.data_credit_available,DEC);
            ack = 0;
            while (!ack)
                process_events();
        }
    }
    process_events();

#if ( !defined(__SAM3X8E__) && !defined(__PIC32MX__) )
    SPCR = spi_old;
#endif
}

