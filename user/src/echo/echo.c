/**
 * @file echo.c
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
	int i;

	for (i = 1; i < argc; i++) {
		write(1, argv[i], strlen(argv[i]));
		if (i + 1 < argc) {
      		write(1, " ", 1);
    	} else {
      		write(1, "\n", 1);
    	}
	}
	exit(0);
}
