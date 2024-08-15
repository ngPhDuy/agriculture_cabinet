create table if not exists users (
    id int primary key auto_increment,
    username varchar(255) not null,
    password varchar(255) not null
);

create table if not exists devices (
    id int primary key,
    name varchar(255) not null,
    state boolean not null,
    description varchar(255) default null
);

create table if not exists timers (
    id int primary key auto_increment,
    name varchar(255) not null,
    device_id int not null,
    start_time time not null,
    duration int not null,
    state boolean not null,
    foreign key (device_id) references devices(id),
    unique (device_id, start_time)
);

create table if not exists temperatures (
    time_stamp timestamp primary key,
    value float not null
)

create table if not exists humidities (
    time_stamp timestamp primary key,
    value float not null
)

DELIMITER //

create procedure get_temperature_per_hour(in today date) 
begin
    select hour(time_stamp) as hour, avg(value) as avg_temperature
    from temperatures
    where date(time_stamp) = today
    group by hour(time_stamp)
    order by hour;
end//

create procedure get_humidity_per_hour(in today date)
begin
    select hour(time_stamp) as hour, avg(value) as avg_humidity
    from humidities
    where date(time_stamp) = today
    group by hour(time_stamp)
    order by hour;
end//

DELIMITER ;

insert into users (username, password) values ('anhbakhia', '123456');

insert into devices (id, name, state, description) values 
(1, 'Thiết bị 1', true, 'Water pump for irrigation 1'),
(3, 'Thiết bị 3', false, 'Fan for ventilation 1'),
(2, 'Thiết bị 2', false, 'Light for plants 1'),
(4, 'Thiết bị 4', false, 'Water pump for irrigation 2'),
(5, 'Thiết bị 5', false, 'Fan for ventilation 2'),
(6, 'Thiết bị 6', true, 'Light for plants 2'),
(7, 'Thiết bị 7', true, 'Water pump for irrigation 3'),
(8, 'Thiết bị 8', false, 'Fan for ventilation 3'),
(9, 'Thiết bị 9', false, 'Light for plants 3'),
(10, 'Thiết bị 10', false, 'Water pump for irrigation 4'),
(11, 'Thiết bị 11', false, 'Fan for ventilation 4'),
(12, 'Thiết bị 12', true, 'Light for plants 4');

insert into timers (name, device_id, start_time, duration, state) values 
('Timer 1', 1, '06:00', 10, true),
('Timer 2', 2, '06:00', 10, false),
('Timer 3', 3, '06:00', 10, false),
('Timer 4', 4, '17:00', 10, true),
('Timer 5', 5, '17:00:', 15, false),
('Timer 6', 6, '17:00', 15, true),
('Timer 7', 7, '08:00', 20, false),
('Timer 8', 8, '08:00', 20, true),
('Timer 9', 9, '08:00', 20, false),
('Timer 10', 10, '20:00', 10, false),
('Timer 11', 11, '20:00', 10, false),
('Timer 12', 12, '20:00', 10, true);

insert into temperatures values 
('2024-08-14 00:00:00', 24.3),
('2024-08-14 00:02:00', 24.7),
('2024-08-14 01:00:00', 24),
('2024-08-14 02:00:00', 24.8),
('2024-08-14 03:00:00', 24.6),
('2024-08-14 03:02:00', 25.1),
('2024-08-14 03:04:00', 25.3),
('2024-08-14 04:00:00', 25.2),
('2024-08-14 05:04:00', 25.3),
('2024-08-14 06:00:00', 25.5),
('2024-08-14 07:00:00', 24.8),
('2024-08-14 08:00:00', 24.1),
('2024-08-14 09:00:00', 24.7),
('2024-08-14 10:00:00', 24.5),
('2024-08-14 11:00:00', 24.3);

insert into humidities values 
('2024-08-14 00:00:00', 49.71),
('2024-08-14 00:02:00', 49.81),
('2024-08-14 01:00:00', 49.01),
('2024-08-14 02:00:00', 49.2),
('2024-08-14 03:00:00', 49.98),
('2024-08-14 03:02:00', 48.78),
('2024-08-14 03:04:00', 48.04),
('2024-08-14 04:00:00', 48.97),
('2024-08-14 05:04:00', 48),
('2024-08-14 06:00:00', 48.12),
('2024-08-14 07:00:00', 49.88),
('2024-08-14 08:00:00', 49.11),
('2024-08-14 09:00:00', 49.87),
('2024-08-14 10:00:00', 49.53),
('2024-08-14 11:00:00', 49.08);
