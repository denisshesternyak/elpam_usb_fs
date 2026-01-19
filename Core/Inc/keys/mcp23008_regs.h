#ifndef INC_KEYS_MCP23008_REGS_H_
#define INC_KEYS_MCP23008_REGS_H_

//  Registers                           //  description              datasheet P9
#define MCP23008_DDR_A          0x00    //  Data Direction Register A   P 10
#define MCP23008_POL_A          0x01    //  Input Polarity A            P 11
#define MCP23008_GPINTEN_A      0x02    //  NOT USED interrupt enable   P 12
#define MCP23008_DEFVAL_A       0x03    //  NOT USED interrupt def      P 13
#define MCP23008_INTCON_A       0x04    //  NOT USED interrupt control  P 14
#define MCP23008_IOCR           0x05    //  IO control register         P 15
#define MCP23008_PUR_A          0x06    //  Pull Up Resistors A         P 16
#define MCP23008_INTF_A         0x07    //  NOT USED interrupt flag     P 17
#define MCP23008_INTCAP_A       0x08    //  NOT USED interrupt capture  P 18
#define MCP23008_GPIO_A         0x09    //  General Purpose IO A        P 19
#define MCP23008_OLAT_A         0x0A    //  NOT USED output latch       P 20


//  IOCR = IO CONTROL REGISTER bit masks   - details datasheet P15
#define MCP23008_IOCR_SEQOP     0x20    //  Sequential Operation mode bit.
#define MCP23008_IOCR_DISSLW    0x10    //  Slew Rate control bit for SDA output.
#define MCP23008_IOCR_HAEN      0x08    //  Hardware Address Enable bit (MCP23S17 only).
#define MCP23008_IOCR_ODR       0x04    //  Configures the INT pin as an open-drain output.
#define MCP23008_IOCR_INTPOL    0x02    //  This bit sets the polarity of the INT output pin.
#define MCP23008_IOCR_NI        0x01    //  Not implemented.

#endif /* INC_KEYS_MCP23008_REGS_H_ */
