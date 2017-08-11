#! /bin/sh

VERSION=$1
MAJOR=`echo ${VERSION} | cut -d. -f1`
MINOR=`echo ${VERSION} | cut -d. -f2`

#[[ -z  $VERSION  ]] && echo "Usage: pass version number formatted X.Y as a parameter" && exit 1
#[[ -z  $MAJOR  ]] && echo "Wrong version number parameter, expected X.Y" && exit 1
#[[ -z  $MINOR  ]] && echo "Wrong version number parameter, expected X.Y" && exit 1

sed -i'' "s/LIB_VERSION_MAJOR [0-9]*/LIB_VERSION_MAJOR ${MAJOR}/" CMakeLists.txt
sed -i'' "s/LIB_VERSION_MINOR [0-9]*/LIB_VERSION_MINOR ${MINOR}/" CMakeLists.txt
sed -i'' "s/version==*.*/version=${VERSION}/g" pkgconfig/libartik-sdk-base.pc
sed -i'' "s/version==*.*/version=${VERSION}/g" pkgconfig/libartik-sdk-bluetooth.pc
sed -i'' "s/version==*.*/version=${VERSION}/g" pkgconfig/libartik-sdk-connectivity.pc
sed -i'' "s/version==*.*/version=${VERSION}/g" pkgconfig/libartik-sdk-media.pc
sed -i'' "s/version==*.*/version=${VERSION}/g" pkgconfig/libartik-sdk-sensor.pc
sed -i'' "s/version==*.*/version=${VERSION}/g" pkgconfig/libartik-sdk-systemio.pc
sed -i'' "s/version==*.*/version=${VERSION}/g" pkgconfig/libartik-sdk-wifi.pc
sed -i'' "s/version==*.*/version=${VERSION}/g" pkgconfig/libartik-sdk-zigbee.pc
sed -i'' "s/version==*.*/version=${VERSION}/g" pkgconfig/libartik-sdk-lwm2m.pc
sed -i'' "s/version==*.*/version=${VERSION}/g" pkgconfig/libartik-sdk-mqtt.pc
sed -i'' "s/Version:            [0-9]*.[0-9]*/Version:            ${VERSION}/" specs/libartik-sdk.spec
sed -i'' "s/PROJECT_NUMBER         = [0-9]*.[0-9]*/PROJECT_NUMBER         = ${VERSION}/" doc/Doxyfile
