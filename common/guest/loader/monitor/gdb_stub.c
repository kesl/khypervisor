#include <serial_s5p.h>
#include <log/uart_print.h>
#define DEBUG
#include <log/print.h>
#include <gdb_stub.h>
#include <asm-arm_inline.h>
#include <guest_monitor.h>
#include <log/sscanf.h>
#include <guest_monitor.h>

#define DEMO

static const char hexchars[] = "0123456789abcdef";

static void hex_byte(char *s, int byte) {
    s[0] = hexchars[(byte >> 4) & 0xf];
    s[1] = hexchars[byte & 0xf];
}


static void reply_ok(char *reply) {
    strcpy(reply, "OK");
}


static void reply_error(int n, char *reply) {
    reply[0] = 'E';
    hex_byte(reply + 1, n);
    reply[3] = 0;
}

static int hex(char ch) {
    if ((ch >= 'a') && (ch <= 'f'))
        return ch - 'a' + 10;
    if ((ch >= '0') && (ch <= '9'))
        return ch - '0';
    if ((ch >= 'A') && (ch <= 'F'))
        return ch - 'A' + 10;
    return -1;
}

static int get_hex_byte(char *s) {
        return (hex(s[0]) << 4) + hex(s[1]);
}

// word -> hex, little endian order
static void hex_word(char *s, unsigned long val) {
    int i;
    for (i = 0; i < 4; i++)
        hex_byte(s + i * 2, (val >> (i * 8)) & 0xff);
}


static void g_reply(char *buf) {
    int i;
    char *p;
    struct arch_regs regs;
    uint32_t sp;
    get_general_reg(&regs, &sp);

    p = buf;

    for (i = 0; i < 13; i++) {
       hex_word(p, regs.gpr[i]);
        p += 8;
    }
    hex_word(p, sp);
    p += 8;
    hex_word(p, regs.lr);
    p += 8;
    hex_word(p, regs.pc);
    p += 8;

    for (i = 0; i < 8; i++) {
        memset(p, '0', 16);
        p += 16;
    }

    hex_word(p, 0);
    p += 8;
    hex_word(p, regs.cpsr);
    p[8] = 0;
}

static void cmd_get_register(char *args, char *reply) {
    int r;

    struct arch_regs regs;
    uint32_t sp;
    get_general_reg(&regs, &sp);

    if (sscanf(args, "%x", &r) != 1) {
        reply_error(0, reply);
        return;
    }

    if (r >= 0 && r < 16) {
        if (r < 13)
            hex_word(reply, regs.gpr[r]);
        else {
            switch(r){
            case 13:
                hex_word(reply, sp);
                break;
            case 14:
                hex_word(reply, regs.lr);
                break;
            case 15:
                hex_word(reply, regs.pc);
                break;
            }
        }
        reply[8] = 0;
    } else if (r == 25) {
        hex_word(reply, regs.cpsr);
        reply[8] = 0;
    } else {
        hex_word(reply, 0);
        reply[8] = 0;
    }
}

void serial_write(unsigned char *buf, int len) {
    int i;
    for (i = 0; i < len; i++)
        serial_putc(buf[i]);
}

static void put_packet(char *buf) {
    int i, checksum;
    int ch;
    char tmp[3];

    do {
        uart_putc('$');

        checksum = 0;
        for (i = 0; buf[i]; i++)
            checksum += buf[i];

        dsb();isb();
        serial_write(buf, i);
        dsb();isb();

        tmp[0] = '#';
        hex_byte(tmp + 1, checksum & 0xff);

        dsb();isb();
        serial_write(tmp, 3);
        dsb();isb();
        ch = uart_getc();

    } while (ch != '+');

}

#define BUFMAX 1024

char packet_buf[BUFMAX];

static void get_packet(char *buf, int len) {
    int count, checksum, escaped;
    int ch;

    while (1) {
        do {
            ch = uart_getc();
        } while (ch != '$');

        checksum = 0;
        count = 0;
        escaped = 0;
        while (count < len) {
            ch = uart_getc();
            if (!escaped) {
                if (ch == '$') {
                    checksum = 0;
                    count = 0;
                } else if (ch == '#')
                    break;
                else if (ch == 0x7d) {
                    escaped = 1;
                    checksum += ch;
                } else {
                    checksum += ch;
                    buf[count] = ch;
                    count++;
                }
            } else {
                escaped = 0;
                checksum += ch;
                buf[count] = ch ^ 0x20;
                count++;
            }
        }
        buf[count] = 0;

        if (ch == '#') {
            int rchksum;

            ch = uart_getc();
            rchksum = hex(ch) << 4;
            ch = uart_getc();
            rchksum += hex(ch);

            if ((checksum & 0xff) != rchksum) {
                set_uart_mode(MODE_LOADER);
                uart_putc('-');
                dsb();
                set_uart_mode(MODE_GDB);
            } else {
                set_uart_mode(MODE_LOADER);
                uart_putc('+');
                dsb();
                set_uart_mode(MODE_GDB);
                return;
            }
        }
    }
}

char reply_buf[BUFMAX];
#define MAX_CMD_STR_SIZE    256


static inline int isxdigit(char c)
{
    return ((c >= '0') && (c <= '9'))
        || ((c >= 'a') && (c <= 'f')) || ((c >= 'A') && (c <= 'F'));
}

static void cmd_query(char *args, char *reply) {
    reply[0] = 0;
}

void cmd_get_memory(char *args, char *reply) {
    volatile uint32_t *base_memory_dump =
            (uint32_t *) (VDEV_MONITORING_BASE + MONITOR_READ_DUMP_MEMORY);
    unsigned long addr, len, i;
    volatile uint32_t *dump_base;

    if (sscanf(args, "%lx,%lx", &addr, &len) != 2) {
        reply_error(0, reply);
        return;
    }

    if (len > (BUFMAX - 16) / 2) {
        reply_error(1, reply);
        return;
    }

    send_monitoring_data(len, addr);

#ifndef DEMO
            set_uart_mode(MODE_GDB);
            printH("Addr is %x len %d\n", addr, len);
            set_uart_mode(MODE_LOADER);
#endif

    dsb();
    isb();

    *base_memory_dump;

    dsb();
    isb();

    /* Shared memory Address */
    addr = 0x4ec00100;

    for (i = 0; i < len; i++)
        hex_byte(reply + i * 2,  *((unsigned char *)(addr + i)));
    reply[len * 2] = 0;
}

static void cmd_put_memory(char *args, char *reply) {
    unsigned long addr, len, i;
    unsigned long shared_address;
    int pos;
    volatile uint32_t *base_memory_dump =
            (uint32_t *) (VDEV_MONITORING_BASE + MONITOR_READ_PUT_MEMORY);

    pos = -1;
    sscanf(args, "%lx,%lx,:%n", &addr, &len, &pos);

    if (pos == -1) {
        reply_error(0, reply);
        return;
    }

    /* Shared memory Address */
    shared_address = 0x4ec00100;

    for (i = 0; i < len; i++)
        *((unsigned char *)(shared_address + i))= get_hex_byte(args + pos + i * 2);

    send_monitoring_data(len, addr);
    *base_memory_dump;

    dsb();
    isb();

    reply_ok(reply);
}

volatile int break_signal = 0;

void reply_signal_from_break(void)
{
    break_signal = 1;

    //test
    char reply[50] = "S05";
    int uart_mode;
    reply[3] = 0;
    uart_mode = check_uart_mode();
    if (uart_mode == MODE_GDB) {
        set_uart_mode(MODE_LOADER);
        dsb();
        isb();
        put_packet(reply);
        dsb();
        set_uart_mode(uart_mode);
    } else {
        dsb();
        isb();
        put_packet(reply);
        dsb();
    }

}

void reply_signal(int n, char *reply)
{
    int signal;
    reply[0] = 'S';
    signal = 5;
    hex_byte(reply + 1, signal);
    reply[3] = 0;
}

void gdb(void)
{
    int i = 0;
    int no_reply;
    char input_cmd[MAX_CMD_STR_SIZE];
    volatile uint32_t *base_stop =
            (uint32_t *) (VDEV_MONITORING_BASE + MONITOR_READ_STOP);
    volatile uint32_t *base_go =
        (uint32_t *) (VDEV_MONITORING_BASE + MONITOR_READ_RUN);
    *base_stop;
    /* Wait gdb query */
    while(1) {
        no_reply = 0;
        set_uart_mode(MODE_GDB);
        get_packet(packet_buf, sizeof(packet_buf) - 1);
        dsb();
        set_uart_mode(MODE_LOADER);
        no_reply = 0;
        switch(packet_buf[0]) {
        case '?':
            reply_signal(0, reply_buf);
            break;
        case 'p':
            cmd_get_register(packet_buf + 1, reply_buf);
            break;
        case 'P':
            break;
        case 'g':
            g_reply(reply_buf);
            break;
        case 'G':
            break;
        case 'm':
            cmd_get_memory(packet_buf + 1, reply_buf);
            break;
        case 'M':
            cmd_put_memory(packet_buf + 1, reply_buf);
            break;
        case 'q':
            if (strcmp("qSupported;qRelocInsn+", packet_buf) == 0) {
                strcpy(reply_buf, "PacketSize=3fff");
            } else if (strcmp("qSymbol::", packet_buf) == 0){
                reply_ok(reply_buf);
            } else if (strcmp("qOffsets", packet_buf) == 0){
                strcpy(reply_buf, "Text=0;Data=0;Bss=0");
            } else {
                cmd_query(packet_buf + 1, reply_buf);
            }
            break;
        case 'c':
            *base_go;
            no_reply = 1;
            break;
        case 's':
            break;
        case 'H':
            reply_ok(reply_buf);
            break;
        case 'k':
            set_uart_mode(MODE_LOADER);
            *base_go;
            return;
        default:
            reply_buf[0] = 0;
        }
        if (!no_reply){
#ifndef DEMO
            set_uart_mode(MODE_GDB);
            printh(" // reply packet -> %s\n", reply_buf);
            set_uart_mode(MODE_LOADER);
#endif
            dsb();
            isb();
            put_packet(reply_buf);
            dsb();
        }
    }
    set_uart_mode(MODE_LOADER);
    *base_go;
}


