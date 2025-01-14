#ifndef STD_STRING_SHIT_H_
#define STD_STRING_SHIT_H_

typedef struct _std_basic_string {
    void *vtable_probably;
    union {
        char buffer[0x10];
        void *data_ptr;
    } data;
    int current_size;
    int max_size;
} std_basic_string;

typedef void (*std_basic_string_constructor_t)(std_basic_string *string, const char *initial_text);

#endif // STD_STRING_SHIT_H_
