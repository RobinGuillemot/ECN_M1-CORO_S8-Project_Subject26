#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DATA_FILE "/usr/share/ecn/lpc/lpc.dat"

int display_data(int nb_args, char *args[])
{

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
	else if (ptr[3] < 48 || ptr[3] > 57 || ptr[4] != '\0')
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
