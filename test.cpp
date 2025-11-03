#include <gtest/gtest.h>
extern "C" {
#include "topo.h"
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void write_file(const char* path,
    const char* content) {
    FILE* f = fopen(path, "wb");
    ASSERT_NE(f, nullptr);
    fwrite(content, 1, strlen(content), f);
    fclose(f);
}

TEST(TopoTest, ParseBasic_no1) {
    write_file("input.txt",
        "a b c\n"
        "a b\n"
        "b c\n"
    );
    Graph g;
    ASSERT_EQ(0, parse_input_file("input.txt", &g));
    ASSERT_EQ(g.n, 3);
    EXPECT_STREQ(g.names[0], "a");
    EXPECT_STREQ(g.names[1], "b");
    EXPECT_STREQ(g.names[2], "c");
    g_free(&g);
}

TEST(TopoTest, LinearOrder_no2) {
    write_file("input.txt",
        "a b c\n"
        "a b\n"
        "b c\n"
    );
    Graph g;
    ASSERT_EQ(0, parse_input_file("input.txt", &g));
    int* ord = (int*)malloc(sizeof(int) * g.n);
    ASSERT_NE(ord, nullptr);
    EXPECT_EQ(0, topo_sort(&g, ord));

    int ia = -1, ib = -1, ic = -1;
    for (int i = 0; i < g.n; i++) {
        if (strcmp(g.names[ord[i]], "a") == 0) ia = i;
        if (strcmp(g.names[ord[i]], "b") == 0) ib = i;
        if (strcmp(g.names[ord[i]], "c") == 0) ic = i;
    }
    EXPECT_LT(ia, ib);
    EXPECT_LT(ib, ic);

    free(ord);
    g_free(&g);
}

TEST(TopoTest, Cycle_no3) {
    write_file("input.txt",
        "x y\n"
        "x y\n"
        "y x\n"
    );
    Graph g;
    ASSERT_EQ(0, parse_input_file("input.txt", &g));
    int* ord = (int*)malloc(sizeof(int) * g.n);
    ASSERT_NE(ord, nullptr);
    EXPECT_EQ(1, topo_sort(&g, ord));
    free(ord);
    g_free(&g);
}

TEST(TopoTest, TrailingEmpty_no4) {
    write_file("input.txt",
        "k m n\n"
        "k m\n"
        "\n"
    );
    Graph g;
    ASSERT_EQ(0, parse_input_file("input.txt", &g));
    int* ord = (int*)malloc(sizeof(int) * g.n);
    ASSERT_NE(ord, nullptr);
    EXPECT_EQ(0, topo_sort(&g, ord));
    free(ord);
    g_free(&g);
}

TEST(TopoTest, UnknownWord_no5) {
    write_file("input.txt",
        "a b\n"
        "a c\n" 
    );
    Graph g;
    EXPECT_EQ(-3, parse_input_file("input.txt", &g));
}

TEST(TopoTest, ExtraTokens_no6) {
    write_file("input.txt",
        "a b\n"
        "a b c\n"
    );
    Graph g;
    EXPECT_EQ(-1, parse_input_file("input.txt", &g));
}

TEST(TopoTest, NoEdges_no7) {
    write_file("input.txt", "aa bb cc dd\n");
    Graph g;
    ASSERT_EQ(0, parse_input_file("input.txt", &g));
    int* ord = (int*)malloc(sizeof(int) * g.n);
    ASSERT_NE(ord, nullptr);
    EXPECT_EQ(0, topo_sort(&g, ord));
    free(ord);
    g_free(&g);
}

TEST(TopoTest, LongWords_no8) {
    char L[201];
    memset(L, 'x', 200);
    L[200] = '\0';
    char buf[600];
    snprintf(buf, sizeof(buf), "%s y\n%s y\n", L, L);
    write_file("input.txt", buf);

    Graph g;
    ASSERT_EQ(0, parse_input_file("input.txt", &g));
    int* ord = (int*)malloc(sizeof(int) * g.n);
    ASSERT_NE(ord, nullptr);
    EXPECT_EQ(0, topo_sort(&g, ord));
    free(ord);
    g_free(&g);
}

TEST(TopoTest, DuplicateEdges_no9) {
    write_file("input.txt",
        "u v w\n"
        "u v\n"
        "u v\n" 
        "v w\n"
    );
    Graph g;
    ASSERT_EQ(0, parse_input_file("input.txt", &g));
    int* ord = (int*)malloc(sizeof(int) * g.n);
    ASSERT_NE(ord, nullptr);
    EXPECT_EQ(0, topo_sort(&g, ord));

    int iu = -1, iv = -1, iw = -1;
    for (int i = 0; i < g.n; i++) {
        if (strcmp(g.names[ord[i]], "u") == 0) iu = i;
        if (strcmp(g.names[ord[i]], "v") == 0) iv = i;
        if (strcmp(g.names[ord[i]], "w") == 0) iw = i;
    }
    EXPECT_LT(iu, iv);

    Беата, [03.11 .2025 22:12]
        EXPECT_LT(iv, iw);

    free(ord);
    g_free(&g);
}

TEST(TopoTest, WriteOK_no10) {
    write_file("input.txt",
        "a b\n"
        "a b\n"
    );
    Graph g;
    ASSERT_EQ(0, parse_input_file("input.txt", &g));
    int* ord = (int*)malloc(sizeof(int) * g.n);
    ASSERT_NE(ord, nullptr);
    ASSERT_EQ(0, topo_sort(&g, ord));

    ASSERT_EQ(0, write_output_ok("output.txt", &g, ord));

    FILE* f = fopen("output.txt", "rb");
    ASSERT_NE(f, nullptr);
    char out[128] = {
      0
    };
    fread(out, 1, sizeof(out) - 1, f);
    fclose(f);

    char* pa = strstr(out, "a");
    char* pb = strstr(out, "b");
    ASSERT_NE(pa, nullptr);
    ASSERT_NE(pb, nullptr);
    EXPECT_LT(pa, pb);

    free(ord);
    g_free(&g);
}

TEST(TopoTest, WriteErr_no11) {
    ASSERT_EQ(0, write_output_err("output.txt"));
    FILE* f = fopen("output.txt", "rb");
    ASSERT_NE(f, nullptr);
    char out[32] = {
      0
    };
    fread(out, 1, sizeof(out) - 1, f);
    fclose(f);
    EXPECT_STREQ(out, "[Error]\n");
}

TEST(TopoTest, LongChain_no12) {
    char header[4096] = {
      0
    };
    size_t off = 0;
    for (int i = 0; i < 100; i++) {
        char name[16];
        snprintf(name, sizeof(name), "v%d", i);
        size_t len = strlen(name);
        if (off + len + 2 < sizeof(header)) {
            memcpy(header + off, name, len);
            off += len;
            header[off++] = (i == 99 ? '\n' : ' ');
            header[off] = '\0';
        }
    }
    FILE* f = fopen("input.txt", "wb");
    ASSERT_NE(f, nullptr);
    fwrite(header, 1, strlen(header), f);
    for (int i = 0; i < 99; i++) {
        char line[32];
        snprintf(line, sizeof(line), "v%d v%d\n", i, i + 1);
        fwrite(line, 1, strlen(line), f);
    }
    fclose(f);

    Graph g;
    ASSERT_EQ(0, parse_input_file("input.txt", &g));
    int* ord = (int*)malloc(sizeof(int) * g.n);
    ASSERT_NE(ord, nullptr);
    ASSERT_EQ(0, topo_sort(&g, ord));

    int p0 = -1, p99 = -1;
    for (int i = 0; i < g.n; i++) {
        if (strcmp(g.names[ord[i]], "v0") == 0) p0 = i;
        if (strcmp(g.names[ord[i]], "v99") == 0) p99 = i;
    }
    EXPECT_LT(p0, p99);

    free(ord);
    g_free(&g);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
