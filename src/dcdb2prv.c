#include "dcdb2prv.h"
#include "parseOptions.h"

#define debug(...) if (verbose) fprintf(stderr, __VA_ARGS__)

char *pwr_fn = NULL;
char *output_fn = NULL;
bool verbose = false;
char *offset = NULL;

int
main(int argc, char **argv)
{
        int ret = 0, err = 0;
        FILE *pwr_fp, *output_fp;
        char *sensor = NULL;
        char *sensor_old = malloc(sizeof(char));
        char *sens = NULL;
        uint64_t first_ts, last_ts, timestamp, energy;
        char *line = NULL;
        char *linecpy = NULL;
        ssize_t read = 0;
        size_t len = 0;
        int node = 0;
        GHashTable *nodes_ht = g_hash_table_new(g_str_hash, g_str_equal);
        GHashTable *nodes_inv_ht = g_hash_table_new(g_str_hash, g_str_equal);
        GPtrArray *sorted_trace_a = g_ptr_array_new();
        int index = 0;
        char *prvnode = malloc(sizeof(char *));

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

        /* Load and sort power trace */
        while ((read = getline (&line, &len, pwr_fp)) != -1 ) {
                linecpy = strndup(line, read);
                sensor = strtok(linecpy, ",");
                sprintf(prvnode, "%d", node);

                if (g_hash_table_insert(nodes_ht, strdup(sensor), strdup(prvnode))) {
                        g_hash_table_insert(nodes_inv_ht, strdup(prvnode), strdup(sensor));
                        node ++;
                }
                g_ptr_array_add(sorted_trace_a, strndup(line, read));
        }

        g_ptr_array_sort(sorted_trace_a, (GCompareFunc)timecmp);

//        g_hash_table_foreach(nodes_ht, (GHFunc)dump_pair, NULL);

        strcpy(line, g_ptr_array_index(sorted_trace_a, 0));
        strtok(line, ",");
        first_ts = strtoul(strtok(NULL, ","), NULL, 10);

        strcpy(line, g_ptr_array_index(sorted_trace_a, sorted_trace_a->len - 1));
        strtok(line, ",");
        last_ts = strtoul(strtok(NULL, ","), NULL, 10);

        writeHeader(output_fp, last_ts - first_ts, node);

        for (index = 0; index < sorted_trace_a->len; index++) {
                line = g_ptr_array_index(sorted_trace_a, index);

                /* Get node */
                sensor = strtok(line, ",");
                prvnode = g_hash_table_lookup(nodes_ht, sensor);

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

                    //"2:%s:2:%s:1:%" PRIu64 ":90000000:%" PRIu64 "\n",
                fprintf(output_fp,
                    "2:%s:1:%s:1:%" PRIu64 ":90000000:%" PRIu64 "\n",
                    prvnode,
                    prvnode,
                    timestamp,
                    energy);
        }

        free(line);
        fclose(pwr_fp);
        fclose(output_fp);

        createPCF(output_fn);
        createROW(output_fn, nodes_inv_ht);

        g_ptr_array_free(sorted_trace_a, true);
        g_hash_table_destroy(nodes_ht);
        g_hash_table_destroy(nodes_inv_ht);

        free(sensor_old);
        free(prvnode);

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
writeHeader(FILE *output_fp, uint64_t duration, int node)
{
        int err = 0;

        time_t now = time(0);
        struct tm *local = localtime(&now);
        char day[3], mon[3], hour[3], min[3];
        int i = 0;

        sprintf(day, "%.2d", local->tm_mday);
        sprintf(mon, "%.2d", local->tm_mon + 1);
        sprintf(hour, "%.2d", local->tm_hour);
        sprintf(min, "%.2d", local->tm_min);
        fprintf(output_fp, "#Paraver (%s/%s/%d at %s:%s):%" PRIu64 "_ns:%i(",
            day,
            mon,
            local->tm_year + 1900,
            hour,
            min,
            duration,
            node
        );

        for (i=0; i<node; i++) {
                fprintf(output_fp, "1,");
        }
        fseek(output_fp, -1, SEEK_CUR);
        fprintf(output_fp, "):1:%i:(", node);

        for (i=0; i<node; i++) {
                fprintf(output_fp, "1:%i,", i+1);
        }
        fseek(output_fp, -1, SEEK_CUR);
        fprintf(output_fp, ")\n");
}

static void
createPCF(char *ofile)
{
        FILE *ofp;
        int err = 0;

        ofile[strlen(ofile) - 4] = 0;
        strncat(ofile, ".pcf", 4);
        debug("Creating %s ...\n", ofile);

        if ((ofp = fopen(ofile, "w")) == NULL) {
                err = errno;
                fprintf(stderr, "%s: %s\n", ofile, strerror(err));
                exit(EXIT_FAILURE);
        }

        fprintf(ofp, "DEFAULT_OPTIONS\n\n"
            "LEVEL               THREAD\n"
            "UNITS               NANOSEC\n"
            "LOOK_BACK           100\n"
            "SPEED               1\n"
            "FLAG_ICONS          ENABLED\n"
            "NUM_OF_STATE_COLORS 1000\n"
            "YMAX_SCALE          37\n\n\n"
            "DEFAULT_SEMANTIC\n\n"
            "THREAD_FUNC          State As Is\n\n\n"
            "STATES\n"
            "0    Idle\n"
            "1    Running\n"
            "2    Not created\n"
            "3    Waiting a message\n"
            "4    Blocking Send\n"
            "5    Synchronization\n"
            "6    Test/Probe\n"
            "7    Scheduling and Fork/Join\n"
            "8    Wait/WaitAll\n"
            "9    Blocked\n"
            "10    Immediate Send\n"
            "11    Immediate Receive\n"
            "12    I/O\n"
            "13    Group Communication\n"
            "14    Tracing Disabled\n"
            "15    Others\n"
            "16    Send Receive\n"
            "17    Memory transfer\n"
            "18    Profiling\n"
            "19    On-line analysis\n"
            "20    Remote memory access\n"
            "21    Atomic memory operation\n"
            "22    Memory ordering operation\n"
            "23    Distributed locking\n"
            "24    Overhead\n"
            "25    One-sided op\n"
            "26    Startup latency\n"
            "27    Waiting links\n"
            "28    Data copy\n"
            "29    RTT\n"
            "30    Allocating memory\n"
            "31    Freeing memory\n\n\n"
            "STATES_COLOR\n"
            "0    {117,195,255}\n"
            "1    {0,0,255}\n"
            "2    {255,255,255}\n"
            "3    {255,0,0}\n"
            "4    {255,0,174}\n"
            "5    {179,0,0}\n"
            "6    {0,255,0}\n"
            "7    {255,255,0}\n"
            "8    {235,0,0}\n"
            "9    {0,162,0}\n"
            "10    {255,0,255}\n"
            "11    {100,100,177}\n"
            "12    {172,174,41}\n"
            "13    {255,144,26}\n"
            "14    {2,255,177}\n"
            "15    {192,224,0}\n"
            "16    {66,66,66}\n"
            "17    {255,0,96}\n"
            "18    {169,169,169}\n"
            "19    {169,0,0}\n"
            "20    {0,109,255}\n"
            "21    {200,61,68}\n"
            "22    {200,66,0}\n"
            "23    {0,41,0}\n"
            "24    {139,121,177}\n"
            "25    {116,116,116}\n"
            "26    {200,50,89}\n"
            "27    {255,171,98}\n"
            "28    {0,68,189}\n"
            "29    {52,43,0}\n"
            "30    {255,46,0}\n"
            "31    {100,216,32}\n\n\n"
            "EVENT_TYPE\n"
            "9   90000000    Power\n");

        fclose(ofp);

        debug("\tDONE!\n");
}

static void
createROW(char *ofile, GHashTable *nodes_inv_ht)
{
        FILE *ofp;
        int err = 0;
        int i = 0;
        int nnodes = g_hash_table_size(nodes_inv_ht);
        char *node_name;
        char *key = malloc(sizeof(char *));

        ofile[strlen(ofile) - 4] = 0;
        strncat(ofile, ".row", 4);
        debug("Creating %s ...", ofile);

        if ((ofp = fopen(ofile, "w")) == NULL) {
                err = errno;
                fprintf(stderr, "%s: %s\n", ofile, strerror(err));
                exit(EXIT_FAILURE);
        }

        fprintf(ofp, "LEVEL CPU SIZE %i\n", nnodes);
        for (i = 1; i<=nnodes; i++) {
                fprintf(ofp, "CPU %i\n", i);
        }

        fprintf(ofp, "\nLEVEL NODE SIZE %i\n", nnodes);
        for (i = 0; i<nnodes; i++) {
                sprintf(key, "%i", i);
                node_name = g_hash_table_lookup(nodes_inv_ht, key);
                fprintf(ofp, "%s\n", node_name);
        }

        fprintf(ofp, "\nLEVEL THREAD SIZE %i\n", nnodes);
        for (i = 1; i<=nnodes; i++) {
                fprintf(ofp, "THREAD 1.1.%i\n", i);
        }

        fclose(ofp);

        debug("\tDONE!\n");
}

gint
timecmp (gconstpointer a, gconstpointer b) {
        char *line_a = strdup(*(char **)a);
        char *line_b = strdup(*(char **)b);
        char *time_as, *time_bs;

        strtok(line_a, ",");
        time_as = strtok(NULL, ",");

        strtok(line_b, ",");
        time_bs = strtok(NULL, ",");

        return strcmp (time_as, time_bs);
}

static void
dump_pair (const char *key, const char *value) {
        g_print("Key: %s Value: %s\n", key, value);
}

/*
 * Modeline for space only BSD KNF code style
 */
/* vim: set textwidth=80 colorcolumn=+0 tabstop=8 softtabstop=8 shiftwidth=8 expandtab cinoptions=\:0l1t0+0.5s(0.5su0.5sm1: */
