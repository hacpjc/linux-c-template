#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "hexdump.h"

/**
 * @brief hexdump to a file
 *
 * @code
         00 01 02 03 04 05 06 07  08 09 0A 0B 0C 0D 0E 0F  |01234567 89ABCDEF|
         -----------------------  -----------------------   -------- --------
00000000 00 01 02 03 04 05 06 07  08 09 0A 0B 0C 0D 0E 0F  |........ ........|
00000010 10 11 12 13 14 15 16 17  18 19 1A 1B 1C 1D 1E 1F  |........ ........|
00000020 20 21 22 23 24 25 26 27  28 29 2A 2B 2C 2D 2E 2F  |.!"#$%&' ()*+,-./|
00000030 30 31 32 33 34 35 36 37  38 39 3A 3B 3C 3D 3E 3F  |01234567 89:;<=>?|
00000040 40 41 42 43 44 45 46 47  48 49 4A 4B 4C 4D 4E 4F  |@ABCDEFG HIJKLMNO|
00000050 50 51 52 53 54 55 56 57  58 59 5A 5B 5C 5D 5E 5F  |PQRSTUVW XYZ[\]^_|
00000060 60 61 62 63 64 65 66 67  68 69 6A 6B 6C 6D 6E 6F  |`abcdefg hijklmno|
00000070 70 71 72 73 74 75 76 77  78 79 7A 7B 7C 7D 7E 7F  |pqrstuvw xyz{|}~.|
00000080 80 81 82 83 84 85 86 87  88 89 8A 8B 8C 8D 8E 8F  |........ ........|
00000090 90 91 92 93 94 95 96 97  98 99 9A 9B 9C 9D 9E 9F  |........ ........|
000000A0 A0 A1 A2 A3 A4 A5 A6 A7  A8 A9 AA AB AC AD AE AF  |........ ........|
000000B0 B0 B1 B2 B3 B4 B5 B6 B7  B8 B9 BA BB BC BD BE BF  |........ ........|
000000C0 C0 C1 C2 C3 C4 C5 C6 C7  C8 C9 CA CB CC CD CE CF  |........ ........|
000000D0 D0 D1 D2 D3 D4 D5 D6 D7  D8 D9 DA DB DC DD DE DF  |........ ........|
000000E0 E0 E1 E2 E3 E4 E5 E6 E7  E8 E9 EA EB EC ED EE EF  |........ ........|
000000F0 F0 F1 F2 F3 F4 F5 F6 F7  F8 F9 FA FB FC FD FE FF  |........ ........|
 * \endcode
 */
void hexdump_f(FILE *fp, const unsigned char *data, const unsigned int nbytes, const unsigned int limit)
{
	register unsigned int i;
	unsigned int line = 0;

	char hd[50] = { 0 };
	char hr[20] = { 0 };
	char hdtmp[8] = { 0 };
	char hrtmp[8] = { 0 };

	fprintf(fp, "%s, len=%u, limit=%u:\n", __FUNCTION__, nbytes, limit);

	for (i = 0; i < 16; i++)
	{
		snprintf(hdtmp, sizeof(hdtmp), "%02X ", i);
		snprintf(hrtmp, sizeof(hrtmp), "%X", i);

		strcat(hd, hdtmp);
		strcat(hr, hrtmp);
		if ((i % 16) == 7)
		{
			strcat(hr, " ");
			strcat(hd, " ");
		}
	}

	fprintf(fp,
		"         %-49s |%s|\n         -----------------------  -----------------------   -------- --------\n",
		hd, hr);
	hd[0] = '\0';
	hr[0] = '\0';

	for (i = 0; i < nbytes; i++)
	{
		snprintf(hdtmp, sizeof(hdtmp), "%02X ", data[i]);
		strcat(hd, hdtmp);

		if (data[i] < 0x21 || data[i] > 0x7e)
		{
			strcat(hr, ".");
		}
		else
		{
			snprintf(hrtmp, sizeof(hrtmp), "%c", data[i]);
			strcat(hr, hrtmp);
		}

		if ((i % 16) == 7)
		{
			strcat(hr, " ");
			strcat(hd, " ");
		}
		else if ((i % 16) == 15)
		{
			fprintf(fp, "%08X %-49s |%-17s|\n", 0x10 * line, hd, hr);
			hr[0] = '\0';
			hd[0] = '\0';
			line++;
		}
	}

	if (strlen(hr) > 0)
	{
		fprintf(fp, "%08X %-49s |%-17s|\n", 0x10 * line, hd, hr);
	}

	fflush(fp);
}

#if 0
#include <stdint.h>

int main(void)
{
	uint8_t ascii[256];

	{
		unsigned int  i;

		for (i = 0; i < 256; i++)
		{
			ascii[i] = i;
		}
	}

	hexdump(ascii, sizeof(ascii));

	return 0;
}
#endif
