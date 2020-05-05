#include <utility.h>

#include <stdlib.h>

const char *get_env_var(
        const char *name,
        const char *def_value
) {
        const char *env_var = getenv(name);
        return (env_var == NULL) ? def_value : env_var;
}
