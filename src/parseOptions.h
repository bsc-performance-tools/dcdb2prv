#pragma once

#include <popt.h>
#include <string.h>
#include <errno.h>

enum
{
	OPT_NONE = 0,
	OPT_MERGE_PRV_FILE,
	OPT_PWR_FILE,
	OPT_OUTPUT,
	OPT_OFFSET,
	OPT_VERBOSE
};

static int parseOptions(int argc, char **argv);

static struct poptOption long_options[] =
{
	{"merge-prv", 'm', POPT_ARG_STRING, NULL, OPT_MERGE_PRV_FILE, "Merge with paraver trace", NULL},
	{"power", 'p', POPT_ARG_STRING, NULL, OPT_PWR_FILE, "Power trace", NULL},
	{"output", 'o', POPT_ARG_STRING, NULL, OPT_OUTPUT, "Output filename", NULL},
	{"offset", 't', POPT_ARG_INT, NULL, OPT_OFFSET, "Initial timestamp", NULL},
	{"verbose", 'v', POPT_ARG_NONE, NULL, OPT_VERBOSE, "Be verbose", NULL},
	POPT_AUTOHELP
	{NULL, 0, 0, NULL, 0}
};
