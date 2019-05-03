#ifndef ASCIIDUMP_ASCIIDUMP_H_
#define ASCIIDUMP_ASCIIDUMP_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static inline __attribute__((unused))
int __is_text(const unsigned char ch)
{
	if ((ch >= ' ' && ch <= '~') && (ch != '\\'))
	{
		return 1;
	}

	return 0;
}

static inline __attribute__((unused))
void asciidump_limit(const unsigned char *data, const int nbytes, const int limit)
{
	int i;
	register unsigned char ch;

    if (data == NULL)
        return;

    printf("\033[0;35m +%d>\033[0m ", nbytes);
    for (i = 0; i < nbytes && (limit == 0 || i < limit); i++)
    {
    	ch = data[i];

    	if (!__is_text(ch))
    	{
    		printf("\033[0;33m\\%02x\033[0m", ch);
    	}
    	else
    	{
    		printf("%c", ch);
    	}
    }

    printf("\n");
}

#define asciidump(__p, __plen) asciidump_limit(__p, __plen, 0)

#endif /* ASCIIDUMP_ASCIIDUMP_H_ */
