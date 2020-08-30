#include "dnsblast.h"
static int
init_context(Context *const context, const int sock, const struct addrinfo *const ai, const _Bool fuzz)
{
    *context = (Context){.received_packets = 0UL, .sent_packets = 0UL, .sock = sock, .fuzz = fuzz, .ai = ai, .sending = 1};
    DNS_Header *const question_header = (DNS_Header *)context->question;
    *question_header = (DNS_Header){.flags = htons(FLAGS_OPCODE_QUERY | FLAGS_RECURSION_DESIRED), .qdcount = htons(1U), .ancount = 0U, .nscount = 0U, .arcount = 0U};
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
encode_name(unsigned char **const encoded_ptr, size_t encoded_size, const char *const name)
{
    unsigned char *encoded = *encoded_ptr;
    const char *name_current = name;
    int name_current_pos;
    assert(encoded_size > (size_t)0U);
    encoded_size--;
    for (;;)
    {
        name_current_pos = find_name_component_len(name_current);
        if (name_current_pos == EOF || encoded_size <= (size_t)name_current_pos)
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
blast(Context *const context, const char *const name, const uint16_t type)
{
    unsigned char *const question = context->question;
    DNS_Header *const question_header = (DNS_Header *)question;
    unsigned char *const question_data = question + sizeof *question_header;
    const size_t sizeof_question_data = sizeof question - sizeof *question_header;
    question_header->id = context->id++;
    unsigned char *msg = question_data;
    assert(sizeof_question_data > (size_t)2U);
    encode_name(&msg, sizeof_question_data - (size_t)2U, name);
    PUT_HTONS(msg, type);
    PUT_HTONS(msg, CLASS_IN);
    const size_t packet_size = (size_t)(msg - question);
    ssize_t sendtov = sendto(context->sock, question, packet_size, 0, context->ai->ai_addr, context->ai->ai_addrlen);
    while (sendtov != (ssize_t)packet_size)
    {
        sendtov = sendto(context->sock, question, packet_size, 0, context->ai->ai_addr, context->ai->ai_addrlen);
    }
    context->sent_packets++;
    return 0;
}
// get queestion
//
static uint16_t
get_question(void)
{
    const size_t weighted_types_len = sizeof weighted_types / sizeof weighted_types[0];
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
    //
    return 0;
}
static int
get_sock(const char *const host, const char *const port, struct addrinfo **const ai_ref)
{
    int flag = 1;
    int sock;
    struct addrinfo *ai, servaddr;
    memset(&servaddr, 0, sizeof servaddr);
    servaddr.ai_family = AF_UNSPEC;
    servaddr.ai_flags = 0;
    servaddr.ai_socktype = SOCK_DGRAM;
    servaddr.ai_protocol = IPPROTO_UDP;
    getaddrinfo(host, port, &servaddr, &ai);
    *ai_ref = ai;
    sock = socket((*ai_ref)->ai_family, (*ai_ref)->ai_socktype, (*ai_ref)->ai_protocol);
    setsockopt(sock, SOL_SOCKET, SO_RCVBUFFORCE, &(int[]){MAX_UDP_BUFFER_SIZE}, sizeof(int));
    setsockopt(sock, SOL_SOCKET, SO_SNDBUFFORCE, &(int[]){MAX_UDP_BUFFER_SIZE}, sizeof(int));
#if defined(IP_PMTUDISC_OMIT)
    setsockopt(sock, IPPROTO_IP, IP_MTU_DISCOVER, &(int[]){IP_PMTUDISC_OMIT}, sizeof(int));
#elif defined(IP_MTU_DISCOVER) && defined(IP_PMTUDISC_DONT)
    setsockopt(sock, IPPROTO_IP, IP_MTU_DISCOVER, &(int[]){IP_PMTUDISC_DONT}, sizeof(int));
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
throttled_receive(Context *const context)
{
    while (receive(context) == 0)
        ;
    int ret = 0;
    struct pollfd pfd = {.fd = context->sock, .events = POLLIN | POLLERR};
    do
    {
        //gets stuck on next line
        ret = poll(&pfd, (nfds_t)1, 1000);
        printf("      POLL3 <-------REMAINING_TIME:%d EVENTS:%d RET:%d\n", 1000, pfd.events, ret);
        printf("      POLL4 <-------ID:%d SENDING:%d SENT_PACKETS:%ld RECEIVED_PACKETS:%ld MAX_PACKETS:%lld ELAPSED:%f\n", context->id, context->sending, context->sent_packets, context->received_packets, 1000, 1000);
        printf("      ---------------------------------------------------------------------------------------------------------------------------\n");
        printf("    TR340 <---------ID:%d SENDING:%d SENT_PACKETS:%ld RECEIVED_PACKETS:%ld MAX_PACKETS:%lld ELAPSED:%f\n", context->id, context->sending, context->sent_packets, context->received_packets, 1000, 1000);
        if (ret == 0)
        {
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
    } while (1 > 0);
    return 0;
}
int main()
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
    host = "10.0.0.4";
    send_count = strtoul("100", NULL, 10);
    pps = strtoul("10", NULL, 10);
    port = "5335";
    sock = get_sock(host, port, &ai);
    init_context(&context, sock, ai, fuzz);
    context.pps = pps;
    srand(clock()); //fixes problem with lack of randomness of rand(). MF 20200629
    assert(send_count > 0UL);
    do
    {
        if (rand() < PTR_PROBABILITY)
        {
            if (rand() > REPEATED_NAME_PROBABILITY)
            {
                get_random_name(name, sizeof name);
            }
            type = get_question();
        }
        else
        {
            get_random_ptr(name, sizeof name);
            type = 12U;
        }
        blast(&context, name, type);
        throttled_receive(&context);
    } while (--send_count > 0UL);
    context.sending = 0;
    while (context.sent_packets != context.received_packets)
    {
        throttled_receive(&context);
    }
    freeaddrinfo(ai);
    assert(close(sock) == 0);
    putchar('\n');
    return 0;
}
