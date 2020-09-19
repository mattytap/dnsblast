#include "dnsblast.h"

/* process command-line arguments */
int
main (int argc, char **argv)
{
  int c;
  char *rv[2];

  int option = 0;
  static struct option longopts[] = {
    {"hostname", required_argument, 0, 'H'},
    {"sourceip", required_argument, 0, 'S'},
    {"sourceif", required_argument, 0, 'I'},
    {"critical", required_argument, 0, 'c'},
    {"warning", required_argument, 0, 'w'},
    {"bytes", required_argument, 0, 'b'},
    {"number", required_argument, 0, 'n'},
    {"target-timeout", required_argument, 0, 'T'},
    {"interval", required_argument, 0, 'i'},
    {"verbose", no_argument, 0, 'v'},
    {"version", no_argument, 0, 'V'},
    {"help", no_argument, 0, 'h'},
    {"use-ipv4", no_argument, 0, '4'},
    {"use-ipv6", no_argument, 0, '6'},
    {0, 0, 0, 0}
  };

  rv[PL] = NULL;
  rv[RTA] = NULL;

  if (argc < 2)
    return ERROR;

  if (!is_option (argv[1])) {
    server_name = argv[1];
    argv[1] = argv[0];
    argv = &argv[1];
    argc--;
  }

  while (1) {
    c = getopt_long (argc, argv, "+hVvH:S:c:w:b:n:T:i:I:46", longopts, &option);

    if (c == -1 || c == EOF || c == 1)
      break;

    switch (c) {
    case '?':                 /* print short usage statement if args not parsable */
      usage5 ();
    case 'h':                 /* help */
      print_help ();
      exit (STATE_UNKNOWN);
    case 'V':                 /* version */
      print_revision (progname, NP_VERSION);
      exit (STATE_UNKNOWN);
    case 'v':                 /* verbose mode */
      verbose = TRUE;
      break;
    case 'H':                 /* hostname */
      if (is_host (optarg) == FALSE) {
        usage2 (_("Invalid hostname/address"), optarg);
      }
      server_name = strscpy (server_name, optarg);
      break;
    case 'S':                 /* sourceip */
      if (is_host (optarg) == FALSE) {
        usage2 (_("Invalid hostname/address"), optarg);
      }
      sourceip = strscpy (sourceip, optarg);
      break;
    case 'I':                 /* sourceip */
      sourceif = strscpy (sourceif, optarg);
    case '4':                 /* IPv4 only */
      address_family = AF_INET;
      break;
    case '6':                 /* IPv6 only */
#ifdef USE_IPV6
      address_family = AF_INET6;
#else
      usage (_("IPv6 support not available\n"));
#endif
      break;
    case 'c':
      get_threshold (optarg, rv);
      if (rv[RTA]) {
        crta = strtod (rv[RTA], NULL);
        crta_p = TRUE;
        rv[RTA] = NULL;
      }
      if (rv[PL]) {
        cpl = atoi (rv[PL]);
        cpl_p = TRUE;
        rv[PL] = NULL;
      }
      break;
    case 'w':
      get_threshold (optarg, rv);
      if (rv[RTA]) {
        wrta = strtod (rv[RTA], NULL);
        wrta_p = TRUE;
        rv[RTA] = NULL;
      }
      if (rv[PL]) {
        wpl = atoi (rv[PL]);
        wpl_p = TRUE;
        rv[PL] = NULL;
      }
      break;
    case 'b':                 /* bytes per packet */
      if (is_intpos (optarg))
        packet_size = atoi (optarg);
      else
        usage (_("Packet size must be a positive integer"));
      break;
    case 'n':                 /* number of packets */
      if (is_intpos (optarg))
        packet_count = atoi (optarg);
      else
        usage (_("Packet count must be a positive integer"));
      break;
    case 'T':                 /* timeout in msec */
      if (is_intpos (optarg))
        target_timeout = atoi (optarg);
      else
        usage (_("Target timeout must be a positive integer"));
      break;
    case 'i':                 /* interval in msec */
      if (is_intpos (optarg))
        packet_interval = atoi (optarg);
      else
        usage (_("Interval must be a positive integer"));
      break;
    }
  }

  if (server_name == NULL)
    usage4 (_("Hostname was not supplied"));

  return OK;
}