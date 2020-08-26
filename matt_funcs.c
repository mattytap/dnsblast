
static unsigned long long
get_nanoseconds(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000000LL + tv.tv_usec * 1000LL;
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
            return weighted_types[i].type;
        }
    } while (++i < weighted_types_len);
    type = weighted_types[rand() % weighted_types_len].type;
    if (type == 12)
    {
        assert(name_size > (size_t)15U);
        int octet1 = (rand() % 256) + 0;
        int octet2 = (rand() % 256) + 0;
        int octet3 = (rand() % 256) + 0;
        int octet4 = (rand() % 256) + 0;
        sprintf(name, "%d%s%d%s%d%s%d", octet1, ".", octet2, ".", octet3, ".", octet4);
    }
    else
    {
        assert(name_size > (size_t)8U);
        const char charset_alnum[36] = "abcdefghijklmnopqrstuvwxyz0123456789";
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
get_sock(const char *const host, const char *const port, struct addrinfo **const ai_ref)
{
    //called by main
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
