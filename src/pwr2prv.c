#include "pwr2prv.h"
#include "parseOptions.h"

#define debug(...) if (verbose) fprintf(stderr, __VA_ARGS__)

char *merge_prv_fn = NULL;
char *pwr_fn = NULL;
char *output_fn = NULL;
bool verbose = false;
char *offset = NULL;

int
main(int argc, char **argv)
{
        int ret = 0, err = 0;
        FILE *pwr_fp, *output_fp;
        uint64_t timestamp, energy;
        char *line = NULL;
        size_t len = 0;
        char *merged_fn;

        ret = parseOptions(argc, argv);

        if (ret < 0) {
                exit(EXIT_FAILURE);
        } else if (ret > 0) {
                exit(EXIT_SUCCESS);
        }

        debug("Reading power trace from %s\n", pwr_fn);
        if ((pwr_fp = fopen(pwr_fn, "r")) == NULL) {
                err = errno;
                fprintf(stderr, "%s: %s\n", pwr_fn, strerror(err));
                exit(EXIT_FAILURE);
        }

        if (output_fn == NULL) {
                output_fn = calloc(strlen(basename(pwr_fn)) + 5, sizeof(char *));
                strncpy(output_fn, basename(pwr_fn), strlen(basename(pwr_fn)) + 1);
        }

        output_fn[strlen(output_fn) - 4] = 0;
        strncat(output_fn, ".prv", 4);

        debug("Writing power trace to %s\n", output_fn);
        if ((output_fp = fopen(output_fn, "w")) == NULL) {
                err = errno;
                fprintf(stderr, "%s: %s\n", output_fn, strerror(err));
                exit(EXIT_FAILURE);
        }

        if (getline(&line, &len, pwr_fp) == -1) {
                exit(EXIT_FAILURE);
        }

        while (getline(&line, &len, pwr_fp) != -1) {
                /* Ditch host name */
                strtok(line, ",");
                /* Get timestamp */
                timestamp = strtoul(strtok(NULL, ","), NULL, 10);
                /* Get energy */
                energy = strtoul(strtok(NULL, ","), NULL, 10);

                /*
                 * If the user doesn't specify an offset as a command line
                 * argument, use the first timestamp as the offset
                */
                if (offset == NULL) {
                        offset = realloc(offset, sizeof(uint64_t));
                        sprintf(offset, "%" PRIu64, timestamp);
                }

                /* Substract offset */
                timestamp -= strtoul(offset, NULL, 10);

                fprintf(output_fp,
                    "2:1:1:1:1:%" PRIu64 ":90000000:%" PRIu64 "\n",
                    timestamp,
                    energy);
        }

        free(line);
        fclose(pwr_fp);
        fclose(output_fp);

        if (merge_prv_fn != NULL) {
                debug("Merging with %s\n", merge_prv_fn);

                merged_fn = calloc(strlen(basename(merge_prv_fn)) + 10,
                    sizeof(char *));
                strncpy(merged_fn, basename(merge_prv_fn),
                    strlen(basename(merge_prv_fn)) + 1);
                merged_fn[strlen(merged_fn) - 4] = 0;
                strncat(merged_fn, "_pwr_merge", 10);

                mergeTraces(merge_prv_fn, output_fn, merged_fn);
                merge_prv_fn[strlen(merge_prv_fn) - 4] = 0;
                strncat(merge_prv_fn, ".pcf", 4);
                addPCFType(merge_prv_fn, merged_fn);
                merge_prv_fn[strlen(merge_prv_fn) - 4] = 0;
                strncat(merge_prv_fn, ".row", 4);
                modifyROW(merge_prv_fn, merged_fn);
        }

        return ret;
}

static int 
parseOptions (int argc, char **argv)
{
        poptContext pc;
        int opt = 0, ret = 0;

        pc = poptGetContext(NULL, argc, (const char **) argv, long_options, 0);
        poptReadDefaultConfig(pc, 0);

        if (argc == 1) {
                poptPrintHelp(pc, stderr, 0);
                ret = 1;
                goto end;
        }

        while ((opt = poptGetNextOpt(pc)) != -1) {
                switch (opt) {
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
        if (pc) {
                poptFreeContext(pc);
        }

        return ret;
}

static void
mergeTraces(char *prv_fn, char *pwr_fn, char *merged_fn)
{
        int err = 0;
        FILE *prv_fp, *pwr_fp, *merged_fp;
        char *line = NULL, *pwrline = NULL, *prvline = NULL;
        size_t len = 0;

        time_t now = time(0);
        struct tm *local = localtime(&now);
        char day[3], mon[3], hour[3], min[3];
        char *str = NULL, *rest = NULL, *restcpy = NULL;
        size_t apps = 0, tspwr = 0, tsprv = 0;

        debug("Reading power trace from %s\n", pwr_fn);
        if ((pwr_fp = fopen(pwr_fn, "r")) == NULL) {
                err = errno;
                fprintf(stderr, "%s: %s\n", pwr_fn, strerror(err));
                exit(EXIT_FAILURE);
        }

        debug("Reading system trace from %s\n", prv_fn);
        if ((prv_fp = fopen(prv_fn, "r")) == NULL) {
                err = errno;
                fprintf(stderr, "%s: %s\n", prv_fn, strerror(err));
                exit(EXIT_FAILURE);
        }

        strncat(merged_fn, ".prv", 4);
        debug("Writing merged trace to %s ...", merged_fn);
        if ((merged_fp = fopen(merged_fn, "w")) == NULL) {
                err = errno;
                fprintf(stderr, "%s: %s\n", merged_fn, strerror(err));
                exit(EXIT_FAILURE);
        }

        if (getline(&line, &len, prv_fp) != -1) {
                strtok(line, ":"); // Date

                sprintf(day, "%.2d", local->tm_mday);
                sprintf(mon, "%.2d", local->tm_mon + 1);
                sprintf(hour, "%.2d", local->tm_hour);
                sprintf(min, "%.2d", local->tm_min);
                fprintf(merged_fp, "#Paraver (%s/%s/%d at %s:%s):",
                    day,
                    mon,
                    local->tm_year + 1900,
                    hour,
                    min
                );

                strtok(NULL, ":"); /* minutes */
                str = strtok(NULL, ":");
                fprintf(merged_fp, "%s:", str);
                str = strtok(NULL, ":");
                fprintf(merged_fp, "%s:", str);
                /* Add power monitoring app */
                apps = strtoul(strtok(NULL, ":"), NULL, 10) + 1;
                fprintf(merged_fp, "%" PRIu64 ":", apps);
                str = strtok(NULL, "\n");
                /* Add the new app to the resources list */
                fprintf(merged_fp, "%s,1(1:1)\n", str);
        } else {
                exit(EXIT_FAILURE);
        }

        if (getline(&line, &len, pwr_fp) != -1) {
                pwrline = realloc(pwrline, strlen(line) * sizeof(char *));
                strncpy(pwrline, line, strlen(line));
                strtok(line, ":"); /* type */
                strtok(NULL, ":"); /* cpu_id */
                strtok(NULL, ":"); /* appl_id */
                strtok(NULL, ":"); /* task_id */
                strtok(NULL, ":"); /* thread_id */
                tspwr = strtoul(strtok(NULL, ":"), NULL, 10); /* timestamp */
                rest = strtok(NULL, "\n"); /* rest */
                restcpy = realloc(restcpy, (1 + strlen(rest)) * sizeof(char *));
                strncpy(restcpy, rest, strlen(rest));
        }

        while (getline(&line, &len, prv_fp) != -1) {
                prvline = realloc(prvline, strlen(line) * sizeof(char *));
                strncpy(prvline, line, strlen(line));
                strtok(prvline, ":"); /* type */
                strtok(NULL, ":"); /* cpu_id */
                strtok(NULL, ":"); /* appl_id */
                strtok(NULL, ":"); /* task_id */
                strtok(NULL, ":"); /* thread_id */
                tsprv = strtoul(strtok(NULL, ":"), NULL, 10); /* timestamp */

                if (tsprv > tspwr) {
                        fprintf(merged_fp, "%d:%d:%zu:%d:%d:%zu:%s\n",
                            2, 1, apps, 1, 1, tspwr, restcpy);
                        if (getline(&line, &len, pwr_fp) != -1) {
                                pwrline = realloc(pwrline,
                                    strlen(line) * sizeof(char *));
                                strncpy(pwrline, line, strlen(line));
                                strtok(line, ":"); /* type */
                                strtok(NULL, ":"); /* cpu_id */
                                strtok(NULL, ":"); /* appl_id */
                                strtok(NULL, ":"); /* task_id */
                                strtok(NULL, ":"); /* thread_id */
                                /* timestamp */
                                tspwr = strtoul(strtok(NULL, ":"), NULL, 10);
                                rest = strtok(NULL, "\n"); /* rest */
                                restcpy = realloc(restcpy,
                                    (1 + strlen(rest)) * sizeof(char *));
                                strncpy(restcpy, rest, strlen(rest));
                        }
                }

                fprintf(merged_fp, "%s", line);
        }
    
        debug("\tDONE!\n");

        free(line);
        free(pwrline);
        free(prvline);
        fclose(pwr_fp);
        fclose(prv_fp);
        fclose(merged_fp);
}

static void
addPCFType(char *ifile, char *ofile)
{
        FILE *ifp, *ofp;
        int err = 0;
        char *line = NULL;
        size_t len = 0;

        ofile[strlen(ofile) - 4] = 0;
        strncat(ofile, ".pcf", 4);
        debug("Adding Power type to %s ...", ofile);

        if ((ifp = fopen(ifile, "r")) == NULL) {
                err = errno;
                fprintf(stderr, "%s: %s\n", ifile, strerror(err));
                exit(EXIT_FAILURE);
        }

        if ((ofp = fopen(ofile, "w")) == NULL) {
                err = errno;
                fprintf(stderr, "%s: %s\n", ofile, strerror(err));
                exit(EXIT_FAILURE);
        }

        while (getline(&line, &len, ifp) != -1) {
                fprintf(ofp, "%s", line);
        }

        fprintf(ofp, "\n\nEVENT_TYPE\n0\t90000000\tPower\n");

        free(line);
        fclose(ifp);
        fclose(ofp);

        debug("\tDONE!\n");
}

static void
modifyROW(char *ifile, char *ofile)
{
        FILE *ifp, *ofp;
        int err = 0;
        char *line = NULL;
        size_t len = 0, apps = 0;

        ofile[strlen(ofile) - 4] = 0;
        strncat(ofile, ".row", 4);
        debug("Adding Power application to %s ...", ofile);

        if ((ifp = fopen(ifile, "r")) == NULL) {
                err = errno;
                fprintf(stderr, "%s: %s\n", ifile, strerror(err));
                exit(EXIT_FAILURE);
        }

        if ((ofp = fopen(ofile, "w")) == NULL) {
                err = errno;
                fprintf(stderr, "%s: %s\n", ofile, strerror(err));
                exit(EXIT_FAILURE);
        }

        while (getline(&line, &len, ifp) != -1) {
                if (strstr(line, "LEVEL APPL") == NULL) {
                        fprintf(ofp, "%s", line);
                } else {
                        strtok(line, " ");
                        strtok(NULL, " ");
                        strtok(NULL, " ");
                        apps = strtoul(strtok(NULL, " "), NULL, 10) + 1;
                        fprintf(ofp, "LEVEL APPL SIZE %zu\n", apps);
                }
        }
        fprintf(ofp, "power\n");

        debug("\tDONE!\n");

        free(line);
        fclose(ifp);
        fclose(ofp);
}

/*
 * Modeline for space only BSD KNF code style
 */
/* vim: set textwidth=80 colorcolumn=+0 tabstop=8 softtabstop=8 shiftwidth=8 expandtab cinoptions=\:0l1t0(0.5su0.5sm1: */
