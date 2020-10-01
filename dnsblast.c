
#include "dnsblast.h"

static unsigned long long
get_milliseconds(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    return tv.tv_sec * 1000LL + tv.tv_usec / 1000LL;
}

static int
init_context(Context *const context, const int sock,
             const struct addrinfo *const ai, const _Bool fuzz)
{
    const unsigned long long now = get_milliseconds();
    *context = (Context){
        .received_packets = 0UL,
        .sent_packets = 0UL,
        .last_status_update = now,
        .startup_date = now,
        .sock = sock,
        .ai = ai,
        .fuzz = fuzz,
        .sending = 1};

    DNS_Header *const question_header = (DNS_Header *)context->question;
    *question_header = (DNS_Header){
        .flags = htons(FLAGS_OPCODE_QUERY | FLAGS_RECURSION_DESIRED),
        .qdcount = htons(1U),
        .ancount = 0U,
        .nscount = 0U,
        .arcount = 0U};

    return 0;
}

static int
find_name_component_len(const char *name)
{
    int name_pos = 0;

    while (name[name_pos] != '.' && name[name_pos] != 0)
    {
        if (name_pos >= UCHAR_MAX)
        {
            return EOF;
        }
        name_pos++;
    }
    return name_pos;
}

static int
encode_name(unsigned char **const encoded_ptr, size_t encoded_size,
            const char *const name)
{
    unsigned char *encoded = *encoded_ptr;
    const char *name_current = name;
    int name_current_pos;

    assert(encoded_size > (size_t)0U);
    encoded_size--;
    for (;;)
    {
        name_current_pos = find_name_component_len(name_current);
        if (name_current_pos == EOF ||
            encoded_size <= (size_t)name_current_pos)
        {
            return -1;
        }
        *encoded++ = (unsigned char)name_current_pos;
        memcpy(encoded, name_current, name_current_pos);
        encoded_size -= name_current_pos - (size_t)1U;
        encoded += name_current_pos;
        if (name_current[name_current_pos] == 0)
        {
            break;
        }
        name_current += name_current_pos + 1U;
    }
    *encoded++ = 0;
    *encoded_ptr = encoded;

    return 0;
}

static int
fuzz(unsigned char *const question, const size_t packet_size)
{
    int p = REFUZZ_PROBABILITY;

    do
    {
        question[rand() % packet_size] = rand() % 0xff;
    } while (rand() < p && (p = p / 2) > 0);

    return 0;
}

static int
blast(Context *const context, const char *const name, const uint16_t type)
{
    unsigned char *const question = context->question;
    DNS_Header *const question_header = (DNS_Header *)question;
    unsigned char *const question_data = question + sizeof *question_header;
    const size_t sizeof_question_data =
        sizeof question - sizeof *question_header;

    question_header->id = context->id++;
    unsigned char *msg = question_data;
    assert(sizeof_question_data > (size_t)2U);
    encode_name(&msg, sizeof_question_data - (size_t)2U, name);
    PUT_HTONS(msg, type);
    PUT_HTONS(msg, CLASS_IN);
    const size_t packet_size = (size_t)(msg - question);
    context->datagram_start[question_header->id] = get_milliseconds();

    if (context->fuzz != 0)
    {
        fuzz(question, packet_size);
    }
    while (sendto(context->sock, question, packet_size, 0,
                  context->ai->ai_addr, context->ai->ai_addrlen) != (ssize_t)packet_size)
    {
        if (errno != EAGAIN && errno != EINTR)
        {
            perror("sendto");
            exit(EXIT_FAILURE);
        }
    }
    if (verbose_flag)
        printf("\rQuestion: %2d %s\b\b\b\b\b\t\t  Question sent to server: %4u (%2ld bytes)", type, name, question_header->id + 1U, packet_size);
    context->sent_packets++;

    return 0;
}

static void
usage(void)
{
    puts("\nUsage: dnsblast <host> [-p <port>] [-c <count>] [-s <pps>] [flags]\n\nflages:\t--verbose\n\t--deterministric\n\t--random\n");
    exit(EXIT_SUCCESS);
}

static struct addrinfo *
resolve(const char *const host, const char *const port)
{
    struct addrinfo *ai, hints;

    memset(&hints, 0, sizeof hints);
    hints = (struct addrinfo){
        .ai_family = AF_UNSPEC, .ai_flags = 0, .ai_socktype = SOCK_DGRAM, .ai_protocol = IPPROTO_UDP};
    const int gai_err = getaddrinfo(host, port, &hints, &ai);
    if (gai_err != 0)
    {
        fprintf(stderr, "[%s:%s]: [%s]\n", host, port, gai_strerror(gai_err));
        exit(EXIT_FAILURE);
    }
    return ai;
}

static int
get_random_name(char *const name, size_t name_size)
{
    const char charset_alnum[36] = "abcdefghijklmnopqrstuvwxyz0123456789";

    assert(name_size > (size_t)8U);
    const int r1 = rand(), r2 = rand();
    name[0] = charset_alnum[(r1) % sizeof charset_alnum];
    name[1] = charset_alnum[(r1 >> 16) % sizeof charset_alnum];
    name[2] = charset_alnum[(r2) % sizeof charset_alnum];
    name[3] = charset_alnum[(r2 >> 16) % sizeof charset_alnum];
    name[4] = '.';
    name[5] = 'c';
    name[6] = 'o';
    name[7] = 'm';
    name[8] = 0;

    return 0;
}

static uint16_t
get_random_type(void)
{
    const size_t weighted_types_len =
        sizeof weighted_types / sizeof weighted_types[0];
    size_t i = 0U;
    const int rnd = rand();
    int pos = RAND_MAX;

    do
    {
        pos -= weighted_types[i].weight;
        if (rnd > pos)
        {
            return weighted_types[i].type;
        }
    } while (++i < weighted_types_len);

    return weighted_types[rand() % weighted_types_len].type;
}

static int
get_random_question(char *const name, size_t name_size, uint16_t type)
{
    assert(name_size > (size_t)8U);
    type = get_random_type();
    if (type == 12)
    {
        assert(name_size > (size_t)15U);
        const int r1 = rand(), r2 = rand(), r3 = rand(), r4 = rand();
        sprintf(name, "%d%s%d%s%d%s%d", (r1 % 256) + 0, ".", (r2 % 256) + 0, ".", (r3 % 256) + 0, ".", (r4 % 256) + 0);
    }
    else
        get_random_name(name, name_size);

    return type;
}

static int
get_sock(const char *const host, const char *const port,
         struct addrinfo **const ai_ref)
{
    int flag = 1;
    int sock;

    *ai_ref = resolve(host, port);
    sock = socket((*ai_ref)->ai_family, (*ai_ref)->ai_socktype,
                  (*ai_ref)->ai_protocol);
    if (sock == -1)
    {
        return -1;
    }
    setsockopt(sock, SOL_SOCKET, SO_RCVBUFFORCE,
               &(int[]){MAX_UDP_BUFFER_SIZE}, sizeof(int));
    setsockopt(sock, SOL_SOCKET, SO_SNDBUFFORCE,
               &(int[]){MAX_UDP_BUFFER_SIZE}, sizeof(int));
#if defined(IP_PMTUDISC_OMIT)
    setsockopt(sock, IPPROTO_IP, IP_MTU_DISCOVER,
               &(int[]){IP_PMTUDISC_OMIT}, sizeof(int));
#elif defined(IP_MTU_DISCOVER) && defined(IP_PMTUDISC_DONT)
    setsockopt(sock, IPPROTO_IP, IP_MTU_DISCOVER,
               &(int[]){IP_PMTUDISC_DONT}, sizeof(int));
#elif defined(IP_DONTFRAG)
    setsockopt(sock, IPPROTO_IP, IP_DONTFRAG, &(int[]){0}, sizeof(int));
#endif
    assert(ioctl(sock, FIONBIO, &flag) == 0);

    return sock;
}

static int
receive(Context *const context)
{
    unsigned char buf[MAX_UDP_DATA_SIZE];
    ssize_t recvv;
    unsigned int received_id;
    unsigned long long elapsed;

    while (1)
    {
        recvv = recv(context->sock, (char *)buf, MAX_UDP_DATA_SIZE, MSG_WAITALL);
        if (recvv != (ssize_t)-1)
            break; // received something
        if (errno == EAGAIN)
        {
            return 1;
        }
        assert(errno == EINTR);
    }
    buf[recvv] = '\0';
    received_id = ((unsigned char)buf[1] << 8) + (unsigned char)buf[0];
    elapsed = get_milliseconds() - context->datagram_start[received_id];
    if (verbose_flag)
    {
        printf("\r\t\t\t\t\t\t\t\t\t\tAnswer from server:  %4d (%3ld bytes)\tQuery time (ans): [%4llu msec] ", received_id + 1U, recvv, elapsed);
        printf("\n");
    }
    context->received_packets++;

    return 0;
}

static int
update_status(const Context *const context)
{
    const unsigned long long elapsed = get_milliseconds() - context->startup_date;
    unsigned long long rate =
        (double)context->received_packets * 1000 / (double)elapsed;



    if (!verbose_flag || context->sending == 0)
        printf("Sent: [%4lu] - Received: [%4lu] - Reply rate: [%4llu pps] - "
               "Ratio: [%.2f%%]  \r",
               context->sent_packets, context->received_packets, rate,
               (double)context->received_packets * 100.0 /
                   (double)context->sent_packets);
    fflush(stdout);

    return 0;
}

static int
periodically_update_status(Context *const context)
{
    unsigned long long now = get_milliseconds();

    if (now - context->last_status_update < UPDATE_STATUS_PERIOD)
    {
        return 1;
    }
    update_status(context);
    context->last_status_update = now;

    return 0;
}

static int
empty_receive_queue(Context *const context)
{
    while (receive(context) == 0)
        ;
    periodically_update_status(context);

    return 0;
}

static int
throttled_receive(Context *const context)
{
    unsigned long long now = get_milliseconds(), now2;
    const unsigned long long elapsed = now - context->startup_date;
    const double max_packets =
        (double)context->pps * (double)elapsed / 1000;

    if (context->sending == 1 && context->sent_packets <= max_packets)
    {
        empty_receive_queue(context);
    }
    const double excess = context->sent_packets - max_packets;
    const double time_to_wait = (double)excess / (double)context->pps; //the more beehibg, he longer the catchup time
    int remaining_time = (double)(time_to_wait * 1000ULL);
    int ret;
    struct pollfd pfd = {.fd = context->sock,
                         .events = POLLIN | POLLERR};
    if (context->sending == 0)
    {
        remaining_time = context->timeout;
    }
    else if (remaining_time < 0)
    {
        remaining_time = 0;
    }
    do
    {
        ret = poll(&pfd, (nfds_t)1, remaining_time);
        if ((ret == 0) && (context->sending == 0))
        {
            context->failed_packets++;
            return 1;
        }
        if (ret == 0)
        {
            periodically_update_status(context);
            return 0;
        }
        if (ret == -1)
        {
            if (errno != EAGAIN && errno != EINTR)
            {
                perror("poll");
                exit(EXIT_FAILURE);
            }
            continue;
        }
        assert(ret == 1);
        empty_receive_queue(context);
        now2 = get_milliseconds();
        remaining_time -= (now2 - now);
        now = now2;
    } while (remaining_time > 0);

    return 0;
}

int main(int argc, char *argv[])
{
    char name[100U] = ".";
    Context context;
    struct addrinfo *ai;
    const char *host;
    const char *port = "domain";
    unsigned long pps = ULONG_MAX;
    unsigned long send_count = ULONG_MAX;
    int sock;
    uint16_t type = 0;
    unsigned long timeout = strtoul("250", NULL, 10);
    _Bool fuzz = 0;

    if (argc < 2 || argc > 11)
    {
        usage();
    }

    host = argv[1];
    argv++;
    argc--;

    int opt;
    while (1)
    {
        opt = getopt_long(argc, argv, "hp:c:s:t:fVh", longopts, NULL);

        /* Detect the end of the options. */
        if (opt == -1)
            break;

        switch (opt)
        {
        case 'h':
            usage();
            exit(1);
        case 'p':
            port = optarg;
            break;
        case 'c':
            send_count = strtoul(optarg, NULL, 10);
            break;
        case 's':
            pps = strtoul(optarg, NULL, 10);
            break;
        case 't':
            timeout = strtoul(optarg, NULL, 10);
            printf("timeout: %ld\n", timeout);
            break;
        case 'f':
            fuzz = 1;
            break;
        case 'V':
            printf("version mattytap/dnsblast 2020\nFork of jedisct1/dnsblast\n");
            break;
        case ':':
            printf("option needs a value\n");
            break;
        case '?':
            printf("unknown option: %c\n", optopt);
            break;
        }
    }

    if (verbose_flag)
        puts("verbose flag is set");
    if (deterministic_flag)
        puts("deterministic flag is set");
    if (!deterministic_flag)
        puts("random flag is set");

    for (; optind < argc; optind++)
    {
        printf("extra arguments: %s\n", argv[optind]);
    }
    if ((sock = get_sock(host, port, &ai)) == -1)
    {
        perror("Oops");
        exit(EXIT_FAILURE);
    }
    if (verbose_flag)
        printf("A404 HOST:%s PORT:%s SOCK:%d AI_DATA:%s AI_ADDRLEN:%d\n", host, port, sock, ai->ai_addr->sa_data, ai->ai_addrlen); //HEADER
    init_context(&context, sock, ai, fuzz);
    context.pps = pps;
    context.timeout = timeout;
    if (deterministic_flag)
    {
        srand(0U); //deterministic
    }
    else
    {
        srand(clock()); //random
    }
    assert(send_count > 0UL);
    do
    {
        if (rand() > REPEATED_NAME_PROBABILITY)
        {
            type = get_random_question(name, sizeof name, type);
        }

        blast(&context, name, type);
        unsigned long packets_received_before_throlled_receive = context.received_packets;
        throttled_receive(&context);
        if (verbose_flag && packets_received_before_throlled_receive == context.received_packets)
            printf("\n");
    } while (--send_count > 0UL);
    update_status(&context);

    context.sending = 0;
    long long now = get_milliseconds();
    while (context.sent_packets != context.received_packets)
    {
        throttled_receive(&context);
        if (get_milliseconds() - now > timeout)
            break;
    }
    freeaddrinfo(ai);
    assert(close(sock) == 0);
    update_status(&context);
    putchar('\n');

    return 0;
}
