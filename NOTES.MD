# Technical notes

This document will contain a few notes and implementation details.

## Prototype notes

The prototype is being developed on a Nucleo-L476RG board with a BlueNRG-MS-based shield for BLE. The MCU lacks PKA, AES and HASH acceleration. For this reason we are using two libraries

1. [uECC](https://github.com/kmackay/micro-ecc) for ECDSA over secp256k1
2. [cifra](https://github.com/ctz/cifra) for AES, SHA-256, SHA-3 (Kekkac), SHA-512, HMAC

The final chip will be of the (yet unreleased) STM32WB family which does have at least PKA and AES acceleration, which means we will most likely be able to replace the uECC library and part of the cifra library. However the latter library will most likely still be needed for SHA-3 (Kekkac).

At the moment the libraries have been imported completely and as-is, relying on the compiler to remove unused code. For auditing and review purposes however, it might be desireable to remove the unused parts from the source code. This will be done later, when it becomes clear which parts we actually need.

To make replacing easier, convenience functions around all calls to these libraries will be created.

The current chip has 64 pins which makes many of them multiplexed. For this reason it won't be possible to use all the available LCD segments. I will calculate the number of actually available segments soon. The target STM32WB chip will exist in a 100-pin variant, but details on the exact pinout do not exist yet.

## HAL

STM32 chips come with an HAL library which we could use in our firmware to speed up development. The downside of this library is that it only supports STM32 chips and it is quite heavy-weight. It is however very complete in peripherals support.

Another alternative library is [libopencm3](https://github.com/libopencm3/libopencm3) which also supports different MCUs. The downside is that not all pheripherals are supported, so support must be implemented separately.

There is also the possibility to just implement everything from scratch, but this will make development a little longer, at least at the beginning.

This decision will be taken as soon as platform-dependent code will be written (quite soon)