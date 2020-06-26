#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#define DATA_FILE "/usr/share/ecn/lpc/lpc.dat"

struct row
{
	int size;
	char *chars;
};

struct config
{
	int cx;
	int cy;
	int rowoff;
	int screenrows;
	int screencols;
	int numrows;
	struct row *rows;
	struct termios orig_termios;
};

struct config conf;

void die (const char* s)
{
	write(STDOUT_FILENO, "\x1b[2J", 4);
	write(STDOUT_FILENO, "\x1b[H", 3);

	perror(s);
	exit(1);
}

void disableRawMode (void)
{
	if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &conf.orig_termios) == -1)
		die("tcsetattr");
}

void enableRawMode (void)
{
	struct termios raw;

	if (tcgetattr(STDIN_FILENO, &conf.orig_termios) == -1)
		die("tcgetattr");

	atexit(disableRawMode);

	raw = conf.orig_termios;

	raw.c_cflag |= (CS8);
	raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
	raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
	raw.c_oflag &= ~(OPOST);
	raw.c_cc[VMIN] = 0;
	raw.c_cc[VTIME] = 1;

	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
		die("tcsetattr");
}

int getCursorPosition (int* rows, int* cols)
{
	char buf[32];
	unsigned int i = 0;

	if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4)
		return -1;

	while (i < sizeof(buf) - 1)
	{
		if (read(STDIN_FILENO, &buf[i], 1) != 1)
			break;
		if (buf[i] == 'R')
			break;
		i++;
	}
	buf[i] = '\0';

	if (buf[0] != '\x1b' || buf[1] != '[')
		return -1;
	if (sscanf(&buf[2], "%d;%d", rows, cols) != 2)
		return -1;

	return 0;
}

int getWindowSize (int* rows, int* cols)
{
	struct winsize ws;

	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0)
	{
		if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12)
			return -1;
		return getCursorPosition(rows, cols);
	}
	else
	{
		*cols = ws.ws_col;
		*rows = ws.ws_row;
		return 0;
	}
}

void  appendRow (char *s, size_t len)
{
	conf.rows = realloc(conf.rows, sizeof(struct row)*(conf.numrows+1));

	int at = conf.numrows;
	conf.rows[at].size = len;
	conf.rows[at].chars = malloc(len+1);
	memcpy(conf.rows[at].chars, s, len);
	conf.rows[at].chars[len] = '\0';
	conf.numrows++;
}

struct abuf
{
	char* b;
	int len;
};

#define ABUF_INIT {NULL, 0}

void abAppend (struct abuf* ab, const char* s, int len)
{
	char *new = realloc(ab->b, ab->len + len);

	if (new == NULL)
		return;
	memcpy(&new[ab->len], s, len);
	ab->b = new;
	ab->len += len;
}

void abFree (struct abuf* ab)
{
	free(ab->b);
}

void drawRows (struct abuf* ab)
{
	int y;
	for (y = 0; y < conf.screenrows; y++)
	{
		int filerow = y + conf.rowoff;
		if (filerow < conf.numrows)
		{
			int len = conf.rows[filerow].size;
			if (len > conf.screencols)
				len = conf.screencols;
			abAppend(ab, conf.rows[filerow].chars, len);
		}

		abAppend(ab, "\x1b[K", 3);
		if (y < conf.screenrows - 1)
			abAppend(ab, "\r\n", 2);
	}
}

void refreshScreen (void)
{
	struct abuf ab = ABUF_INIT;

	abAppend (&ab, "\x1b[?25l", 6);
	abAppend (&ab, "\x1b[H", 3);

	drawRows(&ab);

	char buf[32];
	snprintf(buf, sizeof(buf), "\x1b[%d;%dH", conf.cy-conf.rowoff+1, conf.cx+1);
	abAppend(&ab, buf, strlen(buf));

	abAppend(&ab, "\x1b[?25h", 6);

	write(STDOUT_FILENO, ab.b, ab.len);
	abFree(&ab);
}

void init (void)
{
	conf.cx = 0;
	conf.cy = 0;
	conf.rowoff = 0;
	conf.numrows = 0;
	conf.rows = NULL;

	if (getWindowSize(&conf.screenrows, &conf.screencols) == -1)
		die("getWindowSize");
}

int display_data(int nb_args, char *args[])
{
	char key_pressed, *line = NULL;
	FILE *file;
	size_t linecap = 0;
	ssize_t linelen;

	enableRawMode();
	init();

	if (getWindowSize(&conf.screenrows, &conf.screencols) == -1)
		die("getWindowSize");

	file = fopen(DATA_FILE, "r");

	if (!file)
		die("open");

	while ((linelen = getline(&line, &linecap, file)) != -1)
	{
		while (linelen > 0 && (line[linelen-1] == '\n' ||
			line[linelen-1] == '\r'))
			linelen--;
		appendRow(line, linelen);
	}

	free(line);

	while (key_pressed != 'q')
	{
		read(STDIN_FILENO, &key_pressed, 1);
		if ((linelen = getline(&line, &linecap, file)) != -1)
		{
			while (linelen > 0 && (line[linelen-1] == '\n' ||
				line[linelen-1] == '\r'))
				linelen--;
			appendRow(line, linelen);
		}
		refreshScreen();
	}

	write(STDOUT_FILENO, "\x1b[2J", 4);
	write(STDOUT_FILENO, "\x1b[H", 3);

	fclose(file);

	return EXIT_SUCCESS;
}

int check_args(int nb_args, char *args[])
{
	char *ptr;
	int ret;
	unsigned long val;

	if (nb_args != 3)
		ret = 1;
	else if ((ptr = strstr(args[1], "BAT")) == NULL || ptr != args[1])
		ret = 2;
	else if (ptr[3] < 48 || ptr[3] > 57 || (*ptr && ptr[4] != '\0'))
		ret = 2;
	else
	{
		val = strtoul(args[2], &ptr, 10);

		if (!ptr || (*ptr && (*ptr != '\0')) || val > 100)
			ret = 3;
		else
			ret = -1;
	}

	return ret;
}

int save_data(int nb_args, char *args[])
{
	FILE *save_file;
	int ret;

	ret = check_args(nb_args, args);

	if (ret < 0)
	{
	 	save_file = fopen(DATA_FILE, "a");

		if (save_file == NULL)
			ret = 1;
		else
			fprintf(save_file, "%s %d\n", args[1], atoi(args[2]));

		fclose(save_file);
	}

	return ret;
}

int main(int argc, char *argv[])
{
	int ret;

	if (argc == 1)
		ret = display_data(argc, argv);
	else
		ret = save_data(argc, argv);

	return ret;
}
