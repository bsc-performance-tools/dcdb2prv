#include "prvpwr.h"
#include "parseOptions.h"

#define debug(format, ...) if (verbose)	fprintf(stdout, format, ##__VA_ARGS__)

char *merge_prv_fn = NULL;
char *pwr_fn = NULL;
char *output_fn = NULL;
bool verbose = false;
char *offset = NULL;

int main (int argc, char **argv)
{
	int ret = 0, err = 0;
	FILE *merge_prv_fp, *pwr_fp, *output_fp;
	unsigned long timestamp, energy;
	char line[80];

	ret = parseOptions(argc, argv);

	if (ret < 0)
	{
		exit(EXIT_FAILURE);
	} else if (ret > 0)
	{
		exit(EXIT_SUCCESS);
	}

	debug("Reading power trace from %s\n", pwr_fn);
	if ((pwr_fp = fopen(pwr_fn, "r")) == NULL)
	{
		err = errno;
		fprintf(stderr, "%s: %s\n", pwr_fn, strerror(err));
		exit(EXIT_FAILURE);
	}

	if (output_fn == NULL)
	{
		output_fn = calloc(strlen(basename(pwr_fn)) + 5, sizeof(char *));
		strncpy(output_fn, basename(pwr_fn), strlen(basename(pwr_fn)) + 1);
	}

	output_fn[strlen(output_fn) - 4] = 0;
	strncat(output_fn, ".prv", 4);

	debug("Writing power trace to %s\n", output_fn);
	if ((output_fp = fopen(output_fn, "w")) == NULL)
	{
		err = errno;
		fprintf(stderr, "%s: %s\n", output_fn, strerror(err));
		exit(EXIT_FAILURE);
	}

	if (fgets(line, 80, pwr_fp) == NULL)
	{
		exit(EXIT_FAILURE);
	}

	while (fgets(line, 80, pwr_fp) != NULL)
	{
		// host = strtok(line, ","); host name
		strtok(line, ","); // ditch host name
		timestamp = strtoul(strtok(NULL, ","), NULL, 10); // timestamp
		energy = strtoul(strtok(NULL, ","), NULL, 10); // energy

		if (offset != NULL)
		{
			timestamp -= strtoul(offset, NULL, 10); // substract offset
		}

		fprintf(output_fp, "2:1:1:1:1:%lu:90000000:%lu\n", timestamp, energy);
	}

	fclose(pwr_fp);
	fclose(output_fp);

	return ret;
}

static int parseOptions (int argc, char **argv)
{
	poptContext pc;
	int opt = 0, ret = 0;

	pc = poptGetContext(NULL, argc, (const char **) argv, long_options, 0);
	poptReadDefaultConfig(pc, 0);

	if (argc == 1)
	{
		poptPrintHelp(pc, stderr, 0);
		ret = 1;
		goto end;
	}

	while ((opt = poptGetNextOpt(pc)) != -1)
	{
		switch (opt)
		{
			case OPT_MERGE_PRV_FILE:
				merge_prv_fn = poptGetOptArg(pc);
				if (merge_prv_fn == NULL)
				{
					ret = -EINVAL;
					goto end;
				}
				break;
			case OPT_PWR_FILE:
				pwr_fn = poptGetOptArg(pc);
				if (pwr_fn == NULL)
				{
					ret = -EINVAL;
					goto end;
				}
				break;
			case OPT_OUTPUT:
				output_fn = poptGetOptArg(pc);
				if (output_fn == NULL)
				{
					ret = -EINVAL;
					goto end;
				}
				break;
			case OPT_OFFSET:
				offset = poptGetOptArg(pc);
				break;
			case OPT_VERBOSE:
				verbose = true;
				break;
			default:
				fprintf(stderr, "%s\n", strerror(EINVAL));
				ret = -EINVAL;
				break;
		}
	}

end:
	if (pc)
	{
		poptFreeContext(pc);
	}

	return ret;
}

static void addPCFType(char *file)
{
	FILE *pcf;
	int err = 0;

	output_fn[strlen(output_fn) - 4] = 0;
	strncat(output_fn, ".pcf", 4);
	
	if ((pcf = fopen(file, "w")) == NULL)
	{
		err = errno;
		fprintf(stderr, "%s: %s\n", file, strerror(err));
		exit(EXIT_FAILURE);
	}

	fprintf(pcf, "\n\nEVENT_TYPEn\n" \
			"0\t90000000\tPower\n");

	fclose(pcf);
}
