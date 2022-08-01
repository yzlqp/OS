/**
 * @file find.c
 * @author ylp
 * @brief 
 * @version 0.1
 * @date 2022-07-27
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "user.h"

/*
 * Format the path as a file name
 */
char *fmt_name(char *path)
{
    static char buf[DIRSIZ + 1];
    char *p;

    // Find first character after last slash.
    for (p = path + strlen(path); p >= path && *p != '/'; p--);
    p++;
    memmove(buf, p, strlen(p) + 1);
    return buf;
}

/*
 * If the system file name is the same as the name to be found, print the full path of the system file
 */
void eq_print(char *fileName, char *findName)
{
	if (strcmp(fmt_name(fileName), findName) == 0)
		printf("%s\n", fileName);
}

/*
 * Find a file in a path
 */
void find(char *path, char *findName)
{
	int fd;
	struct stat st;	
	if ((fd = open(path, O_RDONLY)) < 0) {
		fprintf(2, "find: cannot open %s\n", path);
		return;
	}
	if (fstat(fd, &st) < 0) {
		fprintf(2, "find: cannot stat %s\n", path);
		close(fd);
		return;
	}
	char buf[512], *p;	
    struct dirent de;
	switch (st.type) {	
		case T_FILE:
			eq_print(path, findName);			
			break;
		case T_DIR:
			if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf) {
				printf("find: path too long\n");
				break;
			}
			strcpy(buf, path);
			p = buf + strlen(buf);
			*p++ = '/';
			while (read(fd, &de, sizeof(de)) == sizeof(de)) {
				//printf("de.name:%s, de.inum:%d\n", de.name, de.inum);
				if (de.inum == 0 || de.inum == 1 || strcmp(de.name, ".")==0 || strcmp(de.name, "..") == 0)
					continue;				
				memmove(p, de.name, strlen(de.name));
				p[strlen(de.name)] = 0;
				find(buf, findName);
			}
			break;
    }
	close(fd);	
}

int main(int argc, char *argv[])
{
	if (argc < 3) {
		printf("find: find <path> <fileName>\n");
		exit(0);
	}
	find(argv[1], argv[2]);
	exit(0);
}
