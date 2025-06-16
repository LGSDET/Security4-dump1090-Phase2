#ifndef STUB_PTHREAD_H
#define STUB_PTHREAD_H

#ifdef _WIN32

// ======= 타입 정의 =======
typedef int pthread_mutex_t;
typedef int pthread_cond_t;
typedef int pthread_t;  // ✅ reader_thread용

#define PTHREAD_MUTEX_INITIALIZER 0
#define PTHREAD_COND_INITIALIZER  0

// ======= mutex =======
static inline int pthread_mutex_init(pthread_mutex_t *m, const void *a) { return 0; }
static inline int pthread_mutex_destroy(pthread_mutex_t *m) { return 0; }
static inline int pthread_mutex_lock(pthread_mutex_t *m) { return 0; }
static inline int pthread_mutex_unlock(pthread_mutex_t *m) { return 0; }

// ======= cond =======
static inline int pthread_cond_init(pthread_cond_t *c, const void *a) { return 0; }
static inline int pthread_cond_destroy(pthread_cond_t *c) { return 0; }
static inline int pthread_cond_wait(pthread_cond_t *c, pthread_mutex_t *m) { return 0; }
static inline int pthread_cond_signal(pthread_cond_t *c) { return 0; }
static inline int pthread_cond_broadcast(pthread_cond_t *c) { return 0; }

// ======= thread =======
static inline int pthread_create(pthread_t *t, const void *a, void *(*fn)(void *), void *arg) {
    return 0;
}
static inline int pthread_join(pthread_t t, void **ret) { return 0; }

#endif  // _WIN32

#endif  // STUB_PTHREAD_H
