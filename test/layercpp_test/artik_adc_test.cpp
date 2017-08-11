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
#include <signal.h>
#include <string.h>
#include <unistd.h>

#include <artik_adc.hh>

static int end = 1;

void sig_handler(int sig) {
  end = 0;
  fprintf(stdout, "Finish test ADC\n");
}

int main(int argc, char *argv[]) {
  artik_adc_config adc_config;
  artik_error ret = S_OK;

  adc_config.pin_num = 0;
  adc_config.name = strdup("adc");
  artik::Adc *adc = new artik::Adc(adc_config);

  fprintf(stdout, "Launch request ADC Module\n");
  ret = adc->request();
  signal(SIGINT, (void(*)(int))sig_handler);
  while (end) {
    int val = -1;
    adc->get_value(&val);
    fprintf(stdout, "SUCCESS got a ADC module %s %d\n", adc->get_name(), val);
    usleep(2000);
  }
  adc->release();
  delete adc;
  fprintf(stdout, "SUCCESS Realease ADC module\n");
  return ret;
}
