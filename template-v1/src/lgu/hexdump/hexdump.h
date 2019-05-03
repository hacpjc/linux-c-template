#ifndef HEXDUMP_HEXDUMP_H_
#define HEXDUMP_HEXDUMP_H_

#include <stdio.h>
#include <stdlib.h>

void hexdump_f(FILE *fp, const unsigned char *data, const unsigned int nbytes, const unsigned int limit);

/**
 * @brief print content in hexdump command format
 * @param data content to print
 * @param nbytes content length
 */
#define hexdump(_p, _nbytes) hexdump_f(stdout, (_p), (_nbytes), 0)
#define hexdump_limit(_p, _nbytes, _limit) hexdump_f(stdout, (_p), (_nbytes), _limit)

#endif /* HEXDUMP_HEXDUMP_H_ */
