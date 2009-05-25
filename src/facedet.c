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
#include <pthread.h>
#include <cv.h>
#include <highgui.h>

#include "common.h"
#include "facedet.h"

#ifdef HAVE_CONFIG_H
	#include <config.h>
#endif

static IplImage *frame = NULL;
static pthread_mutex_t frame_mutex;
static int shutdown = 0;

static CvMemStorage *storage = NULL;
static CvHaarClassifierCascade *cascade = NULL;
static const char *cascade_file = "/usr/share/opencv/haarcascades/haarcascade_frontalface_alt.xml";

static void usage() {
	fprintf(stderr, "Usage: facedet [-c camera_index] [-d cascade_file] [image_file]\n");
}

static void detect_and_draw(IplImage *img) {
	int i;

	/* Two points to represent the face locations */
	CvPoint pt1, pt2;

	/* Sequence to store detected faces */
	CvSeq *faces;

	/* Rectangle used for marking the face location */
	CvRect *r;

	/* Clear the memory storage */
	cvClearMemStorage(storage);

	/* Detect the faces */
	faces = cvHaarDetectObjects(img, cascade, storage,
	                            1.1, 2, CV_HAAR_DO_CANNY_PRUNING,
	                            cvSize(40, 40));

	/* Mark faces with red rectangles */
	for(i = 0; i < (faces ? faces->total : 0); i++) {
		r = (CvRect *) cvGetSeqElem(faces, i);

		pt1.x = r->x;
		pt1.y = r->y;
		pt2.x = r->x + r->width;
		pt2.y = r->y + r->height;

		cvRectangle(img, pt1, pt2, CV_RGB(255,0,0), 3, 8, 0);
	}
}

static void *capture_thread_func(void *src) {
	source_t *source = (source_t *) src;
	CvCapture *capture = source->capture;

	while (!shutdown) {
		pthread_mutex_lock(&frame_mutex);

		frame = cvQueryFrame(capture);
		if (!frame) {
			shutdown = 1;
		}

		pthread_mutex_unlock(&frame_mutex);
		usleep(100000);
	}

	pthread_exit(NULL);
}

static void *process_thread_func(void *src) {
	source_t *source = (source_t *) src;
	IplImage *img;
	int key;

	while (!frame && !shutdown) {
		sleep(1);
	}

	while (!shutdown) {
		pthread_mutex_lock(&frame_mutex);

		/* Create a copy of the frame */
		img = cvCreateImage(cvSize(frame->width, frame->height),
			                IPL_DEPTH_8U, frame->nChannels);
		if (frame->origin == IPL_ORIGIN_TL) {
			cvCopy(frame, img, 0);
		} else {
			cvFlip(frame, img, 0);
		}

		pthread_mutex_unlock(&frame_mutex);

		detect_and_draw(img);

		cvShowImage("mainWin", img);
		key = cvWaitKey(source->type == CAPTURE ? 10 : 0);
		/* 27 is ESC key */
		if ((key & 0xFF) == 27) {
			shutdown = 1;
		}

		cvReleaseImage(&img);
	}

	pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
	CvCapture *capture = NULL;
	IplImage *img = NULL;
	char *input_file = NULL;
	int cam_index = 0;
	int opt;
	source_t source;
	pthread_t capture_thread;
	pthread_t process_thread;

	pthread_mutex_init(&frame_mutex, NULL);

	/* Parse command arguments */
	while ((opt = getopt(argc, argv, "c:d:")) != -1) {
		switch (opt) {
			case 'c':
				cam_index = atoi(optarg);
				break;
			case 'd':
				cascade_file = optarg;
				break;
			default: /* '?' */
				usage();
				exit(EXIT_FAILURE);
		}
	}

	/* See if there is an input file given */
	if (argc > optind) {
		input_file = argv[optind];
	}

	cascade = (CvHaarClassifierCascade *) cvLoad(cascade_file, 0, 0, 0);
	if (!cascade) {
		exit_with("Failed to load classifier cascade\n");
	}

	storage = cvCreateMemStorage(0);

	cvNamedWindow("mainWin", CV_WINDOW_AUTOSIZE);
	cvMoveWindow("mainWin", 50, 50);

	if (input_file) {
		/* Try and open the supplied image */
		img = cvLoadImage(input_file, CV_LOAD_IMAGE_COLOR);
		if (!img) {
			exit_with("Failed to open image %s\n", input_file);
		}

		frame = img;
		source.type = IMAGE_FILE;
		source.capture = NULL;

		pthread_create(&process_thread, NULL,
		               process_thread_func, (void *) &source);

		pthread_join(process_thread, NULL);

		cvReleaseImage(&img);
	} else {
		// Use the camera
		capture = cvCaptureFromCAM(cam_index);
		if (!capture) {
			exit_with("Failed to open cam #%d\n", cam_index);
		}

		source.type = CAPTURE;
		source.capture = capture;

		pthread_create(&capture_thread, NULL,
		               capture_thread_func, (void *) &source);
		pthread_create(&process_thread, NULL,
		               process_thread_func, (void *) &source);

		pthread_join(capture_thread, NULL);
		pthread_join(process_thread, NULL);

		cvReleaseCapture(&capture);
	}

	pthread_mutex_destroy(&frame_mutex);

	pthread_exit(NULL);
}
