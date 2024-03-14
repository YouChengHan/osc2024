#include "peripherals/mbox.h"

#include "uart0.h"
#include "utli.h"

/* mailbox message buffer */
volatile unsigned int __attribute__((aligned(16))) mbox[36];

int mbox_call(unsigned char ch) {
  unsigned int r = (((unsigned int)((unsigned long)&mbox) & ~0xF) | (ch & 0xF));
  /* wait until we can write to the mailbox */
  do {
    asm volatile("nop");
  } while (get(MBOX_STATUS) & MBOX_FULL);
  /* write the address of our message to the mailbox with channel identifier */

  set(MBOX_WRITE, r);
  /* now wait for the response */

  do {
    asm volatile("nop");
  } while (get(MBOX_STATUS) & MBOX_EMPTY);

  while (1) {
    /* is there a response? */
    do {
      asm volatile("nop");
    } while (get(MBOX_STATUS) & MBOX_EMPTY);
    /* is it a response to our message? */
    if (r == get(MBOX_READ)) /* is it a valid successful response? */
      return (mbox[1] == MBOX_CODE_BUF_RES_SUCC);
  }
  return 0;
}

void get_arm_base_memory_sz() {
  mbox[0] = 8 * 4;              // length of the message
  mbox[1] = MBOX_CODE_BUF_REQ;  // this is a request message

  mbox[2] = MBOX_TAG_GET_ARM_MEM;  // get serial number command
  mbox[3] = 8;                     // buffer size
  mbox[4] = MBOX_CODE_TAG_REQ;
  mbox[5] = 0;  // clear output buffer
  mbox[6] = 0;

  mbox[7] = MBOX_TAG_LAST;

  if (mbox_call(MBOX_CH_PROP)) {
    uart_printf("memory base address: 0x%x\n", mbox[6]);
    uart_printf("memory size: %d bytes\n", mbox[5]);
  } else {
    uart_printf("Unable to query arm memory and size..\n");
  }
}

void get_board_serial() {
  // get the board's unique serial number with a mailbox call
  mbox[0] = 8 * 4;              // length of the message
  mbox[1] = MBOX_CODE_BUF_REQ;  // this is a request message

  mbox[2] = MBOX_TAG_GET_BOARD_SERIAL;  // get serial number command
  mbox[3] = 8;                          // buffer size
  mbox[4] = MBOX_CODE_TAG_REQ;
  mbox[5] = 0;  // clear output buffer
  mbox[6] = 0;

  mbox[7] = MBOX_TAG_LAST;

  if (mbox_call(MBOX_CH_PROP)) {
    uart_printf("Serial number: 0x%x%x\n", mbox[6], mbox[5]);
  } else {
    uart_printf("Unable to query serial number..\n");
  }
}

void get_board_revision() {
  mbox[0] = 7 * 4;  // buffer size in bytes
  mbox[1] = MBOX_CODE_BUF_REQ;

  mbox[2] = MBOX_TAG_GET_BOARD_REVISION;  // tag identifier
  mbox[3] = 4;  // maximum of request and response value buffer's length.
  mbox[4] = MBOX_CODE_TAG_REQ;
  mbox[5] = 0;  // value buffer

  mbox[6] = MBOX_TAG_LAST;  // tags end

  if (mbox_call(MBOX_CH_PROP)) {
    uart_printf("Board revision number: 0x%x\n",
                mbox[5]);  // it should be 0xa020d3 for rpi3 b+
  } else {
    uart_printf("Unable to query board revision..\n");
  }
}