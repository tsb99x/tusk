#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "node.h"

#define SZ_POOL_SIZE 8196
char sz_pool[SZ_POOL_SIZE];
char *sz_pool_pos = sz_pool;

char *place_in_pool(
        const char *src,
        size_t src_len
) {
        char *orig_pos = sz_pool_pos;
        memcpy(orig_pos, src, src_len);
        sz_pool_pos += src_len;
        *sz_pool_pos = '\0';
        sz_pool_pos++;
        return orig_pos;
}

long file_size(
        FILE *file
) {
        fseek(file, 0, SEEK_END);
        long size = ftell(file);
        fseek(file, 0, SEEK_SET);
        return size;
}

void replace_chars(
        char *str_sz,
        size_t str_len
) {
        size_t rn_pos;
        while ((rn_pos = strcspn(str_sz, "\"\r\n")) != str_len) {
                str_sz[rn_pos] = (str_sz[rn_pos] == '"') ? '\'' : ' ';
        }
}

char *skip_spaces(
        char *str
) {
        while (*str == ' ' && *str != '\0')
                str++;
        return str;
}

char *move_memory_block(
        char *dst,
        char *beg,
        char *end
) {
        if (beg != dst) {
                ptrdiff_t diff = beg - dst;
                memmove(dst, beg, end - beg + 1); // include '\0'
                end -= diff;
        }
        return end;
}

void truncate(
        char *str_end
) {
        while (*(--str_end) == ' ');
        *(str_end + 1) = '\0';
}

void cleanup_literal(
        char *lit
) {
        char *lit_end = lit + strlen(lit);

        char *first_char = skip_spaces(lit);
        lit_end = move_memory_block(lit, first_char, lit_end);

        char *ed_brace = lit;
        while ((ed_brace = strchr(ed_brace, '>')) != NULL) {
                ed_brace++; // skip '>'
                char *ch = skip_spaces(ed_brace);
                if (*ch != '<')
                        continue;
                lit_end = move_memory_block(ed_brace, ch, lit_end);
        }

        truncate(lit_end);
}

void split_tokens(
        struct nodes_pool *nodes,
        char *buf,
        char *buf_end
) {
        char *sz_loc;
        char *op_brace;
        char *ed_brace;

        while ((op_brace = strstr(buf, "{{")) != NULL) {
                sz_loc = place_in_pool(buf, op_brace - buf);
                cleanup_literal(sz_loc);
                emplace_node(nodes, LITERAL, sz_loc);

                op_brace += 2; // skip "{{"
                ed_brace = strstr(buf, "}}");
                if (ed_brace == NULL) {
                        puts("Closing }} braces not found!");
                        return; // FIXME : think about this case!
                }

                sz_loc = place_in_pool(op_brace, ed_brace - op_brace);
                cleanup_literal(sz_loc);
                emplace_node(nodes, VARIABLE, sz_loc);
                
                buf = ed_brace + 2; // skip "}}"
        }

        if (buf <= buf_end) {
                sz_loc = place_in_pool(buf, buf_end - buf);
                cleanup_literal(sz_loc);
                emplace_node(nodes, LITERAL, sz_loc);
        }
}

#define SRC_BUF_SIZE 65535
char src_buf[SRC_BUF_SIZE];

size_t load_file_to_buf(
        const char *filename,
        char *buf,
        size_t buf_size
) {
        FILE *file = fopen(filename, "rb");
        if (file == NULL) {
                fprintf(stderr, "Error opening file %s\n", filename);
                exit(EXIT_FAILURE);
        }

        long size = file_size(file);
        if (size >= (long) buf_size) {
                fprintf(stderr, "No enough memory to load source file\n");
                fclose(file);
                exit(EXIT_FAILURE);
        }
        fread(buf, sizeof(char), size, file);
        src_buf[size] = '\0';

        fclose(file);
        return size;
}

#define TAB "        "

struct count_state {
        size_t vars_count;
};

void count_variables(
        struct node *el,
        struct count_state *state
) {
        if (VARIABLE == el->type)
                state->vars_count++;
}

struct output_state {
        FILE *file;
};

void output_var_into_param(
        struct node *el,
        struct output_state *state
) {
        if (el->type != VARIABLE)
                return;
        fprintf(state->file,
                TAB "const char *%s;\n",
                el->value);
}

void output_copy_process(
        struct node *el,
        struct output_state *state
) {
        if (el->type == LITERAL) {
                size_t len = strlen(el->value);
                fprintf(state->file,
                        TAB "memcpy(it, \"%s\", %zu);\n" \
                        TAB "it += %zu;\n",
                        el->value, len, len);
        }
        if (el->type == VARIABLE) {
                fprintf(state->file,
                        TAB "if (params->%s != NULL) {\n" \
                        TAB TAB "size_t len = strlen(params->%s);\n" \
                        TAB TAB "strcpy(it, params->%s);\n" \
                        TAB TAB "it += len;\n" \
                        TAB "}\n",
                        el->value, el->value, el->value);
        }
}

void generate_and_output_to(
        const char *filename,
        const char *base_name,
        struct nodes_pool *nodes
) {
        FILE *file = fopen(filename, "w");
        if (file == NULL) {
                fprintf(stderr, "Error opening file %s\n", filename);
                exit(EXIT_FAILURE);
        }

        fprintf(file,
                "#ifndef __%s_H__\n" \
                "#define __%s_H__\n" \
                "\n" \
                "#include \"string.h\"\n" \
                "\n",
                base_name, base_name);

        struct count_state c_state = {.vars_count = 0};
        for_each_node(nodes, &c_state, count_variables);
        size_t vars_count = c_state.vars_count;

        struct output_state o_state = {.file = file};
        if (vars_count > 0) {
                fprintf(file,
                        "struct %s_params {\n",
                        base_name);
                for_each_node(nodes, &o_state, output_var_into_param);
                fprintf(file,
                        "};\n\n");
        }

        fprintf(file,
                "size_t %s_template(\n",
                base_name);

        if (vars_count > 0)
                fprintf(file,
                        TAB "const struct %s_params *params,\n",
                        base_name);
        
        fprintf(file,
                TAB "char *dst\n" \
                ") {\n" \
                TAB "char *it = dst;\n");
        for_each_node(nodes, &o_state, output_copy_process);
        fprintf(file,
                TAB "return it - dst;\n" \
                "}\n" \
                "\n" \
                "#endif\n");
        fclose(file);
}

#define NODES_POOL_SIZE 256
#define DEF_STR_SIZE 128

int main(
        int argc,
        char **argv
) {
        if (argc < 2) {
                fprintf(stderr, "At least one template base name is required!");
                exit(EXIT_FAILURE);
        }

        struct nodes_pool *nodes = init_nodes_pool(NODES_POOL_SIZE);

        for (int i = 1; i < argc; i++) { // skip program name
                char *base_name = argv[i];

                char *first_dot = strchr(base_name, '.');
                *first_dot = '\0';

                char *last_slash = strrchr(base_name, '/');
                if (last_slash == NULL)
                        last_slash = strrchr(base_name, '\\');
                base_name = last_slash + 1;

                char input[DEF_STR_SIZE];
                snprintf(input, DEF_STR_SIZE, "%s.hbs", argv[i]);

                char output[DEF_STR_SIZE];
                snprintf(output, DEF_STR_SIZE, "%s.h", argv[i]);

                printf("Generating '%s' from '%s' ... ", output, input);
                fflush(stdout);

                size_t input_size = load_file_to_buf(input, src_buf, SRC_BUF_SIZE);
                replace_chars(src_buf, input_size);
                split_tokens(nodes, src_buf, src_buf + input_size);
                generate_and_output_to(output, base_name, nodes);

                cleanup_nodes_pool(nodes);

                printf("Success!\n");
        }

        destroy_nodes_pool(nodes);
        return EXIT_SUCCESS;
}
