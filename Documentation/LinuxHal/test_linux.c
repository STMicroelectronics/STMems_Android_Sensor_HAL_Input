/*
 * STMicroelectronics SensorHAL simple test
 *
 * Copyright 2022 STMicroelectronics Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 */

#undef ANDROID_LOG
#undef LOG_FILE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <poll.h>
#include <fcntl.h>
#include <string.h>
#include <dlfcn.h>
#include <pthread.h>
#include <getopt.h>
#include <stdbool.h>
#include <asm/types.h>
#include <linux/limits.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <hardware/sensors.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <math.h>

#define TEST_LINUX_VERSION		"1.0"

#ifndef LOG_TAG
#define LOG_TAG "test_linux"
#endif

#ifdef ANDROID_LOG
	#include <utils/Log.h>
	#include <android/log.h>
	#define tl_log(...) \
		__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#else /* ANDROID_LOG */
	#define tl_debug(fmt, ...) printf("%s:%s:%s:%d > " fmt "\n", \
			LOG_TAG, __FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__)
	#ifdef LOG_FILE
		#define tl_log(fmt, ...) do { \
			fprintf(logfd, "%s:%s:%s:%d > " fmt "\n", \
				    LOG_TAG, __FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__); \
			} while(0);
	#else /* LOG_FILE */
		#define tl_log(fmt, ...) printf("%s:%s:%s:%d > " fmt "\n", \
				    LOG_TAG, __FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__)
	#endif /* LOG_FILE */
#endif /* ANDROID_LOG */

/* Max event buffer for poll sensor */
#define BUFFER_EVENT			256

/* Defines to enable/disable sensors */
#define SENSOR_ENABLE			1
#define SENSOR_DISABLE			0

/* Translate sansor index from listo to handle */
#define HANDLE_FROM_INDEX(_i) 		(i + 1)
#define INVALID_HANDLE			-1

#ifdef LOG_FILE
#define TL_FILE_DEFAULT_NAME		"log.txt"
static FILE *logfd;
#endif /* LOG_FILE */

static const char *options = "a:g:b:fn:d:s:le:o:Nhv?";
static const int test_sensor_type[] = {
		SENSOR_TYPE_GYROSCOPE,
		SENSOR_TYPE_ACCELEROMETER,
		SENSOR_TYPE_AMBIENT_TEMPERATURE,
		-1,
	};

static uint64_t start_systime, start_sensortime[SENSOR_TYPE_AMBIENT_TEMPERATURE + 1];

static const struct option long_options[] = {
		{"accodr",    required_argument, 0,  'a' },
		{"gyroodr",   required_argument, 0,  'g' },
		{"list",      no_argument,       0,  'l' },
		{"nsample",   required_argument, 0,  'n' },
		{"delay",     required_argument, 0,  'd' },
		{"timeout",   required_argument, 0,  't' },
		{"sensor",    required_argument, 0,  's' },

#ifdef LOG_FILE
		{"logname",   required_argument, 0,  'o' },
#endif /* LOG_FILE */

		{"version",   no_argument,       0,  'v' },
		{"help",      no_argument,       0,  '?' },
		{0,           0,                 0,   0  }
	};

/* Path to SensorHAL.so in target filesystem */
static const char *hal_paths[] = {
	"/tmp/mnt/SensorHAL.so",
	"/home/pi/SensorHAL.so",
	"/system/vendor/lib64/hw/SensorHAL.so",
	"/system/vendor/lib/hw/SensorHAL.so",
};

static const char* types[] = {
	"...",  // 0
	"Acc",
	"Mag",
	"...",
	"Gyro",
	"...",  // 5
	"Press",
	"Temp",
	"...",
	"...",
	"...", // 10
	"...",
	"...",
	"AmbT",
	"...",
	"...", // 15
};

static int num_sample = 10;
static int acc_odr = 104;
static int gyro_odr = 104;
static int other_odr = 100;
static int list_opt = 0;
static int flush_opt = 0;
static int64_t delay_ns = 0;
static volatile int timeout = 0;
static int samples_timeout = 1;
static struct sensors_module_t *hmi;
static struct hw_device_t *dev;
struct sensors_poll_device_1 *poll_dev;
static struct sensor_t const* list;
static int sensor_num;
static int sensor_handle = -1;
static int test_events = 0;

/* Report sensor type in string format */
static const char *type_str(int type)
{
	int type_count = sizeof(types)/sizeof(char *);

	if (type < 0 || type >= type_count)
		return "unknown";

	return types[type];
}

static void dump_event(struct sensors_event_t *e)
{
	struct timeval tv;
	uint64_t sec_usec;

	gettimeofday(&tv, NULL);
	sec_usec = tv.tv_sec * 1000000 + tv.tv_usec;

	if (start_systime == 0LL)
		start_systime = sec_usec;

	switch (e->type) {
	case SENSOR_TYPE_ACCELEROMETER:
		if (start_sensortime[0] == 0LL)
			start_sensortime[0] = e->timestamp;
		tl_log("ACC event: x=%10.2f y=%10.2f z=%10.2f timestamp=%lld systime=%lld delta=%lld",
			e->acceleration.x, e->acceleration.y, e->acceleration.z,
			e->timestamp - start_sensortime[0],
			sec_usec - start_systime,
			((sec_usec - start_systime) * 1000) - (e->timestamp - start_sensortime[0]));
		break;
	case SENSOR_TYPE_GYROSCOPE:
		if (start_sensortime[1] == 0LL)
			start_sensortime[1] = e->timestamp;
		tl_log("GYRO event: x=%10.2f y=%10.2f z=%10.2f timestamp=%lld systime=%lld delta=%lld",
			e->gyro.x, e->gyro.y, e->gyro.z,
			e->timestamp - start_sensortime[1],
			sec_usec - start_systime,
			((sec_usec - start_systime) * 1000) - (e->timestamp - start_sensortime[0]));
		break;
	case SENSOR_TYPE_TEMPERATURE:
	case SENSOR_TYPE_AMBIENT_TEMPERATURE:
		if (start_sensortime[2] == 0LL)
			start_sensortime[2] = e->timestamp;
		tl_log("TEMP event: x=%10.2f timestamp=%lld systime=%lld delta=%lld",
			e->temperature, e->timestamp - start_sensortime[2],
			sec_usec - start_systime,
			((sec_usec - start_systime) * 1000) - (e->timestamp - start_sensortime[0]));
		break;
	case SENSOR_TYPE_MAGNETIC_FIELD:
		if (start_sensortime[2] == 0LL)
			start_sensortime[2] = e->timestamp;
		tl_log("MAG event: x=%10.2f y=%10.2f z=%10.2f timestamp=%lld systime=%lld delta=%lld",
			e->magnetic.x, e->magnetic.y, e->magnetic.z,
			e->timestamp - start_sensortime[1],
			sec_usec - start_systime,
			((sec_usec - start_systime) * 1000) - (e->timestamp - start_sensortime[0]));
		break;
	case SENSOR_TYPE_PRESSURE:
		if (start_sensortime[2] == 0LL)
			start_sensortime[2] = e->timestamp;
		tl_log("PRESS event: x=%10.2f timestamp=%lld systime=%lld delta=%lld",
			e->pressure, e->timestamp - start_sensortime[2],
			sec_usec - start_systime,
			((sec_usec - start_systime) * 1000) - (e->timestamp - start_sensortime[0]));
		break;
	default:
		tl_log("Unhandled events %d", e->type);
		break;
	}
}

static void dump_sensor(const struct sensor_t *s)
{
	int i;

	tl_debug("\nSensor List");
	for (i = 0; i < sensor_num; i++) {
		tl_debug("\n\tName %s", s[i].name);
		tl_debug("\tVendor %s", s[i].vendor);
		tl_debug("\tHandle %d", s[i].handle);
		tl_debug("\t\tType %s (%d)", type_str(s[i].type), s[i].type);
		tl_debug("\t\tVersion %d", s[i].version);
		tl_debug("\t\tMax Range %f", s[i].maxRange);
		tl_debug("\t\tResolution %f", s[i].resolution);
		tl_debug("\t\tPower %f", s[i].power);
		tl_debug("\t\tMin Delay %d", s[i].minDelay);
		tl_debug("\t\tMax Delay %d", s[i].maxDelay);
		tl_debug("\t\tFIFO Reserved Event %d",
		       s[i].fifoReservedEventCount);
		tl_debug("\t\tFIFO Max Event %d", s[i].fifoMaxEventCount);
	}
}

static int get_sensor(const struct sensor_t *s, int type,
		      struct sensor_t **sens)
{
	int i;

	for (i = 0; i < sensor_num; i++) {
		if (type == s[i].type) {
			*sens = (struct sensor_t *)&s[i];

			return (*sens)->handle;
		}
	}

	return INVALID_HANDLE;
}

static void sensors_poll(int maxcount, int timeout_s)
{
	int tot = 0;

	alarm(timeout_s);
	while (tot < maxcount && !timeout) {
		sensors_event_t events[BUFFER_EVENT];
		int i, count;

		count = poll_dev->poll(&poll_dev->v0, events, sizeof(events) / sizeof(sensors_event_t));

		for(i = 0; i < count; i++)
			dump_event(&events[i]);

		tot += count;
	}

	/* clear any pending alarms */
	alarm(0);

	tl_log("%s samples received %d\n",
		timeout ? "Timeout" : "Total", tot);
	timeout = 0;
}

static int sensor_activate(int handle, int enable)
{
	if (handle < 0)
		return -ENODEV;

	return poll_dev->activate(&poll_dev->v0, handle, enable);
}

static int sensor_setdelay(int type, int odr)
{
	struct sensor_t *sensor = NULL;
	int64_t delay;
	int handle;

	handle = get_sensor(list, type, &sensor);
	if (handle < 0 || !sensor)
		return -ENODEV;

	if (odr == 0)
		delay = 0;
	else
		delay = 1000000000 / odr;

	tl_log("Setting %d ODR to %d Hz (%lld ns)\n", type, odr, delay);

	return poll_dev->setDelay(&poll_dev->v0, handle, delay);
}

static int open_hal(char *lcl_path)
{
	const char *lh_path = NULL;
	void *hal;
	int err;

	if (!lcl_path) {
		unsigned i;

		for (i = 0; i < sizeof(hal_paths)/sizeof(const char*); i++) {
			if (!access(hal_paths[i], R_OK)) {
				lh_path = hal_paths[i];
				break;
			}
		}

		if (!lh_path) {
			fprintf(stderr, "ERROR: unable to find HAL\n");
			exit(1);
		}
	} else {
		lh_path = lcl_path;
	}

	hal = dlopen(lh_path, RTLD_NOW);
	if (!hal) {
		fprintf(stderr, "ERROR: unable to load HAL %s: %s\n", lh_path,
			dlerror());
		return -1;
	}

	hmi = dlsym(hal, HAL_MODULE_INFO_SYM_AS_STR);
	if (!hmi) {
		fprintf(stderr, "ERROR: unable to find %s entry point in HAL\n",
			HAL_MODULE_INFO_SYM_AS_STR);
		return -1;
	}

	tl_log("HAL loaded: name %s vendor %s version %d.%d id %s",
	       hmi->common.name, hmi->common.author,
	       hmi->common.version_major, hmi->common.version_minor,
	       hmi->common.id);

	err = hmi->common.methods->open((struct hw_module_t *)hmi,
					SENSORS_HARDWARE_POLL, &dev);
	if (err) {
		fprintf(stderr, "ERROR: failed to initialize HAL: %d\n", err);
		exit(1);
	}

	poll_dev = (sensors_poll_device_1_t *)dev;

	return 0;
}

static void sensor_disable_all(void)
{
	struct sensor_t *sensor = NULL;
	int sindex = 0;
	int handle;

	while(test_sensor_type[sindex] != -1) {
		handle = get_sensor(list, test_sensor_type[sindex], &sensor);
		if (handle < 0)
			break;

		tl_log("Deactivating %s sensor (handle %d)",
		       sensor->name, handle);
		sensor_activate(handle, SENSOR_DISABLE);

		sindex++;
	}
}

static void alarmHandler(int dummy)
{
	timeout = 1;
}

static void help(char *argv)
{
	int index = 0;

	printf("usage: %s [OPTIONS]\n\n", argv);
	printf("OPTIONS:\n");
	printf("\t--%s:\tSet Accelerometer ODR (default %d)\n",
	       long_options[index++].name, acc_odr);
	printf("\t--%s:\tSet Gyroscope ODR (default %d)\n",
	       long_options[index++].name, gyro_odr);
	printf("\t--%s:\t\tShow sensor list\n", long_options[index++].name);
	printf("\t--%s:\tNumber of samples (default %d)\n",
	       long_options[index++].name, num_sample);
	printf("\t--%s:\tSet Delay time in ns\n", long_options[index++].name);
	printf("\t--%s:\tSet timeout (s) for receive samples (default %d s)\n",
	       long_options[index++].name, samples_timeout);
	printf("\t--%s:\tSelect sensor (default all)\n",
	       long_options[index++].name);

#ifdef LOG_FILE
	printf("\t--%s:\tOutput log file (default %s)\n",
	       long_options[index++].name, TL_FILE_DEFAULT_NAME);
#endif /* LOG_FILE */

	printf("\t--%s:\tPrint Version\n",
	       long_options[index++].name);
	printf("\t--%s:\t\tThis help\n", long_options[index++].name);

	exit(0);
}

static int single_sensor_test(int sindex)
{
	struct sensor_t *sensor = NULL;
	int handle;
	int ret;

	/* Activate sensor */
	handle = get_sensor(list, test_sensor_type[sindex], &sensor);
	if (handle != INVALID_HANDLE && sensor) {
		if (handle < 0)
			return -ENODEV;

		tl_log("Activating %s sensor (handle %d)",
			sensor->name, handle);
		ret = sensor_activate(handle, SENSOR_ENABLE);
		if (ret < 0) {
			tl_log("sensor_activate returned error %d",
				ret);

			return ret;
		}

		/* Start polling data */
		tl_log("Polling %s sensor (handle %d) for %d samples in %d s",
		       sensor->name, handle, num_sample, samples_timeout);
		sensors_poll(num_sample, samples_timeout);

		/* Deactivate sensor */
		tl_log("Deactivating %s sensor (handle %d)",
		       sensor->name, handle);
		ret = sensor_activate(handle, SENSOR_DISABLE);
		if (ret < 0) {
			tl_log("sensor_activate returned error %d",
				ret);

			return ret;
		}
	}
}

static int all_sensor_test(void)
{
	struct sensor_t *sensor = NULL;
	int sindex = 0;
	int handle;

	while (test_sensor_type[sindex] != -1) {
		/* Activate sensor */
		handle = get_sensor(list, test_sensor_type[sindex], &sensor);
		if (handle < 0) {
			sindex++;
			continue;
		}

		tl_log("Activating %s sensor (handle %d)", sensor->name, handle);
		sensor_activate(handle, SENSOR_ENABLE);
		sindex++;
	}

	/* Start polling data */
	tl_log("Polling all sensor for %d samples in %d s", num_sample, samples_timeout);
	sensors_poll(num_sample, samples_timeout);

	sensor_disable_all();

	return 0;
}


static void ctrlzHandler(int events)
{
	sensor_disable_all();

#ifdef LOG_FILE
	if (logfd)
		fclose(logfd);
#endif /* LOG_FILE */

	exit(0);
}

int main(int argc, char **argv)
{
	int digit_optind = 0;
	char *ptr;
	int ret;
	int c;

#ifdef LOG_FILE
	char *log_filename = NULL;
	int splitfile = 0;
#endif /* LOG_FILE */

	int notemp = 0;
	int i;

	while (1) {
		int this_option_optind = optind ? optind : 1;
		int option_index = 0;

		c = getopt_long(argc, argv, options,
				long_options, &option_index);
		if (c == -1)
			break;

		switch (c) {
		case 'a':
			acc_odr = atoi(optarg);
			break;
		case 'g':
			gyro_odr = atoi(optarg);
			break;
		case 'l':
			list_opt = 1;
			break;
		case 'd':
			delay_ns = strtoul(optarg, &ptr, 10);
			break;
		case 'f':
			flush_opt = 1;
			break;
		case 't':
			samples_timeout = atoi(optarg);
			break;
		case 'n':
			num_sample = atoi(optarg);
			break;
		case 's':
			sensor_handle = atoi(optarg);
			break;
			break;
#ifdef LOG_FILE
		case 'o':
			log_filename = optarg;
			break;
#endif /* LOG_FILE */

		case 'N':
			notemp = 1;
			break;
		case 'v':
			printf("Version %s\n", TEST_LINUX_VERSION);
			exit(0);
		default:
			help(argv[0]);
		}
	}

#ifdef LOG_FILE
	if (log_filename) {
		logfd = fopen(log_filename, "w+");
		if (!logfd) {
			perror("open");
			exit(-1);
		}
	} else {
		logfd = fopen(TL_FILE_DEFAULT_NAME, "w+");
		if (!logfd) {
			perror("open");
			exit(-1);
		}
	}
#endif /* LOG_FILE */

	ret = open_hal(NULL);
	if (ret) {
		fprintf(stderr, "ERROR: unable to open SensorHAL\n");
		exit(1);
	}

	sensor_num = hmi->get_sensors_list(hmi, &list);
	if (!sensor_num) {
		fprintf(stderr, "ERROR: no sensors available\n");
		exit(1);
	}

	/* Dump sensor list */
	if (list_opt) {
		dump_sensor(list);
		exit(0);
	}

	sensor_disable_all();

	sensor_setdelay(SENSOR_TYPE_ACCELEROMETER, acc_odr);
	sensor_setdelay(SENSOR_TYPE_GYROSCOPE, gyro_odr);
	sensor_setdelay(SENSOR_TYPE_TEMPERATURE, other_odr);
	sensor_setdelay(SENSOR_TYPE_AMBIENT_TEMPERATURE, other_odr);

	signal(SIGINT, ctrlzHandler);
	signal(SIGALRM, alarmHandler);

	start_systime = 0LL;
	for(i = 0; i < 3; i++)
		start_sensortime[i] = 0LL;

	if (sensor_handle >= 0)
		single_sensor_test(sensor_handle);
	else
		all_sensor_test();

	ctrlzHandler(0);

	return 0;
}
