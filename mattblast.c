#include "dnsblast.h"
#include "mattblast_funcs.c"
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
//##
//--
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
throttled_receive(Context *const context)
{
    unsigned long long now = get_nanoseconds();
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
    int ret = 0;
    struct pollfd pfd = {.fd = context->sock, .events = POLLIN | POLLERR};
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
    char name[20U] = ".";
    Context context;
    struct addrinfo *ai;
    const char *host;
    const char *port = "domain";
    unsigned long pps = ULONG_MAX;
    unsigned long send_count = ULONG_MAX;
    int sock;
    uint16_t type = 0;
//    _Bool fuzz = 0;
    if (argc < 2 || argc > 6)
    {
        usage();
    }
    if (strcasecmp(argv[1], "fuzz") == 0)
    {
//        fuzz = 1;
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
