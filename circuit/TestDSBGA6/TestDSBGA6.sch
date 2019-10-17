EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L Connector:USB_B_Micro J?
U 1 1 5DA8C49C
P 2750 2750
F 0 "J?" H 2807 3217 50  0000 C CNN
F 1 "USB_B_Micro" H 2807 3126 50  0000 C CNN
F 2 "Connector_USB:USB_Micro-B_Molex-105017-0001" H 2900 2700 50  0001 C CNN
F 3 "~" H 2900 2700 50  0001 C CNN
	1    2750 2750
	1    0    0    -1  
$EndComp
$Comp
L Device:LED D?
U 1 1 5DA8CA21
P 5350 3000
F 0 "D?" V 5389 2883 50  0000 R CNN
F 1 "LED" V 5298 2883 50  0000 R CNN
F 2 "LED_SMD:LED_0805_2012Metric_Pad1.15x1.40mm_HandSolder" H 5350 3000 50  0001 C CNN
F 3 "~" H 5350 3000 50  0001 C CNN
	1    5350 3000
	0    -1   -1   0   
$EndComp
$Comp
L Device:Battery_Cell BT?
U 1 1 5DA8CDAD
P 6000 3050
F 0 "BT?" H 6118 3146 50  0000 L CNN
F 1 "2.54mm" H 6118 3055 50  0000 L CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_1x02_P2.54mm_Vertical" V 6000 3110 50  0001 C CNN
F 3 "~" V 6000 3110 50  0001 C CNN
	1    6000 3050
	1    0    0    -1  
$EndComp
$Comp
L Device:Battery_Cell BT?
U 1 1 5DA8D080
P 6550 3050
F 0 "BT?" H 6668 3146 50  0000 L CNN
F 1 "1.25mm" H 6668 3055 50  0000 L CNN
F 2 "Connector_PinSocket_1.27mm:PinSocket_1x02_P1.27mm_Vertical" V 6550 3110 50  0001 C CNN
F 3 "~" V 6550 3110 50  0001 C CNN
	1    6550 3050
	1    0    0    -1  
$EndComp
$Comp
L balmer:BQ25101 U?
U 1 1 5DA8E8E9
P 4450 2800
F 0 "U?" H 4425 3175 50  0000 C CNN
F 1 "BQ25101" H 4425 3084 50  0000 C CNN
F 2 "balmer:DSBGA-6_0.4mm" H 4500 2800 50  0001 C CNN
F 3 "" H 4500 2800 50  0001 C CNN
	1    4450 2800
	1    0    0    -1  
$EndComp
$Comp
L Device:C C?
U 1 1 5DA8F887
P 3350 3150
F 0 "C?" H 3465 3196 50  0000 L CNN
F 1 "1u" H 3465 3105 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric" H 3388 3000 50  0001 C CNN
F 3 "~" H 3350 3150 50  0001 C CNN
	1    3350 3150
	1    0    0    -1  
$EndComp
$Comp
L Device:R R?
U 1 1 5DA8FE0A
P 3650 3150
F 0 "R?" H 3720 3196 50  0000 L CNN
F 1 "10k" H 3720 3105 50  0000 L CNN
F 2 "Resistor_SMD:R_0603_1608Metric" V 3580 3150 50  0001 C CNN
F 3 "~" H 3650 3150 50  0001 C CNN
	1    3650 3150
	1    0    0    -1  
$EndComp
$Comp
L Device:R R?
U 1 1 5DA903B5
P 3950 3150
F 0 "R?" H 4020 3196 50  0000 L CNN
F 1 "3.3k" H 4020 3105 50  0000 L CNN
F 2 "Resistor_SMD:R_0603_1608Metric" V 3880 3150 50  0001 C CNN
F 3 "~" H 3950 3150 50  0001 C CNN
	1    3950 3150
	1    0    0    -1  
$EndComp
$Comp
L Device:R R?
U 1 1 5DA91ADB
P 4950 3000
F 0 "R?" H 5020 3046 50  0000 L CNN
F 1 "1k" H 5020 2955 50  0000 L CNN
F 2 "Resistor_SMD:R_0603_1608Metric" V 4880 3000 50  0001 C CNN
F 3 "~" H 4950 3000 50  0001 C CNN
	1    4950 3000
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 5DA93B7A
P 4450 3350
F 0 "#PWR?" H 4450 3100 50  0001 C CNN
F 1 "GND" H 4455 3177 50  0001 C CNN
F 2 "" H 4450 3350 50  0001 C CNN
F 3 "" H 4450 3350 50  0001 C CNN
	1    4450 3350
	1    0    0    -1  
$EndComp
Wire Wire Line
	4850 2650 6000 2650
Wire Wire Line
	6000 2650 6000 2850
Wire Wire Line
	6000 2650 6550 2650
Wire Wire Line
	6550 2650 6550 2850
Connection ~ 6000 2650
Wire Wire Line
	3350 3300 3650 3300
Wire Wire Line
	3650 3300 3950 3300
Connection ~ 3650 3300
Wire Wire Line
	4450 3300 4450 3250
Wire Wire Line
	4450 3300 4450 3350
Connection ~ 4450 3300
Wire Wire Line
	3350 3300 2750 3300
Wire Wire Line
	2750 3300 2750 3150
Connection ~ 3350 3300
Wire Wire Line
	2650 3150 2650 3300
Wire Wire Line
	2650 3300 2750 3300
Connection ~ 2750 3300
Wire Wire Line
	3050 2550 3350 2550
Wire Wire Line
	4000 2550 4000 2650
Wire Wire Line
	3350 3000 3350 2550
Connection ~ 3350 2550
Wire Wire Line
	3350 2550 4000 2550
Wire Wire Line
	4000 2800 3650 2800
Wire Wire Line
	3650 2800 3650 3000
Wire Wire Line
	4000 2950 3950 2950
Wire Wire Line
	3950 2950 3950 3000
Wire Wire Line
	3950 3300 4450 3300
Connection ~ 3950 3300
Wire Wire Line
	4850 2800 4950 2800
Wire Wire Line
	4950 2800 4950 2850
Wire Wire Line
	4950 3150 5350 3150
Wire Wire Line
	5350 2850 5350 2300
Wire Wire Line
	5350 2300 4000 2300
Wire Wire Line
	4000 2300 4000 2550
Connection ~ 4000 2550
Wire Wire Line
	4450 3300 6000 3300
Wire Wire Line
	6550 3300 6550 3150
Wire Wire Line
	6000 3150 6000 3300
Connection ~ 6000 3300
Wire Wire Line
	6000 3300 6550 3300
$EndSCHEMATC
