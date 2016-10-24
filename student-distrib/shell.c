/* TODO */

#include "shell.h"
#include "filesystem.h"
#include "terminal.h"
#include "keyboard.h"
#include "lib.h"

#define SBUFSIZE 33

static const int8_t cmd_ls[2][5]  = {"ls ", "ls\0"};
static const int8_t cmd_cat[2][5] = {"cat ", "cat\0"};
static const int8_t cmd_rtc[2][5] = {"rtc ", "rtc\0"};

/*
 * TODO
 * Supported commands:
 *   ls - list all files in filesystem
 *   cat - print contents of
 */
int
shell_loop()
{
    int8_t buf[MAX_INPUT_BUFFER_SIZE];
    uint32_t bytes_read;
    memset(buf, '\0', MAX_INPUT_BUFFER_SIZE);
    bytes_read = 0;

    terminal_write(0, "Welcome to MazeOS!\n", 25);
    while(1)
    {
        terminal_write(0, "MazeOS $ ", 10);
        /* blocks until we have complete input from the keyboard */
        bytes_read = keyboard_read(0, buf, MAX_INPUT_BUFFER_SIZE);

        /* check control codes */
        if (buf[0] == CTRL_L)
        {
            clear_setpos(0, 0);
            continue;
        }

        /* check commands */
        void * paramstart;
        uint32_t paramsize;
        /* ls */
        if (!strncmp(cmd_ls[0], buf, strlen(cmd_ls[0])))
        {
            paramstart = buf + strlen(cmd_ls[0]);
            paramsize = bytes_read - strlen(cmd_ls[0]);
            command_list((uint8_t *)paramstart, paramsize);
        }
        else if (!strncmp(cmd_ls[1], buf, strlen(cmd_ls[1])))
        {
            paramstart = buf + strlen(cmd_ls[1]);
            paramsize = bytes_read - strlen(cmd_ls[1]);
            command_list((uint8_t *)paramstart, paramsize);
        }
        /* cat */
        else if (!strncmp(cmd_cat[0], buf, strlen(cmd_cat[0])))
        {
            paramstart = buf + strlen(cmd_cat[0]);
            paramsize = bytes_read - strlen(cmd_cat[0]);
            command_cat((uint8_t *)paramstart, paramsize);
        }
        else if (!strncmp(cmd_cat[1], buf, strlen(cmd_cat[1])))
        {
            paramstart = buf + strlen(cmd_cat[1]);
            paramsize = bytes_read - strlen(cmd_cat[1]);
            command_cat((uint8_t *)paramstart, paramsize);
        }
        /* rtc */
        else if (!strncmp(cmd_rtc[0], buf, strlen(cmd_rtc[0])))
        {
            paramstart = buf + strlen(cmd_rtc[0]);
            paramsize = bytes_read - strlen(cmd_rtc[0]);
            command_rtc((uint8_t *)paramstart, paramsize);
        }
        else if (!strncmp(cmd_rtc[1], buf, strlen(cmd_rtc[1])))
        {
            paramstart = buf + strlen(cmd_rtc[1]);
            paramsize = bytes_read - strlen(cmd_rtc[1]);
            command_rtc((uint8_t *)paramstart, paramsize);
        }
        else
            terminal_write(0, "Command not supported!\n", 25);

        /* clear the command buffer */
        memset(buf, '\0', MAX_INPUT_BUFFER_SIZE);
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
        terminal_write(0, "Directory open failed\n", 25);
        return 2;
    }

    uint8_t fname[FILENAME_SIZE + 1] = ".";
    fs_desc_t fd_file;
    fd_file.index = -1;
    memcpy(&(fd_file.filename), fname, 33);

    while (0 != (cnt = fs_read((int32_t)(&fd_file), buf, SBUFSIZE-1))) {
        if (-1 == cnt) {
	        terminal_write(0, "Directory entry read failed\n", 30);
	        return 3;
	    }
	    buf[cnt] = '\n';
	    if (-1 == terminal_write(1, buf, cnt + 1))
	        return 3;
    }

    return 0;
}


/*
 * TODO
 */
int
command_cat(const uint8_t * params, uint32_t paramsize)
{
    if(paramsize == 0)
    {
        terminal_write(1, (uint8_t*)"Provide a filename!\n", 20);
        return -1;
    }

    int32_t fd, cnt;
    uint8_t buf[5000];

    if (-1 == (fd = fs_open(params)))
    {
        terminal_write(1, (uint8_t*)"File not found\n", 15);
        return 2;
    }

    fs_desc_t fd_file;
    fd_file.index = -1;
    memcpy(&(fd_file.filename), params, paramsize);

    cnt = fs_read((int32_t)(&fd_file), buf, 5000);
    if (-1 == cnt)
    {
        terminal_write(1, (uint8_t*)"File read failed\n", 17);
        return 3;
    }
    terminal_write(1, buf, cnt);

    return 0;
}


/*
 * TODO
 */
int
command_rtc(const uint8_t * params, uint32_t paramsize)
{
    return 0;
}
