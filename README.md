# Table of Contents
[Introduction](#Introduction)
* [Use Case](#use-case)

[Example](#example)
* [Electrical Connection Scheme](#electrical-connection-scheme)
* [Wokwi Example](#wokwi-example)

[Behaviour on Startup](#behaviour-on-startup)

[Class Constructor](#class-constructor)
[Functions](#functions)
* [run](#run)
* [stop](#stop)
* [setFrequency](#setfrequency)
* [setAnalogMin](#setanalogmin)
* [setAnalogMax](#setanalogmax)
* [setEnable](#setenable)
* [setInvertPulse](#setinvertpulse)
* [setHighTime](#sethightime)
* [setLowTime](#setlowtime)
* [getPerformance](#getperformance)
* [GetCurrentPosition](#getcurrentposition)

# Introduction
This class is used for a easy actuation of stepper motors with the help of an additional Arduino as "Gateway" which translate a 0-5V Input signal into an adjustable pulse signal. The direction is also automatically set depending of the input signal

## Use Case
* you have a control task and not enough digital outputs available to realize a stepper actuation
* you just want to actuate the stepper motor and you don't want to think about the pulse generation
* you want to easily control a stepper motor by a pid regulator. In this case you can use to 0-5V as your controll variable

# Example
The example is using an Arduino UNO as the Gateway and an TB6600 but it's also work with other step motor drivers which use Enable, Direction and Pulse Pins.

As you can see you only need to define the pins and to create an object of the ```Arduino_StepDriverGate```  Class and then you can run the actuation by calling ```_stepDriverGate.run()```. 
Within the Arduino ```setup()```routine you can setup the Thresholds for the analog input signal and you can setup the step frequency which is mostly given by the data sheet of the stepper motor.

On the Pin ```A0``` you pass the analog signal which you created with you main control MCU (e.g. a different Arduino which you use for your control task). Please consider that both Arduinos have to be common grounded

```cpp
#include <Arduino.h>
#include "arduino_stepDriverGate.hpp"


constexpr static byte analogPin = A0;
constexpr static byte stepPin = 2;
constexpr static byte dirPin = 3;
constexpr static byte enablePin = 4;
Arduino_StepDriverGate _stepDriverGate(analogPin, stepPin, dirPin, enablePin, 400);

void setup() {
  Serial.begin(9600);
  _stepDriverGate.setFrequency(50); // 50Hz
  _stepDriverGate.setAnalogMin(500); // 500mV
  _stepDriverGate.setAnalogMax(5000); // 4500mV
}

void loop() { 
  _stepDriverGate.run();
}
```


## Electrical Connection Scheme
The following picture illustrates the electrical connection with an Arduino UNO and an TB6600 but it's also work with other step motor drivers which use Enable, Direction and Pulse Pins.

<img width="1330" height="442" alt="image" src="https://github.com/user-attachments/assets/b98ca04e-1ec7-4632-a2a9-37008e70207b" />

## Wokwi Example
I created an example on wokwi which you can find here: https://wokwi.com/projects/458113252801290241 

<img width="870" height="728" alt="image" src="https://github.com/user-attachments/assets/0c2bcfea-80f6-410e-b7f1-e1c363a3d04e" />

# Behaviour on Startup
On Startup the class is doing a initialization run which means that the stepper motor is driving in one direction over the entire step-Range. This is necessary to prevent and eliminate step errors

>[!Warning]
>Play safe that the mechanic which is connected to the stepper motor can handle driving against end positions


# Class Constructor
If you create the object of the class you have to pass a few values to the class constructor. The following chapter describes the arguments

```cpp
Arduino_StepDriverGate(byte analogPin, byte stepPin, byte dirPin, byte enPin, uint16_t maxSteps)
```

| Argument | Description |
| ---- | ---- |
| analogPin | 0-5V Analogpin of your potentiometer or external control unit |
| stepPin | Output Pin from the Gateway to the Stepper Driver |
| dirPin | Output Pin from the Gateway to the Stepper Driver |
| enPin | Output Pin from the Gateway to the Stepper Driver |
| maxSteps | Max allowed steps by the stepper motor - see technical datasheet |

# Functions
The following chapter describes the public functions which you can use to run the class

## run
Main function which start the continous actuation depending of the input value and the given parameter.

## stop
Stops the continous actuation and also set the Enable Pin to 0. Please consider that the motor position is freezed on the last position and it's not driving back to the origin position.

## setFrequency

```cpp
auto setFrequency(const uint16_t frequency) -> void
```

| Argument | Description |
| ---- | ---- |
| frequency | Step frequency of the stepper motor - see technical data sheet of the motor |

## setAnalogMin 

```cpp
 auto setAnalogMin(const uint16_t analogMin) -> void
```

| Argument | Description |
| ---- | ---- |
| analogMin | Set the minimum Input Value |

> [!Note]
> it's recommendable to set this up to a plausible value to realize a wire break detection and to prevent useless stepper motor actuation because of floating input signal

## setAnalogMax

```cpp
 auto setAnalogMax(const uint16_t analogMin) -> void
```

| Argument | Description |
| ---- | ---- |
| analogMin | Set the maximum Input Value |

> [!Note]
> it's recommendable to set this up to a plausible value to realize a wire break detection and to prevent useless stepper motor actuation because of floating input signal

## setEnable

```cpp
 auto setEnable(const bool enable) -> void
```

| Argument | Description |
| ---- | ---- |
| enable | Turn on / off the stepper driver |

> [!Warning]
> Please consider that the motor position is freezed on the last position and it's not driving back to the origin position.

## setInvertPulse

```cpp
 auto setInvertPulse(const bool invertPulse) -> void
```

| Argument | Description |
| ---- | ---- |
| invertPulse | invert the actuation logic |

> [!Note]
> prepared for implementation - work in Progress

## setHighTime

```cpp
auto setHighTime(const uint16_t highTime) -> void
```

| Argument | Description |
| ---- | ---- |
| highTime | high time of the pulse signal in micro seconds |

* init value is 20ms which should be in the most cases enough
* this settings are limitted to the given frequency (you cannot setup a hightime of 1000 microseconds if you step frequency should have 50Hz)

## setLowTime

```cpp
auto setLowTime(const uint16_t lowTime) -> void
```

| Argument | Description |
| ---- | ---- |
| lowTime | Low time of the pulse signal in micro seconds |

* init value is 20ms which should be in the most cases enough
* this settings are limitted to the given frequency (you cannot setup a hightime of 1000 microseconds if you step frequency should have 50Hz)

## getPerformance

```cpp
auto getPerformance() -> uint32_t
```

* returns the execution time in microseconds
* this could be helpful to analyze the real-time capability. E.g.: If you only run one instance of this class on the ATmega328P you should achieve a average execution time about 250 micro seconds.

## getCurrentPosition

```cpp
auto getCurrentPosition() -> uint16_t
```

* returns the last calculated position
