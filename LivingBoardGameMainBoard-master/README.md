# Pico Main Board
This defined the expected function and operation of the pi pico on the main board as well as some implementation details

## Message Structure
This is the structure to the main board. The communication is bidirection and the structure is similar but different for which direction the information is going. In parenthesis indicates which direction the information is present as well as which readers the information is required for.

```
message:
    'R'/'W' : A byte indicating if it is from the pico ('R') or to the pico ('W')

    NFC_index(R/W012346) : 1 byte. The Index of the NFC Reader 0-3 is from left to right (1st to last)4 is the index of the trash/chip input. 5 is index of write out NFC reader/writer

    UID (R) : 7 bytes containing the uid.

    Class(R/W6) : 1 byte. Indicates the class of the player figure, or the type of equipment token. All 0 in the case of the figure being removed.

    Stats(R/W012346) : 16 bytes containing the various player/equipment stats. All 0 in the case of the figure being removed

    Text(W6) : 112 bytes. Any flavor text associated with the chip. 112 characters, can be null terminated. I.E, 112 bytes should be sent, but can null terminate string to end early. E.X "Hello" is sent as [H,E,L,L,O,\0,*,*,*,...,*]

    Full Image (W6) : 1 byte. The image index associated with the desired full image. The index and corresponding images are determined beforehand to minimize communication overhead.

    Equipment Image (W6) : 1 byte. The image index of the equipment.
```

## On Write Command to index 6

Index 6 will require writing and moving the chip. As such, whenever this command is issued, the pico will move the motor to the required position until it can read. Then it will write the data to the tag as specified by the write command. It will then move the motor again to dispense the chip, wait a little, then return back to the original position.

It it moves the motor and does not detect a chip, it will send an error message consisting of NFC_ID 0xAA. Rest of messag discarded.

## On Implementing R/W

The pico will loop through each NFC reader. It checks for a UID. If it gets one and it isn't cached yet, it will send a R message to the PI. If it found no UID, however, there was one previous, it sends a R with the UID and a blank `Stats`. After the pico loops through each reader (excluding 6), check if the board sent it any commands. If not, it will continue the loop. This sequential processing was done in order to prevent implementing shared control over the SPI or dealing with SPI being interrupted.

## On Start-up

In order to wait for the game and everything to begin, `S` should be sent to the device.


## NTAG Memory Layout

```
Page 4: 
    uint8_t type : 1;
    uint8_t class : 3; // Allows up to 7 classes (Mutant, Robot, ...)
    uint8_t na : 4; // Unused
    uint8_t reserved; // Unused
    uint8_t unknown; // Unused
    uint8_t last; //Unused 
3 bytes and 4 bits unused

Page 5-32 : Flavor Text (112 bytes)

Page 33-36 : The 16 bytes of player stats

<<Player Only : Type = 0>>
Page 70-129 : 240 bytes to store player portrait. (May move to just an index)

<<Equipment Only : type = 1>>
Page 44-123 : 320 bytes to store an image. (May move to just an index)

Page 124-129 : 24 bytes for equipment image. (May move to just an index)
```