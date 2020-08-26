
#include "dnsblast.h"
#include "matt_funcs.c"
#include "matt_url_gen.c"
#include "matt_encode.c"

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
    int p = REFUZZ_PROBABILITY;
    do
    {
        question[rand() % packet_size] = rand() % 0xff;
    } while (rand() < p && (p = p / 2) > 0);
    ssize_t sendtov = sendto(context->sock, question, packet_size, 0, context->ai->ai_addr, context->ai->ai_addrlen);
    while (sendtov != (ssize_t)packet_size)
    {
        if (errno != EAGAIN && errno != EINTR)
        {
            perror("sendto");
            exit(EXIT_FAILURE);
        }
        sendtov = sendto(context->sock, question, packet_size, 0, context->ai->ai_addr, context->ai->ai_addrlen);
    }
    context->sent_packets++;
    return 0;
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
    printf("Sent: [%lu]   -  Received: [%lu] - Reply rate: [%llu pps] - Ratio: [%.2f%%]  \r", context->sent_packets, context->received_packets, rate, (double)context->received_packets * 100.0 / (double)context->sent_packets);
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
    const float elapseds = elapsed / 1000000UL;
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
    char name[100U] = ".";
    Context context;
    struct addrinfo *ai;
    const char *host;
    const char *port = "domain";
    unsigned long pps = ULONG_MAX;
    unsigned long send_count = ULONG_MAX;
    int sock;
    uint16_t type = 0;
    host = "192.168.240.2";
    send_count = strtoul("100", NULL, 10);
    pps = strtoul("10", NULL, 10);
    port = "53";
    if ((sock = get_sock(host, port, &ai)) == -1)
    {
        perror("Oops");
        exit(EXIT_FAILURE);
    }
    Context *const context2 = &context;
    const int sock2 = sock;
    const struct addrinfo *const ai2 = ai;
    const unsigned long long now2 = get_nanoseconds();
    *context2 = (Context){.received_packets = 0UL, .sent_packets = 0UL, .last_status_update = now2, .startup_date = now2, .sock = sock2, .ai = ai2, .sending = 1};
    DNS_Header *const question_header2 = (DNS_Header *)context2->question;
    *question_header2 = (DNS_Header){.flags = htons(FLAGS_OPCODE_QUERY | FLAGS_RECURSION_DESIRED), .qdcount = htons(1U), .ancount = 0U, .nscount = 0U, .arcount = 0U};
    context.pps = pps;
    srand(clock());
    while (send_count > 0UL)
    {
        get_question(name, sizeof name, type);
        blast(&context, name, type);
        throttled_receive(&context);
        --send_count;
    }
    
    unsigned long long elapsed = get_nanoseconds() - context.startup_date;
    unsigned long long rate = context.received_packets * 1000000000ULL / elapsed;
    if (rate > context.pps)
    {
        rate = context.pps;
    }
    printf("Sent: [%lu]   -  Received: [%lu] - Reply rate: [%llu pps] - Ratio: [%.2f%%]  \r", context.sent_packets, context.received_packets, rate, (double)context.received_packets * 100.0 / (double)context.sent_packets);
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
    printf("Sent: [%lu]   -  Received: [%lu] - Reply rate: [%llu pps] - Ratio: [%.2f%%]  \r", context.sent_packets, context.received_packets, rate, (double)context.received_packets * 100.0 / (double)context.sent_packets);
    fflush(stdout);
    putchar('\n');
    return 0;
}
