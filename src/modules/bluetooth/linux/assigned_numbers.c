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

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include <gio/gio.h>
#pragma GCC diagnostic pop

#include "assigned_numbers.h"

static struct {
	uint16_t uuid;
	const char *name;
	const char *desc;
} bt_uuids[] = {

	/* Protocol Identifiers */
	{0x0001, "SDP", "Bluetooth Core Specification"},
	{0x0002, "UDP", "[NO USE BY PROFILES]"},
	{0x0003, "RFCOMM", "RFCOMM with TS 07.10"},
	{0x0004, "TCP", "[NO USE BY PROFILES]"},
	{0x0005, "TCS-BIN", "Telephony Control Specification / TCS Binary [DEPRECATED]"},
	{0x0006, "TCS-AT", "[NO USE BY PROFILES]"},
	{0x0007, "ATT", "Attribute Protocol"},
	{0x0008, "OBEX", "IrDA Interoperability"},
	{0x0009, "IP", "[NO USE BY PROFILES]"},
	{0x000A, "FTP", "[NO USE BY PROFILES]"},
	{0x000C, "HTTP", "[NO USE BY PROFILES]"},
	{0x000E, "WSP", "[NO USE BY PROFILES]"},
	{0x000F, "BNEP", "Bluetooth Network Encapsulation Protocol (BNEP)"},
	{0x0010, "UPNP", "Extended Service Discovery Profile (ESDP) [DEPRECATED]"},
	{0x0011, "HIDP", "Human Interface Device Profile (HID)"},
	{0x0012, "HardcopyControlChannel", "Hardcopy Cable Replacement Profile (HCRP)"},
	{0x0014, "HardcopyDataChannel", "See Hardcopy Cable Replacement Profile (HCRP)"},
	{0x0016, "HardcopyNotification", "Hardcopy Cable Replacement Profile (HCRP)"},
	{0x0017, "AVCTP", "Audio/Video Control Transport Protocol (AVCTP)"},
	{0x0019, "AVDTP", "Audio/Video Distribution Transport Protocol (AVDTP)"},
	{0x001B, "CMTP", "Common ISDN Access Profile (CIP) [DEPRECATED]"},
	{0x001E, "MCAPControlChannel", "Multi-Channel Adaptation Protocol (MCAP)"},
	{0x001F, "MCAPDataChannel", "Multi-Channel Adaptation Protocol (MCAP)"},
	{0x0100, "L2CAP", "Bluetooth Core Specification"},

	/* Service Class Profile Identifiers */
	{0x1000, "ServiceDiscoveryServerServiceClassID", "Bluetooth Core Specification"},
	{0x1001, "BrowseGroupDescriptorServiceClassID", "Bluetooth Core Specification"},
	{0x1101, "SerialPort", "Serial Port Profile (SPP)"},
	{0x1102, "LANAccessUsingPPP", "LAN Access Profile"},
	{0x1103, "DialupNetworking", "Dial-up Networking Profile"},
	{0x1104, "IrMCSync", "Synchronization Profile"},
	{0x1105, "OBEXObjectPush", "Object Push Profile (OPP)"},
	{0x1106, "OBEXFileTransfer", "File Transfer Profile (FTP)"},
	{0x1107, "IrMCSyncCommand", "Synchronization Profile (SYNC)"},
	{0x1108, "Headset", "Headset Profile (HSP)"},
	{0x1109, "CordlessTelephony", "Cordless Telephony Profile (CTP)"},
	{0x110A, "AudioSource", "Advanced Audio Distribution Profile (A2DP)"},
	{0x110B, "AudioSink", "Advanced Audio Distribution Profile (A2DP)"},
	{0x110C, "A/V_RemoteControlTarget", "Audio/Video Remote Control Profile (AVRCP)"},
	{0x110D, "AdvancedAudioDistribution", "Advanced Audio Distribution Profile (A2DP)"},
	{0x110E, "A/V_RemoteControl", "Audio/Video Remote Control Profile (AVRCP)"},
	{0x110F, "A/V_RemoteControlController", "Audio/Video Remote Control Profile (AVRCP)"},
	{0x1110, "Intercom", "Intercom Profile (ICP)"},
	{0x1111, "Fax", "Fax Profile (FAX)"},
	{0x1112, "Headset - Audio Gateway (AG)", "Headset Profile (HSP)"},
	{0x1113, "WAP", "Interoperability Requirements for Bluetooth technology as a WAP, Bluetooth SIG [DEPRECATED]"},
	{0x1114, "WAP_CLIENT",
		"Interoperability Requirements for Bluetooth technology as a WAP, Bluetooth SIG [DEPRECATED]"},
	{0x1115, "PANU", "Personal Area Networking Profile (PAN)"},
	{0x1116, "NAP", "Personal Area Networking Profile (PAN)"},
	{0x1117, "GN", "Personal Area Networking Profile (PAN)"},
	{0x1118, "DirectPrinting", "Basic Printing Profile (BPP)"},
	{0x1119, "ReferencePrinting", "See Basic Printing Profile (BPP)"},
	{0x111A, "Basic Imaging Profile", "Basic Imaging Profile (BIP)"},
	{0x111B, "ImagingResponder", "Basic Imaging Profile (BIP)"},
	{0x111C, "ImagingAutomaticArchive", "Basic Imaging Profile (BIP)"},
	{0x111D, "ImagingReferencedObjects", "Basic Imaging Profile (BIP)"},
	{0x111E, "Handsfree", "Hands-Free Profile"},
	{0x111F, "HandsfreeAudioGateway", "Hands-free Profile (HFP)"},
	{0x1120, "DirectPrintingReferenceObjectsService", "Basic Printing Profile (BPP)"},
	{0x1121, "ReflectedUI", "Basic Printing Profile (BPP)"},
	{0x1122, "BasicPrinting", "Basic Printing Profile (BPP)"},
	{0x1123, "PrintingStatus", "Basic Printing Profile (BPP)"},
	{0x1124, "HumanInterfaceDeviceService", "Human Interface Device (HID)"},
	{0x1125, "HardcopyCableReplacement", "Hardcopy Cable Replacement Profile (HCRP)"},
	{0x1126, "HCR_Print", "Hardcopy Cable Replacement Profile (HCRP)"},
	{0x1127, "HCR_Scan", "Hardcopy Cable Replacement Profile (HCRP)"},
	{0x1128, "Common_ISDN_Access", "Common ISDN Access Profile (CIP)"},
	{0x112D, "SIM_Access", "SIM Access Profile (SAP)"},
	{0x112E, "Phonebook Access - PCE", "Phonebook Access Profile (PBAP)"},
	{0x112F, "Phonebook Access - PSE", "Phonebook Access Profile (PBAP)"},
	{0x1130, "Phonebook Access", "Phonebook Access Profile (PBAP)"},
	{0x1131, "Headset - HS", "Headset Profile (HSP)"},
	{0x1132, "Message Access Server", "Message Access Profile (MAP)"},
	{0x1133, "Message Notification Server", "Message Access Profile (MAP)"},
	{0x1134, "Message Access Profile", "Message Access Profile (MAP)"},
	{0x1135, "GNSS", "Global Navigation Satellite System Profile (GNSS)"},
	{0x1136, "GNSS_Server", "Global Navigation Satellite System Profile (GNSS)"},
	{0x1137, "3D Display", "3D Synchronization Profile (3DSP)"},
	{0x1138, "3D Glasses", "3D Synchronization Profile (3DSP)"},
	{0x1139, "3D Synchronization", "3D Synchronization Profile (3DSP)"},
	{0x113A, "MPS Profile UUID", "Multi-Profile Specification (MPS)"},
	{0x113B, "MPS SC UUID", "Multi-Profile Specification (MPS)"},
	{0x113C, "CTN Access Service", "Calendar, Task, and Notes (CTN)"},
	{0x113D, "CTN Notification Service", "Calendar Tasks and Notes (CTN)"},
	{0x113E, "CTN Profile", "Calendar Tasks and Notes (CTN)"},
	{0x1200, "PnPInformation", "Device Identification (DID)"},
	{0x1201, "GenericNetworking", ""},
	{0x1202, "GenericFileTransfer", ""},
	{0x1203, "GenericAudio", ""},
	{0x1204, "GenericTelephony", ""},
	{0x1205, "UPNP_Service", "Enhanced Service Discovery Profile (ESDP) [DEPRECATED]"},
	{0x1206, "UPNP_IP_Service", "Enhanced Service Discovery Profile (ESDP) [DEPRECATED]"},
	{0x1300, "ESDP_UPNP_IP_PAN", "Enhanced Service Discovery Profile (ESDP) [DEPRECATED]"},
	{0x1301, "ESDP_UPNP_IP_LAP", "Enhanced Service Discovery Profile (ESDP)[DEPRECATED]"},
	{0x1302, "ESDP_UPNP_L2CAP", "Enhanced Service Discovery Profile (ESDP)[DEPRECATED]"},
	{0x1303, "VideoSource", "Video Distribution Profile (VDP)"},
	{0x1304, "VideoSink", "Video Distribution Profile (VDP)"},
	{0x1305, "VideoDistribution", "Video Distribution Profile (VDP)"},
	{0x1400, "HDP", "Health Device Profile"},
	{0x1401, "HDP Source", "Health Device Profile (HDP)"},
	{0x1402, "HDP Sink", "Health Device Profile (HDP)"},

	/* GATT Services */
	{0x1811, "Alert Notification Service", "org.bluetooth.service.alert_notification"},
	{0x1815, "Automation IO", "org.bluetooth.service.automation_io"},
	{0x180F, "Battery Service", "org.bluetooth.service.battery_service"},
	{0x1810, "Blood Pressure", "org.bluetooth.service.blood_pressure"},
	{0x181B, "Body Composition", "org.bluetooth.service.body_composition"},
	{0x181E, "Bond Management", "org.bluetooth.service.bond_management"},
	{0x181F, "Continuous Glucose Monitoring", "org.bluetooth.service.continuous_glucose_monitoring"},
	{0x1805, "Current Time Service", "org.bluetooth.service.current_time"},
	{0x1818, "Cycling Power", "org.bluetooth.service.cycling_power"},
	{0x1816, "Cycling Speed and Cadence", "org.bluetooth.service.cycling_speed_and_cadence"},
	{0x180A, "Device Information", "org.bluetooth.service.device_information"},
	{0x181A, "Environmental Sensing", "org.bluetooth.service.environmental_sensing"},
	{0x1800, "Generic Access", "org.bluetooth.service.generic_access"},
	{0x1801, "Generic Attribute", "org.bluetooth.service.generic_attribute"},
	{0x1808, "Glucose", "org.bluetooth.service.glucose"},
	{0x1809, "Health Thermometer", "org.bluetooth.service.health_thermometer"},
	{0x180D, "Heart Rate", "org.bluetooth.service.heart_rate"},
	{0x1823, "HTTP Proxy", "org.bluetooth.service.http_proxy"},
	{0x1812, "Human Interface Device", "org.bluetooth.service.human_interface_device"},
	{0x1802, "Immediate Alert", "org.bluetooth.service.immediate_alert"},
	{0x1821, "Indoor Positioning", "org.bluetooth.service.indoor_positioning"},
	{0x1820, "Internet Protocol Support", "org.bluetooth.service.internet_protocol_support"},
	{0x1803, "Link Loss", "org.bluetooth.service.link_loss"},
	{0x1819, "Location and Navigation", "org.bluetooth.service.location_and_navigation"},
	{0x1807, "Next DST Change Service", "org.bluetooth.service.next_dst_change"},
	{0x1825, "Object Transfer", "org.bluetooth.service.object_transfer"},
	{0x180E, "Phone Alert Status Service", "org.bluetooth.service.phone_alert_status"},
	{0x1822, "Pulse Oximeter", "org.bluetooth.service.pulse_oximeter"},
	{0x1806, "Reference Time Update Service", "org.bluetooth.service.reference_time_update"},
	{0x1814, "Running Speed and Cadence", "org.bluetooth.service.running_speed_and_cadence"},
	{0x1813, "Scan Parameters", "org.bluetooth.service.scan_parameters"},
	{0x1824, "Transport Discovery", "org.bluetooth.service.transport_discovery"},
	{0x1804, "Tx Power", "org.bluetooth.service.tx_power"},
	{0x181C, "User Data", "org.bluetooth.service.user_data"},
	{0x181D, "Weight Scale", "org.bluetooth.service.weight_scale"},

	/* Characteristics */
	{0x2A7E, "Aerobic Heart Rate Lower Limit", "org.bluetooth.characteristic.aerobic_heart_rate_lower_limit"},
	{0x2A84, "Aerobic Heart Rate Upper Limit", "org.bluetooth.characteristic.aerobic_heart_rate_upper_limit"},
	{0x2A7F, "Aerobic Threshold", "org.bluetooth.characteristic.aerobic_threshold"},
	{0x2A80, "Age", "org.bluetooth.characteristic.age"},
	{0x2A5A, "Aggregate", "org.bluetooth.characteristic.aggregate"},
	{0x2A43, "Alert Category ID", "org.bluetooth.characteristic.alert_category_id"},
	{0x2A42, "Alert Category ID Bit Mask", "org.bluetooth.characteristic.alert_category_id_bit_mask"},
	{0x2A06, "Alert Level", "org.bluetooth.characteristic.alert_level"},
	{0x2A44, "Alert Notification Control Point", "org.bluetooth.characteristic.alert_notification_control_point"},
	{0x2A3F, "Alert Status", "org.bluetooth.characteristic.alert_status"},
	{0x2AB3, "Altitude", "org.bluetooth.characteristic.altitude"},
	{0x2A81, "Anaerobic Heart Rate Lower Limit", "org.bluetooth.characteristic.anaerobic_heart_rate_lower_limit"},
	{0x2A82, "Anaerobic Heart Rate Upper Limit", "org.bluetooth.characteristic.anaerobic_heart_rate_upper_limit"},
	{0x2A83, "Anaerobic Threshold", "org.bluetooth.characteristic.anaerobic_threshold"},
	{0x2A58, "Analog", "org.bluetooth.characteristic.analog"},
	{0x2A73, "Apparent Wind Direction", "org.bluetooth.characteristic.apparent_wind_direction"},
	{0x2A72, "Apparent Wind Speed", "org.bluetooth.characteristic.apparent_wind_speed"},
	{0x2A01, "Appearance", "org.bluetooth.characteristic.gap.appearance"},
	{0x2AA3, "Barometric Pressure Trend", "org.bluetooth.characteristic.barometric_pressure_trend"},
	{0x2A19, "Battery Level", "org.bluetooth.characteristic.battery_level"},
	{0x2A49, "Blood Pressure Feature", "org.bluetooth.characteristic.blood_pressure_feature"},
	{0x2A35, "Blood Pressure Measurement", "org.bluetooth.characteristic.blood_pressure_measurement"},
	{0x2A9B, "Body Composition Feature", "org.bluetooth.characteristic.body_composition_feature"},
	{0x2A9C, "Body Composition Measurement", "org.bluetooth.characteristic.body_composition_measurement"},
	{0x2A38, "Body Sensor Location", "org.bluetooth.characteristic.body_sensor_location"},
	{0x2AA4, "Bond Management Control Point", "org.bluetooth.characteristic.bond_management_control_point"},
	{0x2AA5, "Bond Management Feature", "org.bluetooth.characteristic.bond_management_feature"},
	{0x2A22, "Boot Keyboard Input Report", "org.bluetooth.characteristic.boot_keyboard_input_report"},
	{0x2A32, "Boot Keyboard Output Report", "org.bluetooth.characteristic.boot_keyboard_output_report"},
	{0x2A33, "Boot Mouse Input Report", "org.bluetooth.characteristic.boot_mouse_input_report"},
	{0x2AA6, "Central Address Resolution", "org.bluetooth.characteristic.gap.central_address_resolution_support"},
	{0x2AA8, "CGM Feature", "org.bluetooth.characteristic.cgm_feature"},
	{0x2AA7, "CGM Measurement", "org.bluetooth.characteristic.cgm_measurement"},
	{0x2AAB, "CGM Session Run Time", "org.bluetooth.characteristic.cgm_session_run_time"},
	{0x2AAA, "CGM Session Start Time", "org.bluetooth.characteristic.cgm_session_start_time"},
	{0x2AAC, "CGM Specific Ops Control Point", "org.bluetooth.characteristic.cgm_specific_ops_control_point"},
	{0x2AA9, "CGM Status", "org.bluetooth.characteristic.cgm_status"},
	{0x2A5C, "CSC Feature", "org.bluetooth.characteristic.csc_feature"},
	{0x2A5B, "CSC Measurement", "org.bluetooth.characteristic.csc_measurement"},
	{0x2A2B, "Current Time", "org.bluetooth.characteristic.current_time"},
	{0x2A66, "Cycling Power Control Point", "org.bluetooth.characteristic.cycling_power_control_point"},
	{0x2A65, "Cycling Power Feature", "org.bluetooth.characteristic.cycling_power_feature"},
	{0x2A63, "Cycling Power Measurement", "org.bluetooth.characteristic.cycling_power_measurement"},
	{0x2A64, "Cycling Power Vector", "org.bluetooth.characteristic.cycling_power_vector"},
	{0x2A99, "Database Change Increment", "org.bluetooth.characteristic.database_change_increment"},
	{0x2A85, "Date of Birth", "org.bluetooth.characteristic.date_of_birth"},
	{0x2A86, "Date of Threshold Assessment", "org.bluetooth.characteristic.date_of_threshold_assessment"},
	{0x2A08, "Date Time", "org.bluetooth.characteristic.date_time"},
	{0x2A0A, "Day Date Time", "org.bluetooth.characteristic.day_date_time"},
	{0x2A09, "Day of Week", "org.bluetooth.characteristic.day_of_week"},
	{0x2A7D, "Descriptor Value Changed", "org.bluetooth.characteristic.descriptor_value_changed"},
	{0x2A00, "Device Name", "org.bluetooth.characteristic.gap.device_name"},
	{0x2A7B, "Dew Point", "org.bluetooth.characteristic.dew_point"},
	{0x2A56, "Digital", "org.bluetooth.characteristic.digital"},
	{0x2A0D, "DST Offset", "org.bluetooth.characteristic.dst_offset"},
	{0x2A6C, "Elevation", "org.bluetooth.characteristic.elevation"},
	{0x2A87, "Email Address", "org.bluetooth.characteristic.email_address"},
	{0x2A0C, "Exact Time 256", "org.bluetooth.characteristic.exact_time_256"},
	{0x2A88, "Fat Burn Heart Rate Lower Limit", "org.bluetooth.characteristic.fat_burn_heart_rate_lower_limit"},
	{0x2A89, "Fat Burn Heart Rate Upper Limit", "org.bluetooth.characteristic.fat_burn_heart_rate_upper_limit"},
	{0x2A26, "Firmware Revision String", "org.bluetooth.characteristic.firmware_revision_string"},
	{0x2A8A, "First Name", "org.bluetooth.characteristic.first_name"},
	{0x2A8B, "Five Zone Heart Rate Limits", "org.bluetooth.characteristic.five_zone_heart_rate_limits"},
	{0x2AB2, "Floor Number", "org.bluetooth.characteristic.floor_number"},
	{0x2A8C, "Gender", "org.bluetooth.characteristic.gender"},
	{0x2A51, "Glucose Feature", "org.bluetooth.characteristic.glucose_feature"},
	{0x2A18, "Glucose Measurement", "org.bluetooth.characteristic.glucose_measurement"},
	{0x2A34, "Glucose Measurement Context", "org.bluetooth.characteristic.glucose_measurement_context"},
	{0x2A74, "Gust Factor", "org.bluetooth.characteristic.gust_factor"},
	{0x2A27, "Hardware Revision String", "org.bluetooth.characteristic.hardware_revision_string"},
	{0x2A39, "Heart Rate Control Point", "org.bluetooth.characteristic.heart_rate_control_point"},
	{0x2A8D, "Heart Rate Max", "org.bluetooth.characteristic.heart_rate_max"},
	{0x2A37, "Heart Rate Measurement", "org.bluetooth.characteristic.heart_rate_measurement"},
	{0x2A7A, "Heat Index", "org.bluetooth.characteristic.heat_index"},
	{0x2A8E, "Height", "org.bluetooth.characteristic.height"},
	{0x2A4C, "HID Control Point", "org.bluetooth.characteristic.hid_control_point"},
	{0x2A4A, "HID Information", "org.bluetooth.characteristic.hid_information"},
	{0x2A8F, "Hip Circumference", "org.bluetooth.characteristic.hip_circumference"},
	{0x2ABA, "HTTP Control Point", "org.bluetooth.characteristic.http_control_point"},
	{0x2AB9, "HTTP Entity Body", "org.bluetooth.characteristic.http_entity_body"},
	{0x2AB7, "HTTP Headers", "org.bluetooth.characteristic.http_headers"},
	{0x2AB8, "HTTP Status Code", "org.bluetooth.characteristic.http_status_code"},
	{0x2ABB, "HTTPS Security", "org.bluetooth.characteristic.https_security"},
	{0x2A6F, "Humidity", "org.bluetooth.characteristic.humidity"},
	{0x2A2A, "IEEE 11073-20601 Regulatory Certification Data List",
		"org.bluetooth.characteristic.ieee_11073-20601_regulatory_certification_data_list"},
	{0x2AAD, "Indoor Positioning Configuration", "org.bluetooth.characteristic.indoor_positioning_configuration"},
	{0x2A36, "Intermediate Cuff Pressure", "org.bluetooth.characteristic.intermediate_cuff_pressure"},
	{0x2A1E, "Intermediate Temperature", "org.bluetooth.characteristic.intermediate_temperature"},
	{0x2A77, "Irradiance", "org.bluetooth.characteristic.irradiance"},
	{0x2AA2, "Language", "org.bluetooth.characteristic.language"},
	{0x2A90, "Last Name", "org.bluetooth.characteristic.last_name"},
	{0x2AAE, "Latitude", "org.bluetooth.characteristic.latitude"},
	{0x2A6B, "LN Control Point", "org.bluetooth.characteristic.ln_control_point"},
	{0x2A6A, "LN Feature", "org.bluetooth.characteristic.ln_feature"},
	{0x2AB1, "Local East Coordinate", "org.bluetooth.characteristic.local_east_coordinate"},
	{0x2AB0, "Local North Coordinate", "org.bluetooth.characteristic.local_north_coordinate"},
	{0x2A0F, "Local Time Information", "org.bluetooth.characteristic.local_time_information"},
	{0x2A67, "Location and Speed", "org.bluetooth.characteristic.location_and_speed"},
	{0x2AB5, "Location Name", "org.bluetooth.characteristic.location_name"},
	{0x2AAF, "Longitude", "org.bluetooth.characteristic.longitude"},
	{0x2A2C, "Magnetic Declination", "org.bluetooth.characteristic.magnetic_declination"},
	{0x2AA0, "Magnetic Flux Density - 2D", "org.bluetooth.characteristic.magnetic_flux_density_2D"},
	{0x2AA1, "Magnetic Flux Density - 3D", "org.bluetooth.characteristic.magnetic_flux_density_3D "},
	{0x2A29, "Manufacturer Name String", "org.bluetooth.characteristic.manufacturer_name_string"},
	{0x2A91, "Maximum Recommended Heart Rate", "org.bluetooth.characteristic.maximum_recommended_heart_rate"},
	{0x2A21, "Measurement Interval", "org.bluetooth.characteristic.measurement_interval"},
	{0x2A24, "Model Number String", "org.bluetooth.characteristic.model_number_string"},
	{0x2A68, "Navigation", "org.bluetooth.characteristic.navigation"},
	{0x2A46, "New Alert", "org.bluetooth.characteristic.new_alert"},
	{0x2AC5, "Object Action Control Point", "org.bluetooth.characteristic.object_action_control_point"},
	{0x2AC8, "Object Changed", "org.bluetooth.characteristic.object_changed "},
	{0x2AC1, "Object First-Created", "org.bluetooth.characteristic.object_first_created"},
	{0x2AC3, "Object ID", "org.bluetooth.characteristic.object_id"},
	{0x2AC2, "Object Last-Modified", "org.bluetooth.characteristic.object_last_modified"},
	{0x2AC6, "Object List Control Point", "org.bluetooth.characteristic.object_list_control_point"},
	{0x2AC7, "Object List Filter", "org.bluetooth.characteristic.object_list_filter "},
	{0x2ABE, "Object Name", "org.bluetooth.characteristic.object_name"},
	{0x2AC4, "Object Properties", "org.bluetooth.characteristic.object_properties"},
	{0x2AC0, "Object Size", "org.bluetooth.characteristic.object_size"},
	{0x2ABF, "Object Type", "org.bluetooth.characteristic.object_type"},
	{0x2ABD, "OTS Feature", "org.bluetooth.characteristic.ots_feature"},
	{0x2A04, "Peripheral Preferred Connection Parameters",
		"org.bluetooth.characteristic.gap.peripheral_preferred_connection_parameters"},
	{0x2A02, "Peripheral Privacy Flag", "org.bluetooth.characteristic.gap.peripheral_privacy_flag"},
	{0x2A5F, "PLX Continuous Measurement", "org.bluetooth.characteristic.plx_continuous_measurement"},
	{0x2A60, "PLX Features", "org.bluetooth.characteristic.plx_features"},
	{0x2A5E, "PLX Spot-Check Measurement", "org.bluetooth.characteristic.plx_spot_check_measurement"},
	{0x2A50, "PnP ID", "org.bluetooth.characteristic.pnp_id"},
	{0x2A75, "Pollen Concentration", "org.bluetooth.characteristic.pollen_concentration"},
	{0x2A69, "Position Quality", "org.bluetooth.characteristic.position_quality"},
	{0x2A6D, "Pressure", "org.bluetooth.characteristic.pressure"},
	{0x2A4E, "Protocol Mode", "org.bluetooth.characteristic.protocol_mode"},
	{0x2A78, "Rainfall", "org.bluetooth.characteristic.rainfall"},
	{0x2A03, "Reconnection Address", "org.bluetooth.characteristic.gap.reconnection_address"},
	{0x2A52, "Record Access Control Point", "org.bluetooth.characteristic.record_access_control_point"},
	{0x2A14, "Reference Time Information", "org.bluetooth.characteristic.reference_time_information"},
	{0x2A4D, "Report", "org.bluetooth.characteristic.report"},
	{0x2A4B, "Report Map", "org.bluetooth.characteristic.report_map"},
	{0x2A92, "Resting Heart Rate", "org.bluetooth.characteristic.resting_heart_rate"},
	{0x2A40, "Ringer Control Point", "org.bluetooth.characteristic.ringer_control_point"},
	{0x2A41, "Ringer Setting", "org.bluetooth.characteristic.ringer_setting"},
	{0x2A54, "RSC Feature", "org.bluetooth.characteristic.rsc_feature"},
	{0x2A53, "RSC Measurement", "org.bluetooth.characteristic.rsc_measurement"},
	{0x2A55, "SC Control Point", "org.bluetooth.characteristic.sc_control_point"},
	{0x2A4F, "Scan Interval Window", "org.bluetooth.characteristic.scan_interval_window"},
	{0x2A31, "Scan Refresh", "org.bluetooth.characteristic.scan_refresh"},
	{0x2A5D, "Sensor Location", "org.blueooth.characteristic.sensor_location"},
	{0x2A25, "Serial Number String", "org.bluetooth.characteristic.serial_number_string"},
	{0x2A05, "Service Changed", "org.bluetooth.characteristic.gatt.service_changed"},
	{0x2A28, "Software Revision String", "org.bluetooth.characteristic.software_revision_string"},
	{0x2A93, "Sport Type for Aerobic and Anaerobic Thresholds",
		"org.bluetooth.characteristic.sport_type_for_aerobic_and_anaerobic_thresholds"},
	{0x2A47, "Supported New Alert Category", "org.bluetooth.characteristic.supported_new_alert_category"},
	{0x2A48, "Supported Unread Alert Category", "org.bluetooth.characteristic.supported_unread_alert_category"},
	{0x2A23, "System ID", "org.bluetooth.characteristic.system_id"},
	{0x2ABC, "TDS Control Point", "org.bluetooth.characteristic.tds_control_point"},
	{0x2A6E, "Temperature", "org.bluetooth.characteristic.temperature"},
	{0x2A1C, "Temperature Measurement", "org.bluetooth.characteristic.temperature_measurement"},
	{0x2A1D, "Temperature Type", "org.bluetooth.characteristic.temperature_type"},
	{0x2A94, "Three Zone Heart Rate Limits", "org.bluetooth.characteristic.three_zone_heart_rate_limits "},
	{0x2A12, "Time Accuracy", "org.bluetooth.characteristic.time_accuracy"},
	{0x2A13, "Time Source", "org.bluetooth.characteristic.time_source"},
	{0x2A16, "Time Update Control Point", "org.bluetooth.characteristic.time_update_control_point"},
	{0x2A17, "Time Update State", "org.bluetooth.characteristic.time_update_state"},
	{0x2A11, "Time with DST", "org.bluetooth.characteristic.time_with_dst"},
	{0x2A0E, "Time Zone", "org.bluetooth.characteristic.time_zone"},
	{0x2A71, "True Wind Direction", "org.bluetooth.characteristic.true_wind_direction"},
	{0x2A70, "True Wind Speed", "org.bluetooth.characteristic.true_wind_speed"},
	{0x2A95, "Two Zone Heart Rate Limit", "org.bluetooth.characteristic.two_zone_heart_rate_limit "},
	{0x2A07, "Tx Power Level", "org.bluetooth.characteristic.tx_power_level"},
	{0x2AB4, "Uncertainty", "org.bluetooth.characteristic.uncertainty"},
	{0x2A45, "Unread Alert Status", "org.bluetooth.characteristic.unread_alert_status"},
	{0x2AB6, "URI", "org.bluetooth.characteristic.uri"},
	{0x2A9F, "User Control Point", "org.bluetooth.characteristic.user_control_point"},
	{0x2A9A, "User Index", "org.bluetooth.characteristic.user_index "},
	{0x2A76, "UV Index", "org.bluetooth.characteristic.uv_index"},
	{0x2A96, "VO2 Max", "org.bluetooth.characteristic.vo2_max"},
	{0x2A97, "Waist Circumference", "org.bluetooth.characteristic.waist_circumference"},
	{0x2A98, "Weight", "org.bluetooth.characteristic.weight"},
	{0x2A9D, "Weight Measurement", "org.bluetooth.characteristic.weight_measurement"},
	{0x2A9E, "Weight Scale Feature", "org.bluetooth.characteristic.weight_scale_feature"},
	{0x2A79, "Wind Chill", "org.bluetooth.characteristic.wind_chill "},

	/* Descriptors */
	{0x2900, "Characteristic Extended Properties",
		"org.bluetooth.descriptor.gatt.characteristic_extended_properties"},
	{0x2901, "Characteristic User Description", "org.bluetooth.descriptor.gatt.characteristic_user_description"},
	{0x2902, "Client Characteristic Configuration",
		"org.bluetooth.descriptor.gatt.client_characteristic_configuration"},
	{0x2903, "Server Characteristic Configuration",
		"org.bluetooth.descriptor.gatt.server_characteristic_configuration"},
	{0x2904, "Characteristic Presentation Format",
		"org.bluetooth.descriptor.gatt.characteristic_presentation_format"},
	{0x2905, "Characteristic Aggregate Format", "org.bluetooth.descriptor.gatt.characteristic_aggregate_format"},
	{0x2906, "Valid Range", "org.bluetooth.descriptor.valid_range"},
	{0x2907, "External Report Reference", "org.bluetooth.descriptor.external_report_reference"},
	{0x2908, "Report Reference", "org.bluetooth.descriptor.report_reference"},
	{0x2909, "Number of Digitals", "org.bluetooth.descriptor.number_of_digitals"},
	{0x290A, "Value Trigger Setting", "org.bluetooth.descriptor.value_trigger_setting"},
	{0x290B, "Environmental Sensing Configuration", "org.bluetooth.descriptor.es_configuration"},
	{0x290C, "Environmental Sensing Measurement", "org.bluetooth.descriptor.es_measurement"},
	{0x290D, "Environmental Sensing Trigger Setting", "org.bluetooth.descriptor.es_trigger_setting"},
	{0x290E, "Time Trigger Setting", "org.bluetooth.descriptor.time_trigger_setting"},
	{ 0 },
};

const char *bt_company_names[] = {
	//	case 0:
	"Ericsson Technology Licensing",
	//	case 1:
	"Nokia Mobile Phones",
	//	case 2:
	"Intel Corp.",
	//	case 3:
	"IBM Corp.",
	//	case 4:
	"Toshiba Corp.",
	//	case 5:
	"3Com",
	//	case 6:
	"Microsoft",
	//	case 7:
	"Lucent",
	//	case 8:
	"Motorola",
	//	case 9:
	"Infineon Technologies AG",
	//	case 10:
	"Cambridge Silicon Radio",
	//	case 11:
	"Silicon Wave",
	//	case 12:
	"Digianswer A/S",
	//	case 13:
	"Texas Instruments Inc.",
	//	case 14:
	"Ceva, Inc. (formerly Parthus Technologies, Inc.)",
	//	case 15:
	"Broadcom Corporation",
	//	case 16:
	"Mitel Semiconductor",
	//	case 17:
	"Widcomm, Inc",
	//	case 18:
	"Zeevo, Inc.",
	//	case 19:
	"Atmel Corporation",
	//	case 20:
	"Mitsubishi Electric Corporation",
	//	case 21:
	"RTX Telecom A/S",
	//	case 22:
	"KC Technology Inc.",
	//	case 23:
	"NewLogic",
	//	case 24:
	"Transilica, Inc.",
	//	case 25:
	"Rohde & Schwarz GmbH & Co. KG",
	//	case 26:
	"TTPCom Limited",
	//	case 27:
	"Signia Technologies, Inc.",
	//	case 28:
	"Conexant Systems Inc.",
	//	case 29:
	"Qualcomm",
	//	case 30:
	"Inventel",
	//	case 31:
	"AVM Berlin",
	//	case 32:
	"BandSpeed, Inc.",
	//	case 33:
	"Mansella Ltd",
	//	case 34:
	"NEC Corporation",
	//	case 35:
	"WavePlus Technology Co., Ltd.",
	//	case 36:
	"Alcatel",
	//	case 37:
	"NXP Semiconductors (formerly Philips Semiconductors)",
	//	case 38:
	"C Technologies",
	//	case 39:
	"Open Interface",
	//	case 40:
	"R F Micro Devices",
	//	case 41:
	"Hitachi Ltd",
	//	case 42:
	"Symbol Technologies, Inc.",
	//	case 43:
	"Tenovis",
	//	case 44:
	"Macronix International Co. Ltd.",
	//	case 45:
	"GCT Semiconductor",
	//	case 46:
	"Norwood Systems",
	//	case 47:
	"MewTel Technology Inc.",
	//	case 48:
	"ST Microelectronics",
	//	case 49:
	"Synopsys, Inc.",
	//	case 50:
	"Red-M (Communications) Ltd",
	//	case 51:
	"Commil Ltd",
	//	case 52:
	"Computer Access Technology Corporation (CATC)",
	//	case 53:
	"Eclipse (HQ Espana) S.L.",
	//	case 54:
	"Renesas Electronics Corporation",
	//	case 55:
	"Mobilian Corporation",
	//	case 56:
	"Terax",
	//	case 57:
	"Integrated System Solution Corp.",
	//	case 58:
	"Matsushita Electric Industrial Co., Ltd.",
	//	case 59:
	"Gennum Corporation",
	//	case 60:
	"BlackBerry Limited (formerly Research In Motion)",
	//	case 61:
	"IPextreme, Inc.",
	//	case 62:
	"Systems and Chips, Inc.",
	//	case 63:
	"Bluetooth SIG, Inc.",
	//	case 64:
	"Seiko Epson Corporation",
	//	case 65:
	"Integrated Silicon Solution Taiwan, Inc.",
	//	case 66:
	"CONWISE Technology Corporation Ltd",
	//	case 67:
	"PARROT SA",
	//	case 68:
	"Socket Mobile",
	//	case 69:
	"Atheros Communications, Inc.",
	//	case 70:
	"MediaTek, Inc.",
	//	case 71:
	"Bluegiga",
	//	case 72:
	"Marvell Technology Group Ltd.",
	//	case 73:
	"3DSP Corporation",
	//	case 74:
	"Accel Semiconductor Ltd.",
	//	case 75:
	"Continental Automotive Systems",
	//	case 76:
	"Apple, Inc.",
	//	case 77:
	"Staccato Communications, Inc.",
	//	case 78:
	"Avago Technologies",
	//	case 79:
	"APT Licensing Ltd.",
	//	case 80:
	"SiRF Technology",
	//	case 81:
	"Tzero Technologies, Inc.",
	//	case 82:
	"J&M Corporation",
	//	case 83:
	"Free2move AB",
	//	case 84:
	"3DiJoy Corporation",
	//	case 85:
	"Plantronics, Inc.",
	//	case 86:
	"Sony Ericsson Mobile Communications",
	//	case 87:
	"Harman International Industries, Inc.",
	//	case 88:
	"Vizio, Inc.",
	//	case 89:
	"Nordic Semiconductor ASA",
	//	case 90:
	"EM Microelectronic-Marin SA",
	//	case 91:
	"Ralink Technology Corporation",
	//	case 92:
	"Belkin International, Inc.",
	//	case 93:
	"Realtek Semiconductor Corporation",
	//	case 94:
	"Stonestreet One, LLC",
	//	case 95:
	"Wicentric, Inc.",
	//	case 96:
	"RivieraWaves S.A.S",
	//	case 97:
	"RDA Microelectronics",
	//	case 98:
	"Gibson Guitars",
	//	case 99:
	"MiCommand Inc.",
	//	case 100:
	"Band XI International, LLC",
	//	case 101:
	"Hewlett-Packard Company",
	//	case 102:
	"9Solutions Oy",
	//	case 103:
	"GN Netcom A/S",
	//	case 104:
	"General Motors",
	//	case 105:
	"A&D Engineering, Inc.",
	//	case 106:
	"MindTree Ltd.",
	//	case 107:
	"Polar Electro OY",
	//	case 108:
	"Beautiful Enterprise Co., Ltd.",
	//	case 109:
	"BriarTek, Inc.",
	//	case 110:
	"Summit Data Communications, Inc.",
	//	case 111:
	"Sound ID",
	//	case 112:
	"Monster, LLC",
	//	case 113:
	"connectBlue AB",
	//	case 114:
	"ShangHai Super Smart Electronics Co. Ltd.",
	//	case 115:
	"Group Sense Ltd.",
	//	case 116:
	"Zomm, LLC",
	//	case 117:
	"Samsung Electronics Co. Ltd.",
	//	case 118:
	"Creative Technology Ltd.",
	//	case 119:
	"Laird Technologies",
	//	case 120:
	"Nike, Inc.",
	//	case 121:
	"lesswire AG",
	//	case 122:
	"MStar Semiconductor, Inc.",
	//	case 123:
	"Hanlynn Technologies",
	//	case 124:
	"A & R Cambridge",
	//	case 125:
	"Seers Technology Co. Ltd",
	//	case 126:
	"Sports Tracking Technologies Ltd.",
	//	case 127:
	"Autonet Mobile",
	//	case 128:
	"DeLorme Publishing Company, Inc.",
	//	case 129:
	"WuXi Vimicro",
	//	case 130:
	"Sennheiser Communications A/S",
	//	case 131:
	"TimeKeeping Systems, Inc.",
	//	case 132:
	"Ludus Helsinki Ltd.",
	//	case 133:
	"BlueRadios, Inc.",
	//	case 134:
	"equinox AG",
	//	case 135:
	"Garmin International, Inc.",
	//	case 136:
	"Ecotest",
	//	case 137:
	"GN ReSound A/S",
	//	case 138:
	"Jawbone",
	//	case 139:
	"Topcon Positioning Systems, LLC",
	//	case 140:
	"Gimbal Inc. (formerly Qualcomm Labs, Inc. and Qualcomm Retail Solutions, Inc.)",
	//	case 141:
	"Zscan Software",
	//	case 142:
	"Quintic Corp.",
	//	case 143:
	"Telit Wireless Solutions GmbH (Formerly Stollman E+V GmbH)",
	//	case 144:
	"Funai Electric Co., Ltd.",
	//	case 145:
	"Advanced PANMOBIL Systems GmbH & Co. KG",
	//	case 146:
	"ThinkOptics, Inc.",
	//	case 147:
	"Universal Electronics, Inc.",
	//	case 148:
	"Airoha Technology Corp.",
	//	case 149:
	"NEC Lighting, Ltd.",
	//	case 150:
	"ODM Technology, Inc.",
	//	case 151:
	"ConnecteDevice Ltd.",
	//	case 152:
	"zer01.tv GmbH",
	//	case 153:
	"i.Tech Dynamic Global Distribution Ltd.",
	//	case 154:
	"Alpwise",
	//	case 155:
	"Jiangsu Toppower Automotive Electronics Co., Ltd.",
	//	case 156:
	"Colorfy, Inc.",
	//	case 157:
	"Geoforce Inc.",
	//	case 158:
	"Bose Corporation",
	//	case 159:
	"Suunto Oy",
	//	case 160:
	"Kensington Computer Products Group",
	//	case 161:
	"SR-Medizinelektronik",
	//	case 162:
	"Vertu Corporation Limited",
	//	case 163:
	"Meta Watch Ltd.",
	//	case 164:
	"LINAK A/S",
	//	case 165:
	"OTL Dynamics LLC",
	//	case 166:
	"Panda Ocean Inc.",
	//	case 167:
	"Visteon Corporation",
	//	case 168:
	"ARP Devices Limited",
	//	case 169:
	"Magneti Marelli S.p.A",
	//	case 170:
	"CAEN RFID srl",
	//	case 171:
	"Ingenieur-Systemgruppe Zahn GmbH",
	//	case 172:
	"Green Throttle Games",
	//	case 173:
	"Peter Systemtechnik GmbH",
	//	case 174:
	"Omegawave Oy",
	//	case 175:
	"Cinetix",
	//	case 176:
	"Passif Semiconductor Corp",
	//	case 177:
	"Saris Cycling Group, Inc",
	//	case 178:
	"Bekey A/S",
	//	case 179:
	"Clarinox Technologies Pty. Ltd.",
	//	case 180:
	"BDE Technology Co., Ltd.",
	//	case 181:
	"Swirl Networks",
	//	case 182:
	"Meso international",
	//	case 183:
	"TreLab Ltd",
	//	case 184:
	"Qualcomm Innovation Center, Inc. (QuIC)",
	//	case 185:
	"Johnson Controls, Inc.",
	//	case 186:
	"Starkey Laboratories Inc.",
	//	case 187:
	"S-Power Electronics Limited",
	//	case 188:
	"Ace Sensor Inc",
	//	case 189:
	"Aplix Corporation",
	//	case 190:
	"AAMP of America",
	//	case 191:
	"Stalmart Technology Limited",
	//	case 192:
	"AMICCOM Electronics Corporation",
	//	case 193:
	"Shenzhen Excelsecu Data Technology Co.,Ltd",
	//	case 194:
	"Geneq Inc.",
	//	case 195:
	"adidas AG",
	//	case 196:
	"LG Electronics",
	//	case 197:
	"Onset Computer Corporation",
	//	case 198:
	"Selfly BV",
	//	case 199:
	"Quuppa Oy.",
	//	case 200:
	"GeLo Inc",
	//	case 201:
	"Evluma",
	//	case 202:
	"MC10",
	//	case 203:
	"Binauric SE",
	//	case 204:
	"Beats Electronics",
	//	case 205:
	"Microchip Technology Inc.",
	//	case 206:
	"Elgato Systems GmbH",
	//	case 207:
	"ARCHOS SA",
	//	case 208:
	"Dexcom, Inc.",
	//	case 209:
	"Polar Electro Europe B.V.",
	//	case 210:
	"Dialog Semiconductor B.V.",
	//	case 211:
	"Taixingbang Technology (HK) Co,. LTD.",
	//	case 212:
	"Kawantech",
	//	case 213:
	"Austco Communication Systems",
	//	case 214:
	"Timex Group USA, Inc.",
	//	case 215:
	"Qualcomm Technologies, Inc.",
	//	case 216:
	"Qualcomm Connected Experiences, Inc.",
	//	case 217:
	"Voyetra Turtle Beach",
	//	case 218:
	"txtr GmbH",
	//	case 219:
	"Biosentronics",
	//	case 220:
	"Procter & Gamble",
	//	case 221:
	"Hosiden Corporation",
	//	case 222:
	"Muzik LLC",
	//	case 223:
	"Misfit Wearables Corp",
	//	case 224:
	"Google",
	//	case 225:
	"Danlers Ltd",
	//	case 226:
	"Semilink Inc",
	//	case 227:
	"inMusic Brands, Inc",
	//	case 228:
	"L.S. Research Inc.",
	//	case 229:
	"Eden Software Consultants Ltd.",
	//	case 230:
	"Freshtemp",
	//	case 231:
	"KS Technologies",
	//	case 232:
	"ACTS Technologies",
	//	case 233:
	"Vtrack Systems",
	//	case 234:
	"Nielsen-Kellerman Company",
	//	case 235:
	"Server Technology, Inc.",
	//	case 236:
	"BioResearch Associates",
	//	case 237:
	"Jolly Logic, LLC",
	//	case 238:
	"Above Average Outcomes, Inc.",
	//	case 239:
	"Bitsplitters GmbH",
	//	case 240:
	"PayPal, Inc.",
	//	case 241:
	"Witron Technology Limited",
	//	case 242:
	"Aether Things Inc. (formerly Morse Project Inc.)",
	//	case 243:
	"Kent Displays Inc.",
	//	case 244:
	"Nautilus Inc.",
	//	case 245:
	"Smartifier Oy",
	//	case 246:
	"Elcometer Limited",
	//	case 247:
	"VSN Technologies Inc.",
	//	case 248:
	"AceUni Corp., Ltd.",
	//	case 249:
	"StickNFind",
	//	case 250:
	"Crystal Code AB",
	//	case 251:
	"KOUKAAM a.s.",
	//	case 252:
	"Delphi Corporation",
	//	case 253:
	"ValenceTech Limited",
	//	case 254:
	"Reserved",
	//	case 255:
	"Typo Products, LLC",
	//	case 256:
	"TomTom International BV",
	//	case 257:
	"Fugoo, Inc",
	//	case 258:
	"Keiser Corporation",
	//	case 259:
	"Bang & Olufsen A/S",
	//	case 260:
	"PLUS Locations Systems Pty Ltd",
	//	case 261:
	"Ubiquitous Computing Technology Corporation",
	//	case 262:
	"Innovative Yachtter Solutions",
	//	case 263:
	"William Demant Holding A/S",
	//	case 264:
	"Chicony Electronics Co., Ltd.",
	//	case 265:
	"Atus BV",
	//	case 266:
	"Codegate Ltd.",
	//	case 267:
	"ERi, Inc.",
	//	case 268:
	"Transducers Direct, LLC",
	//	case 269:
	"Fujitsu Ten Limited",
	//	case 270:
	"Audi AG",
	//	case 271:
	"HiSilicon Technologies Co., Ltd.",
	//	case 272:
	"Nippon Seiki Co., Ltd.",
	//	case 273:
	"Steelseries ApS",
	//	case 274:
	"Visybl Inc.",
	//	case 275:
	"Openbrain Technologies, Co., Ltd.",
	//	case 276:
	"Xensr",
	//	case 277:
	"e.solutions",
	//	case 278:
	"1OAK Technologies",
	//	case 279:
	"Wimoto Technologies Inc",
	//	case 280:
	"Radius Networks, Inc.",
	//	case 281:
	"Wize Technology Co., Ltd.",
	//	case 282:
	"Qualcomm Labs, Inc.",
	//	case 283:
	"Aruba Networks",
	//	case 284:
	"Baidu",
	//	case 285:
	"Arendi AG",
	//	case 286:
	"Skoda Auto a.s.",
	//	case 287:
	"Volkswagen AG",
	//	case 288:
	"Porsche AG",
	//	case 289:
	"Sino Wealth Electronic Ltd.",
	//	case 290:
	"AirTurn, Inc.",
	//	case 291:
	"Kinsa, Inc.",
	//	case 292:
	"HID Global",
	//	case 293:
	"SEAT es",
	//	case 294:
	"Promethean Ltd.",
	//	case 295:
	"Salutica Allied Solutions",
	//	case 296:
	"GPSI Group Pty Ltd",
	//	case 297:
	"Nimble Devices Oy",
	//	case 298:
	"Changzhou Yongse Infotech Co., Ltd",
	//	case 299:
	"SportIQ",
	//	case 300:
	"TEMEC Instruments B.V.",
	//	case 301:
	"Sony Corporation",
	//	case 302:
	"ASSA ABLOY",
	//	case 303:
	"Clarion Co., Ltd.",
	//	case 304:
	"Warehouse Innovations",
	//	case 305:
	"Cypress Semiconductor Corporation",
	//	case 306:
	"MADS Inc",
	//	case 307:
	"Blue Maestro Limited",
	//	case 308:
	"Resolution Products, Inc.",
	//	case 309:
	"Airewear LLC",
	//	case 310:
	"Seed Labs, Inc. (formerly ETC sp. z.o.o.)",
	//	case 311:
	"Prestigio Plaza Ltd.",
	//	case 312:
	"NTEO Inc.",
	//	case 313:
	"Focus Systems Corporation",
	//	case 314:
	"Tencent Holdings Limited",
	//	case 315:
	"Allegion",
	//	case 316:
	"Murata Manufacuring Co., Ltd.",
	//	case 317:
	"WirelessWERX",
	//	case 318:
	"Nod, Inc.",
	//	case 319:
	"B&B Manufacturing Company",
	//	case 320:
	"Alpine Electronics (China) Co., Ltd",
	//	case 321:
	"FedEx Services",
	//	case 322:
	"Grape Systems Inc.",
	//	case 323:
	"Bkon Connect",
	//	case 324:
	"Lintech GmbH",
	//	case 325:
	"Novatel Wireless",
	//	case 326:
	"Ciright",
	//	case 327:
	"Mighty Cast, Inc.",
	//	case 328:
	"Ambimat Electronics",
	//	case 329:
	"Perytons Ltd.",
	//	case 330:
	"Tivoli Audio, LLC",
	//	case 331:
	"Master Lock",
	//	case 332:
	"Mesh-Net Ltd",
	//	case 333:
	"Huizhou Desay SV Automotive CO., LTD.",
	//	case 334:
	"Tangerine, Inc.",
	//	case 335:
	"B&W Group Ltd.",
	//	case 336:
	"Pioneer Corporation",
	//	case 337:
	"OnBeep",
	//	case 338:
	"Vernier Software & Technology",
	//	case 339:
	"ROL Ergo",
	//	case 340:
	"Pebble Technology",
	//	case 341:
	"NETATMO",
	//	case 342:
	"Accumulate AB",
	//	case 343:
	"Anhui Huami Information Technology Co., Ltd.",
	//	case 344:
	"Inmite s.r.o.",
	//	case 345:
	"ChefSteps, Inc.",
	//	case 346:
	"micas AG",
	//	case 347:
	"Biomedical Research Ltd.",
	//	case 348:
	"Pitius Tec S.L.",
	//	case 349:
	"Estimote, Inc.",
	//	case 350:
	"Unikey Technologies, Inc.",
	//	case 351:
	"Timer Cap Co.",
	//	case 352:
	"AwoX",
	//	case 353:
	"yikes",
	//	case 354:
	"MADSGlobal NZ Ltd.",
	//	case 355:
	"PCH International",
	//	case 356:
	"Qingdao Yeelink Information Technology Co., Ltd.",
	//	case 357:
	"Milwaukee Tool (formerly Milwaukee Electric Tools)",
	//	case 358:
	"MISHIK Pte Ltd",
	//	case 359:
	"Bayer HealthCare",
	//	case 360:
	"Spicebox LLC",
	//	case 361:
	"emberlight",
	//	case 362:
	"Cooper-Atkins Corporation",
	//	case 363:
	"Qblinks",
	//	case 364:
	"MYSPHERA",
	//	case 365:
	"LifeScan Inc",
	//	case 366:
	"Volantic AB",
	//	case 367:
	"Podo Labs, Inc",
	//	case 368:
	"F. Hoffmann-La Roche AG",
	//	case 369:
	"Amazon Fulfillment Service",
	//	case 370:
	"Connovate Technology Private Limited",
	//	case 371:
	"Kocomojo, LLC",
	//	case 372:
	"Everykey LLC",
	//	case 373:
	"Dynamic Controls",
	//	case 374:
	"SentriLock",
	//	case 375:
	"I-SYST inc.",
	//	case 376:
	"CASIO COMPUTER CO., LTD.",
	//	case 377:
	"LAPIS Semiconductor Co., Ltd.",
	//	case 378:
	"Telemonitor, Inc.",
	//	case 379:
	"taskit GmbH",
	//	case 380:
	"Daimler AG",
	//	case 381:
	"BatAndCat",
	//	case 382:
	"BluDotz Ltd",
	//	case 383:
	"XTel ApS",
	//	case 384:
	"Gigaset Communications GmbH",
	//	case 385:
	"Gecko Health Innovations, Inc.",
	//	case 386:
	"HOP Ubiquitous",
	//	case 387:
	"To Be Assigned",
	//	case 388:
	"Nectar",
	//	case 389:
	"bel'apps LLC",
	//	case 390:
	"CORE Lighting Ltd",
	//	case 391:
	"Seraphim Sense Ltd",
	//	case 392:
	"Unico RBC",
	//	case 393:
	"Physical Enterprises Inc.",
	//	case 394:
	"Able Trend Technology Limited",
	//	case 395:
	"Konica Minolta, Inc.",
	//	case 396:
	"Wilo SE",
	//	case 397:
	"Extron Design Services",
	//	case 398:
	"Fitbit, Inc.",
	//	case 399:
	"Fireflies Systems",
	//	case 400:
	"Intelletto Technologies Inc.",
	//	case 401:
	"FDK CORPORATION",
	//	case 402:
	"Cloudleaf, Inc",
	//	case 403:
	"Maveric Automation LLC",
	//	case 404:
	"Acoustic Stream Corporation",
	//	case 405:
	"Zuli",
	//	case 406:
	"Paxton Access Ltd",
	//	case 407:
	"WiSilica Inc",
	//	case 408:
	"VENGIT Korlátolt Felelősségű Társaság",
	//	case 409:
	"SALTO SYSTEMS S.L.",
	//	case 410:
	"TRON Forum (formerly T-Engine Forum)",
	//	case 411:
	"CUBETECH s.r.o.",
	//	case 412:
	"Cokiya Incorporated",
	//	case 413:
	"CVS Health",
	//	case 414:
	"Ceruus",
	//	case 415:
	"Strainstall Ltd",
	//	case 416:
	"Channel Enterprises (HK) Ltd.",
	//	case 417:
	"FIAMM",
	//	case 418:
	"GIGALANE.CO.,LTD",
	//	case 419:
	"EROAD",
	//	case 420:
	"Mine Safety Appliances",
	//	case 421:
	"Icon Health and Fitness",
	//	case 422:
	"Asandoo GmbH",
	//	case 423:
	"ENERGOUS CORPORATION",
	//	case 424:
	"Taobao",
	//	case 425:
	"Canon Inc.",
	//	case 426:
	"Geophysical Technology Inc.",
	//	case 427:
	"Facebook, Inc.",
	//	case 428:
	"Nipro Diagnostics, Inc.",
	//	case 429:
	"FlightSafety International",
	//	case 430:
	"Earlens Corporation",
	//	case 431:
	"Sunrise Micro Devices, Inc.",
	//	case 432:
	"Star Micronics Co., Ltd.",
	//	case 433:
	"Netizens Sp. z o.o.",
	//	case 434:
	"Nymi Inc.",
	//	case 435:
	"Nytec, Inc.",
	//	case 436:
	"Trineo Sp. z o.o.",
	//	case 437:
	"Nest Labs Inc.",
	//	case 438:
	"LM Technologies Ltd",
	//	case 439:
	"General Electric Company",
	//	case 440:
	"i+D3 S.L.",
	//	case 441:
	"HANA Micron",
	//	case 442:
	"Stages Cycling LLC",
	//	case 443:
	"Cochlear Bone Anchored Solutions AB",
	//	case 444:
	"SenionLab AB",
	//	case 445:
	"Syszone Co., Ltd",
	//	case 446:
	"Pulsate Mobile Ltd.",
	//	case 447:
	"Hong Kong HunterSun Electronic Limited",
	//	case 448:
	"pironex GmbH",
	//	case 449:
	"BRADATECH Corp.",
	//	case 450:
	"Transenergooil AG",
	//	case 451:
	"Bunch",
	//	case 452:
	"DME Microelectronics",
	//	case 453:
	"Bitcraze AB",
	//	case 454:
	"HASWARE Inc.",
	//	case 455:
	"Abiogenix Inc.",
	//	case 456:
	"Poly-Control ApS",
	//	case 457:
	"Avi-on",
	//	case 458:
	"Laerdal Medical AS",
	//	case 459:
	"Fetch My Pet",
	//	case 460:
	"Sam Labs Ltd.",
	//	case 461:
	"Chengdu Synwing Technology Ltd",
	//	case 462:
	"HOUWA SYSTEM DESIGN, k.k.",
	//	case 463:
	"BSH",
	//	case 464:
	"Primus Inter Pares Ltd",
	//	case 465:
	"August",
	//	case 466:
	"Gill Electronics",
	//	case 467:
	"Sky Wave Design",
	//	case 468:
	"Newlab S.r.l.",
	//	case 469:
	"ELAD srl",
	//	case 470:
	"G-wearables inc.",
	//	case 471:
	"Squadrone Systems Inc.",
	//	case 472:
	"Code Corporation",
	//	case 473:
	"Savant Systems LLC",
	//	case 474:
	"Logitech International SA",
	//	case 475:
	"Innblue Consulting",
	//	case 476:
	"iParking Ltd.",
	//	case 477:
	"Koninklijke Philips Electronics N.V.",
	//	case 478:
	"Minelab Electronics Pty Limited",
	//	case 479:
	"Bison Group Ltd.",
	//	case 480:
	"Widex A/S",
	//	case 481:
	"Jolla Ltd",
	//	case 482:
	"Lectronix, Inc.",
	//	case 483:
	"Caterpillar Inc",
	//	case 484:
	"Freedom Innovations",
	//	case 485:
	"Dynamic Devices Ltd",
	//	case 486:
	"Technology Solutions (UK) Ltd",
	//	case 487:
	"IPS Group Inc.",
	//	case 488:
	"STIR",
	//	case 489:
	"Sano, Inc",
	//	case 490:
	"Advanced Application Design, Inc.",
	//	case 491:
	"AutoMap LLC",
	//	case 492:
	"Spreadtrum Communications Shanghai Ltd",
	//	case 493:
	"CuteCircuit LTD",
	//	case 494:
	"Valeo Service",
	//	case 495:
	"Fullpower Technologies, Inc.",
	//	case 496:
	"KloudNation",
	//	case 497:
	"Zebra Technologies Corporation",
	//	case 498:
	"Itron, Inc.",
	//	case 499:
	"The University of Tokyo",
	//	case 500:
	"UTC Fire and Security",
	//	case 501:
	"Cool Webthings Limited",
	//	case 502:
	"DJO Global",
	//	case 503:
	"Gelliner Limited",
	//	case 504:
	"Anyka (Guangzhou) Microelectronics Technology Co, LTD",
	//	case 505:
	"Medtronic, Inc.",
	//	case 506:
	"Gozio, Inc.",
	//	case 507:
	"Form Lifting, LLC",
	//	case 508:
	"Wahoo Fitness, LLC",
	//	case 509:
	"Kontakt Micro-Location Sp. z o.o.",
	//	case 510:
	"Radio System Corporation",
	//	case 511:
	"Freescale Semiconductor, Inc.",
	//	case 512:
	"Verifone Systems PTe Ltd. Taiwan Branch",
	//	case 513:
	"AR Timing",
	//	case 514:
	"Rigado LLC",
	//	case 515:
	"Kemppi Oy",
	//	case 516:
	"Tapcentive Inc.",
	//	case 517:
	"Smartbotics Inc.",
	//	case 518:
	"Otter Products, LLC",
	//	case 519:
	"STEMP Inc.",
	//	case 520:
	"LumiGeek LLC",
	//	case 521:
	"InvisionHeart Inc.",
	//	case 522:
	"Macnica Inc.",
	//	case 523:
	"Jaguar Land Rover Limited",
	//	case 524:
	"CoroWare Technologies, Inc",
	//	case 525:
	"Simplo Technology Co., LTD",
	//	case 526:
	"Omron Healthcare Co., LTD",
	//	case 527:
	"Comodule GMBH",
	//	case 528:
	"ikeGPS",
	//	case 529:
	"Telink Semiconductor Co. Ltd",
	//	case 530:
	"Interplan Co., Ltd",
	//	case 531:
	"Wyler AG",
	//	case 532:
	"IK Multimedia Production srl",
	//	case 533:
	"Lukoton Experience Oy",
	//	case 534:
	"MTI Ltd",
	//	case 535:
	"Tech4home, Lda",
	//	case 536:
	"Hiotech AB",
	//	case 537:
	"DOTT Limited",
	//	case 538:
	"Blue Speck Labs, LLC",
	//	case 539:
	"Cisco Systems Inc",
	//	case 540:
	"Mobicomm Inc",
	//	case 541:
	"Edamic",
	//	case 542:
	"Goodnet Ltd",
	//	case 543:
	"Luster Leaf Products Inc",
	//	case 544:
	"Manus Machina BV",
	//	case 545:
	"Mobiquity Networks Inc",
	//	case 546:
	"Praxis Dynamics",
	//	case 547:
	"Philip Morris Products S.A.",
	//	case 548:
	"Comarch SA",
	//	case 549:
	"Nestlé Nespresso S.A.",
	//	case 550:
	"Merlinia A/S",
	//	case 551:
	"LifeBEAM Technologies",
	//	case 552:
	"Twocanoes Labs, LLC",
	//	case 553:
	"Muoverti Limited",
	//	case 554:
	"Stamer Musikanlagen GMBH",
	//	case 555:
	"Tesla Motors",
	//	case 556:
	"Pharynks Corporation",
	//	case 557:
	"Lupine",
	//	case 558:
	"Siemens AG",
	//	case 559:
	"Huami (Shanghai) Culture Communication CO., LTD",
	//	case 560:
	"Foster Electric Company, Ltd",
	//	case 561:
	"ETA SA",
	//	case 562:
	"x-Senso Solutions Kft",
	//	case 563:
	"Shenzhen SuLong Communication Ltd",
	//	case 564:
	"FengFan (BeiJing) Technology Co, Ltd",
	//	case 565:
	"Qrio Inc",
	//	case 566:
	"Pitpatpet Ltd",
	//	case 567:
	"MSHeli s.r.l.",
	//	case 568:
	"Trakm8 Ltd",
	//	case 569:
	"JIN CO, Ltd",
	//	case 570:
	"Alatech Technology",
	//	case 571:
	"Beijing CarePulse Electronic Technology Co, Ltd",
	//	case 572:
	"Awarepoint",
	//	case 573:
	"ViCentra B.V.",
	//	case 574:
	"Raven Industries",
	//	case 575:
	"WaveWare Technologies",
	//	case 576:
	"Argenox Technologies",
	//	case 577:
	"Bragi GmbH",
	//	case 578:
	"16Lab Inc",
	//	case 579:
	"Masimo Corp",
	//	case 580:
	"Iotera Inc.",
	//	case 581:
	"Endress+Hauser",
	//	case 582:
	"ACKme Networks, Inc.",
	//	case 583:
	"FiftyThree Inc.",
	//	case 584:
	"Parker Hannifin Corp",
	//	case 585:
	"Transcranial Ltd",
	//	case 586:
	"Uwatec AG",
	//	case 587:
	"Orlan LLC",
	//	case 588:
	"Blue Clover Devices",
	//	case 589:
	"M-Way Solutions GmbH",
	//	case 590:
	"Microtronics Engineering GmbH",
	//	case 591:
	"Schneider Schreibgeräte GmbH",
	//	case 592:
	"Sapphire Circuits LLC",
	//	case 593:
	"Lumo Bodytech Inc.",
	//	case 594:
	"UKC Technosolution",
	//	case 595:
	"Xicato Inc.",
	//	case 596:
	"Playbrush",
	//	case 597:
	"Dai Nippon Printing Co., Ltd.",
	//	case 598:
	"G24 Power Limited",
	//	case 599:
	"AdBabble Local Commerce Inc.",
	//	case 600:
	"Devialet SA",
	//	case 601:
	"ALTYOR",
	//	case 602:
	"University of Applied Sciences Valais/Haute Ecole Valaisanne",
	//	case 603:
	"Five Interactive, LLC dba Zendo",
	//	case 604:
	"NetEase (Hangzhou) Network co.Ltd.",
	//	case 605:
	"Lexmark International Inc.",
	//	case 606:
	"Fluke Corporation",
	//	case 607:
	"Yardarm Technologies",
	//	case 608:
	"SensaRx",
	//	case 609:
	"SECVRE GmbH",
	//	case 610:
	"Glacial Ridge Technologies",
	//	case 611:
	"Identiv, Inc.",
	//	case 612:
	"DDS, Inc.",
	//	case 613:
	"SMK Corporation",
	//	case 614:
	"Schawbel Technologies LLC",
	//	case 615:
	"XMI Systems SA",
	//	case 616:
	"Cerevo",
	//	case 617:
	"Torrox GmbH & Co KG",
	//	case 618:
	"Gemalto",
	//	case 619:
	"DEKA Research & Development Corp.",
	//	case 620:
	"Domster Tadeusz Szydlowski",
	//	case 621:
	"Technogym SPA",
	//	case 622:
	"FLEURBAEY BVBA",
	//	case 623:
	"Aptcode Solutions",
	//	case 624:
	"LSI ADL Technology",
	//	case 625:
	"Animas Corp",
	//	case 626:
	"Alps Electric Co., Ltd.",
	//	case 627:
	"OCEASOFT",
	//	case 628:
	"Motsai Research",
	//	case 629:
	"Geotab",
	//	case 630:
	"E.G.O. Elektro-Gerätebau GmbH",
	//	case 631:
	"bewhere inc",
	//	case 632:
	"Johnson Outdoors Inc",
	//	case 633:
	"steute Schaltgerate GmbH & Co. KG",
	//	case 634:
	"Ekomini inc.",
	//	case 635:
	"DEFA AS",
	//	case 636:
	"Aseptika Ltd",
	//	case 637:
	"HUAWEI Technologies Co., Ltd. ( 华为技术有限公司 )",
	//	case 638:
	"HabitAware, LLC",
	//	case 639:
	"ruwido austria gmbh",
	//	case 640:
	"ITEC corporation",
	//	case 641:
	"StoneL",
	//	case 642:
	"Sonova AG",
	//	case 643:
	"Maven Machines, Inc.",
	//	case 644:
	"Synapse Electronics",
	//	case 645:
	"Standard Innovation Inc.",
	//	case 646:
	"RF Code, Inc.",
	//	case 647:
	"Wally Ventures S.L.",
	//	case 648:
	"Willowbank Electronics Ltd",
	//	case 649:
	"SK Telecom",
	//	case 650:
	"Jetro AS",
	//	case 651:
	"Code Gears LTD",
	//	case 652:
	"NANOLINK APS",
	//	case 653:
	"IF, LLC",
	//	case 654:
	"RF Digital Corp",
	//	case 655:
	"Church & Dwight Co., Inc",
	//	case 656:
	"Multibit Oy",
	//	case 657:
	"CliniCloud Inc",
	//	case 658:
	"SwiftSensors",
	//	case 659:
	"Blue Bite",
	//	case 660:
	"ELIAS GmbH",
	//	case 661:
	"Sivantos GmbH",
	//	case 662:
	"Petzl",
	//	case 663:
	"storm power ltd",
	//	case 664:
	"EISST Ltd",
	//	case 665:
	"Inexess Technology Simma KG",
	//	case 666:
	"Currant, Inc.",
	//	case 667:
	"C2 Development, Inc.",
	//	case 668:
	"Blue Sky Scientific, LLC",
	//	case 669:
	"ALOTTAZS LABS, LLC",
	//	case 670:
	"Kupson spol. s r.o.",
	//	case 671:
	"Areus Engineering GmbH",
	//	case 672:
	"Impossible Camera GmbH",
	//	case 673:
	"InventureTrack Systems",
	//	case 674:
	"LockedUp",
	//	case 675:
	"Itude",
	//	case 676:
	"Pacific Lock Company",
	//	case 677:
	"Tendyron Corporation ( 天地融科技股份有限公司 )",
	//	case 678:
	"Robert Bosch GmbH",
	//	case 679:
	"Illuxtron international B.V.",
	//	case 680:
	"miSport Ltd.",
	//	case 681:
	"Chargelib",
	//	case 682:
	"Doppler Lab",
	//	case 683:
	"BBPOS Limited",
	//	case 684:
	"RTB Elektronik GmbH & Co. KG",
	//	case 685:
	"Rx Networks, Inc.",
	//	case 686:
	"WeatherFlow, Inc.",
	//	case 687:
	"Technicolor USA Inc.",
	//	case 688:
	"Bestechnic(Shanghai),Ltd",
	//	case 689:
	"Raden Inc",
	//	case 690:
	"JouZen Oy",
	//	case 691:
	"CLABER S.P.A.",
	//	case 692:
	"Hyginex, Inc.",
	//	case 693:
	"HANSHIN ELECTRIC RAILWAY CO.,LTD.",
	//	case 694:
	"Schneider Electric",
	//	case 695:
	"Oort Technologies LLC",
	//	case 696:
	"Chrono Therapeutics",
	//	case 697:
	"Rinnai Corporation",
	//	case 698:
	"Swissprime Technologies AG",
	//	case 699:
	"Koha.,Co.Ltd",
	//	case 700:
	"Genevac Ltd",
	//	case 701:
	"Chemtronics",
	//	case 702:
	"Seguro Technology Sp. z o.o.",
	//	case 703:
	"Redbird Flight Simulations",
	//	case 704:
	"Dash Robotics",
	//	case 705:
	"LINE Corporation",
	//	case 706:
	"Guillemot Corporation",
	//	case 707:
	"Techtronic Power Tools Technology Limited",
	//	case 708:
	"Wilson Sporting Goods",
	//	case 709:
	"Lenovo (Singapore) Pte Ltd. ( 联想（新加坡） )",
	//	case 710:
	"Ayatan Sensors",
	//	case 711:
	"Electronics Tomorrow Limited",
	//	case 712:
	"VASCO Data Security International, Inc.",
	//	case 713:
	"PayRange Inc.",
	//	case 714:
	"ABOV Semiconductor",
	//	case 715:
	"AINA-Wireless Inc.",
	//	case 716:
	"Eijkelkamp Soil & Water",
	//	case 717:
	"BMA ergonomics b.v.",
	//	case 718:
	"Teva Branded Pharmaceutical Products R&D, Inc.",
	//	case 719:
	"Anima",
	//	case 720:
	"3M",
	//	case 721:
	"Empatica Srl",
	//	case 722:
	"Afero, Inc.",
	//	case 723:
	"Powercast Corporation",
	//	case 724:
	"Secuyou ApS",
	//	case 725:
	"OMRON Corporation",
	//	case 726:
	"Send Solutions",
	//	case 727:
	"NIPPON SYSTEMWARE CO.,LTD.",
	//	case 728:
	"Neosfar",
	//	case 729:
	"Fliegl Agrartechnik GmbH",
	//	case 730:
	"Gilvader",
	//	case 731:
	"Digi International Inc (R)",
	//	case 732:
	"DeWalch Technologies, Inc.",
	//	case 733:
	"Flint Rehabilitation Devices, LLC",
	//	case 734:
	"Samsung SDS Co., Ltd.",
	//	case 735:
	"Blur Product Development",
	//	case 736:
	"University of Michigan",
	//	case 737:
	"Victron Energy BV",
	//	case 738:
	"NTT docomo",
	//	case 739:
	"Carmanah Technologies Corp.",
	//	case 740:
	"Bytestorm Ltd.",
	//	case 741:
	"Espressif Incorporated ( 乐鑫信息科技(上海)有限公司 )",
	//	case 742:
	"Unwire",
	//	case 743:
	"Connected Yard, Inc.",
	//	case 744:
	"American Music Environments",
	//	case 745:
	"Sensogram Technologies, Inc.",
	//	case 746:
	"Fujitsu Limited",
	//	case 747:
	"Ardic Technology",
	//	case 748:
	"Delta Systems, Inc",
	//	case 749:
	"HTC Corporation",
	//	case 750:
	"Citizen Holdings Co., Ltd.",
	//	case 751:
	"SMART-INNOVATION.inc",
	//	case 752:
	"Blackrat Software",
	//	case 753:
	"The Idea Cave, LLC",
	//	case 754:
	"GoPro, Inc.",
	//	case 755:
	"AuthAir, Inc",
	//	case 756:
	"Vensi, Inc.",
	//	case 757:
	"Indagem Tech LLC",
	//	case 758:
	"Intemo Technologies",
	//	case 759:
	"DreamVisions co., Ltd.",
	//	case 760:
	"Runteq Oy Ltd",
	//	case 761:
	"IMAGINATION TECHNOLOGIES LTD",
	//	case 762:
	"CoSTAR Technologies",
	//	case 763:
	"Clarius Mobile Health Corp.",
	//	case 764:
	"Shanghai Frequen Microelectronics Co., Ltd.",
	//	case 765:
	"Uwanna, Inc.",
	//	case 766:
	"Lierda Science & Technology Group Co., Ltd.",
	//	case 767:
	"Silicon Laboratories",
	//	case 768:
	"World Moto Inc.",
	//	case 769:
	"Giatec Scientific Inc.",
	//	case 770:
	"Loop Devices, Inc",
	//	case 771:
	"IACA electronique",
	//	case 772:
	"Martians Inc",
	//	case 773:
	"Swipp ApS",
	//	case 774:
	"Life Laboratory Inc.",
	//	case 775:
	"FUJI INDUSTRIAL CO.,LTD.",
	//	case 776:
	"Surefire, LLC",
	//	case 777:
	"Dolby Labs",
	//	case 778:
	"Ellisys",
	//	case 779:
	"Magnitude Lighting Converters",
	//	case 780:
	"Hilti AG",
	//	case 781:
	"Devdata S.r.l.",
	//	case 782:
	"Deviceworx",
	//	case 783:
	"Shortcut Labs",
	//	case 784:
	"SGL Italia S.r.l.",
	//	case 785:
	"PEEQ DATA",
	//	case 786:
	"Ducere Technologies Pvt Ltd",
	//	case 787:
	"DiveNav, Inc.",
	//	case 788:
	"RIIG AI Sp. z o.o.",
	//	case 789:
	"Thermo Fisher Scientific",
	//	case 790:
	"AG Measurematics Pvt. Ltd.",
	//	case 791:
	"CHUO Electronics CO., LTD.",
	//	case 792:
	"Aspenta International",
	//	case 793:
	"Eugster Frismag AG",
	//	case 794:
	"Amber wireless GmbH",
	//	case 795:
	"HQ Inc",
	//	case 796:
	"Lab Sensor Solutions",
	//	case 797:
	"Enterlab ApS",
	//	case 798:
	"Eyefi, Inc.",
	//	case 799:
	"MetaSystem S.p.A",
	//	case 800:
	"SONO ELECTRONICS. CO., LTD",
	//	case 801:
	"Jewelbots",
	//	case 802:
	"Compumedics Limited",
	//	case 803:
	"Rotor Bike Components",
	//	case 804:
	"Astro, Inc.",
	//	case 805:
	"Amotus Solutions",
	//	case 806:
	"Healthwear Technologies (Changzhou)Ltd",
	//	case 807:
	"Essex Electronics",
	//	case 808:
	"Grundfos A/S",
	//	case 809:
	"Eargo, Inc.",
	//	case 810:
	"Electronic Design Lab",
	//	case 811:
	"ESYLUX",
	//	case 812:
	"NIPPON SMT.CO.,Ltd",
	//	case 813:
	"BM innovations GmbH",
	//	case 814:
	"indoormap",
	//	case 815:
	"OttoQ Inc",
	//	case 816:
	"North Pole Engineering",
	//	case 817:
	"3flares Technologies Inc.",
	//	case 818:
	"Electrocompaniet A.S.",
	//	case 819:
	"Mul-T-Lock",
	//	case 820:
	"Corentium AS",
	//	case 821:
	"Enlighted Inc",
	//	case 822:
	"GISTIC",
	//	case 823:
	"AJP2 Holdings, LLC",
	//	case 824:
	"COBI GmbH",
	//	case 825:
	"Blue Sky Scientific, LLC",
	//	case 826:
	"Appception, Inc.",
	//	case 827:
	"Courtney Thorne Limited",
	//	case 828:
	"Virtuosys",
	//	case 829:
	"TPV Technology Limited",
	//	case 830:
	"Monitra SA",
	//	case 831:
	"Automation Components, Inc.",
	//	case 832:
	"Letsense s.r.l.",
	//	case 833:
	"Etesian Technologies LLC",
	//	case 834:
	"GERTEC BRASIL LTDA.",
	//	case 835:
	"Drekker Development Pty. Ltd.",
	//	case 836:
	"Whirl Inc",
	//	case 837:
	"Locus Positioning",
	//	case 838:
	"Acuity Brands Lighting, Inc",
	//	case 839:
	"Prevent Biometrics",
	//	case 840:
	"Arioneo",
	//	case 841:
	"VersaMe",
	//	case 842:
	"Vaddio",
	//	case 843:
	"Libratone A/S",
	//	case 844:
	"HM Electronics, Inc.",
	//	case 845:
	"TASER International, Inc.",
	//	case 846:
	"Safe Trust Inc.",
	//	case 847:
	"Heartland Payment Systems",
	//	case 848:
	"Bitstrata Systems Inc.",
	//	case 849:
	"Pieps GmbH",
	//	case 850:
	"iRiding(Xiamen)Technology Co.,Ltd.",
	//	case 851:
	"Alpha Audiotronics, Inc.",
	//	case 852:
	"TOPPAN FORMS CO.,LTD.",
	//	case 853:
	"Sigma Designs, Inc.",
	//	case 854:
	"Spectrum Brands, Inc.",
	//	case 855:
	"Polymap Wireless",
	//	case 856:
	"MagniWare Ltd.",
	//	case 857:
	"Novotec Medical GmbH",
	//	case 858:
	"Medicom Innovation Partner a/s",
	//	case 859:
	"Matrix Inc.",
	//	case 860:
	"Eaton Corporation",
	//	case 861:
	"KYS",
	//	case 862:
	"Naya Health, Inc.",
	//	case 863:
	"Acromag",
	//	case 864:
	"Insulet Corporation",
	//	case 865:
	"Wellinks Inc.",
	//	case 866:
	"ON Semiconductor",
	//	case 867:
	"FREELAP SA",
	//	case 868:
	"Favero Electronics Srl",
	//	case 869:
	"BioMech Sensor LLC",
	//	case 870:
	"BOLTT Sports technologies Private limited",
	//	case 871:
	"Saphe International",
	//	case 872:
	"Metormote AB",
	//	case 873:
	"littleBits",
	//	case 874:
	"SetPoint Medical",
	//	case 875:
	"BRControls Products BV",
	//	case 876:
	"Zipcar",
	//	case 877:
	"AirBolt Pty Ltd",
	//	case 878:
	"KeepTruckin Inc",
	//	case 879:
	"Motiv, Inc.",
	//	case 880:
	"Wazombi Labs OÜ",
	//	case 881:
	"ORBCOMM",
	//	case 882:
	"Nixie Labs, Inc.",
	//	case 883:
	"AppNearMe Ltd",
	//	case 884:
	"Holman Industries",
	//	case 885:
	"Expain AS",
	//	case 886:
	"Electronic Temperature Instruments Ltd",
	//	case 887:
	"Plejd AB",
	//	case 888:
	"Propeller Health",
	//	case 889:
	"Shenzhen iMCO Electronic Technology Co.,Ltd",
	//	case 890:
	"Algoria",
	//	case 891:
	"Apption Labs Inc.",
	//	case 892:
	"Cronologics Corporation",
	//	case 893:
	"MICRODIA Ltd.",
	//	case 894:
	"lulabytes S.L.",
	//	case 895:
	"Nestec S.A.",
	//	case 896:
	"LLC \"MEGA-F service\"",
	//	case 897:
	"Sharp Corporation",
	//	case 898:
	"Precision Outcomes Ltd",
	//	case 899:
	"Kronos Incorporated",
	//	case 900:
	"OCOSMOS Co., Ltd.",
	//	case 901:
	"Embedded Electronic Solutions Ltd. dba e2Solutions",
	//	case 902:
	"Aterica Inc.",
	//	case 903:
	"BluStor PMC, Inc.",
	//	case 904:
	"Kapsch TrafficCom AB",
	//	case 905:
	"ActiveBlu Corporation",
	//	case 906:
	"Kohler Mira Limited",
	//	case 907:
	"Noke",
	//	case 908:
	"Appion Inc.",
	//	case 909:
	"Resmed Ltd",
	//	case 910:
	"Crownstone B.V.",
	//	case 911:
	"Xiaomi Inc.",
	//	case 912:
	"INFOTECH s.r.o.",
	//	case 913:
	"Thingsquare AB",
	//	case 914:
	"T&D",
	//	case 915:
	"LAVAZZA S.p.A.",
	//	case 916:
	"Netclearance Systems, Inc.",
	//	case 917:
	"SDATAWAY",
	//	case 918:
	"BLOKS GmbH",
	//	case 919:
	"LEGO System A/S",
	//	case 920:
	"Thetatronics Ltd",
	//	case 921:
	"Nikon Corporation",
	//	case 922:
	"NeST",
	//	case 923:
	"South Silicon Valley Microelectronics",
	//	case 924:
	"ALE International",
	//	case 925:
	"CareView Communications, Inc.",
	//	case 926:
	"SchoolBoard Limited",
	//	case 927:
	"Molex Corporation",
	//	case 928:
	"IVT Wireless Limited",
	//	case 929:
	"Alpine Labs LLC",
	//	case 930:
	"Candura Instruments",
	//	case 931:
	"SmartMovt Technology Co., Ltd",
	//	case 932:
	"Token Zero Ltd",
	//	case 933:
	"ACE CAD Enterprise Co., Ltd. (ACECAD)",
	//	case 934:
	"Medela, Inc",
	//	case 935:
	"AeroScout",
	//	case 936:
	"Esrille Inc.",
	//	case 937:
	"THINKERLY SRL",
	//	case 938:
	"Exon Sp. z o.o.",
	//	case 939:
	"Meizu Technology Co., Ltd.",
	//	case 940:
	"Smablo LTD",
	//	case 941:
	"XiQ",
	//	case 942:
	"Allswell Inc.",
	//	case 943:
	"Comm-N-Sense Corp DBA Verigo",
	//	case 944:
	"VIBRADORM GmbH",
	//	case 945:
	"Otodata Wireless Network Inc.",
	//	case 946:
	"Propagation Systems Limited",
	//	case 947:
	"Midwest Instruments & Controls",
	//	case 948:
	"Alpha Nodus, inc.",
	//	case 949:
	"petPOMM, Inc",
	//	case 950:
	"Mattel",
	//	case 951:
	"Airbly Inc.",
	//	case 952:
	"A-Safe Limited",
	//	case 953:
	"FREDERIQUE CONSTANT SA",
	//	case 954:
	"Maxscend Microelectronics Company Limited",
	//	case 955:
	"Abbott Diabetes Care",
	//	case 956:
	"ASB Bank Ltd",
	//	case 957:
	"amadas",
	//	case 958:
	"Applied Science, Inc.",
	//	case 959:
	"iLumi Solutions Inc.",
	//	case 960:
	"Arch Systems Inc.",
	//	case 961:
	"Ember Technologies, Inc.",
	//	case 962:
	"Snapchat Inc",
	//	case 963:
	"Casambi Technologies Oy",
	//	case 964:
	"Pico Technology Inc.",
	//	case 965:
	"St. Jude Medical, Inc.",
	//	case 966:
	"Intricon",
	//	case 967:
	"Structural Health Systems, Inc.",
	//	case 968:
	"Avvel International",
	//	case 969:
	"Gallagher Group",
	//	case 970:
	"In2things Automation Pvt. Ltd.",
	//	case 971:
	"SYSDEV Srl",
	//	case 972:
	"Vonkil Technologies Ltd",
	//	case 973:
	"Wynd Technologies, Inc.",
	//	case 974:
	"CONTRINEX S.A.",
	//	case 975:
	"MIRA, Inc.",
	//	case 976:
	"Watteam Ltd",
	//	case 977:
	"Density Inc.",
	//	case 978:
	"IOT Pot India Private Limited",
	//	case 979:
	"Sigma Connectivity AB",
	//	case 980:
	"PEG PEREGO SPA",
	//	case 981:
	"Wyzelink Systems Inc.",
	//	case 982:
	"Yota Devices LTD",
	//	case 983:
	"FINSECUR",
	//	case 984:
	"Zen-Me Labs Ltd",
	//	case 985:
	"3IWare Co., Ltd.",
	//	case 986:
	"EnOcean GmbH",
	//	case 987:
	"Instabeat, Inc",
	//	case 988:
	"Nima Labs",
	//	case 989:
	"Andreas Stihl AG & Co. KG",
	//	case 990:
	"Nathan Rhoades LLC",
	//	case 991:
	"Grob Technologies, LLC",
	//	case 992:
	"Actions (Zhuhai) Technology Co., Limited",
	//	case 993:
	"SPD Development Company Ltd",
	//	case 994:
	"Sensoan Oy",
	//	case 995:
	"Qualcomm Life Inc",
	//	case 996:
	"Chip-ing AG",
	//	case 997:
	"ffly4u",
	//	case 998:
	"IoT Instruments Oy",
	//	case 999:
	"TRUE Fitness Technology",
	//	case 1000:
	"Reiner Kartengeraete GmbH & Co. KG.",
	//	case 1001:
	"SHENZHEN LEMONJOY TECHNOLOGY CO., LTD.",
	//	case 1002:
	"Hello Inc.",
	//	case 1003:
	"Evollve Inc.",
	//	case 1004:
	"Jigowatts Inc.",
	//	case 1005:
	"BASIC MICRO.COM,INC.",
	//	case 1006:
	"CUBE TECHNOLOGIES",
	//	case 1007:
	"foolography GmbH",
	//	case 1008:
	"CLINK",
	//	case 1009:
	"Hestan Smart Cooking Inc.",
	//	case 1010:
	"WindowMaster A/S",
	//	case 1011:
	"Flowscape AB",
	//	case 1012:
	"PAL Technologies Ltd",
	//	case 1013:
	"WHERE, Inc.",
	//	case 1014:
	"Iton Technology Corp.",
	//	case 1015:
	"Owl Labs Inc.",
	//	case 1016:
	"Rockford Corp.",
	//	case 1017:
	"Becon Technologies Co.,Ltd.",
	//	case 1018:
	"Vyassoft Technologies Inc",
	//	case 1019:
	"Nox Medical",
	//	case 1020:
	"Kimberly-Clark",
	//	case 1021:
	"Trimble Navigation Ltd.",
	//	case 1022:
	"Littelfuse",
	//	case 1023:
	"Withings",
	//	case 1024:
	"i-developer IT Beratung UG",
	//	case 1025:
	"",
	//	case 1026:
	"Sears Holdings Corporation",
	//	case 1027:
	"Gantner Electronic GmbH",
	//	case 1028:
	"Authomate Inc",
	//	case 1029:
	"Vertex International, Inc.",
	//	case 1030:
	"Airtago",
	//	case 1031:
	"Swiss Audio SA",
	//	case 1032:
	"ToGetHome Inc.",
	//	case 1033:
	"AXIS",
	//	case 1034:
	"Openmatics",
	//	case 1035:
	"Jana Care Inc.",
	//	case 1036:
	"Senix Corporation",
	//	case 1037:
	"NorthStar Battery Company, LLC",
	//	case 1038:
	"SKF (U.K.) Limited",
	//	case 1039:
	"CO-AX Technology, Inc.",
	//	case 1040:
	"Fender Musical Instruments",
	//	case 1041:
	"Luidia Inc",
	//	case 1042:
	"SEFAM",
	//	case 1043:
	"Wireless Cables Inc",
	//	case 1044:
	"Lightning Protection International Pty Ltd",
	//	case 1045:
	"Uber Technologies Inc",
	//	case 1046:
	"SODA GmbH",
	//	case 1047:
	"Fatigue Science",
	//	case 1048:
	"Alpine Electronics Inc.",
	//	case 1049:
	"Novalogy LTD",
	//	case 1050:
	"Friday Labs Limited",
	//	case 1051:
	"OrthoAccel Technologies",
	//	case 1052:
	"WaterGuru, Inc.",
	//	case 1053:
	"Benning Elektrotechnik und Elektronik GmbH & Co. KG",
	//	case 1054:
	"Dell Computer Corporation",
	//	case 1055:
	"Kopin Corporation",
	//	case 1056:
	"TecBakery GmbH",
	//	case 1057:
	"Backbone Labs, Inc.",
	//	case 1058:
	"DELSEY SA",
	//	case 1059:
	"Chargifi Limited",
	//	case 1060:
	"Trainesense Ltd.",
	//	case 1061:
	"Unify Software and Solutions GmbH & Co. KG",
	//	case 1062:
	"Husqvarna AB",
	//	case 1063:
	"Focus fleet and fuel management inc",
	//	case 1064:
	"SmallLoop, LLC",
	//	case 1065:
	"Prolon Inc.",
	//	case 1066:
	"BD Medical",
	//	case 1067:
	"iMicroMed Incorporated",
	//	case 1068:
	"Ticto N.V.",
	//	case 1069:
	"Meshtech AS",
	//	case 1070:
	"MemCachier Inc.",
	//	case 1071:
	"Danfoss A/S",
	//	case 1072:
	"SnapStyk Inc.",
	//	case 1073:
	"Amway Corporation",
	//	case 1074:
	"Silk Labs, Inc.",
	//	case 1075:
	"Pillsy Inc.",
	//	case 1076:
	"Hatch Baby, Inc.",
	//	case 1077:
	"Blocks Wearables Ltd.",
	//	case 1078:
	"Drayson Technologies (Europe) Limited",
	//	case 1079:
	"eBest IOT Inc.",
	//	case 1080:
	"Helvar Ltd",
	//	case 1081:
	"Radiance Technologies",
	//	case 1082:
	"Nuheara Limited",
	//	case 1083:
	"Appside co., ltd.",
	//	case 1084:
	"DeLaval",
	//	case 1085:
	"Coiler Corporation",
	//	case 1086:
	"Thermomedics, Inc.",
	//	case 1087:
	"Tentacle Sync GmbH",
	//	case 1088:
	"Valencell, Inc.",
	//	case 1089:
	"iProtoXi Oy",
	//	case 1090:
	"SECOM CO., LTD.",
	//	case 1091:
	"Tucker International LLC",
	//	case 1092:
	"Metanate Limited",
	//	case 1093:
	"Kobian Canada Inc.",
	//	case 1094:
	"NETGEAR, Inc.",
	//	case 1095:
	"Fabtronics Australia Pty Ltd",
	//	case 1096:
	"Grand Centrix GmbH",
	//	case 1097:
	"1UP USA.com llc",
	//	case 1098:
	"SHIMANO INC.",
	//	case 1099:
	"Nain Inc.",
	//	case 1100:
	"LifeStyle Lock, LLC",
	//	case 1101:
	"VEGA Grieshaber KG",
	//	case 1102:
	"Xtrava Inc.",
	//	case 1103:
	"TTS Tooltechnic Systems AG & Co. KG",
	//	case 1104:
	"Teenage Engineering AB",
	//	case 1105:
	"Tunstall Nordic AB",
	//	case 1106:
	"Svep Design Center AB",
	//	case 1107:
	"GreenPeak Technologies BV",
	//	case 1108:
	"Sphinx Electronics GmbH & Co KG",
	//	case 1109:
	"Atomation",
	//	case 1110:
	"Nemik Consulting Inc",
	//	case 1111:
	"RF INNOVATION",
	//	case 1112:
	"Mini Solution Co., Ltd.",
	//	case 1113:
	"Lumenetix, Inc",
	//	case 1114:
	"2048450 Ontario Inc",
	//	case 1115:
	"SPACEEK LTD",
	//	case 1116:
	"Delta T Corporation",
	//	case 1117:
	"Boston Scientific Corporation",
	//	case 1118:
	"Nuviz, Inc.",
	//	case 1119:
	"Real Time Automation, Inc.",
	//	case 1120:
	"Kolibree",
	//	case 1121:
	"vhf elektronik GmbH",
	//	case 1122:
	"Bonsai Systems GmbH",
	//	case 1123:
	"Fathom Systems Inc.",
	//	case 1124:
	"Bellman & Symfon",
	//	case 1125:
	"International Forte Group LLC",
	//	case 1126:
	"CycleLabs Solutions inc.",
	//	case 1127:
	"Codenex Oy",
	//	case 1128:
	"Kynesim Ltd",
	//	case 1129:
	"Palago AB",
	//	case 1130:
	"INSIGMA INC.",
	//	case 1131:
	"PMD Solutions",
	//	case 1132:
	"Qingdao Realtime Technology Co., Ltd.",
	//	case 1133:
	"BEGA Gantenbrink-Leuchten KG",
	//	case 1134:
	"Pambor Ltd.",
	//	case 1135:
	"Develco Products A/S",
	//	case 1136:
	"iDesign s.r.l.",
	//	case 1137:
	"TiVo Corp",
	//	case 1138:
	"Control-J Pty Ltd",
	//	case 1139:
	"Steelcase, Inc.",
	//	case 1140:
	"iApartment co., ltd.",
	//	case 1141:
	"Icom inc.",
	//	case 1142:
	"Oxstren Wearable Technologies Private Limited",
	//	case 1143:
	"Blue Spark Technologies",
	//	case 1144:
	"FarSite Communications Limited",
	//	case 1145:
	"mywerk system GmbH",
	//	case 1146:
	"Sinosun Technology Co., Ltd.",
	//	case 1147:
	"MIYOSHI ELECTRONICS CORPORATION",
	//	case 1148:
	"POWERMAT LTD",
	//	case 1149:
	"Occly LLC",
	//	case 1150:
	"OurHub Dev IvS",
	//	case 1151:
	"Pro-Mark, Inc.",
	//	case 1152:
	"Dynometrics Inc.",
	//	case 1153:
	"Quintrax Limited",
	//	case 1154:
	"POS Tuning Udo Vosshenrich GmbH & Co. KG",
	//	case 1155:
	"Multi Care Systems B.V.",
	//	case 1156:
	"Revol Technologies Inc",
	//	case 1157:
	"SKIDATA AG",
	//	case 1158:
	"DEV TECNOLOGIA INDUSTRIA, COMERCIO E MANUTENCAO DE EQUIPAMENTOS LTDA. - ME",
	//	case 1159:
	"Centrica Connected Home",
	//	case 1160:
	"Automotive Data Solutions Inc",
	//	case 1161:
	"Igarashi Engineering",
	//	case 1162:
	"Taelek Oy",
	//	case 1163:
	"CP Electronics Limited",
	//	case 1164:
	"Vectronix AG",
	//	case 1165:
	"S-Labs Sp. z o.o.",
	//	case 1166:
	"Companion Medical, Inc.",
	//	case 1167:
	"BlueKitchen GmbH",
	//	case 1168:
	"Matting AB",
	//	case 1169:
	"SOREX - Wireless Solutions GmbH",
	//	case 1170:
	"ADC Technology, Inc.",
	//	case 1171:
	"Lynxemi Pte Ltd",
	//	case 1172:
	"SENNHEISER electronic GmbH & Co. KG",
	//	case 1173:
	"LMT Mercer Group, Inc",
	//	case 1174:
	"Polymorphic Labs LLC",
	//	case 1175:
	"Cochlear Limited",
	//	case 1176:
	"METER Group, Inc. USA",
	//	case 1177:
	"Ruuvi Innovations Ltd.",
	//	case 1178:
	"Situne AS",
	//	case 1179:
	"nVisti, LLC",
	//	case 1180:
	"DyOcean",
	//	case 1181:
	"Uhlmann & Zacher GmbH",
	//	case 1182:
	"AND!XOR LLC",
	//	case 1183:
	"tictote AB",
	//	case 1184:
	"Vypin, LLC",
	//	case 1185:
	"PNI Sensor Corporation",
	//	case 1186:
	"ovrEngineered, LLC",
	//	case 1187:
	"GTronix",
	//	case 1188:
	"Herbert Waldmann GmbH & Co. KG",
	//	case 1189:
	"Guangzhou FiiO Electronics Technology Co.,Ltd",
	//	case 1190:
	"Vinetech Co., Ltd"
};

const char *_get_company_name(const int id)
{
	if (id == 65535)
		return "internal use";
	else if (id > 1190)
		return "not assigned";
	else
		return bt_company_names[id];
}

const char *_get_uuid_name(const char *uuid)
{
	uint32_t uuid32;
	int i;

	if (!uuid || strlen(uuid) != 36)
		return NULL;

	if (g_strcmp0(uuid + 8, "-0000-1000-8000-00805f9b34fb"))
		return "Vendor specific";

	if (sscanf(uuid, "%08x-0000-1000-8000-00805f9b34fb", &uuid32) != 1)
		return NULL;

	if ((uuid32 & 0xffff0000) != 0x0000)
		return "Unknown";

	for (i = 0; bt_uuids[i].uuid; i++) {
		if (bt_uuids[i].uuid == (uuid32 & 0x0000ffff))
			return bt_uuids[i].name;
	}

	return "Unknown";
}
