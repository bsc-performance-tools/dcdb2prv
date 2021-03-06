#pragma once

#include <popt.h>
#include <string.h>
#include <errno.h>

enum
{
        OPT_NONE = 0,
        OPT_PWR_FILE,
        OPT_OUTPUT,
        OPT_OFFSET,
        OPT_VERBOSE
};

static int      parseOptions(int _argc, char **_argv);

static struct poptOption long_options[] =
{
        {"power", 'p', POPT_ARG_STRING, NULL, OPT_PWR_FILE,
            "Power trace", NULL},
        {"output", 'o', POPT_ARG_STRING, NULL, OPT_OUTPUT,
            "Output filename", NULL},
        {"offset", 't', POPT_ARG_INT, NULL, OPT_OFFSET,
            "Initial timestamp (ns)", NULL},
        {"verbose", 'v', POPT_ARG_NONE, NULL, OPT_VERBOSE,
            "Be verbose", NULL},
        POPT_AUTOHELP
        {NULL, 0, 0, NULL, 0}
};

/*
 * Modeline for space only BSD KNF code style
 */
/* vim: set textwidth=80 colorcolumn=+0 tabstop=8 softtabstop=8 shiftwidth=8 expandtab cinoptions=\:0l1t0+0.5s(0.5su0.5sm1: */
