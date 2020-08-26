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
get_question(char *const name, size_t name_size, uint16_t type)
{
    assert(name_size > (size_t)8U);
    type = get_random_type();
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