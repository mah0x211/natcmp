#include "../src/natcmp.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// テストユーティリティ関数とマクロ
static int total_tests  = 0;
static int passed_tests = 0;

// Standard comparison callback for non-digit portions
static int strcmp_cb(const unsigned char *a, const unsigned char *b,
                     unsigned char **end_a, unsigned char **end_b)
{
    const unsigned char *p = a;
    const unsigned char *q = b;

    while (*p && *q && !isdigit(*p) && !isdigit(*q) && *p == *q) {
        p++;
        q++;
    }

    *end_a = (unsigned char *)p;
    *end_b = (unsigned char *)q;

    if (isdigit(*p) || isdigit(*q)) {
        return 0; // Reached a digit
    }

    return (*p == *q) ? 0 : (*p < *q ? -1 : 1);
}

#define TEST_SECTION(name) printf("\n[%s]\n", name)

// 拡張アサートマクロ - コールバック関数を指定可能
#define assert_natcmp_ex(cmpfn, a, b, op, expected)                            \
    do {                                                                       \
        total_tests++;                                                         \
        int actual = natcmp((const unsigned char *)(a),                        \
                            (const unsigned char *)(b), cmpfn);                \
        if (actual op expected) {                                              \
            passed_tests++;                                                    \
            printf("    PASS: natcmp(\"%s\", \"%s\", %s) %s %d\n", a, b,       \
                   #cmpfn, #op, expected);                                     \
        } else {                                                               \
            printf("    FAIL: natcmp(\"%s\", \"%s\", %s) = %d %s %d\n", a, b,  \
                   #cmpfn, actual, #op, expected);                             \
            assert(actual op expected);                                        \
        }                                                                      \
    } while (0)

// 既存のマクロ定義を拡張マクロを使用して再定義
#define assert_natcmp(a, b, op, expected)                                      \
    assert_natcmp_ex(strcmp_cb, a, b, op, expected)

// 組み込み関数用のマクロ
#define assert_natcmp_builtin(a, b, op, expected)                              \
    assert_natcmp_ex(natcmp_nondigit_cmp_ascii, a, b, op, expected)

// NULLコールバック用のマクロを追加
#define assert_natcmp_null(a, b, op, expected)                                 \
    assert_natcmp_ex(NULL, a, b, op, expected)

// 便宜的なマクロ（カスタムコールバック用）
#define assert_natcmp_eq(a, b) assert_natcmp(a, b, ==, 0)
#define assert_natcmp_lt(a, b) assert_natcmp(a, b, <, 0)
#define assert_natcmp_gt(a, b) assert_natcmp(a, b, >, 0)

// 便宜的なマクロ（組み込みコールバック用）
#define assert_natcmp_builtin_eq(a, b) assert_natcmp_builtin(a, b, ==, 0)
#define assert_natcmp_builtin_lt(a, b) assert_natcmp_builtin(a, b, <, 0)
#define assert_natcmp_builtin_gt(a, b) assert_natcmp_builtin(a, b, >, 0)

// 便宜的なマクロ（NULLコールバック用）
#define assert_natcmp_null_eq(a, b) assert_natcmp_null(a, b, ==, 0)
#define assert_natcmp_null_lt(a, b) assert_natcmp_null(a, b, <, 0)
#define assert_natcmp_null_gt(a, b) assert_natcmp_null(a, b, >, 0)

// Test basic comparisons with custom callback
static void test_basic_comparison(void)
{
    TEST_SECTION("Basic String Comparison (Custom Callback)");

    printf("  Equal strings:\n");
    assert_natcmp_eq("abc", "abc");

    printf("\n  Regular string comparison (no numbers):\n");
    assert_natcmp_lt("abc", "abd");
    assert_natcmp_gt("abd", "abc");
}

// Test numeric comparisons with custom callback
static void test_numeric_comparison(void)
{
    TEST_SECTION("Numeric String Comparison (Custom Callback)");

    printf("  Basic number comparison:\n");
    assert_natcmp_lt("2", "10");
    assert_natcmp_gt("10", "2");

    printf("\n  Mixed string and number comparison:\n");
    assert_natcmp_lt("file2.txt", "file10.txt");
    assert_natcmp_gt("file10.txt", "file2.txt");
}

// Test using the built-in natcmp_nondigit_cmp_ascii function
static void test_builtin_function(void)
{
    TEST_SECTION("Using natcmp_nondigit_cmp_ascii");

    printf("  Basic case-insensitive comparison:\n");
    assert_natcmp_builtin_eq("abc", "ABC");
    assert_natcmp_builtin_eq("ABC", "abc");
    assert_natcmp_builtin_lt("abc", "ABD");
    assert_natcmp_builtin_gt("ABD", "abc");

    printf("\n  Mixed case with numbers:\n");
    assert_natcmp_builtin_lt("File2.txt", "file10.txt");
    assert_natcmp_builtin_gt("file10.TXT", "File2.txt");
    assert_natcmp_builtin_eq("file2.TXT", "FILE2.txt");

    printf("\n  Numbers with different cases in prefix:\n");
    assert_natcmp_builtin_eq("a10", "A10");
    assert_natcmp_builtin_lt("a10", "B10");
    assert_natcmp_builtin_gt("C10", "b10");
}

// Compare behavior between custom and built-in callbacks
static void test_callback_comparison(void)
{
    TEST_SECTION("Callback Comparison (Case Sensitivity)");

    printf("  Custom callback (case-sensitive):\n");
    assert_natcmp("abc", "ABC", !=, 0); // Custom callback is case-sensitive

    printf("\n  Built-in callback (case-insensitive):\n");
    assert_natcmp_builtin("abc", "ABC", ==, 0); // Built-in is case-insensitive

    printf("\n  Mixed case with numbers:\n");
    // These two will give different results due to case sensitivity
    int res_custom = natcmp((const unsigned char *)"File10.txt",
                            (const unsigned char *)"file2.txt", strcmp_cb);
    int res_builtin =
        natcmp((const unsigned char *)"File10.txt",
               (const unsigned char *)"file2.txt", natcmp_nondigit_cmp_ascii);

    printf("    Custom callback: natcmp(\"File10.txt\", \"file2.txt\") = %d\n",
           res_custom);
    printf(
        "    Built-in callback: natcmp(\"File10.txt\", \"file2.txt\") = %d\n",
        res_builtin);
}

// Test remaining basic functionality with both callbacks
static void test_common_cases(void)
{
    TEST_SECTION("Common Test Cases");

    printf("  Empty strings:\n");
    assert_natcmp_eq("", "");
    assert_natcmp_builtin_eq("", "");

    printf("\n  Leading zeros:\n");
    assert_natcmp_lt("file02.txt", "file002.txt");
    assert_natcmp_builtin_lt("file02.txt", "file002.txt");

    printf("\n  Mixed digit and non-digit:\n");
    assert_natcmp_lt("1abc", "abc");
    assert_natcmp_builtin_lt("1abc", "abc");

    printf("\n  String length differences:\n");
    assert_natcmp_lt("abc", "abcd");
    assert_natcmp_builtin_lt("abc", "abcd");

    printf("\n  Different number values with same digit count:\n");
    // カバレッジ: strncmp(an.digits, bn.digits, an.len)が0以外の場合
    assert_natcmp_lt("file123.txt", "file456.txt");
    assert_natcmp_gt("file456.txt", "file123.txt");
    assert_natcmp_builtin_lt("file123.txt", "file456.txt");
    assert_natcmp_builtin_gt("file456.txt", "file123.txt");

    printf("\n  String prefix relationships:\n");
    // カバレッジ: 文字列の長さ比較（*a, *bの条件分岐）
    assert_natcmp_lt("abc", "abc123"); // aがbの接頭辞
    assert_natcmp_gt("abc123", "abc"); // bがaの接頭辞
    assert_natcmp_builtin_lt("abc", "abc123");
    assert_natcmp_builtin_gt("abc123", "abc");

    // 数字部分で終わるパターンも追加
    printf("\n  Strings ending with numbers:\n");
    assert_natcmp_lt("abc123", "abc123xyz");
    assert_natcmp_gt("abc123xyz", "abc123");
    assert_natcmp_builtin_lt("abc123", "abc123xyz");
    assert_natcmp_builtin_gt("abc123xyz", "abc123");
}

// 文字列長の違いに特化したテストを追加
static void test_string_length_edge_cases(void)
{
    TEST_SECTION("String Length Edge Cases");

    printf("  Empty string vs. non-empty:\n");
    assert_natcmp_lt("", "a");
    assert_natcmp_gt("a", "");
    assert_natcmp_builtin_lt("", "a");
    assert_natcmp_builtin_gt("a", "");

    printf("\n  String with number vs. same prefix:\n");
    assert_natcmp_lt("file", "file1");
    assert_natcmp_gt("file1", "file");
    assert_natcmp_builtin_lt("file", "file1");
    assert_natcmp_builtin_gt("file1", "file");

    printf("\n  Prefix of number strings:\n");
    assert_natcmp_lt("file1", "file12");
    assert_natcmp_gt("file12", "file1");
    assert_natcmp_builtin_lt("file1", "file12");
    assert_natcmp_builtin_gt("file12", "file1");
}

// Test NULL callback (should use natcmp_nondigit_cmp_ascii)
static void test_null_callback(void)
{
    TEST_SECTION("NULL Callback (Default to natcmp_nondigit_cmp_ascii)");

    printf("  Basic case-insensitive comparison:\n");
    assert_natcmp_null_eq("abc", "ABC"); // Case-insensitive like built-in
    assert_natcmp_null_eq("ABC", "abc");
    assert_natcmp_null_lt("abc", "ABD");
    assert_natcmp_null_gt("ABD", "abc");

    printf("\n  Compare with explicit built-in function:\n");
    // These should give identical results
    int res_null = natcmp((const unsigned char *)"File10.txt",
                          (const unsigned char *)"file2.txt", NULL);
    int res_builtin =
        natcmp((const unsigned char *)"File10.txt",
               (const unsigned char *)"file2.txt", natcmp_nondigit_cmp_ascii);

    printf(
        "    NULL callback: natcmp(\"File10.txt\", \"file2.txt\", NULL) = %d\n",
        res_null);
    printf("    Built-in callback: natcmp(\"File10.txt\", \"file2.txt\", "
           "natcmp_nondigit_cmp_ascii) = %d\n",
           res_builtin);

    // Results should be identical
    total_tests++;
    if (res_null == res_builtin) {
        passed_tests++;
        printf("    PASS: NULL callback equals built-in callback\n");
    } else {
        printf("    FAIL: NULL callback (%d) differs from built-in callback "
               "(%d)\n",
               res_null, res_builtin);
        assert(res_null == res_builtin);
    }

    printf("\n  Numeric comparison with NULL callback:\n");
    assert_natcmp_null_lt("file2.txt", "file10.txt");
    assert_natcmp_null_gt("file10.txt", "file2.txt");

    printf("\n  Leading zeros with NULL callback:\n");
    assert_natcmp_null_lt("file02.txt", "file002.txt");
    assert_natcmp_null_gt("file002.txt", "file02.txt");
}

int main(void)
{
    printf("=== NATCMP TEST SUITE ===\n");

    // Run all test categories
    test_basic_comparison();
    test_numeric_comparison();
    test_builtin_function();
    test_callback_comparison();
    test_common_cases();
    test_string_length_edge_cases();
    test_null_callback(); // 追加

    // Summary
    printf("\n=== TEST SUMMARY ===\n");
    printf("Total tests: %d\n", total_tests);

    if (passed_tests == total_tests) {
        printf("All tests passed successfully! (%d/%d)\n", passed_tests,
               total_tests);
    } else {
        printf("Passed: %d/%d\n", passed_tests, total_tests);
    }

    return 0;
}
