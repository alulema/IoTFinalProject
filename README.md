# Intro to IoT - Final Project

## Cloud Services for Thermostat Simulator

### Getting Started

This project contains a Service running on an AWS EC2 Linux Instance able to receive data from our Thermostat Simulator. The supported content type to be received is JSON, and it supports 2 actions depending on the following HTTP verbs:

1. **POST**: Stores the information submitted by the user. Internally, it stores the received information, along with the request timestamp. The data structure that this method receives is the following:

```json
{
   "id": "THE-001",
   "status": "OK",
   "measurement": 65,
   "unit": "F"
}
```

2. **GET**: Retrieves the latest 10 registers stored in database, as an array of items containing the registered information, along with the request timestamp. For example:  

```json
[
  {
    "id": 2,
    "deviceId": "THE-001",
    "deviceStatus": "OK",
    "measurement": 85,
    "unit": "F",
    "timestamp": "2018-10-06T20:55:12"
  },
  {
    "id": 1,
    "deviceId": "THE-001",
    "deviceStatus": "OK",
    "measurement": 65,
    "unit": "F",
    "timestamp": "2018-10-06T20:50:42"
  }
]
```
#### How to use it

##### POST
curl -H "Content-Type: application/json" -X POST -d '{ "id": "THE-001", "status": "OK", "measurement": 65, "unit": "F" }' https://ec2-52-67-91-154.sa-east-1.compute.amazonaws.com:5001/api/thermostat

or,

curl -H "Content-Type: application/json" -X POST -d '{ "id": "THE-001", "status": "OK", "measurement": 65, "unit": "F" }' https://52.67.91.154:5001/api/thermostat


##### GET
curl https://ec2-52-67-91-154.sa-east-1.compute.amazonaws.com:5001/api/thermostat

or,

curl https://52.67.91.154:5001/api/thermostat



### Technical Details

This service with created by using the following technical design:

1. Platform: Amazon Linux 2 AMI
2. .NET Core 2.1 (C# 7.0)
3. MySQL 5.7
4. Running on port 8080