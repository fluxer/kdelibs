
#ifndef KENTITIES_H
#define KENTITIES_H

struct entity {
    const char* name;
    int code;
};
class EntitiesHash {
public:
    // Not inlined for non-GCC compilers
    static const struct entity *kde_findEntity (register const char *str, register unsigned int len);
};

#endif
