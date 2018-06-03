# Communication protocol

## Transport protocol

The device will communicate with the client through BLE connection. BLE has been chosen to allow interfacing with virtually all smartphones, including iOS and Android based ones. However this protocol has been designed for low throughput, mostly to transfer the output from sensors and to control simple actuators. The default MTU size is 23 bytes (3 of which are header) and this cannot be changed with the current chip used for development. Sticking to the default MTU size also maximizes compatibility with existing handsets since they might or might not allow change to this parameter.

For these reasons our application protocol will not be designed around BLE. The device will only have two services

1. The [Device Information Service](https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.service.device_information.xml), which is standardized and has an UID of 0x180A
2. The Status Hardware Wallet Service, described below

### Status Hardware Wallet Service

The Status Hardware Wallet service is a non-standard service with UUID 3adbfde2-9b0c-4045-aeff-b698aeb269b4 with a single characteristic with UID f99f3a02-9b99-46cf-96f0-e4947feae20e with Write Without Response and Notify permissions. The name of the characteristic is "IO".

From the transport protocol point of view, both the client and server can initate communication over the I/O characteristic, either by writing (client to server) or by notifying (server to client). The transport layer has a 1 byte header, where the two upper bits are used for segmentation and the other bits are reserved for opcodes, the meaning of which is specific to the application.

The responsibility of the transport protocol is solely to handle transfer of segments and pass the reassembled APDU to the upper layer.

When the application wants to transfer an APDU, it passes it to the transport layer which splits it in several segments and reassembles it on the receiver side. The first byte always contains the application-specific opcode and the segmentation info. The uppermost bit indicates if the segment is the first in the sequence. If this bit is set, any pending segment is discarded and this segment is treated as the first one. On subsequent segments this bit must be 0. The second uppermost bit indicates if the segment is the last one in the sequence. If this bit is set, the APDU is considered to be complete and is passed to the upper layer of the protocol stack. A segment can be the first and only segment if both bits are set. The opcode is carried in all segments and must match the current sequence. The first segment defines which opcode will be accepted. Segments carrying a different opcode will silently be rejected. After a reset or after a segment with the "last segment" indication has been received, only segments with the "first segment" indication will be accepted.

The reassembled APDU will contain the opcode (without segmentation info) on the first byte.

The BLE channel used for the communication must be encrypted and authorized. More details on this aspect TBD.

## Application protocol 

The client communicates with the device through a command interface. The client is always the initiator and the application always responds to commands. If no response comes within 15 seconds the request has to be considered as timed out.

### Commands

All commands have their own structure, there is only one header byte indicating the opcode. The opcode is coded in the lower 6 bits, leaving the two upper bits to transport-level specific details.

Currently the following opcodes are defined

| Opcode |  Command   |
|--------|------------|
|  0x00  | Initialize |
|  0x01  |  Restore   |
|  0x02  |    Sign    |
|  0x03  |Disable PIN |
|  0x04  | Enable PIN |
|  0x05  | Change PIN |
|  0x06  | Get Status |
|  0x10  |  Load FW   |
|  0x11  | Upgrade FW |

Each command will be described in details later in the document

### Response

All commands generate a response. The exact format of the response depends on the command, but the first byte always carries the opcode of the command which is being responded as well as the transport-level specific details as for the commands. The second byte of the response always carries the status code.

Currently the following status codes are defined

|  Code  |     Status     |
|--------|----------------|
|  0x00  |    No error    |
|  0x01  |  Unauthorized  |
|  0x02  | User cancelled |
|  0x03  |  Invalid data  |
|  0x04  | Uninitialized  |
|  0x10  |   Invalid FW   |
|  0x3f  |  Unkown error  |

### Initialize

TBD

### Restore

TBD

### Sign

TBD

### Disable PIN

TBD

### Enable PIN

TBD

### Change PIN

TBD

### Get Status

TBD

### Load FW

TBD

### Upgrade FW

TBD
