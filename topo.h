#pragma once

#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int* to;
    int size;
    int cap;
}
EdgeList;

typedef struct {
    int n;
    char** names;
    EdgeList* adj;
    int* indeg;
}
Graph;

int g_init(Graph* g, int n);
void g_free(Graph* g);

int g_index_of(const Graph* g,
    const char* name);

int g_add_name(Graph* g, int idx,
    const char* name);

int g_add_edge(Graph* g, int u, int v);

int parse_input_file(const char* inpath, Graph* g);

int topo_sort(const Graph* g, int* out_idx);

int write_output_ok(const char* outpath,
    const Graph* g,
    const int* order);

int write_output_err(const char* outpath);
