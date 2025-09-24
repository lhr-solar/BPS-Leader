# Quality Assurance Checklist
To make reviews more efficient, please make sure the board meets the following standards and check everything off once the board meets the quality check. Once everything has been checked, the assigned reviewers will begin the review process. _Edit this description to check off the list._

There are exceptions with all guidelines. As long as your decisions are justified, then you are good! Contact the reviewers or the leads about any exceptions.

**Please *read* every word on *every* bullet point before checking off the corresponding box.**

## Minimum Prerequisites
- [ ] The board passes ERC and DRC.
    - Note some ERC warnings are acceptable (**_CAUTION_** please be careful)
    - Note some DRC warnings are acceptable (less so than ERC: **_CAUTION_** please be careful).
- [ ] All traces are routed.
- [ ] Units are in metric.
- [ ] Gerbers are in a zip file in the main directory (after physical design is done)

**Please *read* every word on *every* bullet point before checking off the corresponding box.**

## Schematic Level Requirements 
- [ ] Is proper noise resistance given to all peripheral devices (bypass caps and coils/ferrites)?  
- [ ] Is proper ESD protection given to all MCU input pins (zener diodes)? You could also write the input pin rating for the MCU on the schematic.  
- [ ] Is proper power protection given to peripheral devices (zener diodes)?  
- [ ] Are peripheral units used properly (reading datasheet)?  
- [ ] Are testing points added at useful places?  
- [ ] Is there proper short to GND protection at MCU outputs (inline resistors)?  
- [ ] Do ADC inputs have caps (appropriately sized to prevent RC delay)?  
- [ ] Are ADC inputs biased so there is room above expected value to determine if value is being overflowed?  
- [ ] Are LEDs located at useful places (comm, power, debugging, extra GPIO)? They should be placed in locations   
- [ ] Are parts chosen easy to collect?  
- [ ] Are parts chosen easy to solder?  
- [ ] Is there reverse polarity protection on inputs?  
- [ ] Do parts in the schematic match the actual part datasheet?

**Please *read* every word on *every* bullet point before checking off the corresponding box.**

## BOM Requirements
- [ ] **Bill Of Materials**: is there an Excel Sheet BOM and a Mouser Cart link?
- [ ] Are *all* the correct parts in the BOM?
- [ ] Is each part in the Mouser Cart in stock (or do we have the part in stock)?
- [ ] Does each part in the Mouser Cart have a margin for part quantity?
- [ ] Does every component in the schematic have a footprint, mouser link/datasheet, and reference number associated with it?

**Please *read* every word on *every* bullet point before checking off the corresponding box.**

## Layout Level Requirements 
### 2D Spacing
- [ ] The components are spaced out at an optimal distance.
    - All components can be easily hand-soldered.
    - IPC-SM-782A Standard requires a minimum distance of 1.0mm from edge to edge.
- [ ] Components that are in parallel with each other are spaced out at an equal distance when possible.
- [ ] The components are aligned with each other when possible.
- [ ] Components are grouped based off of functionality.
    - E.g. all components for CAN should be grouped.
- [ ] Bypass capacitors are less than 1cm away from their respective IC's power pins.

### 3D Spacing
- [ ] Layout of components have been placed with mounting location and usage of the board in mind.
    - Are PCBs going to be stacked on above/below this board? Are tall components placed accordingly?
    - Are buttons and lights easily accessible and viewable?
    - When mounted into the car, are the heights of the components and connectors accounted for?
- [ ] I am not reading every word and I need to start this checklist over.
    - Please uncheck everything you have checked and start over.
- [ ] Location of connectors and wires going out of the board will prevent messy wiring.
    - Are all the wires that are going in the same direction placed on the same edge of the board?

### Components
- [ ] Do footprints match the schematic?
    - Make sure pins on the footprint correspond to pins in the schematic 
    - (e.g. counterclockwise pins on footprint and alternating pins on schematic is *bad*)
- [ ] Custom footprints have been double checked with the datasheet.
- [ ] Footprints are consistent across the PCB: *i.e. all 0805 for Resistors/Capacitors*
- [ ] Pin 1 of the footprint is labeled in some way. 
- [ ] Are LED's in easy to see places? 
- [ ] Are test points in easy to reach places?  
- [ ] Are critical paths of switching converters as small as possible?  

### Copper Layer 
- [ ] The trace widths and trace clearances are greater than JLCPCB's minimum requirements (https://jlcpcb.com/capabilities/Capabilities). 
    - [ ] Are signal traces are .15mm unless provided with reasoning?   
        - NOTE: One net can have multiple trace widths  
- [ ] The trace lengths are as short as possible. 
    - Can there be a more optimal route if you go to another layer? 
- [ ] Each trace's width is capable of handling the expected current flow. 
    - Use PCB width calculator to calculate trace width. 
- [ ] *No sharp corners. No trace angles equal to or less than 90 degrees. 
    - Orthogonal traces should have vias if necessary. 
- [ ] Are edges of board surrounded by clean ground on both layers with stitching vias?  
- [ ] Traces are in parallel with each other when possible (exceptions can be made to prevent signal coupling). 
    - E.g. traces connected between an IC and MCU can be placed in parallel with each other. 
- [ ] There are no random trace appendages. 
- [ ] Vias placed in copper pads are fully encompassed in the copper pads. 
- [ ] Through-hole components do not have extraneous vias. 
- [ ] Vias should be appropriately sized according to the connected net(s) and expected current capacity.

*Not really a problem for modern manufacturing techniques but good practice and important for high speed signal integrity.

### Silkscreen Layer
- [ ] Visible text sizes are greater than .8mm(L), .8mm(H), .15mm(W).
- [ ] All visible text is on the silkscreen layer.
- [ ] All reference labels of each component are not overlapping a copper pad or another component.
- [ ] All connector pins are labeled with a meaningful and helpful name.
- [ ] All LEDs are labeled with a meaningful and helpful name.
- [ ] Silkscreens are mostly facing one direction (or 2). 

### Edge Cut Layer
- [ ] The dimensions of the board and the mounting holes are nice values in metric i.e. no long decimals.
- [ ] The physical outline of the board is on the edge cut layer.
- [ ] Edge cuts are straight and parallel with opposite edge of the board.
- [ ] Mounting holes are aligned and support M3 screws. All mounting holes should have distances between them shown in comments layer.
- [ ] Corners are curved and contoured to mounting holes.

**Please *read* every word on *every* bullet point before checking off the corresponding box.**

## IMPORTANT NOTICE
- [ ] I am confident that this board will be safe to use in a safety critical environment.
- [ ] I have read every word on every bullet point and have only checked off a bullet point if I have read every word.
- [ ] ((( **OPTIONAL** ))) I had fun :^)

## Other Comments
_Write any comments about the board that would help the reviewers here._
