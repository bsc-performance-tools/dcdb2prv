#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <errno.h>
#include <stdbool.h>
#include <libgen.h>
#include <time.h>
#include <glib.h>

static void     writeHeader(FILE *_output_fp, uint64_t _duration, int _node);
static void     createPCF(char *_ofile);
static void     createROW(char *_ofile, GHashTable * _nodes_inv_ht);
static gint     timecmp(gconstpointer a, gconstpointer b);
static void     dump_pair (const char *key, const char *value);

/*
 * Modeline for space only BSD KNF code style
 */
/* vim: set textwidth=80 colorcolumn=+0 tabstop=8 softtabstop=8 shiftwidth=8 expandtab cinoptions=\:0l1t0+0.5s(0.5su0.5sm1: */
