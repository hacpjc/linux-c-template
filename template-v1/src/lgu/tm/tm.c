/*!
 * \file tm.c
 * \brief Time/date related implementations
 *
 * \date Create: 2012/4/6
 * \author hac Ping-Jhih Chen
 * \sa tm.h
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include <sys/sysinfo.h>

#include "tm.h"

/*!
 * \brief Get time string from given timestamp (format is given in fmt)
 *
 * \param ts     input time (sec)
 * \param fmt    output format e.g. RFC 822-conformant dates: "%a, %d %b %Y %H:%M:%S %z" (Tue May 12 11:11:30 CST 2009)
 * \param result container_buf to save time string
 * \param len    container_buf length
 *
 * \details Plz take a reference at man page strftime(3)
 * \details Plz take a reference at man page time(2)
 * \details
 * \par Example:
 * \code
 * char buf[128];
 *
 * memset(buf, 0x00, sizeof(buf));
 * tm_strftime(time(NULL), "%a, %d %b %Y %H:%M:%S %z", buf, sizeof(buf));
 * printf("current time: %s\n", buf);
 * \endcode
 */
extern char *tm_strftime(time_t ts, const char *fmt, char *result, size_t len)
{
	struct tm *lp;

	if (result == NULL || len <= 0 || fmt == NULL)
		return NULL;

	lp = localtime(&ts);

	if (strftime(result, len, fmt, lp) == 0) {
		return NULL; // this is not exactly an error :)
	} else {
		return result;
	}
}

/*!
 * \brief Get time string in format "%a, %d %b %Y %T %z" (RFC 2822) from given timestamp
 *
 * \param ts     input time (sec)
 * \param result container_buf to save time string
 * \param len    container_buf length
 *
 * \return the result string pointer
 * \return NULL if there's error
 */
extern char *tm_time2str(time_t ts, char *result, size_t len)
{
	return tm_strftime(ts, "%a, %d %b %Y %T %z", result, len);
}

/*!
 * \brief Get system up time (sec)
 *
 * \return system up time (sec)
 * \return 0 if we cannot get up time.
 */
extern long tm_uptime(void)
{
	struct sysinfo info;
	int res;

	res = sysinfo(&info);
	if (res < 0) {
		return 0;
	}

	return info.uptime;
}
