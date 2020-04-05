#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

enum node_type {
        LITERAL,
        VARIABLE
};

struct node {
        enum node_type type;
        const char *value;
};

#define NODES_BUF_SIZE 256
struct node nodes[NODES_BUF_SIZE];
struct node *nodes_pos = nodes;

struct node *new_node(
        enum node_type type,
        const char *value
) {
        struct node *ptr = nodes_pos;
        ptr->type = type;
        ptr->value = value;
        nodes_pos++;
        return ptr;
}

struct node *next_node(
        struct node *prev
) {
        struct node *next = prev++;
        return (next >= nodes_pos) ? NULL : next;
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
                size_t diff = beg - dst;
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
        char *buf,
        char *buf_end
) {
        char *sz_loc;
        char *op_brace;
        char *ed_brace;

        while ((op_brace = strstr(buf, "{{")) != NULL) {
                sz_loc = place_in_pool(buf, op_brace - buf);
                cleanup_literal(sz_loc);
                new_node(LITERAL, sz_loc);

                op_brace += 2; // skip "{{"
                ed_brace = strstr(buf, "}}");
                if (ed_brace == NULL) {
                        puts("Closing }} braces not found!");
                        return; // FIXME : think about this case!
                }

                sz_loc = place_in_pool(op_brace, ed_brace - op_brace);
                cleanup_literal(sz_loc);
                new_node(VARIABLE, sz_loc);
                
                buf = ed_brace + 2; // skip "}}"
        }

        if (buf <= buf_end) {
                sz_loc = place_in_pool(buf, buf_end - buf);
                cleanup_literal(sz_loc);
                new_node(LITERAL, sz_loc);
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

void generate_and_output_to(
        const char *filename,
        const char *base_name
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

        size_t vars_count = 0;
        for (struct node *it = nodes; it < nodes_pos; it++)
                if (VARIABLE == it->type)
                        vars_count++;

        if (vars_count > 0) {
                fprintf(file,
                        "struct %s_params {\n",
                        base_name);
                for (struct node *it = nodes; it < nodes_pos; it++) {
                        if (it->type != VARIABLE)
                                continue;
                        fprintf(file,
                                TAB "const char *%s;\n",
                                it->value);
                }
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
        for (struct node *it = nodes; it < nodes_pos; it++) {
                if (it->type == LITERAL) {
                        size_t len = strlen(it->value);
                        fprintf(file,
                                TAB "memcpy(it, \"%s\", %zu);\n" \
                                TAB "it += %zu;\n",
                                it->value, len, len);
                }
                if (it->type == VARIABLE) {
                        fprintf(file,
                                TAB "if (params->%s != NULL) {\n" \
                                TAB TAB "size_t len = strlen(params->%s);\n" \
                                TAB TAB "strcpy(it, params->%s);\n" \
                                TAB TAB "it += len;\n" \
                                TAB "}\n",
                                it->value, it->value, it->value);
                }
        }
        fprintf(file,
                TAB "return it - dst;\n" \
                "}\n" \
                "\n" \
                "#endif\n");
        fclose(file);
}

#define STR_BUF_SIZE 128

int main(
        int argc,
        char **argv
) {
        if (argc < 2) {
                fprintf(stderr, "At least one template base name is required!");
                exit(EXIT_FAILURE);
        }

        for (int i = 1; i < argc; i++) { // skip program name
                char *base_name = argv[i];

                char *first_dot = strchr(base_name, '.');
                *first_dot = '\0';

                char *last_slash = strrchr(base_name, '/');
                if (last_slash == NULL)
                        last_slash = strrchr(base_name, '\\');
                base_name = last_slash + 1;

                char input[STR_BUF_SIZE];
                snprintf(input, STR_BUF_SIZE, "%s.hbs", argv[i]);

                char output[STR_BUF_SIZE];
                snprintf(output, STR_BUF_SIZE, "%s.h", argv[i]);

                printf("Generating '%s' from '%s' ... ", output, input);
                fflush(stdout);

                size_t input_size = load_file_to_buf(input, src_buf, SRC_BUF_SIZE);
                replace_chars(src_buf, input_size);
                split_tokens(src_buf, src_buf + input_size);
                generate_and_output_to(output, base_name);

                nodes_pos = nodes; // discard all nodes used

                printf("Success!\n");
        }
        return EXIT_SUCCESS;
}
