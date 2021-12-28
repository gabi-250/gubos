#ifndef __PS2_H__
#define __PS2_H__

#include <stdint.h>
#include <stdbool.h>

// ======================================================================
// I/O ports
// ======================================================================
//
// The data read from this port originates either from the PS/2 device or from
// the PS/2 controller. Similarly, data written to this port is received either
// by the PS/2 device or by the controller itself (depending on the command
// used)
#define PS2_DATA                          0x60
// Write-only. Any bytes written to this port are interpreted by the PS/2
// controller as commands.
#define PS2_CTRL_CMD_REGISTER             0x64
// Read-only. The value of this register reflects the state of the controller.
#define PS2_CTRL_STATUS_REGISTER          0x64

// ======================================================================
// PS/2 controller commands
// ======================================================================
#define PS2_CMD_CTRL_SELF_TEST            0xaa
// Keyboard interface self-test
#define PS2_CMD_PORT1_SELF_TEST           0xab
// Mouse interface self-test
#define PS2_CMD_PORT2_SELF_TEST           0xa0
#define PS2_CMD_DISABLE_PORT1             0xad
#define PS2_CMD_ENABLE_PORT1              0xae
#define PS2_CMD_DISABLE_PORT2             0xa7
#define PS2_CMD_ENABLE_PORT2              0xa8
#define PS2_CMD_CTRL_READ_CONFIG_BYTE     0x20
#define PS2_CMD_CTRL_WRITE_CONFIG_BYTE    0x60

// ======================================================================
// PS/2 controller status register flags
// ======================================================================
#define PS2_CTRL_STATUS_OUT_BUF_FULL     1
#define PS2_CTRL_STATUS_IN_BUF_FULL      (1 << 1)
#define PS2_CTRL_STATUS_SYSTEM_FLAG      (1 << 2)
#define PS2_CTRL_STATUS_CMD_DATA         (1 << 3)

// ======================================================================
// PS/2 controller command responses
// ======================================================================
#define PS2_CTRL_SELF_TEST_OK             0x55
#define PS2_PORT1_SELF_TEST_OK            0x00
#define PS2_PORT1_SELF_TEST_CLOCK_LOW     0x01
#define PS2_PORT1_SELF_TEST_CLOCK_HIGH    0x02
#define PS2_PORT1_SELF_TEST_DATA_LOW      0x03
#define PS2_PORT1_SELF_TEST_DATA_HIGH     0x04
#define PS2_PORT1_RESEND                  0xfe

// ======================================================================
// PS/2 controller configuration byte flags
// ======================================================================
#define PS2_CTRL_CONFIG_PORT1_INTERRUPTS  1
#define PS2_CTRL_CONFIG_PORT2_INTERRUPTS  (1 << 1)
#define PS2_CTRL_CONFIG_SYSTEM_FLAG       (1 << 2)
#define PS2_CTRL_CONFIG_PORT1_CLOCK       (1 << 4)
#define PS2_CTRL_CONFIG_PORT2_CLOCK       (1 << 5)
// Whether to enable translation to scancode set 1
#define PS2_CTRL_CONFIG_PORT1_TRANSLATION (1 << 6)
// The number of scancodes to buffer
#define PS2_SCANCODE_QUEUE_SIZE           10

bool ps2_controller_input_buffer_full();
bool ps2_controller_output_buffer_full();
void ps2_controller_send_cmd(uint8_t);
void ps2_keyboard_send_cmd(uint8_t);
void ps2_handle_irq1();
void ps2_init_devices();

#endif /* __PS2_H__ */
