#include "topo.h"
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    const char* in = "input.txt";
    const char* out = "output.txt";

    Graph g;
    int rc = parse_input_file(in, &g);
    if (rc != 0) {
        write_output_err(out);
        return 1;
    }
    int* order = (int*)malloc((size_t)g.n * sizeof(int));
    if (!order) {
        g_free(&g);
        write_output_err(out);
        return 2;
    }
    int ok = topo_sort(&g, order);
    if (ok == 0) write_output_ok(out, &g, order);
    else write_output_err(out);

    free(order);
    g_free(&g);
    return 0;
}
