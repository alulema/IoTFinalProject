CREATE SCHEMA iot_projectw6;
USE iot_projectw6;

create table iot_projectw6.Temperature
(
  id int auto_increment primary key,
  device_id varchar(16) charset utf8 not null,
  device_status varchar(8) charset utf8 not null,
  measurement float not null,
  unit varchar(4) charset utf8 not null,
  timestamp datetime not null
);
