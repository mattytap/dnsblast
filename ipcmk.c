#include "dnsblast.h"

int main(int argc, char *argv[])
{
    int permission = 0644;
    int opt;
    size_t size = 0;
    int nsems = 0;
    int ask_shm = 0, ask_msg = 0, ask_sem = 0;
    static const struct option longopts[] = {
        {"shmem", required_argument, NULL, 'M'},
        {"semaphore", required_argument, NULL, 'S'},
        {"queue", no_argument, NULL, 'Q'},
        {"mode", required_argument, NULL, 'p'},
        {"version", no_argument, NULL, 'V'},
        {"help", no_argument, NULL, 'h'},
        {NULL, 0, NULL, 0}};

    while ((opt = getopt_long(argc, argv, "hM:QS:p:Vh", longopts, NULL)) != -1)
    {
        switch (opt)
        {
        case 'M':
            //    size = strtou64_or_err(optarg, _("failed to parse size"));
            ask_shm = 1;
            break;
        case 'Q':
            ask_msg = 1;
            break;
        case 'S':
            //    nsems = strtos32_or_err(optarg, _("failed to parse elements"));
            ask_sem = 1;
            break;
        case 'p':
            permission = strtoul(optarg, NULL, 8);
            break;
        case 'h':
            puts("\nUsage: dnsblast [fuzz] <host> [<count>] [<pps>] [<port>]\n");

            break;
        case 'V':
            printf("UTIL_LINUX_VERSION");
            return EXIT_SUCCESS;
        default:
            ask_shm = ask_msg = ask_sem = 0;
            break;
        }
    }

    if (!ask_shm && !ask_msg && !ask_sem)
        //    usage(stderr);

        if (ask_shm)
        {
            int shmid;
            //    if (-1 == (shmid = create_shm(size, permission)))
            //        err(EXIT_FAILURE, _("create share memory failed"));
            //    else
            printf(("Shared memory id: %d\n"), shmid);
        }

    if (ask_msg)
    {
        int msgid;
        //    if (-1 == (msgid = create_msg(permission)))
        //        err(EXIT_FAILURE, _("create message queue failed"));
        //else
        printf(("Message queue id: %d\n"), msgid);
    }

    if (ask_sem)
    {
        int semid;
        //if (-1 == (semid = create_sem(nsems, permission)))
        //    err(EXIT_FAILURE, _("create semaphore failed"));
        //else
        printf(("Semaphore id: %d\n"), semid);
    }

    return EXIT_SUCCESS;
}