/* See LICENSE file for copyright and license details. */
#include "libcontacts.h"

#include <alloca.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#define DESTROY_ALL(LIST, FUNC)\
	do {\
		void *destroy_all_temp__;\
		if ((destroy_all_temp__ = (LIST))) {\
			for (; *(LIST); (LIST)++)\
				FUNC(*(LIST));\
			free(destroy_all_temp__);\
		}\
	} while (0)
