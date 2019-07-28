

#ifndef _util_types_h_
#define _util_types_h_


int str2int(const char* str, int* i)
{
    int in = 0;
    char ch;

    int negative = 1;

    if ((ch = *str) == '-') {
        negative = -1;
    }
    else if (ch >= '0' && ch <= '9') {
        in = ch - '0';
    }
    else if (ch == '+') {
        // just ignore it.
    }
    else {
        return -1;
    }

    while (ch = *(++str)) {
        if (ch < '0' || ch > '9') {
            return -1;
        }
        in *= 10;
        in += (ch - '0');
    }

    *i = negative * in;
    return 0;
}


#endif /* _util_types_h_ */