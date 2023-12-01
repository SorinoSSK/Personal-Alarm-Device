# Introduction
This repository serves as a record and learning journey on hardware used for my capstone project. The objective of the project is to implement a small, affordable and unrestrictive IoT deviced that enables individuals to call for help in an emergency.

## PCB_Design_GSM_GPS
With the failure to find an appropriate module that carries both the functionality of GPS and GSM, I have decided to design a customised module using [SIM868](https://www.alibaba.com/product-detail/SIM868-Signal-Circuits-GSM-GPS-GNSS_1600272320629.html). SIM868 enables both GSM and GPS communication that is inline with the frequency band used in Singapore, however SIM868 is built for 2G communication and 2G communication is no longer used in Singapore. 

***Update: SIM868 is mostly no longer in production.***\
***SIM868 could not detect 3G or 4G network***

## Schematic_1
Upon searching for SIM868, I stumble across the module A9G and thus drew the schematic to be used for feasibility testing of our idea. A9G also uses GPRS protocol which still runs mostly on a [2G network](https://tinyurl.com/mwhuy22j). 

***Update: A9G could not detect 3G or 4G network***