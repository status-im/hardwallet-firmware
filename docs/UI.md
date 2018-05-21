# HW Pro UI

Being a widget with a segmented display and a limited number of physical buttons (possibly one), defining the UI is crucial to determine which features we can support and even what security measures can be implemented.

## Security related issues

To maximize security the device should require all sensitive input to come from its own input devices to avoid a compromised client to record and forward sensitive information or even autonomously performing transactions. For the same reasons, the device should be able to display as much information as possible about the transaction to be signed. These are significant challenges given how we aim for a very reduced device size.

## Input

The device will have either capacitive touch buttons or resistive ones. Buttons take PCB space and MCU I/O lines so the device will have to use a limited amount of them. Here are some options that we have to discuss

### Single button option

Giving a single button means eithe relying on external input for most things or using timing to encode additional information. A proposed solution is like this

1. If the device is off, pressing the button turns it on
2. If any other state, a long press of 2 seconds cancels the current task, 4 seconds turn the device off (actual time may vary)
3. If we are doing pin entry, a short press means +1 and a long press means confirm.
4. Theoretically mnemonics can be entered in the same way, but it would be extremely tedious. To make things much faster, after each letter is entered only letters available in the remaining list of possible works are shown. Also only the unique part of the word needs to be entered
5. In other situations a short press means OK, a longer one means cancel.

Delegating PIN and mnemonics entry (the latter used to restore from backup) to the mobile device is possible but a very bad idea, especially with regards to the mnemonics (since these alone are enough to do anything with the coins/tokens on the account).

The advantage of this method is that it the minimum cost in terms of PCB space, components and I/O lines.

The disadvantage is convenience. If the PIN is 9285 for example and we start from 0000 the user has to press 9 times shortly and one time long, then 2 times short and 1 long, 8 short and 1 long, 5 short and 1 long.

### Two buttons options

Using two buttons (UP/DOWN) we can separate the OK/Cancel and Increase/Decrease functionality so

1. If the device is off, pressing any button turns it on
2. If it is on, pressing the cancel button long enough turns it off
3. PIN entry would go as follow: UP goes to next number, DOWN to previous, in a cyclic manner. Pressing UP longer confirms, pressing down longer deletes the last digit (if no digit has been entered, cancels PIN entry)
4. Mnemonic entry goes the same way, with the same shortcut described above
5. In other situations, UP means OK, DOWN means cancel.

This increases convenience (the previous example with PIN 9285 becomes DOWN, UPlong, UP, UP, UPlong, DOWN, DOWN, UPlong, UP, UP, UP, UP, UP, UPlong which is only 14 keypresses compared to 28 of the previous example, at least if the user takes the shortest path. The cost is at least 1 additional I/O line, some PCB space and a capacitor is we used capacitative buttons.

### Scroll wheel 

A rough touch scroll wheel could be implemented as three-capacitative buttons and a larger one in the middle. PIN/mnemonics entry would be sped up by using clockwise/counterclockwise rotation to increase/decrease and the center button to confirm. This avoids long presses altogether. Additionally the 3 scrollwheel segment can be assigned functions of their own since they are still buttons. Altough quite convenient the cost in PCB space is quite large, since the wheel would need to be rather large to be useable. It is also equivalent to 4 buttons in terms I/O lines and component usage. This can only be implemented with capacitative buttons

## Output

The output device will be a segment-based LCD display. Most segments will be used for numerical/alphanumerical address characters, while some will be for custom icons. The MCU used is able to drive LCD screens directly, up to a theoretical 320 segments (multiplexed 8x40, thus requiring 48 pins). The problem is that some LCD pins are multiplexed to other devices (such as SPI, Touch Sense Controller) and thus not all channels are actually available in our case and in the end the safer choice is to keep it at around half of that. Also the more segments we drive, the more power we are going to use. PCB space increases because of the additional tracks and possibly using a larger version of the same MCU with more pins. Also a screen with more segments might need to be physically larger to be legible. If we really need more segments that the MCU can provide, we will need an external LCD controller which adds cost and requires extra PCB space. Chip-On-Glass LCD (controller is in the screen itself) is also possible, but the costs are much higher than the other solutions.

At the same time, we need to be able to display as much information as possible on a transaction as we can and we must be able to display the mnemonics on the device's screen, so they never reach the client device.

As it happens, cryptocurrency transactions are probably the worst case scenario for devices with limited screen estate, since they usually contain many digits and addresses are long. For this reason we will have to find some compromises.

We will need space for some icons, such as the currency icon (ETH, SNT), battery icon (with 3 bars).

The main decision points are

1. How many lines of digits/letters we will have. This will determine how we organize the information
2. How many digits we want to be able to display. We could limit this by adding some unit-of-measure icons (micro, milli, etc) and trimming off the least significant digits. If done correctly the rounding would have no financial meaning for transactions of any size
3. How to handle the decimal point. In a two lines display the upper line can be the integer part and the lower one the decimal part. Otherwise we will need a dot segment between any two digits if we want the decimal point to be at arbitrary locations.
4. How detailed should alphabetic characters be displayed. A classical 7-segment digit is enough to for all hexadecimal digits legibly. Other letters can also be represented, but those with diagonal lines (such as W, M, Z, X, K), require quite a bit of imagination to represent and the user will need a translation table. Some letters are also only possible as lowercase and some as uppercase. A 14-segment display can display all letters legibly (altough some in lowercase and some in uppercase only) but each character costs twice the number of segments.
5. How we display the destination address. We can display the amount and the address sequentially (so first you confirm the amount, then you confirm the destination or viceversa) or we can show them at the same time on separate lines. The first solution requires more interaction but allows showing more detailed information. Since Ethereum addresses are very long, we can only display a part. In a two-lines design the upper line would display the first digits and the lower line the last digits.
6. In a hierachic wallet, how we display the one we are trying to use? We can have a few predetermined possibilities with a dedicated icon, or it can again be its address displayed in sequence with the other information.

A screen is a little difficult to describe by words, so I will work on a few alternative designs to bring up for discussion.



