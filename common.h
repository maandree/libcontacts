/* See LICENSE file for copyright and license details. */
#include "libcontacts.h"

#include <alloca.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#define TIME_MAX ((time_t)((1ULL << (8 * sizeof(time_t) - 1)) - 1ULL))


#define DESTROY_ALL_OBJECTS(LIST, FUNC)\
	do {\
		void *destroy_all_temp__ = (LIST);\
		if (destroy_all_temp__) {\
			for (; *(LIST); (LIST)++) {\
				FUNC(*(LIST));\
				free(*(LIST));\
			}\
			free(destroy_all_temp__);\
		}\
	} while (0)


#define DESTROY_ALL_STRINGS(LIST)\
	do {\
		void *destroy_all_temp__ = (LIST);\
		if (destroy_all_temp__) {\
			for (; *(LIST); (LIST)++)\
				free(*(LIST));\
			free(destroy_all_temp__);\
		}\
	} while (0)
