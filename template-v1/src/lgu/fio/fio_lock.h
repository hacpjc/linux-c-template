#ifndef FIO_FIO_LOCK_H_
#define FIO_FIO_LOCK_H_

extern int fio_lock(const char *path);
extern int fio_trylock(const char *path);
extern int fio_unlock(const int fd);

#endif
