/*
 * acc_func.c
 *
 *  Created on: Feb 9, 2018
 *      Author: nks
 */

#include <stdint.h>

void acc_init(){
	 accWriteReg(0x2a,0x01);


}
uint16_t read_acc_axis(uint8_t axis) {

	switch ( axis ) {
	case 0:
		axis = 0x01;
		break;

	case 1:
		axis = 0x03;
		break;

	case 2:
		axis = 0x05;
		break;

	}

	  uint16_t acc_val = 0;
	  uint8_t acc_buf = accReadReg(axis);
	  acc_val = acc_buf;
	  acc_val <<= 8;
	  acc_buf = accReadReg(axis + 1);
	  acc_val |= acc_buf;
	  acc_val >>= 2;

	  int16_t out = 0;

	  if(acc_val & (1 << 13))
	  {
		  out = 0 - (acc_val & 0x1fff);
		  //UART_print("suss\n");
	  }

	  else
	  {
		  out = acc_val & 0x1fff;
	  }
	  return out;
}

