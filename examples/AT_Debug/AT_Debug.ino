/*
  Copyright 2023, 5G HUB

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
  associated documentation files (the "Software"), to deal in the Software without restriction, including
  without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the
  following conditions:

  The above copyright notice and this permission notice shall be included in all copies or substantial
  portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
  TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
  CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  IN THE SOFTWARE.

*/
#include "USBHost_t36.h"

typedef enum cmd_response {
  UNKNOWN_RESPONSE  = -2,
  TIMEOUT_RESPONSE  = -1,
  FIAL_RESPONSE  =  0,
  SUCCESS_RESPONSE  = 1
} Cmd_Response_t;

typedef enum cmd_status {
  READ_MODE = 0,
  WRITE_MODE = 1,
} Cmd_Status_t;

typedef enum net_status {
  NOT_REGISTERED = 0,
  REGISTERED = 1,
  SEARCHING = 2,
  REGISTRATION_DENIED = 3,
  UNKNOWN = 4,
  REGISTERED_ROAMING = 5,
} Net_Status_t;

/*#define USBBAUD 115200
  uint32_t baud = USBBAUD;
  uint32_t format = USBHOST_SERIAL_8N1;*/
USBHost myusb;
USBSerial_BigBuffer userial(myusb, 1); // Handles anything up to 512 bytes

/*USBDriver *drivers[] = {&userial};
  #define CNT_DEVICES (sizeof(drivers)/sizeof(drivers[0]))
  const char * driver_names[CNT_DEVICES] = {"USERIAL1"};
  bool driver_active[CNT_DEVICES] = {false};
*/
#define BOARD_RESET_MDM_GPIO GPIO4
#define BOARD_RESET_MDM_GPIO_PIN (16U)
#define BOARD_DISABLE_MDM_GPIO GPIO4
#define BOARD_DISABLE_MDM_GPIO_PIN (13U)

/* GPIO configuration of RESET_MDM on IOMUXC_GPIO_EMC_16_GPIO4_IO16 */
gpio_pin_config_t RESET_MDM_config = {
  .direction = kGPIO_DigitalOutput,
  .outputLogic = 0U,
  .interruptMode = kGPIO_NoIntmode
};

unsigned long waittime;

void setup()
{
  /* Initialize GPIO functionality on IOMUXC_GPIO_EMC_16_GPIO4_IO16 */
  GPIO_PinInit(BOARD_RESET_MDM_GPIO, BOARD_RESET_MDM_GPIO_PIN, &RESET_MDM_config);
  /* Initialize GPIO functionality on IOMUXC_GPIO_EMC_13_GPIO4_IO13 */
  GPIO_PinInit(BOARD_DISABLE_MDM_GPIO, BOARD_DISABLE_MDM_GPIO_PIN, &RESET_MDM_config);

  // ENABLE MDM
  GPIO_PinWrite(BOARD_DISABLE_MDM_GPIO, BOARD_DISABLE_MDM_GPIO_PIN, 0U);

  // RESET MDM
  GPIO_PinWrite(BOARD_RESET_MDM_GPIO, BOARD_RESET_MDM_GPIO_PIN, 1U);
  delay(460);
  GPIO_PinWrite(BOARD_RESET_MDM_GPIO, BOARD_RESET_MDM_GPIO_PIN, 0U);

  Serial.begin(115200);  // We will echo stuff Through Serial1...
  while (!Serial && (millis() < 5000)) ; // wait for Arduino Serial Monitor
  Serial.println("\n\nUSB Host Testing - Serial");

  myusb.begin();
  waittime = millis();
}

void loop()
{
  if (userial) {
    SetDevCommandEcho(false);

    char inf[64];
    if (GetDevInformation(inf)) {
      Serial.println(inf);
    }

    Net_Status_t i_status;
    Cmd_Response_t init_status;
    unsigned long start_time = millis();
    while (!DevSimPIN("", READ_MODE)) {
      if (millis() - start_time >= 10 * 1000UL) {
        Serial.println("\r\nAPN ERROR: No SIM card detected!\r\n");
      }
    }
    Serial.println("\r\nSIM card detected!\r\n");

    start_time = millis();
    while (i_status != REGISTERED && i_status != REGISTERED_ROAMING) {
      i_status = DevNetRegistrationStatus();
      if (millis() - start_time >= 120 * 1000UL) {
        Serial.println("\r\nAPN ERROR: Can't registered to the Operator network!\r\n");
      }

      if (i_status == REGISTERED)
        Serial.println("\r\nRegistered to the Operator network!\r\n");
      delay(3000);
    }
  }
}

// AT commands response
const char RESPONSE_READY[] = "RDY";
const char RESPONSE_OK[] = "OK";
const char RESPONSE_CRLF_OK[] = "\r\n\r\nOK";
const char RESPONSE_ERROR[] = "ERROR";
const char RESPONSE_POWER_DOWN[] = "POWERED DOWN";
const char RESPONSE_CONNECT[] = "CONNECT";
const char RESPONSE_SEND_OK[] = "SEND OK";
const char RESPONSE_SEND_FAIL[] = "SEND FAIL";

// common AT commands
const char DEV_AT[] = "";
const char DEV_INFORMATION[] = "I";
const char DEV_VERSION[] = "+CGMR";
const char DEV_IMEI[] = "+CGSN";
const char DEV_FUN_LEVEL[] = "+CFUN";
const char DEV_LOCAL_RATE[] = "+IPR";
const char DEV_SIM_IMSI[] = "+CIMI";
const char DEV_SIM_PIN[] = "+CPIN";
const char DEV_SIM_ICCID[] = "+QCCID";
const char DEV_NET_STATUS[] = "+CREG";
const char DEV_NET_STATUS_G[] = "+CGREG";
const char DEV_EPS_NET_STATUS[] = "+CEREG";
const char DEV_NET_RSSI[] = "+CSQ";
const char DEV_NET_OPERATOR[] = "+COPS";
const char DEV_NET_INFORMATION[] = "+QNWINFO";
const char DEV_NET_PACKET_COUNTER[] = "+QGDCNT";
const char DEV_POWER_DOWN[] = "+QPOWD";
const char DEV_CLOCK[] = "+CCLK";

#define RX_BUFFER_LENGTH  1024
char rxBuffer[RX_BUFFER_LENGTH];
unsigned int bufferHead;
int errorCode = -1;

void cleanBuffer()
{
  memset(rxBuffer, '\0', RX_BUFFER_LENGTH);
  bufferHead = 0;
}

int serialAvailable()
{
  unsigned int ret;
  ret = userial.available();
  return ret;
}

char *searchStrBuffer(const char *test_str)
{
  int buf_len = strlen((const char *)rxBuffer);
  if (buf_len < RX_BUFFER_LENGTH) {
    return strstr((const char *)rxBuffer, test_str);
  } else {
    return NULL;
  }
}

char *searchChrBuffer(const char test_chr)
{
  int buf_len = strlen((const char *)rxBuffer);
  if (buf_len < RX_BUFFER_LENGTH) {
    return strchr((const char *)rxBuffer, test_chr);
  } else {
    return NULL;
  }
}

unsigned int readResponseByteToBuffer()
{
  char c = userial.read();
  rxBuffer[bufferHead] = c;
  bufferHead = (bufferHead + 1) % RX_BUFFER_LENGTH;
  //#if defined UART_DEBUG
  if (c == '\n') {
    Serial.print(c);
    Serial.print("<- ");
  } else {
    Serial.print(c);
  }
  //#endif
  return 1;
}

Cmd_Response_t readResponseAndSearch(char *test_str, char *e_test_str, unsigned int timeout)
{
  unsigned long start_time = millis();
  unsigned int recv_len = 0;
  errorCode = -1;
  cleanBuffer();
  while (millis() - start_time < timeout * 1000UL) {
    if (serialAvailable()) {
      recv_len += readResponseByteToBuffer();
      if (searchStrBuffer(test_str)) {
        return SUCCESS_RESPONSE;
      } else if (searchStrBuffer(e_test_str)) {
        start_time = millis();
        while (millis() - start_time < 100UL) {
          if (serialAvailable()) {
            recv_len += readResponseByteToBuffer();
          }
        }
        char *str_buf = searchStrBuffer(": ");
        if (str_buf != NULL) {
          char err_code[8];
          strcpy(err_code, str_buf + 2);
          char *end_buf = strstr(err_code, "\r\n");
          *end_buf = '\0';
          errorCode = atoi(err_code);
        }
        return FIAL_RESPONSE;
      }
    }
  }
  if (recv_len > 0) {
    return UNKNOWN_RESPONSE;
  } else {
    return TIMEOUT_RESPONSE;
  }
}

Cmd_Response_t readResponseAndSearch1(const char *test_str, unsigned int timeout)
{
  unsigned long start_time = millis();
  unsigned int recv_len = 0;
  cleanBuffer();
  while (millis() - start_time < timeout * 1000UL) {
    if (serialAvailable()) {
      recv_len += readResponseByteToBuffer();
      if (searchStrBuffer(test_str)) {
        return SUCCESS_RESPONSE;
      }
    }
  }
  if (recv_len > 0) {
    return UNKNOWN_RESPONSE;
  } else {
    return TIMEOUT_RESPONSE;
  }
}

bool sendATcommand(const char *command)
{
  //  delay(100);
  while (userial.read() >= 0);
  userial.write("AT");
  int cmd_len = strlen(command);
  int send_bytes = userial.write(command);
  //#if defined UART_DEBUG
  Serial.print("\r\n");
  Serial.print("-> ");
  Serial.print("AT");
  Serial.print(command);
  Serial.print("\r\n");
  //#endif
  if (send_bytes != cmd_len) {
    return false;
  }
  userial.write("\r\n");
  return true;
}

Cmd_Response_t sendAndSearch(const char *command, const char *test_str, unsigned int timeout)
{
  for (int i = 0; i < 3; i++) {
    if (sendATcommand(command)) {
      if (readResponseAndSearch1(test_str, timeout) == SUCCESS_RESPONSE) {
        return SUCCESS_RESPONSE;
      }
    }
  }
  return TIMEOUT_RESPONSE;
}

bool SetDevCommandEcho(bool echo)
{
  const char *cmd;
  if (echo == true) {
    cmd = "E1";
  } else {
    cmd = "E0";
  }
  if (sendAndSearch(cmd, RESPONSE_OK, 2)) {
    return true;
  }
  return false;
}

bool GetDevInformation(char *inf)
{
  if (sendAndSearch(DEV_INFORMATION, RESPONSE_OK, 2)) {
    char *end_buf = searchStrBuffer(RESPONSE_CRLF_OK);
    *end_buf = '\0';
    strcpy(inf, rxBuffer);
    return true;
  }
  return false;
}

bool DevSimPIN(char *pin, Cmd_Status_t status)
{
  char cmd[16];
  strcpy(cmd, DEV_SIM_PIN);
  if (status == READ_MODE) {
    strcat(cmd, "?");
    if (sendAndSearch(cmd, "READY", 2)) {
      //pin = "READY";
      return true;
    }
  } else if (status == WRITE_MODE) {
    char buf[16];
    sprintf(buf, "=\"%s\"", pin);
    strcat(cmd, buf);
    if (sendAndSearch(cmd, RESPONSE_OK, 2)) {
      return true;
    }
  }
  return false;
}

Net_Status_t DevNetRegistrationStatus()
{
  char cmd[16];
  Net_Status_t n_status = NOT_REGISTERED;
  strcpy(cmd, DEV_NET_STATUS_G);
  strcat(cmd, "?");
  if (sendAndSearch(cmd, RESPONSE_OK, 2)) {
    char *end_buf = searchStrBuffer(RESPONSE_CRLF_OK);
    *end_buf = '\0';
    char *sta_buf = searchChrBuffer(',');
    n_status = (Net_Status_t)atoi(sta_buf + 1);
    switch (n_status)
    {
      case REGISTERED:
      case REGISTERED_ROAMING:
        return n_status;
      default:
        break;
    }
  }

  strcpy(cmd, DEV_EPS_NET_STATUS);
  strcat(cmd, "?");
  if (sendAndSearch(cmd, RESPONSE_OK, 2)) {
    char *end_buf = searchStrBuffer(RESPONSE_CRLF_OK);
    *end_buf = '\0';
    char *sta_buf = searchChrBuffer(',');
    n_status = (Net_Status_t)atoi(sta_buf + 1);
    switch (n_status)
    {
      case REGISTERED:
      case REGISTERED_ROAMING:
        return n_status;
      default:
        break;
    }
  }
  return n_status;
}
