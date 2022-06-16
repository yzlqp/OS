/**
 * @file sysfile.c
 * @author ylp
 * @brief 
 * @version 0.1
 * @date 2022-03-19
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "arg.h"
#include "../file/fcntl.h"
#include "../include/stdint.h"
#include "../include/param.h"
#include "../file/file.h"
#include "../fs/fs.h"
#include "../fs/log.h"
#include "../proc/proc.h"
#include "../printf.h"
#include "../lib/string.h"
#include "../pipe/pipe.h"

/*
 * Allocate a file descriptor for the given file.
 * Takes over file reference from caller on success.
 */
static int fdalloc(struct file *f)
{
	int fd;
	struct proc *p = myproc();

	for (fd = 0; fd < NOFILE; fd++) {
		if (p->ofile[fd] == 0) {
      		p->ofile[fd] = f;
      		return fd;
   		}
  	}
	return -1;
}

/*
 * Gets the file descriptor
 */
static int argfd(int n, int64_t *pfd, struct file **pf)
{
	int64_t fd;
	struct file *f;

	if (argint(n, (uint64_t *)&fd) < 0)
    	return -1;
	if (fd < 0 || fd >= NOFILE || (f = myproc()->ofile[fd]) == 0)
    	return -1;
	if (pfd)
    	*pfd = fd;
	if (pf)
    	*pf = f;
	return 0;
}

/*
 * Create is used in three cases 
 * open with the O_CREATE flag makes a new ordinary file
 * mkdir makes a new directory  
 * mkdev makes a new device file
 */
static struct inode *create(char *path, int16_t type, int16_t major, int16_t minor)
{
	// creates a new name for a new inode
    char name[DIRSIZ];
	// get the inode of the parent directory
    struct inode *dp = nameiparent(path, name);
    if(dp == NULL)
        return 0;

    ilock(dp);

	// check whether the name already exists
    struct inode *ip = dirlookup(dp, name, 0);
    if (ip != NULL) {
		// name does exists
        iunlockandput(dp);
        ilock(ip);
        if(type == T_FILE && (ip->type == T_FILE || ip->type == T_DEVICE))
            return ip;
        iunlockandput(dp);
        return 0;
    }

	// If the name does not already exist,create now allocates a new inode with ialloc
    if ((ip = ialloc(dp->dev, type)) == NULL)
        panic("create: ialloc failed. \n");
    
    ilock(ip);

	// From here, create holds both IP and DP locks
	// There is no possibility of deadlock, because the inode ip is freshly allocated
  	// thus no other process in the system will hold ip’s lock and then try to lock dp
    ip->major = major;
    ip->minor = minor;
    ip->nlink = 1;
    iupdate(ip);
    if (type == T_DIR) {
		// If the new inode is a directory,create initializes it with . and .. entries
        dp->nlink++;
        iupdate(dp);
        if (dirlink(ip, ".", ip->inum) < 0 || dirlink(ip, "..", dp->inum) < 0)
            panic("create: create . and .. link failed.\n");
    }

 	// now that the data is initialized properly, link it into the parent directory
    if (dirlink(dp, name, ip->inum) < 0)
        panic("create: dir link failed.\n");
    
    iunlockandput(dp);
    return ip;
}

/*
 *  Create a device file.
 * int mknod(path, major, minor);
 */
int64_t sys_mknod()
{
    char *path = NULL;
    int64_t major, minor;
    struct inode *ip;
    if (argstr(0, &path) < 0)
        return -1;

    begin_op();
    if (argint(1, (uint64_t *)&major) < 0 || 
        argint(2, (uint64_t *)&minor) < 0 ||
        (ip = create(path, T_DEVICE, major, minor)) == NULL) {
            end_op();
            return -1;
    }
    iunlockandput(ip);
    end_op();
    return 0;
}

/*
 * Open a file; flags indicate read/write; returns an fd (file descriptor).
 * int open(const char* path, int mode);
 */
int64_t sys_open()
{
    char *path;
    int64_t fd;
    int64_t mode;
    int path_size;
	struct inode *ip;
    struct file *file;

    if ((path_size = argstr(0, &path)) < 0 || argint(1, (uint64_t *)&mode) < 0)
        return -1;
    
    begin_op();

	// If open is passed the O_CREATE flag, it calls create
	if (mode & O_CREATE) {
        ip = create(path, T_FILE, 0, 0);
        if (ip == NULL) {
            end_op();
            return -1;
        }
    } else {
        // Otherwise, it calls namei
        if ((ip = namei(path)) == NULL) {
            end_op();
            return -1;
        }
		// Create returns a locked inode, but namei does not, so sys_open must lock the inode itself.  
        ilock(ip);
        // This provides a convenient place to check that directories are only opened for reading, not writing.
        if (ip->type == T_DIR && mode != O_RDONLY) {
            iunlockandput(ip);
            end_op();
            return -1;
        }
    }
    // The current IP is valid, if the device file is opened, but there is a problem with the main device number
    if (ip->type == T_DEVICE && (ip->major < 0 || ip->major >= NDEV)) {
        iunlockandput(ip);
        end_op();
        return -1;
    }
    // allocates a file and a file descriptor
  	// no other process can access the partially initialized file since it is only in the current process’s table.
    if ((file = filealloc()) == NULL || (fd = fdalloc(file)) < 0) {
        if(file != NULL)
            fileclose(file);
        iunlockandput(ip);
        end_op();
        return -1;
    }

    if (ip->type == T_DEVICE) {
        file->type = FD_DEVICE;
        file->major = ip->major;
    } else {
        file->type = FD_INODE;
        file->off = 0;
    }

    file->ip = ip;
	// If it's not just written, it's read
    file->readable = !(mode & O_WRONLY); 
	// If set to write only or read and write, then it is writable
    file->writable = (mode & O_WRONLY) || (mode & O_RDWR);
	// Truncate the file content as needed
    if ((mode & O_TRUNC) && ip->type == T_FILE)
        itrunc(ip); 
    
    iunlock(ip);
    end_op();
    return fd;
}

/*
 * Release open file fd.
 * int close(int fd);
 */
int64_t sys_close()
{
    int64_t fd;
    struct file *file;
    if (argfd(0, &fd, &file) < 0)
        return -1;
    myproc()->ofile[fd] = 0;
    fileclose(file);
    return 0;
}

/*
 * Read n bytes into buf; returns number read; or 0 if end of file.
 * int read(int, void*, int);
 */
int64_t sys_read()
{
    struct file *file;
    int64_t n;
    char *p;
    if (argfd(0, 0, &file) < 0 || argint(2, (uint64_t *)&n) < 0 || argptr(1, &p, n) < 0)
        return -1;
    return fileread(file, p, n);
}

/*
 * Write n bytes from buf to file descriptor fd; returns n.
 * int write(int, void*, int);
 */
int64_t sys_write()
{
    struct file *file;
    int64_t n;
    char *p;
    if(argfd(0, 0, &file) < 0 || argint(2, (uint64_t *)&n) < 0 || argptr(1, &p, n) < 0)
        return -1;
    return filewrite(file, p, n);
}

extern int exec(char *path, char **argv);
/*
 * int exec(char* path, char**); 
 */
int64_t sys_exec()
{
    char *path;
    char *argv[MAXARG];
    uint64_t uargv;
    int64_t uarg = 0;
    if(argstr(0, &path) < 0 || argint(1, (uint64_t *)&uargv) < 0) {
        cprintf("sys_exec: invalid arguments\n");
        return -1;
    }
    
    memset(argv, 0, sizeof(argv));
    for (int i = 0; ; ++i) {
        if (i >= ARRAY_SIZE(argv)) {
            cprintf("sys_exec: too many arguments.\n");
            return -1;
        }
        if (fetchint64ataddr(uargv + sizeof(uint64_t) * i, (uint64_t*)&uarg) < 0) {
            cprintf("sys_exec: failed to fetch uarg.\n");
            return -1;
        }
        if (uarg == 0) {
            argv[i] = 0;
            break;
        }
        if (fetchstr(uarg, &argv[i]) < 0) {
            cprintf("sys_exec: failed to fetch argument.\n");
            return -1;
        }
        //cprintf("sys_exec: argv[%d] = '%s'\n", i, argv[i]);
    }
    return exec(path,argv);
}

/*
 * Return a new file descriptor referring to the same file as fd.
 * int dup(int);
 */
int64_t sys_dup()
{
    struct file *file;
    int64_t fd;
    if (argfd(0,NULL,&file) < 0) {
        cprintf("sys_dup: failed to fetch fd.\n");
        return -1;
    }
    if ((fd = fdalloc(file)) < 0) {
        cprintf("sys_dup: failed to allocate file.\n");
        return -1;
    }
    filedup(file);
    return fd;
}

/* 
 * Change the current directory.
 * int chdir(const char *);
 */
int64_t sys_chdir()
{
    char *path;
    struct proc *p = myproc();
    if (argstr(0, &path) < 0)
        return -1;

    begin_op();
    struct inode *ip = namei(path);
    if (ip == NULL) {
        end_op();
        return -1;
    }
    ilock(ip);
    if (ip->type != T_DIR) {
        iunlockandput(ip);
        end_op();
        return -1;
    }
    iunlock(ip);
    iput(p->cwd);
    end_op();
    p->cwd = ip;
    return 0;
}

/*
 * Place info about an open file into *st.
 */
int64_t sys_fstat()
{
    struct file *f;
    // user pointer to struct stat
    struct stat *st;

    if (argfd(0, 0, &f) < 0 || argptr(1, (char **)&st, sizeof(struct stat)) < 0)
        return -1;
    return filestat(f, st);
}

/*
 * Create a new directory
 * int mkdir(path);
 */
int64_t sys_mkdir()
{
    char *path;
    if(argstr(0, &path) < 0) 
		return -1;
    begin_op();
    struct inode *ip = create(path, T_DIR, 0, 0);
    if (ip == NULL) {
        end_op();
        return -1;
    }
    iunlockandput(ip);
    end_op();
    return 0;
}

/*
 * Create the path new as a link to the same inode as old. 
 * Create a new hard link, Hard links cannot be created for directories
 * int link(const char *old, const char *new);
 */
int64_t sys_link()
{
	char *old, *new;
	struct inode *dp, *ip;

	// X0 passes in the old name, x1 passes in the new name
    if (argstr(0, &old) < 0 || argstr(1, &new) < 0)
        return -1;

    begin_op();
    if ((ip = namei(old)) == NULL) {
        end_op();
        return -1;
    }

    ilock(ip);
    if (ip->type == T_DIR) {
		// Do not link directories
        iunlockandput(ip); 
        end_op();
        return -1;
    }

	// old exists and is not a directory
	ip->nlink++;
    iupdate(ip);
    iunlock(ip);

	// find the parent directory and final path element of new
	// The new parent directory must exist and be on the same device as the existing inode
    char name[DIRSIZ];
    if((dp = nameiparent(new, name)) == NULL)
        goto bad;
    ilock(dp);
	// creates a new directory entry pointing at old’s inode
    if (dp->dev != ip->dev || dirlink(dp,name,ip->inum) < 0) {
        iunlockandput(dp);
        goto bad;
	}
    iunlockandput(dp);
    iput(ip);
    end_op();
    return 0;

bad:
    ilock(ip);
    ip->nlink--;
    iupdate(ip);
    iunlockandput(ip);
    end_op();
    return -1;
}

/*
 * If this directory only contains. and .., it is empty
 */
static int is_dir_empty(struct inode *dp)
{
	int off;
    struct dirent de;
    // Skip the first two because they are.. and..
    for (off = 2 * sizeof(de); off < dp->size; off += sizeof(de)) {
        if (readi(dp, (char *)&de, off, sizeof(de)) != sizeof(de))
            panic("is_dir_empty: readi failed.\n");
        if (de.inum != 0)
            return 0;
    }
    return 1;
}

/*
 * Remove a file.
 * int unlink(const char*);
 */
int64_t sys_unlink(void)
{
	char *path;
    char name[DIRSIZ];
	struct inode *dp, *ip;
    if (argstr(0, &path) < 0)
        return -1;

    begin_op();
    if ((dp = nameiparent(path, name)) == NULL) {
        end_op();
        return -1;
    }
    ilock(dp);
    if (namecmp(name, ".") == 0 || namecmp(name, ".."))
        goto bad;

    struct dirent de;
    uint32_t off;
    if ((ip = dirlookup(dp, name, &off)) == NULL)
        goto bad;
    ilock(ip);
    if(ip->nlink < 1)
        panic("unlink: nlink < 1\n");

    if (ip->type == T_DIR && !is_dir_empty(ip)) {
        // This folder is not allowed to be empty
        iunlockandput(ip);
        goto bad;
    }
	memset(&de, 0, sizeof(de));
    if (writei(dp, (char*)&de, off, sizeof(de)) != sizeof(de))
        panic("unlink: writei failed.\n");

    if (ip->type == T_DIR) {
        dp->nlink--;
        iupdate(dp);
    }
    iunlockandput(dp);

    ip->nlink-- ;
    iupdate(ip);
    iunlockandput(ip);
    end_op();
	return 0;

bad:
	iunlockandput(dp);
	end_op();
	return -1;
}

/*
 * Create a pipe, put read/write file descriptors in p[0] and p[1].
 * A system call to create an (anonymous) pipe
 */
int64_t sys_pipe(void)
{
	int (*fdarray)[2];
	struct file *rf, *wf;
	struct proc *p = myproc();
	int fd0 = -1, fd1 = -1;

    if (argptr(0, (char **)&fdarray, sizeof(fdarray)) < 0) 
        return -1;
    if (pipealloc(&rf, &wf) < 0)
        return -1;
	// Assign two new file descriptors fd0 and fd1
    if ((fd0 = fdalloc(rf)) < 0 || (fd1 = fdalloc(wf)) < 0) {
        if (fd0 >= 0) 
            p->ofile[fd0] = NULL;
        fileclose(rf);
        fileclose(wf);
        return -1;
    }
	// Copy the two file descriptors fd0 and fd1 back into the user
    (*fdarray)[0] = fd0;
    (*fdarray)[1] = fd1;
    return 0;
}