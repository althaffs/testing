#!/bin/bash

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
echo "Running ARTIK test script, using configuraions of $SCRIPT_DIR/artik_test.conf"
source "$SCRIPT_DIR/artik_test.conf"

# function checking result
CHECK_RESULT()
{
	if [ $? -eq 0 ]
	then echo SUCCEEDED
	else echo FAILED*
	fi
}

# absolute paths for tests
ADC_TEST=${ARTIK_TEST_PATH}/adc-test
BT_TEST=${ARTIK_TEST_PATH}/bluetooth-test
CLOUD_TEST=${ARTIK_TEST_PATH}/cloud-test
GPIO_TEST=${ARTIK_TEST_PATH}/gpio-test
HTTP_TEST=${ARTIK_TEST_PATH}/http-test
I2C_TEST=${ARTIK_TEST_PATH}/i2c-test
LOOP_TEST=${ARTIK_TEST_PATH}/loop-test
MEDIA_TEST=${ARTIK_TEST_PATH}/media-test
MODULE_TEST=${ARTIK_TEST_PATH}/module-test
PWM_TEST=${ARTIK_TEST_PATH}/pwm-test
SEC_TEST=${ARTIK_TEST_PATH}/security-test
SERIAL_TEST=${ARTIK_TEST_PATH}/serial-test
SPI_TEST=${ARTIK_TEST_PATH}/spi-test
TIME_TEST=${ARTIK_TEST_PATH}/time-test
WEBSOCKET_TEST=${ARTIK_TEST_PATH}/websocket-test
WIFI_TEST=${ARTIK_TEST_PATH}/wifi-test
LWM2M_TEST=${ARTIK_TEST_PATH}/lwm2m-test
NETWORK_TEST=${ARTIK_TEST_PATH}/network-test
SENSOR_TEST=${ARTIK_TEST_PATH}/sensor-test
ZIGBEE_TEST=${ARTIK_TEST_PATH}/zigbee-test
SDR_TEST=${ARTIK_TEST_PATH}/sdr-test

#run automatic tests
/bin/bash -c '$1;exit $?' -- $ADC_TEST
ADC_RESULT=$(CHECK_RESULT);
/bin/bash -c '$1 -t $2 -d $3 -u $4 -p $5 -m $6 -a $7 -y $8;exit $?' -- $CLOUD_TEST $CLOUD_TOKEN $CLOUD_DEVICE_ID $CLOUD_USER_ID $CLOUD_APP_ID $CLOUD_MESSAGE $CLOUD_ACTION $CLOUD_DT_ID
CLOUD_RESULT=$(CHECK_RESULT)
/bin/bash -c '$1;exit $?' -- $HTTP_TEST
HTTP_RESULT=$(CHECK_RESULT)
/bin/bash -c '$1;exit $?' -- $I2C_TEST
I2C_RESULT=$(CHECK_RESULT)
/bin/bash -c '$1;exit $?' -- $LOOP_TEST
LOOP_RESULT=$(CHECK_RESULT)
/bin/bash -c '$1 -f $2;exit $?' -- $MEDIA_TEST $MEDIA_FILE
MEDIA_RESULT=$(CHECK_RESULT)
/bin/bash -c '$1;exit $?' -- $MODULE_TEST
MODULE_RESULT=$(CHECK_RESULT)
/bin/bash -c '$1;exit $?' -- $NETWORK_TEST
NETWORK_RESULT=$(CHECK_RESULT)
/bin/bash -c '$1;exit $?' -- $PWM_TEST
PWM_RESULT=$(CHECK_RESULT)
/bin/bash -c '$1;exit $?' -- $SEC_TEST
SEC_RESULT=$(CHECK_RESULT)
/bin/bash -c '$1;exit $?' -- $SENSOR_TEST
SENSOR_RESULT=$(CHECK_RESULT)
/bin/bash -c '$1;exit $?' -- $SPI_TEST
SPI_RESULT=$(CHECK_RESULT)
/bin/bash -c '$1;exit $?' -- $TIME_TEST
TIME_RESULT=$(CHECK_RESULT)
/bin/bash -c '$1 -t "$2" -d "$3" -m "$4";exit $?' -- $WEBSOCKET_TEST $WEBSOCKET_ACCESS_TOKEN $WEBSOCKET_DEVICE_ID $WEBSOCKET_MESSAGE
WEBSOCKET_RESULT=$(CHECK_RESULT)
/bin/bash -c '$1 -s "$2" -p "$3";exit $?' -- $WIFI_TEST $WIFI_SSID $WIFI_PSK
WIFI_RESULT=$(CHECK_RESULT)

#run manual-interactive tests
if [ $RUN_MANUAL_TESTS = true ]
then
	/bin/bash -c '$1 -t $2;exit $?' -- $BT_TEST $BT_ADDR
	BT_RESULT=$(CHECK_RESULT)
	/bin/bash -c '$1;exit $?' -- $GPIO_TEST
	GPIO_RESULT=$(CHECK_RESULT)
	/bin/bash -c '$1;exit $?' -- $LWM2M_TEST
	LWM2M_RESULT=$(CHECK_RESULT)
	/bin/bash -c '$1;exit $?' -- $SERIAL_TEST
	SERIAL_RESULT=$(CHECK_RESULT)
	/bin/bash -c '$1;exit $?' -- $ZIGBEE_TEST
	ZIGBEE_RESULT=$(CHECK_RESULT)
	/bin/bash -c '$1 -x "$2" -v "$2" -g "$4";exit $?' -- $SDR_TEST $SDR_DT_ID $SDR_VENDOR_ID $SDR_MESSAGE
	SDR_RESULT=$(CHECK_RESULT)
fi

# print the results from automatic tests
echo "ADC TEST $ADC_RESULT"
echo "CLOUD TEST $CLOUD_RESULT"
echo "HTTP TEST $HTTP_RESULT"
echo "I2C TEST $I2C_RESULT"
echo "LOOP TEST $LOOP_RESULT"
echo "MEDIA TEST $MEDIA_RESULT"
echo "MODULE TEST $MODULE_RESULT"
echo "NETWORK TEST $NETWORK_RESULT"
echo "PWM TEST $PWM_RESULT"
echo "SECURITY TEST $SEC_RESULT"
echo "SENSOR TEST $SENSOR_RESULT"
echo "SPI TEST $SPI_RESULT"
echo "TIME TEST $LOOP_RESULT"
echo "WEBSOCKET TEST $WEBSOCKET_RESULT"
echo "WIFI TEST $WIFI_RESULT"

# print the results from manual-interactive tests
if [ $RUN_MANUAL_TESTS = true ]
then
	echo "BLUETOOTH TEST $BT_RESULT"
	echo "GPIO TEST $GPIO_RESULT"
	echo "LWM2M TEST $LWM2M_RESULT"
	echo "SERIAL TEST $SERIAL_RESULT"
	echo "ZIGBEE TEST $ZIGBEE_RESULT"
	echo "SDR TEST $SDR_RESULT"
fi
