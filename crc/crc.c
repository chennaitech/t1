#include <stdio.h>
#include <string.h>

// Generator polynomial (CRC-4: x^4 + x + 1 => "10011")
#define GEN_POLY "10011"

// Perform XOR division
void xor_divide(char *data, const char *poly) {
    int poly_len = strlen(poly);
    int data_len = strlen(data);

    for (int i = 0; i <= data_len - poly_len; i++) {
        if (data[i] == '1') {
            for (int j = 0; j < poly_len; j++) {
                data[i + j] = (data[i + j] == poly[j]) ? '0' : '1';
            }
        }
    }
}

int main() {
    char data[64] = "1101011011";
    char codeword[128];
    const char *poly = GEN_POLY;
    int poly_len = strlen(poly);

    printf("--- CRC Encoder (Sender) ---\n");
    printf("Original Binary Data : %s\n", data);
    printf("Generator Polynomial : %s\n", poly);

    // 1. Append (poly_len - 1) zeros
    strcpy(codeword, data);
    for (int i = 0; i < poly_len - 1; i++) {
        strcat(codeword, "0");
    }

    // 2. Perform XOR division to calculate CRC bits
    char temp[128];
    strcpy(temp, codeword);
    xor_divide(temp, poly);

    char remainder[16];
    strncpy(remainder, temp + strlen(data), poly_len - 1);
    remainder[poly_len - 1] = '\0';
    printf("Calculated CRC Bits  : %s\n", remainder);

    // 3. Transmitted Codeword = Original Data + CRC Bits
    strcpy(codeword, data);
    strcat(codeword, remainder);
    printf("Transmitted Codeword : %s\n", codeword);

    printf("\n--- CRC Checker (Receiver) ---\n");
    strcpy(temp, codeword);
    xor_divide(temp, poly);

    int error = 0;
    for (size_t i = strlen(data); i < strlen(codeword); i++) {
        if (temp[i] != '0') {
            error = 1;
            break;
        }
    }

    if (!error) {
        printf("[Test 1 - Valid] Remainder is ZERO. No error detected!\n");
    } else {
        printf("[Test 1 - Valid] Error detected!\n");
    }

    // Inject 1-bit error
    codeword[2] = (codeword[2] == '0') ? '1' : '0';
    printf("\nInjecting Bit Flip at index 2: %s\n", codeword);
    strcpy(temp, codeword);
    xor_divide(temp, poly);

    error = 0;
    for (size_t i = strlen(data); i < strlen(codeword); i++) {
        if (temp[i] != '0') {
            error = 1;
            break;
        }
    }

    if (error) {
        printf("[Test 2 - Corrupted] Remainder is NON-ZERO: ERROR DETECTED by CRC!\n");
    } else {
        printf("[Test 2 - Corrupted] Error escaped detection!\n");
    }

    return 0;
}