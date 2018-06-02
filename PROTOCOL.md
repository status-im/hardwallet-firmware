# Communication protocol

## Transport protocol

The device will communicate with the client through BLE connection. Further details TBD

## Application protocol 

The client communicates with the device through a command interface. The client is always the initiator and the application always responds to commands. 

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

All commands generate a response. The exact format of the response depends on the command, but the first byte always carries the status code. The status code is carried in the lower 6 bits, whereas the two upper bits are reserved for transport-level specific details.

Currently the following status codes are defined

|  Code  |     Status     |
|--------|----------------|
|  0x00  |    No error    |
|  0x01  |  Unauthorized  |
|  0x02  | User cancelled |
|  0x03  |  Invalid data  |
|  0x04  | Uninitialized  |
|  0x10  |   Invalid FW   |
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
