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
