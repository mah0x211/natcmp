# natcmp

[![test](https://github.com/mah0x211/natcmp/actions/workflows/test.yml/badge.svg)](https://github.com/mah0x211/natcmp/actions/workflows/test.yml)
[![codecov](https://codecov.io/gh/mah0x211/natcmp/branch/master/graph/badge.svg)](https://codecov.io/gh/mah0x211/natcmp)


A header-only C library for natural order comparison of strings.


## Overview

Natural order comparison sorts strings containing numbers in a way that better matches human expectations. For example:

```
Standard string comparison: "file1.txt", "file10.txt", "file2.txt"
Natural order comparison:   "file1.txt", "file2.txt", "file10.txt"
```

## Features

- Header-only implementation (no compilation required)
- Customizable non-digit comparison through callback function
- Handles numeric portions as actual numbers
- When numbers are equal, sorts by number of digits (fewer digits first)
- MIT licensed


## Installation

Simply copy the `natcmp.h` file to your project directory, and include it in your source files.

```c
#include "natcmp.h"
```


## Usage

```c
#include <stdio.h>
#include <string.h>
#include "natcmp.h"

// Example callback function for comparing non-digit portions
static int strcmp_cb(const unsigned char *a, const unsigned char *b,
                 unsigned char **end_a, unsigned char **end_b) {
    const unsigned char *p = a;
    const unsigned char *q = b;
    
    // compare leading non-digit characters
    while (*p && *q && !isdigit(*p) && !isdigit(*q) && *p == *q) {
        p++;
        q++;
    }

    // set the end pointers to the current position
    *end_a = (unsigned char *)p;
    *end_b = (unsigned char *)q;
    
    // compare the non-digit portions
    // if they are equal, return 0
    // if not, return -1 or 1 based on the comparison
    return (*p == *q) ? 0 : (*p < *q ? -1 : 1);
}


int main() {
    const char *s1 = "file2.txt";
    const char *s2 = "file10.txt";
    
    int result = natcmp((const unsigned char *)s1, (const unsigned char *)s2, strcmp_cb);
    
    if (result < 0) {
        printf("%s comes before %s\n", s1, s2);
    } else if (result > 0) {
        printf("%s comes before %s\n", s2, s1);
    } else {
        printf("%s and %s are equal\n", s1, s2);
    }
    
    return 0;
}
```

## API

### Callback Function Type

```c
typedef int (*natcmp_nondigit_cmp_func_t)(const unsigned char *a,
                                          const unsigned char *b,
                                          unsigned char **end_a,
                                          unsigned char **end_b);
```

This callback type is used for comparing non-digit portions of strings.

**Parameters:**

- `a`: First string to compare
- `b`: Second string to compare
- `end_a`: Pointer to store the end position of comparison in string `a`
- `end_b`: Pointer to store the end position of comparison in string `b`

**Return value:**

- `negative`: String `a` is less than string `b`
- `0`: Strings are equal
- `positive`: String `a` is greater than string `b`


### Natural Comparison Function

```c
int natcmp(const unsigned char *a, const unsigned char *b,
           natcmp_nondigit_cmp_func_t compare);
```

**Parameters:**

- `a`: First string to compare
- `b`: Second string to compare
- `compare`: Callback function for comparing non-digit portions

If `compare` is NULL, the built-in `natcmp_nondigit_cmp_ascii` function is used,
which performs a case-insensitive comparison of non-digit portions.

**Return value:**

- `-1`: String `a` is less than string `b`
- `0`: Strings are equal
- `1`: String `a` is greater than string `b`


### Built-in Callback Function

```c
int natcmp_nondigit_cmp_ascii(const unsigned char *a,
                              const unsigned char *b,
                              unsigned char **end_a,
                              unsigned char **end_b);
```

This is a built-in callback function that performs case-insensitive ASCII comparison of non-digit portions of strings.

**Features:**

- Case-insensitive comparison (e.g., `"abc"` and `"ABC"` are considered equal)
- Scans strings until reaching a digit or end of string
- When non-digit portions differ, returns the comparison result
- When lengths differ but content is equal, shorter string is considered less

**Parameters:**

- `a`: First string to compare
- `b`: Second string to compare
- `end_a`: Pointer to store the position of first digit or end in string `a`
- `end_b`: Pointer to store the position of first digit or end in string `b`

**Return value:**

- `-1`: Non-digit portion of `a` is less than `b`
- `0`: Non-digit portions are equal
- `1`: Non-digit portion of `a` is greater than `b`

**Example:**

```c
#include <stdio.h>
#include "natcmp.h"

int main() {
    const char *s1 = "File2.txt";
    const char *s2 = "file10.txt";
    
    // Using the built-in callback for case-insensitive comparison
    int result = natcmp(
        (const unsigned char *)s1, 
        (const unsigned char *)s2, 
        natcmp_nondigit_cmp_ascii
    );
    
    if (result < 0) {
        printf("%s comes before %s\n", s1, s2);
    } else if (result > 0) {
        printf("%s comes before %s\n", s2, s1);
    }
    
    return 0;
}

// Output: "File2.txt comes before file10.txt"
```


## License

MIT License - Copyright (C) 2025 Masatoshi Fukunaga

