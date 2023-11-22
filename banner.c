#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>
#include <errno.h>
#include <string.h>

// for system font
#define FONT_FILE "/usr/share/consolefonts/Lat15-VGA8.psf.gz"
struct psf_header {
	unsigned char magic[2];
	unsigned char filemode;
	unsigned char fontheight;
};
struct psf_header header;
static unsigned char *chars = NULL;

int font_open() {
	int cc = 0;
	gzFile font_file = NULL;

	font_file = gzopen(FONT_FILE, "r");
	if (font_file == NULL) {
		cc = errno;
		printf("Failed to open %s.\n", FONT_FILE);
		printf("(%s)\n", strerror(cc));
		return cc;
	}

	if (gzread(font_file, &header, sizeof (header)) < 1) {
		char *msg;
		int rc = 0;
		msg = (char *) gzerror(font_file, &rc);
		printf("Failed to gzread.\n");
		if (rc == Z_ERRNO) {
			cc = errno;
			printf("(%s)-(%s)\n", msg, strerror(cc));
		}
		else {
			cc = rc;
			printf("%d (%s)\n", rc, msg); 
		}
		gzclose(font_file);
		return cc;
	}

	chars = malloc(header.fontheight * 256);
	if (chars == NULL) {
		cc = errno;
		printf("Failed to malloc.\n");
		printf("(%s)\n", strerror(cc));
		gzclose(font_file);
		return cc;
	}

	if (gzread(font_file, chars, header.fontheight * 256) < 1) {
		char *msg;
		int rc = 0;
		msg = (char *) gzerror(font_file, &rc);
		if (rc == Z_STREAM_END || rc == Z_OK) {
			cc = 0; // not an error
		}
		else {
			printf("Failed to gzread.\n");
			if (rc == Z_ERRNO) {
				cc = errno;
				printf("(%s)-(%s)\n", msg, strerror(cc));
			}
			else {
				cc = rc;
				printf("%d (%s)\n", rc, msg);  
			}
		}
	}

	gzclose(font_file);

	return cc;
}

// Display a particular row of pixels for a given character in a previously 
// loaded font file. This is done by highlighting the charecter/pixel in
// the 'basline' array.
// 
void plot_pixel_row(unsigned char ch, unsigned char row_num, unsigned char *baseline) {
	unsigned char row;

	// all fonts appear to be 8 bits wide, but maybe not all are used?
	row = chars[ch * header.fontheight + row_num];
	for (int i = 0; i < 8; i++) {
		if (row & 0x80) {
			// bit set - highlight equivalent baseline char
			printf("\033[1;100m%c\033[0m", baseline[i]);
		}
		else {
			// bit not set - lowlight equivalent baseline char
			printf("\033[90m%c\033[0m", baseline[i]);
		}
		row <<= 1;
	}
}

int main(int argc, char *argv[]) {
	char pixel_row[8];
	int cc = 0;

	if ((cc = font_open()) == 0) {
		printf("\n");
		for (int r = 0; r < 8; r++) {
			for (unsigned char *c = argv[1]; *c; c++)  {
				for (int p = 0; p < 8; p++) {
					pixel_row[p] = '0' + ((((*c << p) & 0x80) >> 7) & 1);
				}
				plot_pixel_row(*c, r, pixel_row);
			}
			printf("\n");
		}
		printf("\n");
	}
	exit(cc);
}
