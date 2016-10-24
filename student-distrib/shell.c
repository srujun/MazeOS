/* shell.c - defines the shell functions */

#include "shell.h"
#include "filesystem.h"
#include "terminal.h"
#include "keyboard.h"
#include "rtc.h"
#include "lib.h"

#define SBUFSIZE         33
#define MAX_PRINT_LINE   80

static const int8_t cmd_ls[2][4]  = {"ls ", "ls\0"};
static const int8_t cmd_cat[2][5] = {"cat ", "cat\0"};
static const int8_t cmd_rtc[2][5] = {"rtc ", "rtc\0"};

static const uint8_t greeting[] = "Welcome to MazeOS!\nSupported commands:\n";
static const uint8_t help[3][46] = {
    "    ls - prints files in filesystem\n",
    "    cat <filename> - prints contents of file\n",
    "    rtc - runs the RTC test\n"
};
static const uint8_t prompt[] = "MazeOS $ ";
static const uint8_t unsupported[] = "Command not supported!\n";
static const uint8_t buffer_full[] = "\nInput buffer is full!\n";
static const uint8_t noparam[] = "Provide a filename!\n";
static const uint8_t nofile[] = "File not found!\n";
static const uint8_t read_failed[] = "File read failed!\n";


/*
 * shell_loop
 *   DESCRIPTION: Enters an infinite loop which checks command input from the
 *                keyboard buffer, parses the command, and then executes it
 *   INPUTS: none
 *   OUTPUTS: the output of the command that is run is printed on the screen
 *   RETURN VALUE: 0 - success (never happens)
 *   SIDE EFFECTS: none
 */
int
shell_loop()
{
    int8_t buf[MAX_INPUT_BUFFER_SIZE];
    uint32_t bytes_read, i;

    memset(buf, '\0', MAX_INPUT_BUFFER_SIZE);
    bytes_read = 0;

    /* print greeting */
    terminal_write(0, greeting, strlen((int8_t *)greeting));
    for (i = 0; i < 3; i++)
        terminal_write(0, help[i], strlen((int8_t *)help[i]));

    while(1)
    {
        terminal_write(0, prompt, strlen((int8_t *)prompt));
        /* blocks until we have complete input from the keyboard */
        bytes_read = keyboard_read(0, buf, MAX_INPUT_BUFFER_SIZE);

        if (bytes_read >= MAX_INPUT_BUFFER_SIZE - 1)
        {
            terminal_write(0, buffer_full, strlen((int8_t *)buffer_full));
            /* clear the command buffer */
            memset(buf, '\0', MAX_INPUT_BUFFER_SIZE);
            continue;
        }

        /* check control codes */
        if (buf[0] == CTRL_L)
        {
            clear_setpos(0, 0);
            /* clear the command buffer */
            memset(buf, '\0', MAX_INPUT_BUFFER_SIZE);
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
        // else if (!strncmp(cmd_cat[1], buf, strlen(cmd_cat[1])))
        // {
        //     paramstart = buf + strlen(cmd_cat[1]);
        //     paramsize = bytes_read - strlen(cmd_cat[1]);
        //     command_cat((uint8_t *)paramstart, paramsize);
        // }
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
        else if (buf[0] == '\0' || buf[0] == ' ' || buf[0] == '\n');
        else
            terminal_write(0, unsupported, strlen((int8_t *)unsupported));

        /* clear the command buffer */
        memset(buf, '\0', MAX_INPUT_BUFFER_SIZE);
    }

    return 0;
}


/*
 * command_list
 *   DESCRIPTION: Test to print the list of files in the filesystem and their
 *                sizes
 *   INPUTS: params - the string of parameters
 *           paramsize - the length of the params
 *   OUTPUTS: none
 *   RETURN VALUE: 0 - success
 *   SIDE EFFECTS: Prints list of files to the screen
 */
int
command_list(const uint8_t * params, uint32_t paramsize)
{
    uint32_t fsize, i;
    uint8_t buf[MAX_PRINT_LINE];
    dentry_t d;

    i = 0;

    /* loop through files until there are no more */
    while (-1 != read_dentry_by_index(i, &d))
    {
        i++;
        memset(buf, ' ', MAX_PRINT_LINE);
        memcpy(buf, d.filename, SBUFSIZE);

        fsize = get_file_size(&d);
        uint8_t extra[] = "  Size: ";
        memcpy(buf + SBUFSIZE, extra, strlen((int8_t *)extra));

        /* convert the size from int to string */
        itoa(fsize, (int8_t *)(buf + SBUFSIZE + strlen((int8_t *)extra)), 10);
        buf[MAX_PRINT_LINE - 1] = '\n';

        terminal_write(1, buf, MAX_PRINT_LINE);
    }

    return 0;
}


/*
 * command_cat
 *   DESCRIPTION: Test to print the contents of a given file in the filesystem
 *   INPUTS: params - the string of parameters
 *           paramsize - the length of the params
 *   OUTPUTS: none
 *   RETURN VALUE: 0 - success, >0 - failed
 *   SIDE EFFECTS: Prints file contents to the screen
 */
int
command_cat(const uint8_t * params, uint32_t paramsize)
{
    /* we need filename */
    if(paramsize == 0)
    {
        terminal_write(1, noparam, strlen((int8_t *)noparam));
        return -1;
    }

    int32_t fd, cnt;
    uint8_t buf[9000];
    memset(buf, '\0', 9000);

    /* check if the file exists */
    if (-1 == (fd = fs_open(params)))
    {
        terminal_write(1, nofile, strlen((int8_t *)nofile));
        return 2;
    }

    fs_desc_t fd_file;
    fd_file.index = -1;
    memset(fd_file.filename, '\0', FILENAME_SIZE + 1);
    memcpy(fd_file.filename, params, paramsize);

    /* read bytes from the file */
    cnt = fs_read((int32_t)(&fd_file), buf, 9000);
    if (0 == cnt)
    {
        terminal_write(1, read_failed, strlen((int8_t *)read_failed));
        return 3;
    }
    terminal_write(1, buf, cnt);
    putc('\n');

    return 0;
}


/*
 * command_rtc
 *   DESCRIPTION: Test to check rtc_read and rtc_write
 *   INPUTS: params - the string of parameters
 *           paramsize - the length of the params
 *   OUTPUTS: none
 *   RETURN VALUE: 0 - success, >0 - failed
 *   SIDE EFFECTS: none (it resets the RTC to original speed)
 */
int
command_rtc(const uint8_t * params, uint32_t paramsize)
{
    uint8_t buf[MAX_BUFFER_SIZE];
    uint32_t freq = 2;
    // rtc_write(0, &freq, sizeof(uint32_t));
    clear_setpos(0, 0);

    while (1)
    {
        memset(buf, '\0', MAX_BUFFER_SIZE);
        /* get a copy of the keyboard buffer */
        terminal_read(0, buf, MAX_BUFFER_SIZE);

        /* check if we want to quit */
        if (buf[0] == CTRL_C)
        {
            putc('\n');
            get_kb_buffer(buf);
            /* reset to the previous frequency */
            freq = 2;
            rtc_write(0, &freq, sizeof(uint32_t));
            return 0;
        }

        /* increase frequency limit (by a factor of 2) */
        if (buf[0] == CTRL_A)
        {
            clear_setpos(0, 0);
            freq *= 2;
            if (freq > 1024)
                freq = 2;
            rtc_write(0, &freq, sizeof(uint32_t));
        }

        putc('R');
        /* wait until the next RTC interrupt */
        rtc_read(0, "", 0);
    }

    return 0;
}
