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

When the application wants to transfer an APDU, it passes it to the transport layer which splits it in several segments and reassembles it on the receiver side. The first byte always contains the application-specific opcode and the segmentation info. The leftmost bit indicates if the segment is the first in the sequence. If this bit is set, any pending segment is discarded and this segment is treated as the first one. On subsequent segments this bit must be 0. The second leftmost bit indicates if the segment is the last one in the sequence. If this bit is set, the APDU is considered to be complete and is passed to the upper layer of the protocol stack. A segment can be the first and only segment if both bits are set. The opcode is carried in all segments and must match the current sequence. The first segment defines which opcode will be accepted. Segments carrying a different opcode will silently be rejected. After a reset or after a segment with the "last segment" indication has been received, only segments with the "first segment" indication will be accepted.

The reassembled APDU will contain the opcode (without segmentation info) on the first byte.

The BLE channel used for the communication must be encrypted and authorized. More details on this aspect TBD.

## Application protocol 

The client communicates with the device through a command interface. The client is always the initiator and the application always responds to commands. The protocol has no timeouts, so the client application must always provide a way for the user to cancel the operation (i.e. stop waiting for a response). The reason for this is that some operation require user input, which can take an indefinite amount of time. A possible solution would be to send heartbeat packets to confirm that the server is still processing the request, however since client and server are by design a few centimeters away from each other and both are in the field of view of the user, it is simpler and more energy-efficient to let the user assess visually if the operation is taking a disproportionate amount of time (especially since no operation will take more than a few seconds, excluding the time for user input) and hit the cancel button.

The maximum APDU size is 4kb.

### Commands

All commands have their own structure, there is only one header byte indicating the opcode. The opcode is coded in the lower 6 bits, leaving the two upper bits to transport-level specific details.

Currently the following opcodes are defined

| Opcode |  Command   |
|--------|------------|
|  0x00  | Initialize |
|  0x01  |    Sign    |
|  0x02  |Get Address |
|  0x03  |Disable PIN |
|  0x04  | Enable PIN |
|  0x05  | Change PIN |
|  0x06  | Get Status |
|  0x10  |  Load FW   |
|  0x11  | Upgrade FW |

Each command will be described in details later in the document

### Response

All commands generate a response. The exact format of the response depends on the command, but the first byte always carries the opcode of the command which is being responded as well as the transport-level specific details as for the commands. The second byte of the response always carries the status code, coded on the lower 6 bits. The 2 leftmost bits are reserved for warning flags

Currently the following status codes are defined

|  Code  |     Status     |                          Description                          |          
|--------|----------------|---------------------------------------------------------------|
|  0x00  |    No error    | Correct execution                                             |
|  0x01  |  Unauthorized  | User did not authorize the operation or failed to provide PIN |
|  0x02  | User cancelled | User cancelled an operation (expect authorization requests)   |
|  0x03  |  Invalid data  | The command is malformed                                      |
|  0x04  | Uninitialized  | The device is unitialized                                     |
|  0x05  | Limit exceeded | A limit has been execeed (for example amount of pinless paths)|                                   
|  0x10  |   Invalid FW   | The loaded firmware is malformed or signature does not match  |
|  0x20  |   Low battery  | Command refused because of low battery                        |
|  0x3f  |  Unknown error | Unexpected system error                                       |

the leftmost bit is a reserved warning flags. The second leftmost bit is the low battery warning.

### Initialize

This command is sent to initialize the device. The initialization is composed of several phase. Each phase is triggered by a separate command so instructions can be displayed on the client and it is possible to resume the initialization procedure if needed. Some phases are alternative to each other and depend on whether the device was already initialized or not and if you want to restore a seed from mnemonics. The initialization phases must be executed in the correct order and no step can be repeated twice (if execution was succesful)

Phase 0:
This step authorizes the initialization (by prompting for PIN) and erases all device data. This step is only necessary if the device has been previously initialized.

Phase 1:
During this phase a PIN is generated and shown to the user. The PIN can be changed later after initialization finishes.

Phase 2a:
During this phase the seed is generated and mnemonic are displayed to the user.

Phase 2b (encoded as 3):
This is alternative to Phase 2a. In this case the seed is not generated but the mnemonics are entered on the device

Phase 4:
The master wallet is generated from the seed and its address is sent to the client. After this phase the wallet is initialized and the only possible transition is back to Phase 0.

* Parameters: Initialization phase number on 1 byte. In phase 2a or 2b the length of the checksum of the mnemonic (between 4 and 8) on the next byte, which determines the length of the sentence in words. In phase 4 a 0-64 characters long passphrase to protect the seed. The length is encoded on the first byte.
* Response: Status only on all phases except Phase 4. On phase 4 the address of the master wallet is sent.
* Low battery execution: No

### Sign

This command is used to sign a transaction. It requires PIN authorization, unless PIN has been disabled for the given key path. The response contains the RLP-encoded V, R and S (not the entire transaction for bandwidth saving reasons).

The key path is absolute and is used to derive keys according to the algorithms defined in BIP32. The implementation should keep a cache to avoid recalculating the keys every time. The maximum key path depth is 9.

The signature follows EIP-155 and the transaction is expected to contain the chain id (on 1 byte only for now) in place of V and empty R, S. 

* Parameters: RLP encoded key path as list of numbers followed by the RLP encoded Ethereum transaction.
* Response: RLP encoded V, R, S
* Low battery execution: Yes

### Get Address

This command is issued to get the address of a wallet with the given key path. The maximum key path depth is 9.

* Parameters: RLP encoded key path as list of numbers
* Response: The address of the wallet
* Low battery execution: Yes

### Disable PIN

Disables PIN entry for the given key path. The maximum key path depth is 9. When the PIN entry is disabled a simple yes/no confirmation will be required instead. PIN Authentication is required for this command.

* Parameters: RLP encoded key path as list of numbers
* Response: Only status code
* Low battery execution: Yes

### Enable PIN

Enables PIN entry for the given key path. The maximum key path depth is 9. This command only removes the given key path from the list of pinless paths. If the keypath was not in the list the command still succeeds. PIN Authentication is required for this command.

* Parameters: RLP encoded key path as list of numbers
* Response: Only status code
* Low battery execution: Yes

### Change PIN

Changes the current PIN. This happens completely on the device so the command is only used to trigger the change. This first asks for the current PIN, then for a new one which must be entered twice.

* Parameters: None
* Response: Status code only
* Low battery execution: Yes

### Get Status

Returns the current state of the device. This will include the initialization status, remaining PIN entry attempts, maybe the list of PIN-less wallets (potential security risk?) etc. The data will be RLP encoded. Exact field structure TBD.

### Load FW

Loads and writes to flash a page of firmware update. The data is not checked in any way and is simply written to the page specified by the command (relative to the firmware update area). The data length must be equal or less than the size of a physical page (2kb with the current chip). The page will always be fully programmed. If the data is shorter than the page, the missing bytes will be zeroed.

* Parameters: page number on 1 byte, data length on 2 bytes (most significant byte first) followed by the actual data.
* Response: Status code only
* Low battery execution: No

### Upgrade FW

Triggers a firmware upgrade, using the image previously loaded with the Load FW command. This command verifies the firmware's signature and if succesful reboots, so that the bootloader can perform the upgrade. When the device responds to this command "No error", it only means that the reboot has been performed, not that the upgrade is finished.

* Parameters: None
* Response: Status code only
* Low battery execution: No
