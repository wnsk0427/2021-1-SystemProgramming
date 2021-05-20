#include <stdio.h>
#include <stdlib.h>

typedef unsigned char* pointer;

// Print bit representation of the given data
// This code was implemented in our lecture
void print_bit(pointer a, int len) {
    for (int i = 0; i < len; ++i) {
        for (int j = 7; j >= 0; --j) {
            printf("%d", (a[i] >> j) & 0x01);
        }
        printf(" ");
    }
    printf("\n");
}

// Problem 1
void reverse_bit(pointer a, int len) {
    // Implement here
}

// Problem 2
void inverse_bit(pointer a, int len) {
    // Implement here
}

// Problem 3
void split_bit(pointer a, pointer out1, pointer out2, int len) {
    // Implement here
}

// Problem 4
unsigned short partial_mul(unsigned short a, unsigned short b) {
    // Implement here
    return 0; // modify this
}

// Problem 5
void get_date(unsigned int date, int* pYear, int* pMonth, int* pDay) {
    // Implement here
    *pYear = 0; // modify this
    *pMonth = 0; // modify this
    *pDay = 0; // modify this
}

int main() {
    printf("Problem 1\n");
    unsigned int v1 = 0x1234CDEF;
    print_bit((pointer)&v1, sizeof(v1));
    reverse_bit((pointer)&v1, sizeof(v1));
    print_bit((pointer)&v1, sizeof(v1));


    printf("Problem 2\n");
    unsigned int v2 = 0x1234CDEF;
    print_bit((pointer)&v2, sizeof(v2));
    inverse_bit((pointer)&v2, sizeof(v2));
    print_bit((pointer)&v2, sizeof(v2));


    printf("Problem 3\n");
    unsigned int v3 = 0x1234CDEF;
    unsigned short out1 = 0, out2 = 0;
    print_bit((pointer)&v3, sizeof(v3));
    split_bit((pointer)&v3, (pointer)&out1, (pointer)&out2, sizeof(v3));
    print_bit((pointer)&out1, sizeof(out1));
    print_bit((pointer)&out2, sizeof(out2));


    printf("Problem 4\n");
    unsigned short v4 = 0xF0BD;
    print_bit((pointer)&v4, sizeof(v4));
    unsigned short v4_out = partial_mul(v4, v4);
    print_bit((pointer)&v4_out, sizeof(v4_out));


    printf("Problem 5\n");
    unsigned int date = 1034867;
    int year, month, day;
    get_date(date, &year, &month, &day);
    printf("%d -> %d/%d/%d\n", date, year, month, day);

    return 0;
}