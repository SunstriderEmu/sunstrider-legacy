#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define __USE_GNU
#include <dlfcn.h>

#define ADDR(x) (void *(*)())dlsym(RTLD_NEXT, (x))

void *__real_malloc(size_t);
void *__real_realloc(void *, size_t);
void *__real_calloc(size_t, size_t);
void __real_free(void *);
pthread_mutex_t tsm_lock = PTHREAD_MUTEX_INITIALIZER;
static int loaded=0;

//void load_addr() __attribute__((constructor));

/*void load_addr()
{
    printf("loading addresses...\n");
    if (loaded)
        goto exit;

    __real_malloc = ADDR("malloc");
    __real_calloc = ADDR("calloc");
    __real_realloc = ADDR("realloc");
    __real_free = (void (*)())ADDR("free");
    printf("symbols fetched\n");
    if (!__real_malloc || !__real_calloc || !__real_realloc || !__real_free)
        abort();
//    pthread_mutex_init(&tsm_lock, NULL);

    printf("done.\n");
    loaded = 1;
    return;
exit:
    printf("failed.\n");
}*/

void *__wrap_calloc(size_t nmemb, size_t size)
{
    void *ret;
    pthread_mutex_lock(&tsm_lock);
    ret = __real_calloc(nmemb, size);
    pthread_mutex_unlock(&tsm_lock);
    return ret;
}

void *__wrap_malloc(size_t size)
{
    void *ret;
    pthread_mutex_lock(&tsm_lock);
    ret = __real_malloc(size);
    pthread_mutex_unlock(&tsm_lock);
    return ret;
}

void __wrap_free(void *ptr)
{
    pthread_mutex_lock(&tsm_lock);
    __real_free(ptr);
    pthread_mutex_unlock(&tsm_lock);
}

void *__wrap_realloc(void *ptr, size_t size)
{
    void *ret;
    pthread_mutex_lock(&tsm_lock);
    ret = __real_realloc(ptr, size);
    pthread_mutex_unlock(&tsm_lock);
    return ret;
}
