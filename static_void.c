#include "dnsblast.h"
#include <string.h>

static void
main (int argc, char **argv)
{
	int              ret;
	int                c;
	_Bool error = 0;

	struct option long_options[] = {
		{"help",            no_argument,       NULL, 'h'},
		{"version",         no_argument,       NULL, 'V'},
		{"debug",           no_argument,       NULL, 'x'},
		{"unsecure",        no_argument,       NULL, 'u'},
		{"internal-tcp",    no_argument,       NULL, 't'},
		{"disable-iocache", no_argument,       NULL, 'i'},
		{"bind",            optional_argument, NULL, 'b'},
		{"appdir",          required_argument, NULL, 'd'},
		{"port",            required_argument, NULL, 'p'},
		{"target",          required_argument, NULL, 'C'},
		{"threads",         required_argument, NULL, 'T'},
		{NULL, 0, NULL, 0}
	};

	while ((c = getopt_long(argc, argv, "hVxutib::d:p:C:T:", long_options, NULL)) != -1) {
		switch(c) {
		case 'b':
			free (bind_to);
			if (optarg) {
				bind_to = strdup(optarg);
			} else if (argv[optind] && argv[optind][0] != '-') {
				bind_to = strdup(argv[optind]);
				optind++;
			} else {
				bind_to = NULL;
			}
			break;
		case 'p':
			ret = atoi(optarg, &port);
			if (ret != ret_ok) {
				error = 1;
			}
			break;
		case 'T':
			ret = atoi(optarg, &thread_num);
			if (ret != ret_ok) {
				error = 1;
			}
			break;
		case 'd':
			free (document_root);
			document_root = strdup(optarg);
			break;
		case 'C':
			free (config_file);
			config_file = strdup(optarg);
			break;
		case 'x':
			debug   = 1;
			iocache = 0;
			break;
		case 'u':
			unsecure = 1;
			break;
		case 'i':
			iocache = 0;
			break;
		case 't':
			scgi_port = 0;
			break;
		case 'V':
			printf (APP_NAME " " PACKAGE_VERSION "\n" APP_COPY_NOTICE);
			exit(0);
		case 'h':
		case '?':
		default:
			error = 1;
		}

		if (error) {
			print_help();
			exit (1);
		}
	}

	/* Check for trailing parameters
	 */
	for (c = optind; c < argc; c++) {
		if ((argv[c] != NULL) && (strlen(argv[c]) > 0)) {
			print_help();
			exit (1);
		}
	}
}