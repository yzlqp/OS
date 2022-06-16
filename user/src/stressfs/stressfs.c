/**
 * @file stressfs.c
 * @author ylp
 * @brief 
 * @version 0.1
 * @date 2022-05-01
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "user.h"

int main(int argc, char *argv[])
{
	int fd, i;
	char path[] = "stressfs0";
	char data[512];

	printf("stressfs starting\n");
	memset(data, 'a', sizeof(data));

	for(i = 0; i < 4; i++) {
		if(fork() > 0)
			break;
	}

	printf("write %d\n", i);

	path[8] += i;
	fd = open(path, O_CREATE | O_RDWR);
	for(i = 0; i < 20; i++)
		write(fd, data, sizeof(data));
	close(fd);

	printf("read\n");

	fd = open(path, O_RDONLY);
	for (i = 0; i < 20; i++)
		read(fd, data, sizeof(data));
	close(fd);

	wait(0);

	exit(0);
}