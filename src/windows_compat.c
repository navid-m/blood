// Windows support layer for Blood.
//
// The generated C source is produced by the Spectre compiler's Windows/MSVC
// backend. This file supplies the POSIX-shaped primitives Blood relies on and
// a small process table backed by CreateProcess.

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#if defined(_WIN32)

#include <windows.h>
#include <io.h>
#include <fcntl.h>
#include <process.h>
#include <direct.h>
#include <sys/stat.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int _fltused = 0;

/* ------------------------------------------------------------------------- */
/* File descriptors                                                          */

static const char *bx_path(const char *path) {
    if (path && strcmp(path, "/dev/null") == 0) {
        return "NUL";
    }
    return path;
}

int64_t bx_read(int fd, void *buf, int64_t n) {
    if (n <= 0) {
        return 0;
    }
    return (int64_t)_read(fd, buf, (unsigned int)n);
}

int64_t bx_write(int fd, const void *buf, int64_t n) {
    const char *p = (const char *)buf;
    int64_t done = 0;
    while (done < n) {
        int chunk = (n - done > 0x7fffffff) ? 0x7fffffff : (int)(n - done);
        int w = _write(fd, p + done, (unsigned int)chunk);
        if (w <= 0) {
            break;
        }
        done += w;
    }
    return done;
}

int bx_open_read(const char *path) {
    return _open(bx_path(path), _O_RDONLY | _O_BINARY | _O_NOINHERIT);
}

int bx_open_write(const char *path, int append) {
    int flags = _O_WRONLY | _O_CREAT | _O_BINARY | _O_NOINHERIT;
    if (append) {
        flags |= _O_APPEND;
    } else {
        flags |= _O_TRUNC;
    }
    return _open(bx_path(path), flags, _S_IREAD | _S_IWRITE);
}

int bx_close(int fd) { return _close(fd); }
int bx_dup(int fd) { return _dup(fd); }
int bx_dup2(int oldfd, int newfd) { return _dup2(oldfd, newfd); }
int bx_pipe(int fds[2]) { return _pipe(fds, 4096, _O_BINARY | _O_NOINHERIT); }
int bx_getpid(void) { return _getpid(); }

int bx_setenv(const char *name, const char *value) {
    if (!name) {
        return -1;
    }
    if (!SetEnvironmentVariableA(name, value ? value : "")) {
        return -1;
    }
    return 0;
}

int bx_unsetenv(const char *name) {
    if (!name) {
        return -1;
    }
    if (!SetEnvironmentVariableA(name, NULL)) {
        return -1;
    }
    return 0;
}

/* ------------------------------------------------------------------------- */
/* Processes                                                                 */

#define BX_MAX_PROC 256

typedef struct {
    HANDLE handle;
    DWORD pid;
    int used;
} BxProc;

static BxProc bx_procs[BX_MAX_PROC];

static int bx_proc_add(HANDLE handle, DWORD pid) {
    int i;
    for (i = 0; i < BX_MAX_PROC; i++) {
        if (!bx_procs[i].used) {
            bx_procs[i].handle = handle;
            bx_procs[i].pid = pid;
            bx_procs[i].used = 1;
            return i + 1;
        }
    }
    return -1;
}

static BxProc *bx_proc_get(int id) {
    if (id <= 0 || id > BX_MAX_PROC) {
        return NULL;
    }
    if (!bx_procs[id - 1].used) {
        return NULL;
    }
    return &bx_procs[id - 1];
}

static void bx_quote_arg(char **out, size_t *cap, size_t *len, const char *arg) {
    size_t need;
    int quote = 0;
    const char *p;

    if (!arg) {
        arg = "";
    }
    if (*arg == 0) {
        quote = 1;
    }
    for (p = arg; *p; p++) {
        if (*p == ' ' || *p == '\t' || *p == '"') {
            quote = 1;
            break;
        }
    }

    need = strlen(arg) * 2 + 8;
    if (*len + need > *cap) {
        size_t next = (*cap + need) * 2;
        char *grown = (char *)realloc(*out, next);
        if (!grown) {
            return;
        }
        *out = grown;
        *cap = next;
    }

    if (quote) {
        (*out)[(*len)++] = '"';
    }
    {
        size_t backslashes = 0;
        for (p = arg; *p; p++) {
            if (*p == '\\') {
                backslashes++;
                (*out)[(*len)++] = *p;
            } else if (*p == '"') {
                size_t k;
                for (k = 0; k < backslashes; k++) {
                    (*out)[(*len)++] = '\\';
                }
                (*out)[(*len)++] = '\\';
                (*out)[(*len)++] = '"';
                backslashes = 0;
            } else {
                backslashes = 0;
                (*out)[(*len)++] = *p;
            }
        }
        if (quote) {
            size_t k;
            for (k = 0; k < backslashes; k++) {
                (*out)[(*len)++] = '\\';
            }
            (*out)[(*len)++] = '"';
        }
    }
}

static char *bx_join_argv(char **argv) {
    size_t cap = 256;
    size_t len = 0;
    char *out = (char *)malloc(cap);
    int i;
    if (!out) {
        return NULL;
    }
    out[0] = 0;
    for (i = 0; argv[i]; i++) {
        if (i > 0) {
            if (len + 2 > cap) {
                cap *= 2;
                out = (char *)realloc(out, cap);
                if (!out) return NULL;
            }
            out[len++] = ' ';
            out[len] = 0;
        }
        bx_quote_arg(&out, &cap, &len, argv[i]);
        if (!out) {
            return NULL;
        }
        out[len] = 0;
    }
    return out;
}

static HANDLE bx_dup_handle(HANDLE h) {
    HANDLE out = NULL;
    if (!h || h == INVALID_HANDLE_VALUE) {
        return NULL;
    }
    if (!DuplicateHandle(GetCurrentProcess(), h, GetCurrentProcess(), &out,
                         0, TRUE, DUPLICATE_SAME_ACCESS)) {
        return NULL;
    }
    return out;
}

static HANDLE bx_std_handle(int fd, DWORD which) {
    intptr_t raw;
    if (fd < 0) {
        return bx_dup_handle(GetStdHandle(which));
    }
    raw = _get_osfhandle(fd);
    if (raw == -1) {
        return bx_dup_handle(GetStdHandle(which));
    }
    return bx_dup_handle((HANDLE)raw);
}

int bx_spawn_argv(char **argv, int in_fd, int out_fd, int err_fd,
                  const char *cwd) {
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    HANDLE hin, hout, herr;
    char *cmdline;
    int id;
    const char *workdir = (cwd && cwd[0]) ? cwd : NULL;

    if (!argv || !argv[0]) {
        return -1;
    }

    cmdline = bx_join_argv(argv);
    if (!cmdline) {
        return -1;
    }

    memset(&si, 0, sizeof(si));
    memset(&pi, 0, sizeof(pi));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = bx_std_handle(in_fd, STD_INPUT_HANDLE);
    si.hStdOutput = bx_std_handle(out_fd, STD_OUTPUT_HANDLE);
    si.hStdError = bx_std_handle(err_fd, STD_ERROR_HANDLE);

    if (!si.hStdInput) si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    if (!si.hStdOutput) si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    if (!si.hStdError) si.hStdError = GetStdHandle(STD_ERROR_HANDLE);

    if (!CreateProcessA(NULL, cmdline, NULL, NULL, TRUE, 0, NULL, workdir, &si,
                        &pi)) {
        if (si.hStdInput) CloseHandle(si.hStdInput);
        if (si.hStdOutput) CloseHandle(si.hStdOutput);
        if (si.hStdError) CloseHandle(si.hStdError);
        free(cmdline);
        return -1;
    }

    if (si.hStdInput) CloseHandle(si.hStdInput);
    if (si.hStdOutput) CloseHandle(si.hStdOutput);
    if (si.hStdError) CloseHandle(si.hStdError);
    CloseHandle(pi.hThread);
    free(cmdline);

    id = bx_proc_add(pi.hProcess, pi.dwProcessId);
    if (id < 0) {
        TerminateProcess(pi.hProcess, 1);
        CloseHandle(pi.hProcess);
        return -1;
    }
    return id;
}

int bx_wait_pid(int id, int nohang) {
    BxProc *p = bx_proc_get(id);
    DWORD code = 127;
    DWORD wait;
    if (!p) {
        return -1;
    }
    wait = WaitForSingleObject(p->handle, nohang ? 0 : INFINITE);
    if (wait == WAIT_TIMEOUT) {
        return -1;
    }
    if (wait != WAIT_OBJECT_0) {
        p->used = 0;
        CloseHandle(p->handle);
        return 127;
    }
    if (!GetExitCodeProcess(p->handle, &code)) {
        code = 127;
    }
    CloseHandle(p->handle);
    p->used = 0;
    return (int)code;
}

int bx_wait_pid_timeout(int id, int timeout_ms) {
    BxProc *p = bx_proc_get(id);
    DWORD code = 127;
    DWORD wait;
    if (!p) {
        return -1;
    }
    wait = WaitForSingleObject(p->handle, timeout_ms < 0 ? INFINITE : (DWORD)timeout_ms);
    if (wait == WAIT_TIMEOUT) {
        return -1;
    }
    if (wait != WAIT_OBJECT_0) {
        p->used = 0;
        CloseHandle(p->handle);
        return 127;
    }
    if (!GetExitCodeProcess(p->handle, &code)) {
        code = 127;
    }
    CloseHandle(p->handle);
    p->used = 0;
    return (int)code;
}

int bx_kill(int id) {
    BxProc *p = bx_proc_get(id);
    if (!p) {
        return -1;
    }
    TerminateProcess(p->handle, 1);
    return 0;
}

/* ------------------------------------------------------------------------- */
/* Polling                                                                   */

int bx_poll_fd(int fd, int timeout_ms) {
    intptr_t raw;
    HANDLE h;
    DWORD started;
    DWORD wait;

    if (fd < 0) {
        return -1;
    }
    raw = _get_osfhandle(fd);
    if (raw == -1) {
        return -1;
    }
    h = (HANDLE)raw;
    started = GetTickCount();

    if (_isatty(fd)) {
        wait = WaitForSingleObject(h, timeout_ms < 0 ? INFINITE : (DWORD)timeout_ms);
        return wait == WAIT_OBJECT_0 ? 1 : (wait == WAIT_TIMEOUT ? 0 : -1);
    }

    for (;;) {
        DWORD avail = 0;
        if (PeekNamedPipe(h, NULL, 0, NULL, &avail, NULL)) {
            if (avail > 0) {
                return 1;
            }
        } else {
            // Not a pipe (regular file or other handle).
            wait = WaitForSingleObject(h, 0);
            return wait == WAIT_TIMEOUT ? 0 : 1;
        }
        if (timeout_ms < 0) {
            Sleep(10);
            continue;
        }
        if ((int)(GetTickCount() - started) >= timeout_ms) {
            return 0;
        }
        Sleep(10);
    }
}

/* Blood's PollFd layout. */
typedef struct {
    int fd;
    short events;
    short revents;
} BxPollFd;

int poll(void *fds, int64_t n, int timeout) {
    BxPollFd *p = (BxPollFd *)fds;
    int64_t i;
    int ready = 0;
    DWORD started = GetTickCount();
    if (!p) {
        return -1;
    }
    for (;;) {
        ready = 0;
        for (i = 0; i < n; i++) {
            p[i].revents = 0;
            if (bx_poll_fd(p[i].fd, 0) > 0) {
                p[i].revents = 1;
                ready++;
            }
        }
        if (ready > 0 || timeout == 0) {
            return ready;
        }
        if (timeout > 0 && (int)(GetTickCount() - started) >= timeout) {
            return 0;
        }
        Sleep(10);
    }
}

/* ------------------------------------------------------------------------- */
/* Terminal                                                                  */

static HANDLE bx_conin;
static HANDLE bx_conout;
static DWORD bx_saved_in_mode;
static DWORD bx_saved_out_mode;
static int bx_have_console;
static int bx_raw_active;

int bx_term_init(void) {
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
    bx_conin = GetStdHandle(STD_INPUT_HANDLE);
    bx_conout = GetStdHandle(STD_OUTPUT_HANDLE);
    bx_have_console = 0;
    bx_raw_active = 0;
    if (bx_conin && bx_conin != INVALID_HANDLE_VALUE &&
        GetConsoleMode(bx_conin, &bx_saved_in_mode)) {
        bx_have_console = 1;
    }
    if (bx_conout && bx_conout != INVALID_HANDLE_VALUE &&
        GetConsoleMode(bx_conout, &bx_saved_out_mode)) {
        SetConsoleMode(bx_conout, bx_saved_out_mode | 0x0004);
        bx_have_console = 1;
    }
    return bx_have_console;
}

void bx_term_raw(void) {
    DWORD mode;
    if (!bx_have_console || bx_raw_active) {
        return;
    }
    mode = bx_saved_in_mode;
    mode &= ~(DWORD)(0x0001 | 0x0002 | 0x0004 | 0x0040); // processed/line/echo/quick-edit
    mode |= 0x0200;                                      // virtual terminal input
    SetConsoleMode(bx_conin, mode);
    bx_raw_active = 1;
}

void bx_term_cooked(void) {
    if (!bx_have_console || !bx_raw_active) {
        return;
    }
    SetConsoleMode(bx_conin, bx_saved_in_mode);
    bx_raw_active = 0;
}

int bx_term_size(int64_t *rows, int64_t *cols) {
    CONSOLE_SCREEN_BUFFER_INFO info;
    if (!bx_conout || bx_conout == INVALID_HANDLE_VALUE ||
        !GetConsoleScreenBufferInfo(bx_conout, &info)) {
        return -1;
    }
    if (rows) {
        *rows = (int64_t)(info.srWindow.Bottom - info.srWindow.Top + 1);
    }
    if (cols) {
        *cols = (int64_t)(info.srWindow.Right - info.srWindow.Left + 1);
    }
    return 0;
}

/* ------------------------------------------------------------------------- */
/* Symbols expected by Spectre's hosted Windows runtime                      */

int sx_hosted_usleep(unsigned usec) {
    Sleep(usec / 1000u);
    return 0;
}

int sx_hosted_sleep(unsigned seconds) {
    Sleep(seconds * 1000u);
    return 0;
}

typedef struct {
    uint64_t ino;
    uint32_t mode;
    uint32_t uid;
    int64_t size;
} BxStat;

int sx_hosted_stat(const char *path, void *buf) {
    WIN32_FILE_ATTRIBUTE_DATA fad;
    BxStat *st = (BxStat *)buf;
    if (!path || !buf) {
        return -1;
    }
    if (!GetFileAttributesExA(path, GetFileExInfoStandard, &fad)) {
        return -1;
    }
    st->ino = 0;
    st->mode = (fad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? 0040000u : 0100000u;
    st->uid = 0;
    st->size = ((int64_t)fad.nFileSizeHigh << 32) | (int64_t)fad.nFileSizeLow;
    return 0;
}

char *sx_hosted_realpath(const char *path, char *out) {
    if (!path) {
        return NULL;
    }
    if (out) {
        return _fullpath(out, path, 4096);
    }
    return (char *)path;
}

int sx_hosted_readlink(const char *path, char *buf, size_t n) {
    (void)path;
    (void)buf;
    (void)n;
    return -1;
}

char *sx_hosted_strtok_r(char *s, const char *delim, char **save) {
    return strtok_s(s, delim, save);
}

int gettimeofday(void *tv, void *tz) {
    FILETIME ft;
    ULARGE_INTEGER u;
    unsigned long long us;
    (void)tz;
    GetSystemTimeAsFileTime(&ft);
    u.LowPart = ft.dwLowDateTime;
    u.HighPart = ft.dwHighDateTime;
    us = (u.QuadPart - 116444736000000000ULL) / 10ULL;
    if (tv) {
        unsigned long long *p = (unsigned long long *)tv;
        p[0] = us / 1000000ULL;
        p[1] = us % 1000000ULL;
    }
    return 0;
}

int usleep(unsigned usec) { return sx_hosted_usleep(usec); }
int sleep(unsigned seconds) { return sx_hosted_sleep(seconds); }

typedef BOOLEAN(WINAPI *BxRtlGenRandom)(PVOID, ULONG);

int64_t getrandom(void *buf, size_t len, unsigned flags) {
    static BxRtlGenRandom fn;
    static int loaded;
    (void)flags;
    if (!loaded) {
        HMODULE mod = LoadLibraryA("advapi32.dll");
        if (mod) {
            fn = (BxRtlGenRandom)GetProcAddress(mod, "SystemFunction036");
        }
        loaded = 1;
    }
    if (fn && fn(buf, (ULONG)len)) {
        return (int64_t)len;
    }
    return -1;
}

void arc4random_buf(void *buf, size_t len) { (void)getrandom(buf, len, 0); }

#endif
