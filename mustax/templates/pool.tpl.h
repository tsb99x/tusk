#ifndef PREFIX
#error "No PREFIX provided for pool template"
#endif

#ifndef T
#error "No T (base type) provided for pool template"
#endif

#define CONCAT(a, b) a ## b
#define EVAL(x, y) CONCAT(x, y)
#define S EVAL(PREFIX, _pool)

struct S;

struct S *EVAL(init_, S)(
        size_t size
);

void EVAL(destroy_, S)(
        struct S *self
);

void EVAL(cleanup_, S)(
        struct S *self
);

#undef T
#undef PREFIX
