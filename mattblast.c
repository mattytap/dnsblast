#include "dnsblast.h"

static unsigned long long
get_nanoseconds(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000000LL + tv.tv_usec * 1000LL;
}
static int
init_context(Context *const context, const int sock,
             const struct addrinfo *const ai, const _Bool fuzz)
{
    const unsigned long long now = get_nanoseconds();
    *context = (Context){
        .received_packets = 0UL, .sent_packets = 0UL, .last_status_update = now, .startup_date = now, .sock = sock, .ai = ai, .fuzz = fuzz, .sending = 1};
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
static void
usage(void)
{
    puts("\nUsage: dnsblast [fuzz] <host> [<count>] [<pps>] [<port>]\n");
    exit(EXIT_SUCCESS);
}
// get question
//
static int
get_question(char *const name, size_t name_size, uint16_t type)
{
    assert(name_size > (size_t)8U);
    const size_t weighted_types_len = sizeof weighted_types / sizeof weighted_types[0];
    size_t i = 0U;
    const int rnd = rand();
    int pos = RAND_MAX;
    do
    {
        pos -= weighted_types[i].weight;
        if (rnd > pos)
        {
            type = weighted_types[i].type;
            break;
        }
        else
        {
            type = weighted_types[rand() % weighted_types_len].type;
        }
    } while (++i < weighted_types_len);
    if (type == 12)
    {
        assert(name_size > (size_t)19U);
        int octet1 = (rand() % 256) + 0;
        int octet2 = (rand() % 256) + 0;
        int octet3 = (rand() % 256) + 0;
        int octet4 = (rand() % 256) + 0;
        sprintf(name, "%d%s%d%s%d%s%d", octet1, ".", octet2, ".", octet3, ".", octet4);
    }
    else
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
    }
    return 0;
}
static int
get_random_ptr(char *const name, size_t name_size)
{
    assert(name_size > (size_t)15U);
    int octet1 = (rand() % 256) + 0;
    int octet2 = (rand() % 256) + 0;
    int octet3 = (rand() % 256) + 0;
    int octet4 = (rand() % 256) + 0;
    sprintf(name, "%d%s%d%s%d%s%d", octet1, ".", octet2, ".", octet3, ".", octet4);
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
}static int
get_sock(const char *const host, const char *const port,
         struct addrinfo **const ai_ref)
{
    int flag = 1;
    int sock;
    struct addrinfo *ai, hints;
    memset(&hints, 0, sizeof hints);
    hints = (struct addrinfo){.ai_family = AF_UNSPEC, .ai_flags = 0, .ai_socktype = SOCK_DGRAM, .ai_protocol = IPPROTO_UDP};
    const int gai_err = getaddrinfo(host, port, &hints, &ai);
    if (gai_err != 0)
    {
        fprintf(stderr, "[%s:%s]: [%s]\n", host, port, gai_strerror(gai_err));
        exit(EXIT_FAILURE);
    }
    *ai_ref = ai;
    sock = socket((*ai_ref)->ai_family, (*ai_ref)->ai_socktype, (*ai_ref)->ai_protocol);
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
    ssize_t recvv = recv(context->sock, buf, sizeof buf, 0);
    while (recvv == (ssize_t)-1)
    {
        if (errno == EAGAIN)
        {
            return 1;
        }
        assert(errno == EINTR);
        recvv = recv(context->sock, buf, sizeof buf, 0);
    }
    context->received_packets++;
    return 0;
}
static int
update_status(const Context *const context)
{
    const unsigned long long now = get_nanoseconds();
    const unsigned long long elapsed = now - context->startup_date;
    unsigned long long rate =
        context->received_packets * 1000000000ULL / elapsed;
    if (rate > context->pps)
    {
        rate = context->pps;
    }
    printf("Sent: [%lu] - Received: [%lu] - Reply rate: [%llu pps] - "
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
    unsigned long long now = get_nanoseconds();
    if (now - context->last_status_update < UPDATE_STATUS_PERIOD)
    {
        return 1;
    }
    const unsigned long long elapsed = get_nanoseconds() - context->startup_date;
    unsigned long long rate = context->received_packets * 1000000000ULL / elapsed;
    if (rate > context->pps)
    {
        rate = context->pps;
    }
    printf("Sent: [%lu]   -  Received: [%lu] - Reply rate: [%llu pps] - Ratio: [%.2f%%]  \n", context->sent_packets, context->received_packets, rate, (double)context->received_packets * 100.0 / (double)context->sent_packets);
    fflush(stdout);
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
    unsigned long long now = get_nanoseconds(), now2;
    const unsigned long long elapsed = now - context->startup_date;
    const unsigned long long max_packets =
        context->pps * elapsed / 1000000000UL;
    if (context->sending == 1 && context->sent_packets <= max_packets)
    {
        while (receive(context) == 0)
            ;
        periodically_update_status(context);
    }
    const unsigned long long excess = context->sent_packets - max_packets;
    const unsigned long long time_to_wait = excess / context->pps;
    int remaining_time = (int)(time_to_wait * 1000ULL);
    int ret;
    struct pollfd pfd = {.fd = context->sock,
                         .events = POLLIN | POLLERR};
    if (context->sending == 0)
    {
        remaining_time = -1;
    }
    else if (remaining_time < 0)
    {
        remaining_time = 0;
    }
    do
    {
        //gets stuck on next line
        ret = poll(&pfd, (nfds_t)1, remaining_time);
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
        while (receive(context) == 0)
            ;
        periodically_update_status(context);
        remaining_time -= (get_nanoseconds() - now) / 1000;
        now = remaining_time * 1000 + now;
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
    _Bool fuzz = 0;
    if (argc < 2 || argc > 6)
    {
        usage();
    }
    if (strcasecmp(argv[1], "fuzz") == 0)
    {
        fuzz = 1;
        argv++;
        argc--;
    }
    if (argc < 1)
    {
        usage();
    }
    host = argv[1];
    if (argc > 2)
    {
        send_count = strtoul(argv[2], NULL, 10);
    }
    if (argc > 3)
    {
        pps = strtoul(argv[3], NULL, 10);
    }
    if (argc > 4)
    {
        port = argv[4];
    }
    //host = "10.0.0.4";
    //send_count = strtoul("10", NULL, 10);
    //pps = strtoul("1", NULL, 10);
    //port = "5335";
    if ((sock = get_sock(host, port, &ai)) == -1)
    {
        perror("Oops");
        exit(EXIT_FAILURE);
    }
    printf("A404 HOST:%s PORT:%s SOCK:%d AI_DATA:%s AI_ADDRLEN:%d\n", host, port, sock, ai->ai_addr->sa_data, ai->ai_addrlen); //HEADER
    Context *const context2 = &context;
    const struct addrinfo *const ai2 = ai;
    const unsigned long long now2 = get_nanoseconds();
    *context2 = (Context){.received_packets = 0UL, .sent_packets = 0UL, .last_status_update = now2, .startup_date = now2, .sock = sock, .ai = ai2, .sending = 1};
    DNS_Header *const question_header2 = (DNS_Header *)context2->question;
    *question_header2 = (DNS_Header){.flags = htons(FLAGS_OPCODE_QUERY | FLAGS_RECURSION_DESIRED), .qdcount = htons(1U), .ancount = 0U, .nscount = 0U, .arcount = 0U};
    context.pps = pps;
    srand(clock()); //fixes problem with lack of randomness of rand(). MF 20200629
    while (send_count > 0UL)
    {
        get_question(name, sizeof name, type);
        printf("Question: %s \n", name);
        Context *const context3 = &context;
        const char *const name3 = name;
        unsigned char *const question3 = context3->question;
        DNS_Header *const question3_header = (DNS_Header *)question3;
        unsigned char *const question3_data = question3 + sizeof *question3_header;
        const size_t sizeof_question3_data =
            sizeof question3 - sizeof *question3_header;
        question3_header->id = context3->id++;
        unsigned char *msg = question3_data;
        assert(sizeof_question3_data > (size_t)2U);
        encode_name(&msg, sizeof_question3_data - (size_t)2U, name3);
        PUT_HTONS(msg, type);
        PUT_HTONS(msg, CLASS_IN);
        const size_t packet_size = (size_t)(msg - question3);
        int p = REFUZZ_PROBABILITY;
        do
        {
            question3[rand() % packet_size] = rand() % 0xff;
        } while (rand() < p && (p = p / 2) > 0);
        ssize_t sendtov = sendto(context3->sock, question3, packet_size, 0, context3->ai->ai_addr, context3->ai->ai_addrlen);
        while (sendtov != (ssize_t)packet_size)
        {
            if (errno != EAGAIN && errno != EINTR)
            {
                perror("sendto");
                exit(EXIT_FAILURE);
            }
            sendtov = sendto(context3->sock, question3, packet_size, 0, context3->ai->ai_addr, context3->ai->ai_addrlen);
        }
        context3->sent_packets++;
        throttled_receive(&context);
        --send_count;
    }
    unsigned long long elapsed = get_nanoseconds() - context.startup_date;
    unsigned long long rate = context.received_packets * 1000000000ULL / elapsed;
    if (rate > context.pps)
    {
        rate = context.pps;
    }
    printf("Sent: [%lu]   -  Received: [%lu] - Reply rate: [%llu pps] - Ratio: [%.2f%%]  \n", context.sent_packets, context.received_packets, rate, (double)context.received_packets * 100.0 / (double)context.sent_packets);
    fflush(stdout);
    context.sending = 0;
    while (context.sent_packets != context.received_packets)
    {
        throttled_receive(&context);
    }
    freeaddrinfo(ai);
    assert(close(sock) == 0);
    elapsed = get_nanoseconds() - context.startup_date;
    rate = context.received_packets * 1000000000ULL / elapsed;
    if (rate > context.pps)
    {
        rate = context.pps;
    }
    printf("Sent: [%lu]   -  Received: [%lu] - Reply rate: [%llu pps] - Ratio: [%.2f%%]  \n", context.sent_packets, context.received_packets, rate, (double)context.received_packets * 100.0 / (double)context.sent_packets);
    fflush(stdout);
    putchar('\n');
    return 0;
}

