
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
