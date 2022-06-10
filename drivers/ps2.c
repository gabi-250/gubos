#include <stdint.h>
#include <stdbool.h>
#include <ps2.h>
#include <portio.h>
#include <pic.h>
#include <printk.h>

struct scancode_queue_context {
    uint32_t start;
    uint32_t end;
    uint8_t buf[PS2_SCANCODE_QUEUE_SIZE];
} scancode_queue_ctx;

static void
init_scancode_queue() {
    scancode_queue_ctx.start = 0;
    scancode_queue_ctx.end = 0;
}

static void
ps2_controller_wait_ready_for_cmd() {
    // XXX: introduce a timeout/error handling so we don't loop forever if the
    // PS/2 controller misbehaves.
    while (ps2_controller_input_buffer_full()) {
        // busy loop until the controller is ready to accept the command
        printk_warn("PS/2 controller not ready");
    }
}

static void
ps2_flush_controller_output_buffer() {
    // Make sure there is nothing in the buffer.
    while (ps2_controller_output_buffer_full()) {
        inb(PS2_DATA);
    }
}

static uint8_t
ps2_controller_configure() {
    // The configuration byte of the controller.
    uint8_t ps2_controller_config = inb(PS2_CMD_CTRL_READ_CONFIG_BYTE);
    ps2_controller_send_cmd(PS2_CMD_CTRL_WRITE_CONFIG_BYTE);
    ps2_controller_config |= PS2_CTRL_CONFIG_PORT1_INTERRUPTS;
    ps2_controller_config |= PS2_CTRL_CONFIG_PORT2_INTERRUPTS;
    ps2_controller_config |= PS2_CTRL_CONFIG_SYSTEM_FLAG;
    ps2_controller_config |= PS2_CTRL_CONFIG_PORT1_CLOCK;
    ps2_controller_config |= PS2_CTRL_CONFIG_SYSTEM_FLAG;
    // NOTE: the translation bit is not set (i.e. the translation from scancode
    // set 2 to scancode set 1 is disabled), so this effectively tells the
    // controller to use scancode set 2 We use scancode set 2 because it's
    // supposed to be supported by most (all?) keyboards, unlike scancode sets 1
    // and 3.
    ps2_controller_config &= (~PS2_CTRL_CONFIG_PORT1_TRANSLATION);
    outb(PS2_DATA, ps2_controller_config);
    return ps2_controller_config;
}

static void
ps2_controller_self_test(uint8_t ps2_controller_config) {
    ps2_controller_send_cmd(PS2_CMD_CTRL_SELF_TEST);
    uint8_t err;
    if ((err = inb(PS2_DATA)) != PS2_CTRL_SELF_TEST_OK) {
        printk_warn("PS/2 controller self-test: failed (err=%d)\n", err);
    } else {
        printk_info("PS/2 controller self-test: OK\n");
    }
    // Restore the configuration, just in case the self-test caused it to reset.
    outb(PS2_DATA, ps2_controller_config);
}

static void
ps2_interface_self_test() {
    ps2_controller_send_cmd(PS2_CMD_PORT1_SELF_TEST);
    uint8_t err = inb(PS2_DATA);
    while ((err = inb(PS2_DATA)) == PS2_PORT1_RESEND) {
        ps2_controller_send_cmd(PS2_CMD_PORT1_SELF_TEST);
        printk_warn("PS/2 keyboard self-test: failed (err=%d)\n", err);
    }
    if (err != PS2_PORT1_SELF_TEST_OK) {
        printk_warn("PS/2 keyboard self-test: failed (err=%d)\n", err);
    } else {
        printk_info("PS/2 keyboard self-test: OK\n");
    }
}

bool
ps2_controller_input_buffer_full() {
    return (inb(PS2_CTRL_STATUS_REGISTER) & PS2_CTRL_STATUS_IN_BUF_FULL) != 0;
}

bool
ps2_controller_output_buffer_full() {
    return (inb(PS2_CTRL_STATUS_REGISTER) & PS2_CTRL_STATUS_OUT_BUF_FULL) != 0;
}

void
ps2_controller_send_cmd(uint8_t cmd) {
    ps2_controller_wait_ready_for_cmd();
    outb(PS2_CTRL_CMD_REGISTER, cmd);
}

void
ps2_keyboard_send_cmd(uint8_t cmd) {
    // The commands sent to the keyboard still go through the keyboard
    // controller, so the controller must be ready to accept more input.
    ps2_controller_wait_ready_for_cmd();
    outb(PS2_DATA, cmd);
}

void
ps2_handle_irq1() {
    uint8_t scancode = inb(PS2_DATA);
    scancode_queue_ctx.buf[scancode_queue_ctx.end] = scancode;
    scancode_queue_ctx.end = (scancode_queue_ctx.end + 1) % PS2_SCANCODE_QUEUE_SIZE;
    for (int i = 0; i < PS2_SCANCODE_QUEUE_SIZE; ++i) {
        printk_debug("%d: %d\n", i, scancode_queue_ctx.buf[i]);
    }
    pic_send_eoi(PIC_IRQ1);
}

void
ps2_init_devices() {
    // XXX: check if the PS/2 controller exists (see the "IA PC Boot
    // Architecture Flags" field of the FADT).
    init_scancode_queue();
    // Disable the keyboard/mouse while the PS/2 controller is being
    // initialized.
    ps2_controller_send_cmd(PS2_CMD_DISABLE_PORT1);
    ps2_controller_send_cmd(PS2_CMD_DISABLE_PORT2);
    ps2_flush_controller_output_buffer();
    uint8_t ps2_controller_config = ps2_controller_configure();
    ps2_controller_self_test(ps2_controller_config);
    ps2_interface_self_test();
    ps2_controller_send_cmd(PS2_CMD_ENABLE_PORT1);
    ps2_controller_send_cmd(PS2_CMD_ENABLE_PORT2);
}
