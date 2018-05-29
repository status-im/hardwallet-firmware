# Technical notes

This document will contain a few notes and implementation details.

## Prototype notes

The prototype is being developed on a Nucleo-L476RG board with a BlueNRG-MS-based shield for BLE. The MCU lacks PKA, AES and HASH acceleration. For this reason we are using two libraries

1. [uECC](https://github.com/kmackay/micro-ecc) for ECDSA over secp256k1
2. [cifra](https://github.com/ctz/cifra) for AES, SHA-256, SHA-3 (patched to support Kekkac), SHA-512, HMAC

The final chip will be of the (yet unreleased) STM32WB family which does have at least PKA and AES acceleration, which means we will most likely be able to replace the uECC library and part of the cifra library. However the latter library will most likely still be needed for SHA-3 (Kekkac).

At the moment the libraries have been imported completely and as-is, relying on the compiler to remove unused code. For auditing and review purposes however, it might be desireable to remove the unused parts from the source code. This will be done later, when it becomes clear which parts we actually need.

To make replacing easier, convenience functions around all calls to these libraries will be created.

## HAL - Hardware Abstraction Layer

Current status: no HAL will be used at the moment, since they add extra complexity (especially the ST provided one). Only the CMSIS will be imported (defining constants and a few macros). Below is a list of options that have been considered. We might use the BlueNRG middleware for BLE, but implement our own HAL layer for it.

STM32 chips come with a HAL library which we could use in our firmware to speed up development. The downside of this library is that it only supports STM32 chips and it is quite heavy-weight. It is however very complete in peripherals support.

Another alternative library is [libopencm3](https://github.com/libopencm3/libopencm3) which also supports different MCUs. The downside is that not all pheripherals are supported, so support must be implemented separately.

There is also the possibility to just implement everything from scratch, but this will make development a little longer, at least at the beginning.

## LCD Controller

The Nucleo-L476RG comes with a 64-pin version of the STM32L476RG chip, which means not all the theoretical 320 segments are available. Using an 8 commons configuration the maximum I can get is

1. 224 segments with all other peripherals disabled. Communicating with the Bluetooth module will happen via a software-based SPI implementation over GPIO pins (very inefficient) and input through resistive buttons
2. 208 segments if enabling the SPI peripheral or the TSC peripheral (for capacititative inputs).
3. 192 segments if enabling both the SPI and the TSC peripheral

Regardless of the solution, I will need to make some hardware modification to the BLE shield, because the current pin-out conflicts with the LCD_COM0 pin so the LCD cannot be used at all. However the involved pins are just regular GPIO, so I must simply solder a bridge to another pin and break a connection (or connect with wires instead of the regular connector).

On the target STM32WB chip the SPI interface will not be needed. They also come in 68-pin and 100-pin configuration, although some of those are surely reserved for the bluetooth external components. For this reason I think is not unreasonable to develop with the limit of 208 segments in mind. On the prototype board regular switches can be used instead of capacitative buttons.

In terms of digits, 208 segments means a theoretical amount of

1. 29 digits + 5 spare segments using 7-segment digits
2. 23 digits + 1 spare segment using 9-segment digit
3. 18 digits + 10 spare segments using 11-segment digit
4. 14 digits + 12 spare segments using 14-segment digits

Of course we will probably need at least 10-15 segments for symbols other than digits and also we must consider the area of the screen to keep the digit large enough to be readable. However it turns out that even on the development board we are not really limited by the amount of pins, unless we go for 14-segment digits.