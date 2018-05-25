# HW Pro UI

Being a widget with a segmented display and two buttons, defining the UI is crucial to determine which features we can support and even what security measures can be implemented.

## Security related issues

To maximize security the device should require all sensitive input to come from its own input devices to avoid a compromised client to record and forward sensitive information or even autonomously performing transactions. For the same reasons, the device should be able to display as much information as possible about the transaction to be signed. These are significant challenges given how we aim for a very reduced device size.

## Input

The device will have either capacitive touch buttons or resistive ones (cheaper/highly realiable, but require a flexible dome-like structure with a conductive pad if we want a sealed device to avoid exposing contacts). The device will have two buttons, UP and DOWN. If we go for capacitative buttons we might design them as to be two halves of a circle. The other option is placing classic round buttons next to each other.

The interaction will go as follows:

1. If the device is off, pressing any button turns it on
2. If it is on, pressing the DOWN button long enough turns it off
3. PIN entry would go as follow: UP increases the digit, DOWN decreases it, in a cyclic manner (pressing DOWN when you have 0 gives you 9, pressing up when you have 9 gives 0). Pressing UP longer confirms the digit (if it is the last one, confirms the whole PIN). Pressing down longer deletes the last digit (if no digit has been entered, cancels PIN entry)
4. Mnemonic entry goes the same way, with the same shortcut described above. To make things faster, after each letter is entered only letters available in the remaining list of possible words are shown. Also only the unique part of the word needs to be entered (I think the first 4 letters are enough?)
5. In other situations, UP means OK, DOWN means cancel.

## Output

The output device will be a segment-based LCD display. Most segments will be used for numerical/alphanumerical address characters, while some will be for custom icons. The MCU used is able to drive LCD screens directly, up to a theoretical 320 segments (multiplexed 8x40, thus requiring 48 pins). I have calculated that we can actually drive 192-208 segments when enabling the other peripherals we need. Also the more segments we drive, the more power we are going to use. PCB space increases because of the additional tracks and because a screen with more segments might need to be physically larger to be legible.

At the same time, we need to be able to display as much information as possible on a transaction as we can and we must be able to display the mnemonics on the device's screen, so they never reach the client device.

As it happens, cryptocurrency transactions are probably the worst case scenario for devices with limited screen estate, since they usually contain many digits and addresses are long. For this reason we will have to find some compromises.

We will need space for some icons, such as the currency icon (ETH, ERC20), battery icon (with 3 bars).

The main decision points are

1. How many lines of digits/letters we will have. This will determine how we organize the information
2. How many digits we want to be able to display. We could limit this by adding some unit-of-measure icons (micro, milli, etc) and trimming off the least significant digits. If done correctly the rounding would have no financial meaning for transactions of any size
3. How to handle the decimal point. In a two lines display the upper line can be the integer part and the lower one the decimal part. Otherwise we will need a dot segment between any two digits if we want the decimal point to be at arbitrary locations.
4. How detailed should alphabetic characters be displayed. A classical 7-segment digit is enough to for all hexadecimal digits legibly. Other letters can also be represented, but those with diagonal lines (such as W, M, Z, X, K), require quite a bit of imagination to represent and the user will need a translation table. Some letters are also only possible as lowercase and some as uppercase. A 14-segment display can display all letters legibly (altough some in lowercase and some in uppercase only) but each character costs twice the number of segments.
5. How we display the destination address. We can display the amount and the address sequentially (so first you confirm the amount, then you confirm the destination or viceversa) or we can show them at the same time on separate lines. The first solution requires more interaction but allows showing more detailed information. Since Ethereum addresses are very long, we can only display a part. In a two-lines design the upper line would display the first digits and the lower line the last digits.
6. In a hierachic wallet, how we display the one we are trying to use? We can have a few predetermined possibilities with a dedicated icon, or it can again be its address displayed in sequence with the other information.

A screen is a little difficult to describe by words, so I will work on a few alternative designs to bring up for discussion.



