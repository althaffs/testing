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
#include <string.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>

#include <artik_module.h>
#include <artik_platform.h>

#include <artik_pwm.hh>

static int end = 15;

void sig_handler(void) {
  end = 0;
  fprintf(stdout, "Finish test PWM \n");
}

typedef void (*t_ptr_func) (int);

int main(void) {
  artik_pwm_config pwm_config;
  artik_error ret = S_OK;
  int platid = artik_get_platform();

  if (platid == ARTIK520) {
    pwm_config.pin_num = ARTIK_A520_PWM1;
  }
  if (platid == ARTIK710) {
    pwm_config.pin_num = ARTIK_A710_PWM0;
  }
  if (platid == ARTIK1020) {
    pwm_config.pin_num = ARTIK_A1020_PWM1;
  }
  pwm_config.name = const_cast<char*>("pwm");
  pwm_config.period = 400000;
  pwm_config.duty_cycle = 200000;
  pwm_config.polarity = ARTIK_PWM_POLR_NORMAL;
  artik::Pwm *pwm = new artik::Pwm(pwm_config);

  fprintf(stdout, "Launch request PWM Module\n");

  ret = pwm->request();

  if (ret != S_OK) {
    fprintf(stdout, "ERROR : cannot request a PWM module\n");
  } else {
    t_ptr_func prev = signal(SIGINT, (void(*)(int))sig_handler);
    fprintf(stdout, "SUCCESS : Request a PWM module : %s\n", pwm->get_name());
    while (end) {
      if (end != 0)
        end--;
      sleep(1);
    }
    signal(SIGINT, prev);
    pwm->release();
    fprintf(stdout, "SUCCESS : Release PWM module : %s\n", pwm->get_name());
  }
  delete(pwm);
  return ret;
}
