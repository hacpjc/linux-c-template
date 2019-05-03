/*!
 * \file tm.h
 * \brief Time/date related implementations
 *
 * \date Create: 2012/4/6
 * \author hac Ping-Jhih Chen
 * 
 * \details
 * \par Example:
 * \code
#include <stdio.h>
#include <time.h>

#include "tm.h"

int main(void)
{
	time_t ts;
	char buf[TM_TIMESTR_LEN];

	ts = time(NULL);

	// convert the current date time from time(2) to a string
	tm_time2str(ts, buf, sizeof(buf));
	printf("current date/time: %s\n", buf);

	return 0;
}
 * \endcode
 * \par Example:
 * \code
#include <stdio.h>

#include "tm.h"

int main(void)
{
	long uptime;

	// get system uptime
	uptime = tm_uptime();
	printf("current system uptime: %ld\n", uptime);

	return 0;
}
 * \endcode
 */

#ifndef TM_H_
#define TM_H_

#include <time.h>

/*!
 * Recommended time string length
 */
#define TM_TIMESTR_LEN 32

extern char *tm_strftime(time_t ts, const char *fmt, char *result, size_t len);
extern char *tm_time2str(time_t ts, char *result, size_t len);

extern long tm_uptime(void);

#endif /* TM_H_ */
