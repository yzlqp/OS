/**
 * @file user.h
 * @author ylp
 * @brief 
 * @version 0.1
 * @date 2022-05-03
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _USER_H_
#define _USER_H_

typedef signed char 		int8_t;
typedef unsigned char 		uint8_t;
typedef signed short 		int16_t;
typedef unsigned short 		uint16_t;
typedef signed int 			int32_t;
typedef unsigned int 		uint32_t; 
typedef signed long int   	int64_t;
typedef unsigned long int   uint64_t;
typedef unsigned long 		size_t;
typedef unsigned int   		uint;
typedef unsigned short 		ushort;
typedef unsigned char 		uchar;
typedef unsigned char 		uint8;
typedef unsigned short 		uint16;
typedef unsigned int  		uint32;
typedef unsigned long 		uint64;

#define ARRAY_SIZE(a)  (sizeof(a) / sizeof(a[0]))
#define DIRSIZ 		   14

#undef NULL
#if defined(__cplusplus)
#define NULL 0
#else
#define NULL ((void *)0)
#endif

#define O_RDONLY  0x000
#define O_WRONLY  0x001
#define O_RDWR    0x002
#define O_CREATE  0x200
#define O_TRUNC   0x400

#define T_DIR     1     // Directory
#define T_FILE    2     // File
#define T_DEVICE  3     // Device

#define CONSOLE   1

struct dirent {
    uint16_t inum;
    char name[DIRSIZ];
};

struct stat {
	int dev;     		// File system's disk device
	uint32_t ino;   	// Inode number
	short type;  		// Type of file
	short nlink; 		// Number of links to file
	uint64_t size; 		// Size of file in bytes
};

/*
 * User system call C function prototype
 */
int fork(void);
int wait(int *state);
int exec(char *path, char **argv);
void yield(void);
int kill(int pid);
int exit(int state) __attribute__((noreturn));
int getpid(void);
int pipe(int (*fd)[2]);
int chdir(const char *path);
char *sbrk(int nbytes);
int sleep(int nsec);
int uptime(void);
int fstat(int fd, struct stat *stat);
int mknod(const char *path, short major, short minor);
int mkdir(const char *path);
int open(const char *path, int mode);
int close(int fd);
int read(int fd, void *buf, int nbytes);
int write(int fd, const void *buf, int nbytes);
int dup(int fd);
int link(const char *, const char *);
int unlink(const char *path);

/*
 * User library functions
 */
int stat(const char *n, struct stat *st);
char *strcpy(char *dst, const char *src);
void *memmove(void *dst, const void *src, int n);
char *strchr(const char *s, char c);
int strcmp(const char *p, const char *q);
void fprintf(int fd, const char *fmt, ...);
void printf(const char *fmt, ...);
char *gets(char *buf, int max); 
uint strlen(const char *s);
void *memset(void *dst, int c, uint n);
void *malloc(uint nbytes);
void free(void *addr);
int atoi(const char *s);
int memcmp(const void *s1, const void *s2, uint n);
void *memcpy(void *dst, const void *src, uint n);

#endif /* _USER_H_ */