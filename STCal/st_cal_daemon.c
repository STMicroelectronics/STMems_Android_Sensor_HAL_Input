/*
 * STMicroelectronics calibration daemon
 *
 * Copyright 2015 STMicroelectronics Inc.
 *
 * Giuseppe Barba <giuseppe.barba@st.com>
 *
 * Licensed under the GPL-2.
 */

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/inotify.h>
#include <stdio.h>
#include <cutils/log.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#define concat(first, second) first second

#define PACKAGENAME		"com.st.mems.st_calibrationtool"
#define CAL_FILE 		"calibration.txt"

#define APP_DATA_DIR		"/data/data/"
#define APP_FILE_PRIVATE_DIR	"/files/"
#define APP_PRIVATE_DATA_DIR	concat(concat(APP_DATA_DIR,PACKAGENAME),APP_FILE_PRIVATE_DIR)
#define CAL_FILE_PATH		concat(APP_PRIVATE_DATA_DIR,CAL_FILE)
#define CAL_DIR			"/data/"
#define CAL_OUT_FILE_PATH	concat(CAL_DIR,CAL_FILE)
#define OBS_MASK 		(IN_MODIFY | IN_DELETE | IN_CREATE)
#define NUM_OF_SENSORS		4
#define NUM_OF_AXIS		3
#define THD_SLEEP_USEC		3000000
#define EVENT_SIZE		(sizeof(struct inotify_event))
#define MAX_CHARS_PER_LINE	512
#define DELIMITER		" "
#define MAX_TOKENS_PER_LINE	4
#define EVENT_BUF_SIZE		(1024 * (EVENT_SIZE + 16))

int eventCheck(int fd)
{
	fd_set rfds;

	FD_ZERO (&rfds);
	FD_SET (fd, &rfds);
	/* Wait until an event happens */

	return select(FD_SETSIZE, &rfds, NULL, NULL, NULL);
}

int copy_file(const char *infile, const char *outfile)
{
	FILE *fin, *fout;
	char *buffer;
	struct stat st;
	int ret;

	stat(infile, &st);
	buffer = (char *)malloc(sizeof(char) * st.st_size);
	if (!buffer) {
		ALOGE("Error to allocate temp buffer");

		return -1;
	}

	fin = fopen(infile, "r");
	if (!fin) {
		ALOGE("Error to open input file: %s error: %d", infile, errno);

		return -1;
	}

	fout = fopen(outfile, "w");
	if (!fout) {
		ALOGE("Error to open output file: %s error: %d", outfile, errno);

		return -3;
	}

	fread(buffer, sizeof(char), st.st_size, fin);
	fclose(fin);

	fwrite(buffer, sizeof(char), st.st_size, fout);
	fclose(fout);

	ret = chmod(outfile,
		S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);

	return ret;
}

int main(__attribute__((unused)) int argc, __attribute__((unused)) char* argv[])
{
	char event_buf[EVENT_BUF_SIZE];
	struct inotify_event* event;
	int num_bytes = 0;
	int event_pos;
	int event_size;
	struct stat s;
	int watch_fd;
	int observer_fd = 0;
	int ret;

	ALOGI("Start ST Calibration Daemon");
	while (stat(APP_PRIVATE_DATA_DIR, &s) < 0) {
		/* sleep until the calibration directory is present */
		usleep(THD_SLEEP_USEC);
		ALOGI("wait for cal dir %s", APP_PRIVATE_DATA_DIR);
	}

	//Copy stored calibration data to apk
	ret = copy_file(CAL_OUT_FILE_PATH, CAL_FILE_PATH);
	if (ret)
		ALOGE("Error while coping file (%d)", ret);

	observer_fd = inotify_init();
	if (observer_fd < 0) {
		ALOGE("Error to start inotify!!!");
		return -1;
	}

	watch_fd = inotify_add_watch(observer_fd, APP_PRIVATE_DATA_DIR, OBS_MASK);
	if (!watch_fd) {
		ALOGE("Error while adding inotify watcher!!!\n");

		return -1;
	}

	while (1) {
		ret = eventCheck(observer_fd);
		if (ret) {
			num_bytes = read(observer_fd, event_buf, EVENT_BUF_SIZE);
			event_pos = 0;

			while (num_bytes >= (int)sizeof(*event)) {
				event = (struct inotify_event *)(event_buf + event_pos);
				if (event->len) {
					if (event->mask & (IN_MODIFY | IN_CREATE)) {
						if (!strcmp(event->name, CAL_FILE)) {
							ALOGI("Changes on Calibration file detected");
							ret = copy_file(CAL_FILE_PATH, CAL_OUT_FILE_PATH);
							if (ret)
								ALOGE("Error while coping file (%d)", ret);
						}
					} else if (event->mask & (IN_DELETE)) {
						ALOGI("Calibration file deleted");

					} else {
						ALOGE("Event not used %d", event->mask);
					}
				}
				event_size = sizeof(*event) + event->len;
				num_bytes -= event_size;
				event_pos += event_size;
			}
		}
		usleep(THD_SLEEP_USEC);
	}
	ALOGI("Exit from ST Calibration Daemon");
}