
#include <assert.h> /* assert */
#include <fcntl.h>  /* O_CREAT */
#include <limits.h> /* NGROUPS_MAX */
#include <stddef.h> /* NULL, size_t */
#include <unistd.h> /* confstr, getcwd, getgroups, ... */
#include <errno.h>  /* ENOSYS, ENOMEM */
#include <stdarg.h> /* va_list, va_start, va_end */
#include <stdlib.h> /* calloc */
#include <dlfcn.h>  /* dlsym */
#include <string.h> /* strcmp */

#include "alias.h" /* alias */

/**
 * Get configurable variables, with buffer overflow checking.
 *
 * LSB 5.0: LSB-Core-generic/baselib---confstr-chk-1.html
 */
size_t __confstr_chk(int name, char *buf, size_t len, size_t buflen)
{
	assert(buf != NULL ? buflen >= len : len == 0);

	return confstr(name, buf, len);
}

/**
 * Get the pathname of the current working directory, with buffer overflow
 * checking.
 *
 * LSB 5.0: LSB-Core-generic/baselib---getcwd-chk-1.html
 */
char *__getcwd_chk(char *buf, size_t len, size_t buflen)
{
	assert(buf != NULL);
	assert(buflen >= len);

	return getcwd(buf, len);
}

/**
 * Get supplementary group IDs, with buffer overflow checking.
 *
 * LSB 5.0: LSB-Core-generic/baselib---getgroups-chk-1.html
 */
int __getgroups_chk(int gidsetsize, gid_t *grouplist, size_t listlen)
{
	assert(grouplist != NULL);
	assert(listlen / sizeof(*grouplist) >= (size_t) gidsetsize);

	return getgroups(gidsetsize, grouplist);
}

/**
 * Get name of current host, with buffer overflow checking.
 *
 * LSB 5.0: LSB-Core-generic/baselib---gethostname-chk-1.html
 */
int __gethostname_chk(char *name, size_t namelen, size_t buflen)
{
	assert(name != NULL);
	assert(buflen >= namelen);

	return gethostname(name, namelen);
}

/**
 * Get login name, with buffer overflow checking.
 *
 * LSB 5.0: LSB-Core-generic/baselib---getlogin-r-chk-1.html
 */
int __getlogin_r_chk(char *name, size_t namelen, size_t buflen)
{
	assert(name != NULL);
	assert(buflen >= namelen);

	return getlogin_r(name, namelen);
}

/**
 * Get memory page size.
 *
 * LSB 5.0: LSB-Core-generic/baselib---getpagesize.html
 */
int __getpagesize(void)
{
	return getpagesize();
}

/**
 * Get the process group ID for a process.
 *
 * LSB 5.0: LSB-Core-generic/baselib---getpgid-1.html
 */
pid_t __getpgid(pid_t pid)
{
	return getpgid(pid);
}

int __open_2(const char *path, int oflag)
{
	assert(!(oflag & O_CREAT));

	return open(path, oflag);
}
alias(__open_2, __open64_2);

/**
 * Read from a file, with buffer overflow checking.
 *
 * LSB 5.0: LSB-Core-generic/baselib---pread-chk-1.html
 */
ssize_t __pread_chk(int fd, void *buf, size_t nbytes, off_t offset,
                    size_t buflen)
{
	assert(buf != NULL);
	assert(buflen >= nbytes);

	return pread(fd, buf, nbytes, offset);
}
alias(__pread_chk, __pread64_chk);

/**
 * Read from a file, with buffer overflow checking.
 *
 * LSB 5.0: LSB-Core-generic/baselib---read-chk-1.html
 */
ssize_t __read_chk(int fd, void *buf, size_t nbytes, size_t buflen)
{
	assert(buf != NULL);
	assert(buflen >= nbytes);

	return read(fd, buf, nbytes);
}

/**
 * Read the contents of a symbolic link, with buffer overflow checking.
 *
 * LSB 5.0: LSB-Core-generic/baselib---readlink-chk-1.html
 */
ssize_t __readlink_chk(const char *path, char *buf, size_t len, size_t buflen)
{
	assert(buf != NULL);
	assert(buflen >= len);

	return readlink(path, buf, len);
}

/**
 * Get configurable system variables.
 *
 * LSB 5.0: LSB-Core-generic/baselib---sysconf.html
 */
long __sysconf(int name)
{
	return sysconf(name);
}

/**
 * Find the pathname of a terminal, with buffer overflow checking.
 *
 * LSB 5.0: LSB-Core-generic/baselib---ttyname-r-chk-1.html
 */
int __ttyname_r_chk(int fd, char *name, size_t namelen, size_t buflen)
{
	assert(name != NULL);
	assert(buflen >= namelen);

	return ttyname_r(fd, name, namelen);
}

/**
 * Test whether a process is in a group.
 */
int group_member(gid_t gid)
{
	gid_t groups[NGROUPS_MAX];
	int ngroups = getgroups(NGROUPS_MAX, groups);

	for (int i = 0; i < ngroups; ++i) {
		if (groups[i] == gid) {
			return 1;
		}
	}

	return 0;
}

#ifndef LOADER
#error LOADER must be defined
#endif

static int (*real_execve)(const char *pathname, char *const argv[], char *const envp[]);
int execve(const char *pathname, char *const argv[], char *const envp[]) {
	if(real_execve == NULL) {
		real_execve = dlsym(RTLD_NEXT, "execve");
		if(real_execve == NULL) {
			errno = ENOSYS;
			return -1;
		}
	}

	if(!strcmp(pathname, "/proc/self/exe")) {
		char **new_argv;
		char target[PATH_MAX] = "";
		int argc = 0, i = 0;
		while(argv[i++] != 0) argc++;

		i = readlink("/proc/self/exe", target, sizeof(target));
		if(i < 0 || i == sizeof(target)) {
			errno = ENOMEM;
			return -1;
		}

		new_argv = calloc(argc + 7, sizeof(char *));
		new_argv[0] = LOADER;
		new_argv[1] = "--argv0";
		new_argv[2] = argv[0];
		new_argv[3] = "--preload";
		new_argv[4] = "/lib/libgcompat.so.0";
		new_argv[5] = "--";
		new_argv[6] = target;
		for(int j = 1, i = 7; j < argc; ++i, ++j) {
			new_argv[i] = argv[j];
		}
		return execve(LINKER, new_argv, envp);
	}
	return real_execve(pathname, argv, envp);
}

extern char **environ;
int execv(const char *pathname, char *const argv[]) {
	return execve(pathname, argv, environ);
}

static int (*real_execvp)(const char *file, char *const argv[]);
int execvp(const char *file, char *const argv[]) {
	if(!strcmp(file, "/proc/self/exe")) {
		return execv(file, argv);
	}
	if(real_execvp == NULL) {
		real_execvp = dlsym(RTLD_NEXT, "execvp");
		if(real_execvp == NULL) {
			errno = ENOSYS;
			return -1;
		}
	}
	return real_execvp(file, argv);
}

int __close(int fd)
{
	return close(fd);
}

int fcntl64(int fd, int cmd, ...) {
	int ret;
	va_list va;

	va_start(va, cmd);
	ret = fcntl(fd, cmd, va);
	va_end(va);

	return ret;
}
