/* TODO */

#include "shell.h"
#include "filesystem.h"
#include "terminal.h"

#define SBUFSIZE 33

static const unsigned char cmd_ls[]  = "ls";
static const unsigned char cmd_cat[] = "cat";
static const unsigned char cmd_rtc[] = "rtc";

/*
 * TODO
 * Supported commands:
 *   ls - list all files in filesystem
 *   cat - print contents of
 */
int
shell_loop()
{
    while(1)
    {
        uint8_t buf[MAX_INPUT_BUFFER_SIZE];
        uint32_t bytes_read;
        /* blocks until we have complete input from the keyboard */
        bytes_read = keyboard_read(0, buf, MAX_INPUT_BUFFER_SIZE);

        if (!strncmp(cmd_ls, buf, sizeof(cmd_ls)))
            command_list(buf + 1 + sizeof(cmd_ls), bytes_read);
        else if (!strncmp(cmd_cat, buf, sizeof(cmd_cat)))
            command_cat(buf + 1 + sizeof(cmd_cat), bytes_read);
        else if (!strncmp(cmd_rtc, buf, sizeof(cmd_rtc)))
            command_rtc(buf + 1 + sizeof(cmd_rtc), bytes_read);
        else
            terminal_write("Command not supported!\n");
    }

    return 0;
}


/*
 * TODO
 */
int
command_list(const uint8_t * params, uint32_t paramsize)
{
    int32_t fd, cnt;
    uint8_t buf[SBUFSIZE];

    if (-1 == (fd = fs_open((uint8_t*)".")))
    {
        terminal_write("Directory open failed\n");
        return 2;
    }

    while (0 != (cnt = fs_read(fd, buf, SBUFSIZE-1))) {
        if (-1 == cnt) {
	        terminal_write("Directory entry read failed\n");
	        return 3;
	    }
	    buf[cnt] = '\n';
	    if (-1 == terminal_write(1, buf, cnt + 1))
	        return 3;
    }
}


/*
 * TODO
 */
int
command_cat(const uint8_t * params, uint32_t paramsize)
{
    
}


/*
 * TODO
 */
int
command_rtc(const uint8_t * params, uint32_t paramsize)
{
    
}
