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

#include <unistd.h>
#include <stdio.h>
#include <artik_gpio.hh>
#include <artik_platform.h>

#define LED_GPIO ARTIK_A710_GPIO1

int main(int argc, char *argv[]) {
  artik_error ret = S_OK;
  artik::Gpio* led = new artik::Gpio(LED_GPIO, const_cast<char*>("led"),
      GPIO_OUT, GPIO_EDGE_NONE, 0);

  ret = led->request();
  if (ret != S_OK) {
    fprintf(stderr, "Failed to request gpio %d - err=%d\n", LED_GPIO, ret);
    goto exit;
  }

  fprintf(stdout, "Toggling GPIO: %s\n", led->get_name());

  led->write(1);
  usleep(1000 * 1000);
  led->write(0);

  led->release();
  delete led;

exit:
  return ret;
}
