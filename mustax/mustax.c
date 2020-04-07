#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include "str_pool.h"
#include "node.h"

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
        struct str_pool *strings,
        struct nodes_pool *nodes,
        char *buf,
        char *buf_end
) {
        char *sz_loc;
        char *op_brace;
        char *ed_brace;

        while ((op_brace = strstr(buf, "{{")) != NULL) {
                sz_loc = copy_into_pool(strings, buf, op_brace - buf);
                cleanup_literal(sz_loc);
                emplace_node(nodes, LITERAL, sz_loc);

                op_brace += 2; // skip "{{"
                ed_brace = strstr(buf, "}}");
                if (ed_brace == NULL) {
                        puts("Closing }} braces not found!");
                        return; // FIXME : think about this case!
                }

                sz_loc = copy_into_pool(strings, op_brace, ed_brace - op_brace);
                cleanup_literal(sz_loc);
                emplace_node(nodes, VARIABLE, sz_loc);
                
                buf = ed_brace + 2; // skip "}}"
        }

        if (buf <= buf_end) {
                sz_loc = copy_into_pool(strings, buf, buf_end - buf);
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
        void *state
) {
        struct count_state *c_state = (struct count_state *) state;
        if (VARIABLE == el->type)
                c_state->vars_count++;
}

struct output_state {
        FILE *file;
};

void output_var_into_param(
        struct node *el,
        void *state
) {
        struct output_state *o_state = (struct output_state *) state;
        if (el->type != VARIABLE)
                return;
        fprintf(o_state->file,
                TAB "const char *%s;\n",
                el->value);
}

void output_copy_process(
        struct node *el,
        void *state
) {
        struct output_state *o_state = (struct output_state *) state;
        if (el->type == LITERAL) {
                size_t len = strlen(el->value);
                fprintf(o_state->file,
                        TAB "memcpy(it, \"%s\", %zu);\n" \
                        TAB "it += %zu;\n",
                        el->value, len, len);
        }
        if (el->type == VARIABLE) {
                fprintf(o_state->file,
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

// TEST

void remove_file_ext(
        char *path
) {
        char *dot = strchr(path, '.');
        if (dot != NULL)
                *dot = '\0';
}

// TEST

char *pick_filename(
        char *path
) {
        char *sep = strrchr(path, '/');
        if (sep == NULL)
                sep = strrchr(path, '\\');
        return (sep == NULL) ? path : sep + 1;
}

#define STR_POOL_SIZE 65535
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

        struct str_pool *strings = init_str_pool(STR_POOL_SIZE);
        struct nodes_pool *nodes = init_nodes_pool(NODES_POOL_SIZE);

        for (int i = 1; i < argc; i++) { // skip program name
                char *base_name = argv[i];
                base_name = pick_filename(base_name);
                remove_file_ext(base_name);

                char input[DEF_STR_SIZE];
                snprintf(input, DEF_STR_SIZE, "%s.hbs", argv[i]);

                char output[DEF_STR_SIZE];
                snprintf(output, DEF_STR_SIZE, "%s.h", argv[i]);

                printf("Generating '%s' from '%s' ... ", output, input);
                fflush(stdout);

                size_t input_size = load_file_to_buf(input, src_buf, SRC_BUF_SIZE);
                replace_chars(src_buf, input_size);
                split_tokens(strings, nodes, src_buf, src_buf + input_size);
                generate_and_output_to(output, base_name, nodes);

                cleanup_str_pool(strings);
                cleanup_nodes_pool(nodes);

                printf("Success!\n");
        }

        destroy_nodes_pool(nodes);
        destroy_str_pool(strings);
        return EXIT_SUCCESS;
}
