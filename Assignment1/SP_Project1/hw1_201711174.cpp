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
    unsigned int vA = 0;
    pointer pA = (pointer)&vA;

    for (int i = 0; i < len; ++i) {
        for (int j = 7; j >= 0; --j) {
            pA[i] <<= 1;
            pA[i] |= ((a[i] << j) >> 7) & 0x01;
        }
    }

    for (int i = 0; i < len; ++i) {
        a[i] = pA[len - i - 1];
    }
}

// Problem 2
void inverse_bit(pointer a, int len) {
    // Implement here
    for (int i = 0; i < len; ++i) {
        a[i] = ~a[i];
    }
}

// Problem 3
void split_bit(pointer a, pointer out1, pointer out2, int len) {
    // Implement here
    for (int i = 0; i < len; ++i) {
        for (int j = 7; j >= 0; --j) {
            if (j % 2 == 1) {
                out1[i / 2] <<= 1;
                out1[i / 2] |= (a[i] >> j) & 0x01;
            }
            else {
                out2[i / 2] <<= 1;
                out2[i / 2] |= (a[i] >> j) & 0x01;
            }
        }
    }
}

// Problem 4
unsigned short partial_mul(unsigned short a, unsigned short b) {
    // Implement here
    pointer pA = (pointer)&a, pB = (pointer)&b;

    unsigned short vTA = 0, vTB = 0;
    pointer pTA = (pointer)&vTA, pTB = (pointer)&vTB;

    for (int j = 5; j >= 0; --j) {
        *pTA <<= 1;
        *pTA |= (((pA[0] << 2) >> 2) >> j) & 0x01;
    }

    for (int j = 6; j >= 0; --j) {
        *pTB <<= 1;
        *pTB |= (((pB[1] << 1) >> 1) >> j) & 0x01;
    }

    return vTA * vTB; // modify this
}

// Problem 5
void get_date(unsigned int date, int* pYear, int* pMonth, int* pDay) {
    // Implement here
    pointer datep = (pointer)&date;

    *pYear = 0; // modify this
    *pMonth = 0; // modify this
    *pDay = 0; // modify this

    for (int j = 4; j >= 0; --j) { // 5 bit
        *pDay <<= 1;
        *pDay |= (datep[j / 8] >> (j % 8)) & 0x01;
    }

    for (int j = 3; j >= 0; --j) { // 4 bit
        *pMonth <<= 1;
        *pMonth |= (datep[(j + 5) / 8] >> ((j + 5) % 8)) & 0x01;
    }

    for (int j = 22; j >= 0; --j) { // 23 bit
        *pYear <<= 1;
        *pYear |= (datep[(j + 5 + 4) / 8] >> ((j + 5 + 4) % 8)) & 0x01;
    }
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