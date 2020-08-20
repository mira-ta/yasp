#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


void fputshelp(FILE* handle, const char* execname) {
    fputs(
        execname,
        handle
    );
        
    fputs(
        "\n"
        "    -h, --help         Print help info.\n"
        "    -C, --count        Amount of bytes (no default value)\n"
        "    -S, --skip         Byte offset (default: 0)\n"
        "    -B, --block-size   Block size in bytes (default: 1)\n",
        handle
    );
}


struct arg_profile {
    size_t count;
    size_t skip;
    size_t block_size;
};


#define _ARG_TEST(name, short_form, long_form) \
    char _arg_test_##name(const char* arg) { return !strcmp(arg, short_form) || !strcmp(arg, long_form); }

#define ARG_TEST(name, value) \
    _arg_test_##name(value)

_ARG_TEST(count, "-C", "--count")
_ARG_TEST(skip, "-S", "--skip")
_ARG_TEST(block_size, "-B", "--block-size")

#undef _ARG_TEST


int arg_branches(struct arg_profile* profile, const int argc, const char* const* const argv) {
    if (argc < 2) {
        fputshelp(stderr, argv[0]);
        return 0x1;
    }

    for (int argument = 1; argument < argc; argument++) {
        const char* arg = argv[argument];

        if (ARG_TEST(count, arg))
            profile->count = atoi(argv[++argument]);
        else if (ARG_TEST(skip, arg))
            profile->skip = atoi(argv[++argument]);
        else if (ARG_TEST(block_size, arg))
            profile->block_size = atoi(argv[++argument]);
        else {
            fputshelp(stderr, argv[0]);
            return 0x1;
        }
    }

    return 0x0;
}


int go(struct arg_profile* profile) {
    void* buffer = malloc(profile->block_size); // no need for zeroing the buffer.

    // `fseek`/`lseek` does not work on pipes, FIFO and sockets.
    for (; profile->skip; profile->skip--) {
        int read_bytes_amount = fread(buffer, 1, profile->block_size, stdin);
        
        if (read_bytes_amount != profile->block_size) {
            int filestream_error = ferror(stdin);

            if (filestream_error)
                return filestream_error | 0x10000;
            else
                return 0x1;
        }
    }

    for (; profile->count; profile->count--) {
        size_t read_bytes_amount = fread(buffer, 1, profile->block_size, stdin);

        if (!read_bytes_amount)
            return 0x2;

        else if (read_bytes_amount != profile->block_size) {
            int filestream_error = ferror(stdin);

            if (filestream_error)
                return filestream_error | 0x1000;
            else
                profile->count = 0; // we have reached the end of the file
        }

        fwrite(buffer, 1, read_bytes_amount, stdout);
        fflush(stdout);
    }
    
    return 0x0;
}


int main(const int argc, const char* const* const argv) {
    struct arg_profile profile = {
        .count = 0,
        .skip = 0,
        .block_size = 1
    };

    int arg_branches_result = arg_branches(&profile, argc, argv);

    if (arg_branches_result)
        return arg_branches_result;

    // fprintf(stderr, "profile.count: %llu\nprofile.skip: %llu\nprofile.block_size: %llu\n", profile.count, profile.skip, profile.block_size);

    int go_result = go(&profile);

    if (go_result & 0x1000)
        fputs("File stream error occured.\n", stderr);
    else if (go_result & 0x10000)
        fputs("Skipping error occured (maybe, stream has reached end-of-file earlier than it has been supposed?).\n", stderr);
    else if (go_result)
        fprintf(stderr, "Error occured with code %u.\n", go_result);
    
    return go_result;
}

