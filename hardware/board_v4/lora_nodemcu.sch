EESchema Schematic File Version 4
LIBS:lora_nodemcu-cache
EELAYER 29 0
EELAYER END
$Descr A3 16535 11693
encoding utf-8
Sheet 1 4
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
L lora_nodemcu-rescue:Mounting_Hole-Mechanical MK1
U 1 1 5ABC6521
P 6800 7650
F 0 "MK1" H 6900 7696 50  0000 L CNN
F 1 "Mounting_Hole" H 6900 7605 50  0000 L CNN
F 2 "MountingHole:MountingHole_3.2mm_M3" H 6800 7650 50  0001 C CNN
F 3 "" H 6800 7650 50  0001 C CNN
	1    6800 7650
	1    0    0    -1  
$EndComp
$Comp
L lora_nodemcu-rescue:Mounting_Hole-Mechanical MK3
U 1 1 5ABC68A6
P 7650 7650
F 0 "MK3" H 7750 7696 50  0000 L CNN
F 1 "Mounting_Hole" H 7750 7605 50  0000 L CNN
F 2 "MountingHole:MountingHole_3.2mm_M3" H 7650 7650 50  0001 C CNN
F 3 "" H 7650 7650 50  0001 C CNN
	1    7650 7650
	1    0    0    -1  
$EndComp
$Comp
L lora_nodemcu-rescue:Mounting_Hole-Mechanical MK2
U 1 1 5ABC6A8E
P 6800 8000
F 0 "MK2" H 6900 8046 50  0000 L CNN
F 1 "Mounting_Hole" H 6900 7955 50  0000 L CNN
F 2 "MountingHole:MountingHole_3.2mm_M3" H 6800 8000 50  0001 C CNN
F 3 "" H 6800 8000 50  0001 C CNN
	1    6800 8000
	1    0    0    -1  
$EndComp
$Comp
L lora_nodemcu-rescue:Mounting_Hole-Mechanical MK4
U 1 1 5ABC6A95
P 7650 8000
F 0 "MK4" H 7750 8046 50  0000 L CNN
F 1 "Mounting_Hole" H 7750 7955 50  0000 L CNN
F 2 "MountingHole:MountingHole_3.2mm_M3" H 7650 8000 50  0001 C CNN
F 3 "" H 7650 8000 50  0001 C CNN
	1    7650 8000
	1    0    0    -1  
$EndComp
Wire Notes Line
	6550 7400 6550 8200
Wire Notes Line
	6550 8200 8450 8200
Wire Notes Line
	8450 8200 8450 7400
Wire Notes Line
	8450 7400 6550 7400
$Comp
L power:GND #PWR0127
U 1 1 5AB4404A
P 9750 8150
F 0 "#PWR0127" H 9750 7900 50  0001 C CNN
F 1 "GND" H 9755 7977 50  0000 C CNN
F 2 "" H 9750 8150 50  0001 C CNN
F 3 "" H 9750 8150 50  0001 C CNN
	1    9750 8150
	1    0    0    -1  
$EndComp
$Comp
L Device:RF_Shield_One_Piece J7
U 1 1 5AB43EB6
P 9750 7750
F 0 "J7" H 10380 7741 50  0000 L CNN
F 1 "RF_Shield_RFM95" H 10380 7650 50  0000 L CNN
F 2 "RF_Shielding:Wuerth_36103305_30x30mm" H 9750 7650 50  0001 C CNN
F 3 "~" H 9750 7650 50  0001 C CNN
	1    9750 7750
	1    0    0    -1  
$EndComp
$Sheet
S 5550 2750 900  850 
U 5C0025BF
F0 "ESP32" 50
F1 "ESP32.sch" 50
$EndSheet
$Sheet
S 7100 2750 900  850 
U 5BFE367E
F0 "SOLAR" 50
F1 "PWR.sch" 50
$EndSheet
$Sheet
S 8650 2750 900  850 
U 5BFE48D4
F0 "LORA" 50
F1 "LORA.sch" 50
$EndSheet
$EndSCHEMATC
