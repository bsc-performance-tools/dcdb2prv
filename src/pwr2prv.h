#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <errno.h>
#include <stdbool.h>
#include <libgen.h>
#include <time.h>

static void     mergeTraces(char *_prv, char *_pwr, char *_merged_fn);
static void     addPCFType(char *_ifile, char *_ofile);
static void     modifyROW(char *_ifile, char *_ofile);

/*
 * Modeline for space only BSD KNF code style
 */
/* vim: set textwidth=80 colorcolumn=+0 tabstop=8 softtabstop=8 shiftwidth=8 expandtab cinoptions=\:0l1t0(0.5su0.5sm1: */
