#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <errno.h>
#include <stdbool.h>
#include <libgen.h>
#include <time.h>

static void mergeTraces(char *prv, char *pwr, char *merged_fn);
static void addPCFType(char *ifile, char *ofile);
