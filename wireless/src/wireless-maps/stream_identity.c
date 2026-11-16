/* Wireless CarPlay Maps. Maintainer: omonob. Source date: 2026-08-29. */
/* Exact accepted receiver: its alternate-screen thread temporarily sets a
 * process-global classification bit while constructing an alternate stream.
 * A concurrent main-screen constructor can inherit that bit. Keep that bit
 * per thread instead. No stream pixels, authentication or session grants are
 * generated here. All ordinary getenv calls still reach libc unchanged.
 */
#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>

static pthread_once_t stream_once = PTHREAD_ONCE_INIT;
static pthread_key_t stream_key;
static int stream_ready;
#ifdef V75_STREAM_HOST_TEST
static uintptr_t legacy_stream_kind;
#else
#define legacy_stream_kind (*(volatile uint32_t *)0x112a38)
#endif
static void stream_initialize(void) {
    stream_ready = pthread_key_create(&stream_key, NULL) == 0;
}
__attribute__((visibility("hidden")))
char *v75_stream_getenv_dispatch(uintptr_t *saved) {
    uintptr_t caller = saved[13], value = saved[0];
    int is_set = (caller == 0xdea6c || caller == 0xdea84) && value <= 1;
    int is_get = caller == 0xe5e80 && value == 0x112a38;
    if (is_set || is_get) {
        int previous_errno = errno;
        uintptr_t result;
        pthread_once(&stream_once, stream_initialize);
        if (stream_ready) {
            if (is_set) {
                /* No global flag is changed after pthread_once completes.
                 * The deployment's native TLS self-test must pass first.
                 */
                if (pthread_setspecific(stream_key, (void *)value) != 0) {
                    errno = previous_errno;
                    return (char *)value;
                }
                result = value;
            } else result = (uintptr_t)pthread_getspecific(stream_key);
        } else result = is_set ? value : legacy_stream_kind;
        /* Failure falls back to the receiver's original classification; it
         * must never turn a failed allocation into a synthetic live session.
         */
        if (is_set && !stream_ready) legacy_stream_kind = value;
        errno = previous_errno;
        return (char *)result;
    }
    return getenv((const char *)value);
}
