#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>

#include "fio_lock.h"

/**
 * @brief Lock a file at this process.
 *
 * @note Cannot lock twice at the same process.
 */
int fio_lock(const char *path)
{
	int fd;

	fd = open(path, O_RDONLY | O_CREAT, S_IRWXU | S_IRGRP | S_IROTH);
	if (fd < 0)
	{
		return -1;
	}

	if (flock(fd, LOCK_EX) < 0) {
		close(fd);
		return -1;
	}

	return fd;
}

int fio_trylock(const char *path)
{
	int fd;

	fd = open(path, O_RDONLY | O_CREAT, S_IRWXU | S_IRGRP | S_IROTH);
	if (fd < 0)
	{
		return -1;
	}

	if (flock(fd, LOCK_NB | LOCK_EX))
	{
		close(fd);
		return -1;
	}

	return fd;
}

int fio_unlock(const int fd)
{
	if (fd < 0)
	{
		return -1;
	}

	flock(fd, LOCK_UN);
	close(fd);
	return 0;
}
