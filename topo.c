#include "topo.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

static char* dup_str(const char* s) {
    if (!s) return NULL;
    size_t n = strlen(s) + 1;
    char* p = (char*)malloc(n);
    if (p) memcpy(p, s, n);
    return p;
}

static void trim(char* s) {
    if (!s) return;
    size_t len = strlen(s);
    while (len && (s[len - 1] == '\n' || s[len - 1] == '\r' || isspace((unsigned char)s[len - 1])))
        s[--len] = '\0';
    size_t i = 0;
    while (s[i] && isspace((unsigned char)s[i])) i++;
    if (i) memmove(s, s + i, strlen(s + i) + 1);
}

int g_init(Graph* g, int n) {
    if (!g || n < 0) return -1;
    g->n = n;
    g->names = (char**)calloc(n, sizeof(char*));
    g->adj = (EdgeList*)calloc(n, sizeof(EdgeList));
    g->indeg = (int*)calloc(n, sizeof(int));
    if (!g->names || !g->adj || !g->indeg) return -1;
    return 0;
}

void g_free(Graph* g) {
    if (!g) return;
    if (g->names) {
        for (int i = 0; i < g->n; i++) free(g->names[i]);
        free(g->names);
    }
    if (g->adj) {
        for (int i = 0; i < g->n; i++) free(g->adj[i].to);
        free(g->adj);
    }
    free(g->indeg);
    g->n = 0;
    g->names = NULL;
    g->adj = NULL;
    g->indeg = NULL;
}

int g_add_name(Graph* g, int idx,
    const char* name) {
    if (!g || idx < 0 || idx >= g->n) return -1;
    if (g->names[idx]) return 0; /* уже есть */
    g->names[idx] = dup_str(name);
    return g->names[idx] ? 0 : -1;
}

int g_index_of(const Graph* g,
    const char* name) {
    if (!g || !name) return -1;
    for (int i = 0; i < g->n; i++) {
        if (g->names[i] && strcmp(g->names[i], name) == 0) return i;
    }
    return -1;
}

int g_add_edge(Graph* g, int u, int v) {
    if (!g || u < 0 || v < 0 || u >= g->n || v >= g->n) return -1;
    EdgeList* el = &g->adj[u];
    if (el->size == el->cap) {
        int newcap = el->cap ? el->cap * 2 : 4;
        int* nw = (int*)realloc(el->to, newcap * sizeof(int));
        if (!nw) return -1;
        el->to = nw;
        el->cap = newcap;
    }
    el->to[el->size++] = v;
    g->indeg[v]++;
    return 0;
}

static int split_words(char* line, char** out, int maxn) {
    int cnt = 0;
    char* p = line;
    while (*p) {
        while (*p && isspace((unsigned char)*p)) p++;
        if (!*p) break;
        char* start = p;
        while (*p && !isspace((unsigned char)*p)) p++;
        if (cnt < maxn) {
            int len = (int)(p - start);
            char* tmp = (char*)malloc((size_t)len + 1);
            if (!tmp) return -2;
            memcpy(tmp, start, (size_t)len);
            tmp[len] = '\0';
            out[cnt++] = tmp;
        }
        else {
            return -2;
        }
    }
    return cnt;
}

int parse_input_file(const char* inpath, Graph* g) {
    if (!inpath || !g) return -1;
    FILE* f = fopen(inpath, "r");
    if (!f) return -1;

    char buf[4096];
    if (!fgets(buf, sizeof(buf), f)) {
        fclose(f);
        return -1;
    }
    trim(buf);
    if (buf[0] == '\0') {
        fclose(f);
        return -1;
    }

    char* tmpnames[2048];
    for (int i = 0; i < 2048; i++) tmpnames[i] = NULL;

    char* copy = dup_str(buf);
    if (!copy) {
        fclose(f);
        return -2;
    }

    int count = 0;

    {
        char* p = copy;
        while (*p) {
            while (*p && isspace((unsigned char)*p)) p++;
            if (!*p) break;
            char* start = p;
            while (*p && !isspace((unsigned char)*p)) p++;
            int len = (int)(p - start);
            if (len <= 0) {
                free(copy);
                fclose(f);
                return -1;
            }
            char* nm = (char*)malloc((size_t)len + 1);
            if (!nm) {
                free(copy);
                fclose(f);
                return -2;
            }
            memcpy(nm, start, (size_t)len);
            nm[len] = '\0';
            if (count >= 2048) {
                free(nm);
                free(copy);
                fclose(f);
                return -1;
            }
            tmpnames[count++] = nm;
        }
    }
    free(copy);

    if (count <= 0) {
        fclose(f);
        return -1;
    }

    if (g_init(g, count) != 0) {
        fclose(f);
        for (int i = 0; i < count; i++) free(tmpnames[i]);
        return -2;
    }

    for (int i = 0; i < count; i++) {
        if (g_add_name(g, i, tmpnames[i]) != 0) {
            for (int j = 0; j < count; j++) free(tmpnames[j]);
            fclose(f);
            g_free(g);
            return -2;
        }
    }
    for (int i = 0; i < count; i++) free(tmpnames[i]);

    while (fgets(buf, sizeof(buf), f)) {
        trim(buf);
        if (buf[0] == '\0') continue;
        char u[1024], v[1024], extra[2];
        int m = sscanf(buf, " %1023s %1023s %1s", u, v, extra);
        if (m < 2 || m > 2) {
            fclose(f);
            g_free(g);
            return -1;
        }
        int iu = g_index_of(g, u);
        int iv = g_index_of(g, v);
        if (iu < 0 || iv < 0) {
            fclose(f);
            g_free(g);
            return -3;
        }
        if (g_add_edge(g, iu, iv) != 0) {
            fclose(f);
            g_free(g);
            return -2;
        }
    }
    fclose(f);
    return 0;
}

int topo_sort(const Graph* g, int* out_idx) {
    if (!g || !out_idx) return 1;
    int n = g->n;
    int* ind = (int*)malloc((size_t)n * sizeof(int));
    if (!ind) return 1;
    for (int i = 0; i < n; i++) ind[i] = g->indeg[i];

    int* q = (int*)malloc((size_t)n * sizeof(int));
    if (!q) {
        free(ind);
        return 1;
    }
    int qs = 0, qe = 0;

    for (int i = 0; i < n; i++)
        if (ind[i] == 0) q[qe++] = i;

    int outc = 0;
    while (qs < qe) {
        int u = q[qs++];
        out_idx[outc++] = u;
        for (int k = 0; k < g->adj[u].size; k++) {
            int v = g->adj[u].to[k];
            if (--ind[v] == 0) q[qe++] = v;
        }
    }
    free(q);
    free(ind);
    return (outc == n) ? 0 : 1;
}

int write_output_ok(const char* outpath,
    const Graph* g,
    const int* order) {
    FILE* o = fopen(outpath, "w");
    if (!o) return -1;
    for (int i = 0; i < g->n; i++) {
        if (i) fputc(' ', o);
        fputs(g->names[order[i]], o);
    }
    fputc('\n', o);
    fclose(o);
    return 0;
}

int write_output_err(const char* outpath) {
    FILE* o = fopen(outpath, "w");
    if (!o) return -1;
    fputs("[Error]\n", o);
    fclose(o);
    return 0;
}
