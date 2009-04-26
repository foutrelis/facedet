/* facedet.c
 *
 * Copyright (C) 2009 Evangelos Foutras <foutrelis@gmail.com>
 *
 * This file is part of facedet.
 *
 * facedet is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * facedet is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with facedet.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <cv.h>
#include <highgui.h>

#include "common.h"

#ifdef HAVE_CONFIG_H
	#include <config.h>
#endif

static void usage() {
	fprintf(stderr, "Usage: facedet [-c camera_index] [image_file]\n");
}

int main(int argc, char *argv[]) {
	CvCapture *capture = NULL;
	IplImage *img = NULL;
	char *input_file = NULL;
	int cam_index = 0;
	int opt;

	/* Parse command arguments */
	while ((opt = getopt(argc, argv, "c:")) != -1) {
		switch (opt) {
			case 'c':
				cam_index = atoi(optarg);
				break;
			default: /* '?' */
				usage();
				exit(EXIT_FAILURE);
		}
	}

#ifdef DEBUG
	fprintf(stderr, "argc = %d\n", argc);
	fprintf(stderr, "optind = %d\n", optind);
#endif
	/* See if there is an input file given */
	if (argc > optind) {
		input_file = argv[optind];
	}

	if (input_file) {
		/* Try and open the supplied image */
		img = cvLoadImage(input_file, CV_LOAD_IMAGE_COLOR);
		if (!img) {
			exit_with("Failed to open image %s\n", input_file);
		}
	} else {
		capture = cvCaptureFromCAM(cam_index);
		if (!capture) {
			exit_with("Failed to open cam #%d\n", cam_index);
		}
	}

	exit(EXIT_SUCCESS);
}
