## General

White space at the beginning of a line is ignored.

Tokens are separated by space characters.

Comments begin with a # mark

Labels consist of a ! separated from the label by single whitespace. Only the first word is used for the label.

Only the first character of the first word is considered for the command. Commands are case insensitive.

## Commands

0 = S (SetState)
1 = D (DelayTime)
2 = J (JumpTo)
3 = R (RandomTo)

### SetState

The state can be specifed either by a single 16 bit hex value or as a series of segments which are lit.

The hex value must begin with 0x to be recognized. If multiple hex values are encountered as tokens, even in the middle of a series of segments, the hex value immeeiately wins and sets the state. Subsequent segements will then turn on the named segment. There is no way to turn off a segment other than through ommision.

#### Segment Names

These are case insensitive. For the non-activity segments only the first character is considered. For all action mark segments the second character identifies the specific action mark surronding the named character.

The number after the name shown here is the bit shift position for this bit.

w  (elcome) = 0
h  (om)     = 1
o  (letter) = 2
e  (letter) = 3

P  (ink-dancer-on-left) = 4
P1 (Left-action-mark)  = 8
P2 (Right-action-mark) = 9

B  (lue-dancer-on-right) = 6
B1 (Left-action-mark)   = 10
B2 (Right-action-mark)  = 11

L  (eft fucker) = 5
L1 (Top left sex action mark aka sex3) = 14
L2 (Bottom left sex action mark aka sex2) = 13

R  (ight fucker) = 7
R1 (Top right sex action mark aka sex4) = 15
R2 (Bottom right sex action mark aka sex1) = 12


### DelayTime

Will cause a delay for the specified number of milliseconds. This command requires a single duration parameter. If multiple are given the last one wins.


