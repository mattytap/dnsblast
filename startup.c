#include "dnsblast.h"

static void
main(int argc, char *argv[])
{
    struct adhoc_opts *options;
    static struct option long_options[] =
        {
            {"echo-all", no_argument, NULL, 'a'},
            {"no-align", no_argument, NULL, 'A'},
            {"command", required_argument, NULL, 'c'},
            {"dbname", required_argument, NULL, 'd'},
            {"echo-queries", no_argument, NULL, 'e'},
            {"echo-errors", no_argument, NULL, 'b'},
            {"echo-hidden", no_argument, NULL, 'E'},
            {"file", required_argument, NULL, 'f'},
            {"field-separator", required_argument, NULL, 'F'},
            {"field-separator-zero", no_argument, NULL, 'z'},
            {"host", required_argument, NULL, 'h'},
            {"html", no_argument, NULL, 'H'},
            {"list", no_argument, NULL, 'l'},
            {"log-file", required_argument, NULL, 'L'},
            {"no-readline", no_argument, NULL, 'n'},
            {"single-transaction", no_argument, NULL, '1'},
            {"output", required_argument, NULL, 'o'},
            {"port", required_argument, NULL, 'p'},
            {"pset", required_argument, NULL, 'P'},
            {"quiet", no_argument, NULL, 'q'},
            {"record-separator", required_argument, NULL, 'R'},
            {"record-separator-zero", no_argument, NULL, '0'},
            {"single-step", no_argument, NULL, 's'},
            {"single-line", no_argument, NULL, 'S'},
            {"tuples-only", no_argument, NULL, 't'},
            {"table-attr", required_argument, NULL, 'T'},
            {"username", required_argument, NULL, 'U'},
            {"set", required_argument, NULL, 'v'},
            {"variable", required_argument, NULL, 'v'},
            {"version", no_argument, NULL, 'V'},
            {"no-password", no_argument, NULL, 'w'},
            {"password", no_argument, NULL, 'W'},
            {"expanded", no_argument, NULL, 'x'},
            {"no-psqlrc", no_argument, NULL, 'X'},
            {"help", optional_argument, NULL, 1},
            {NULL, 0, NULL, 0}};

    int optindex;
    int c;

    memset(options, 0, sizeof *options);

    while ((c = getopt_long(argc, argv, "aAbc:d:eEf:F:h:HlL:no:p:P:qR:sStT:U:v:VwWxXz?01",
                            long_options, &optindex)) != -1)
    {
        switch (c)
        {
        case 'a':
            SetVariable(pset.vars, "ECHO", "all");
            break;
        case 'A':
            pset.popt.topt.format = PRINT_UNALIGNED;
            break;
        case 'b':
            SetVariable(pset.vars, "ECHO", "errors");
            break;
        case 'c':
            if (optarg[0] == '\\')
                simple_action_list_append(&options->actions,
                                          ACT_SINGLE_SLASH,
                                          optarg + 1);
            else
                simple_action_list_append(&options->actions,
                                          ACT_SINGLE_QUERY,
                                          optarg);
            break;
        case 'd':
            options->dbname = pg_strdup(optarg);
            break;
        case 'e':
            SetVariable(pset.vars, "ECHO", "queries");
            break;
        case 'E':
            SetVariableBool(pset.vars, "ECHO_HIDDEN");
            break;
        case 'f':
            simple_action_list_append(&options->actions,
                                      ACT_FILE,
                                      optarg);
            break;
        case 'F':
            pset.popt.topt.fieldSep.separator = pg_strdup(optarg);
            pset.popt.topt.fieldSep.separator_zero = false;
            break;
        case 'h':
            options->host = pg_strdup(optarg);
            break;
        case 'H':
            pset.popt.topt.format = PRINT_HTML;
            break;
        case 'l':
            options->list_dbs = true;
            break;
        case 'L':
            options->logfilename = pg_strdup(optarg);
            break;
        case 'n':
            options->no_readline = true;
            break;
        case 'o':
            if (!setQFout(optarg))
                exit(EXIT_FAILURE);
            break;
        case 'p':
            options->port = pg_strdup(optarg);
            break;
        case 'P':
        {
            char *value;
            char *equal_loc;
            bool result;

            value = pg_strdup(optarg);
            equal_loc = strchr(value, '=');
            if (!equal_loc)
                result = do_pset(value, NULL, &pset.popt, true);
            else
            {
                *equal_loc = '\0';
                result = do_pset(value, equal_loc + 1, &pset.popt, true);
            }

            if (!result)
            {
                fprintf(stderr, _("%s: could not set printing parameter \"%s\"\n"), pset.progname, value);
                exit(EXIT_FAILURE);
            }

            free(value);
            break;
        }
        case 'q':
            SetVariableBool(pset.vars, "QUIET");
            break;
        case 'R':
            pset.popt.topt.recordSep.separator = pg_strdup(optarg);
            pset.popt.topt.recordSep.separator_zero = false;
            break;
        case 's':
            SetVariableBool(pset.vars, "SINGLESTEP");
            break;
        case 'S':
            SetVariableBool(pset.vars, "SINGLELINE");
            break;
        case 't':
            pset.popt.topt.tuples_only = true;
            break;
        case 'T':
            pset.popt.topt.tableAttr = pg_strdup(optarg);
            break;
        case 'U':
            options->username = pg_strdup(optarg);
            break;
        case 'v':
        {
            char *value;
            char *equal_loc;

            value = pg_strdup(optarg);
            equal_loc = strchr(value, '=');
            if (!equal_loc)
            {
                if (!DeleteVariable(pset.vars, value))
                {
                    fprintf(stderr, _("%s: could not delete variable \"%s\"\n"),
                            pset.progname, value);
                    exit(EXIT_FAILURE);
                }
            }
            else
            {
                *equal_loc = '\0';
                if (!SetVariable(pset.vars, value, equal_loc + 1))
                {
                    fprintf(stderr, _("%s: could not set variable \"%s\"\n"),
                            pset.progname, value);
                    exit(EXIT_FAILURE);
                }
            }

            free(value);
            break;
        }
        case 'V':
            showVersion();
            exit(EXIT_SUCCESS);
        case 'w':
            pset.getPassword = TRI_NO;
            break;
        case 'W':
            pset.getPassword = TRI_YES;
            break;
        case 'x':
            pset.popt.topt.expanded = true;
            break;
        case 'X':
            options->no_psqlrc = true;
            break;
        case 'z':
            pset.popt.topt.fieldSep.separator_zero = true;
            break;
        case '0':
            pset.popt.topt.recordSep.separator_zero = true;
            break;
        case '1':
            options->single_txn = true;
            break;
        case '?':
            /* Actual help option given */
            if (strcmp(argv[optind - 1], "-?") == 0)
            {
                usage(NOPAGER);
                exit(EXIT_SUCCESS);
            }
            /* unknown option reported by getopt */
            else
                goto unknown_option;
            break;
        case 1:
        {
            if (!optarg || strcmp(optarg, "options") == 0)
                usage(NOPAGER);
            else if (optarg && strcmp(optarg, "commands") == 0)
                slashUsage(NOPAGER);
            else if (optarg && strcmp(optarg, "variables") == 0)
                helpVariables(NOPAGER);
            else
                goto unknown_option;

            exit(EXIT_SUCCESS);
        }
        break;
        default:
        unknown_option:
            fprintf(stderr, _("Try \"%s --help\" for more information.\n"),
                    pset.progname);
            exit(EXIT_FAILURE);
            break;
        }
    }

    /*
	 * if we still have arguments, use it as the database name and username
	 */
    while (argc - optind >= 1)
    {
        if (!options->dbname)
            options->dbname = argv[optind];
        else if (!options->username)
            options->username = argv[optind];
        else if (!pset.quiet)
            fprintf(stderr, _("%s: warning: extra command-line argument \"%s\" ignored\n"),
                    pset.progname, argv[optind]);

        optind++;
    }
}