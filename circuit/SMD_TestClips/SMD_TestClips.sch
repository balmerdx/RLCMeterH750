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
L Connector_Generic:Conn_01x04 J1
U 1 1 5DEBEAB2
P 6400 3450
F 0 "J1" H 6480 3442 50  0000 L CNN
F 1 "IN" H 6480 3351 50  0000 L CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x04_P2.54mm_Vertical" H 6400 3450 50  0001 C CNN
F 3 "~" H 6400 3450 50  0001 C CNN
	1    6400 3450
	1    0    0    -1  
$EndComp
$Comp
L Device:R R3
U 1 1 5DEBF23F
P 5300 3500
F 0 "R3" H 5370 3546 50  0000 L CNN
F 1 "0603" H 5370 3455 50  0000 L CNN
F 2 "Resistor_SMD:R_0603_1608Metric" V 5230 3500 50  0001 C CNN
F 3 "~" H 5300 3500 50  0001 C CNN
	1    5300 3500
	1    0    0    -1  
$EndComp
$Comp
L Device:R R2
U 1 1 5DEBF3AB
P 4850 3500
F 0 "R2" H 4920 3546 50  0000 L CNN
F 1 "0805" H 4920 3455 50  0000 L CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 4780 3500 50  0001 C CNN
F 3 "~" H 4850 3500 50  0001 C CNN
	1    4850 3500
	1    0    0    -1  
$EndComp
$Comp
L Device:R R1
U 1 1 5DEBF4D6
P 4400 3500
F 0 "R1" H 4470 3546 50  0000 L CNN
F 1 "1206" H 4470 3455 50  0000 L CNN
F 2 "Resistor_SMD:R_1206_3216Metric" V 4330 3500 50  0001 C CNN
F 3 "~" H 4400 3500 50  0001 C CNN
	1    4400 3500
	1    0    0    -1  
$EndComp
$Comp
L Device:R R4
U 1 1 5DEBF832
P 5750 3500
F 0 "R4" H 5820 3546 50  0000 L CNN
F 1 "0402" H 5820 3455 50  0000 L CNN
F 2 "Resistor_SMD:R_0402_1005Metric" V 5680 3500 50  0001 C CNN
F 3 "~" H 5750 3500 50  0001 C CNN
	1    5750 3500
	1    0    0    -1  
$EndComp
Wire Wire Line
	6200 3450 6100 3450
Wire Wire Line
	6100 3450 6100 3350
Wire Wire Line
	6100 3350 6200 3350
Wire Wire Line
	6100 3350 5750 3350
Connection ~ 6100 3350
Wire Wire Line
	5750 3350 5300 3350
Connection ~ 5750 3350
Wire Wire Line
	5300 3350 4850 3350
Connection ~ 5300 3350
Wire Wire Line
	4850 3350 4400 3350
Connection ~ 4850 3350
Wire Wire Line
	6200 3550 6100 3550
Wire Wire Line
	6100 3550 6100 3650
Wire Wire Line
	6100 3650 6200 3650
Wire Wire Line
	6100 3650 5750 3650
Connection ~ 6100 3650
Wire Wire Line
	5750 3650 5300 3650
Connection ~ 5750 3650
Wire Wire Line
	5300 3650 4850 3650
Connection ~ 5300 3650
Wire Wire Line
	4850 3650 4400 3650
Connection ~ 4850 3650
$EndSCHEMATC
