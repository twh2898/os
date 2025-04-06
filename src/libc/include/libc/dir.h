#ifndef LIBC_DIR_H
#define LIBC_DIR_H

typedef int dir_t;

dir_t dir_open(const char * path);
void  dir_close(dir_t);
int   dir_read(dir_t, void * dir_entry);
int   dir_seek(dir_t, int offset, int origin);
int   dir_tell(dir_t);

#endif // LIBC_DIR_H
