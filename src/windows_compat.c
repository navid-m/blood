// C-linkage symbols required by Spectre's generated Windows runtime.
// The Blood-side Windows layer lives in src/core/win.sx.

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#if defined(_WIN32)

#include <windows.h>
#include <stdint.h>

int _fltused = 0;

typedef struct {
    uint64_t ino;
    uint32_t mode;
    uint32_t uid;
    int64_t size;
} BxStat;

int sx_hosted_stat(const char *path, void *buf) {
    WIN32_FILE_ATTRIBUTE_DATA fad;
    BxStat *st = (BxStat *)buf;
    if (!path || !buf || !GetFileAttributesExA(path, GetFileExInfoStandard, &fad)) {
        return -1;
    }
    st->ino = 0;
    st->mode = (fad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? 0040000u : 0100000u;
    st->uid = 0;
    st->size = ((int64_t)fad.nFileSizeHigh << 32) | (int64_t)fad.nFileSizeLow;
    return 0;
}

int sx_hosted_usleep(unsigned usec) {
    Sleep(usec / 1000u);
    return 0;
}

#endif
