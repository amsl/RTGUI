#include <io.h>
#include <fcntl.h>
#include <sys/types.h>

#include "dfs.h"

int dfs_open(const char* fn, int mode, int permission)
{
	char full_name[256];

	sprintf(full_name, "%s%s", DFS_ROOT, fn);

	/* check filename */
	return _open(full_name, mode | O_BINARY, permission);
}

int close(int fd)
{
	return _close(fd);
}

int read(int fd, void *buf, size_t len)
{
	return _read(fd, buf, len);
}

int write(int fd, const void *buf, size_t len)
{
	return _write(fd, buf, len);
}

off_t lseek(int fd, off_t offset, int whence)
{
	return _lseek(fd, offset, whence);
}
