IoTHON_2019
======================================

Our wokr during IoTHON 2019 in Aalto. 

An important part of the challenge was to demonstrate how cellular IoT (Narrowband IoT or LTE-M) could be used in the solution. To do that, we designed and implemented a undergrond pipeline monitor system that can prevent urban flooding and water pollution problem. The system is composed of an Arduino-based detection device, a visualization platform and a data
processing server using LSTM model. We attached the sensors to the Arduino board. It then transimt the data to the AWS via NB-IoT network. After processing the data, it is displayed with the Grafana. The LSTM model in this project is to predict the water pollution level of next month given the measurement data of last few monthes.   

## Team:

* Jun Zhang
* Yichen Cao
* Yuanzong Zhang 
* Yiyan Ye          

## Contents 

This repository is organized as follows:

* `Arduino` contains the sources code on the Arduino board written in C.
* `DL Model` contains the LSTM model used for water pollution level prediction. 


This is the winning project for Ericsson Challenge in the Hackathon. For more details, please refer to this [article](https://www.ericsson.com/en/blog/2019/7/iothon-2019-tackling-urban-flooding-cellular-iot).
