#include <stdio.h>

int main(int argc, char** argv) {
    printf("\nConvert TAB symbols to 2 space symbols\nAuthor: Vladimir Tatarinov, 2020\n");
    if (argc != 3) {
        printf("Oops! Specify the files please.\nUsage: %s [input] [output]\n", argv[0]);
        return 1;
    }
    FILE* in = fopen(argv[1], "rb");
    if (in == NULL) {
        printf("Can't open input file\n");
        return 2;
    }
    fseek(in, 0, SEEK_END);
    long long size = ftell(in);
    fseek(in, 0, SEEK_SET);
    FILE* out = fopen(argv[2], "wb");
    if (out == NULL) {
        printf("Can't open output file\n");
        return 2;
    }
    for (long long i = 0; i < size; ++i) {
        char c = getc(in);
        if (c == '\t') {
            for (int i =0; i < 2; ++i)
                fputc(' ', out);
        }

        else
            fputc(c, out);
    }
    fclose(in);
    fclose(out);
}
