#include <stdlib.h>

#include "utility.h"

char *get_env_var(
        char *name,
        char *def_value
) {
        char *env_var = getenv(name);
        return (env_var == NULL) ? def_value : env_var;
}
