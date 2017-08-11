/*
 *
 * Copyright 2017 Samsung Electronics All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the License.
 *
 */

#include <stdio.h>
#include <artik_serial.hh>
#include <artik_platform.h>

#define MAX_RX_BUF  64

static artik_serial_config config = {
    ARTIK_A520_SCOM_XSCOM4,
    const_cast<char*>("UART3"),
    ARTIK_SERIAL_BAUD_115200,
    ARTIK_SERIAL_PARITY_NONE,
    ARTIK_SERIAL_DATA_8BIT,
    ARTIK_SERIAL_STOP_1BIT,
    ARTIK_SERIAL_FLOWCTRL_NONE,
    NULL
};

artik_error test_serial_loopback(void) {
  artik_error ret = S_OK;
  artik::Serial *sp = new artik::Serial(config.port_num,
      const_cast<char*>("serial"), config.baudrate, config.parity,
      config.data_bits, config.stop_bits, config.flowctrl);

  unsigned char tx_buf[] = "This is a test buffer containing test data";
  int tx_len = sizeof(tx_buf)/sizeof(tx_buf[0]);
  unsigned char rx_buf[MAX_RX_BUF] = "";
  int read_bytes = 0;
  int len = tx_len;

  fprintf(stdout, "TEST: %s\n", __func__);

  ret = sp->request();
  if (ret != S_OK) {
    fprintf(stderr, "TEST: %s failed to request serial port (%d)\n", __func__,
        ret);
    return ret;
  }

  /* Send test data */
  ret = sp->write(tx_buf, &tx_len);
  if (ret < S_OK) {
    fprintf(stderr, "TEST: %s failed to send data (%d)\n", __func__, ret);
    goto exit;
  }

  /* Loop until we read all the data we expect */
  while (read_bytes < tx_len) {
    ret = sp->read(rx_buf + read_bytes, &len);

    if (ret == S_OK) {
      read_bytes += len;
    }
  }

  /* Compare with sent data */
  if (memcmp(tx_buf, rx_buf, tx_len)) {
    fprintf(stderr,
        "TEST: %s failed (%d). Tx and Rx data don't match (%s != %s)\n",
        __func__, ret, tx_buf, rx_buf);
    goto exit;
  } else {
    fprintf(stdout, "TEST: refound the completed rx_buf '%s'\n", rx_buf);
  }
  fprintf(stdout, "TEST: %s succeeded\n", __func__);

exit:
  sp->release();
  delete sp;
  return ret == S_OK;
}

int main(int argc, char *argv[]) {
  artik_error ret = S_OK;
  int platid = artik_get_platform();

  switch (platid) {
  case ARTIK520:
    config.port_num = ARTIK_A520_SCOM_XSCOM4;
    config.name = const_cast<char*>("UART3");
    break;
  case ARTIK1020:
    config.port_num = ARTIK_A1020_SCOM_XSCOM2;
    config.name = const_cast<char*>("UART1");
    break;
  case ARTIK710:
    config.port_num = ARTIK_A710_UART0;
    config.name = const_cast<char*>("UART0");
    break;
  default:
    goto exit;
  }

  ret = test_serial_loopback();

exit:
  return (ret == S_OK) ? 0 : -1;
}
