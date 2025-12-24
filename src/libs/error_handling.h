
#ifndef __ERROR_HANDLING_HEADER__
#define __ERROR_HANDLING_HEADER__

#define NO_ERROR 0

#define try_else(fn, err) \
{ \
    if (fn != NO_ERROR) { \
        err; \
    } \
}


#endif
