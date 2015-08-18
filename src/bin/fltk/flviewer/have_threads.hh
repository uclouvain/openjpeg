#ifndef _HAVE_THREADS_HH_
#define _HAVE_THREADS_HH_

#if defined(HAVE_NPTH_THREADS) && defined(_WIN32)

#define PTHREAD_mutex_lock npth_mutex_lock
#define PTHREAD_mutex_unlock npth_mutex_unlock
#define PTHREAD_mutex_init npth_mutex_init
#define PTHREAD_mutex_destroy npth_mutex_destroy

#define PTHREAD_cond_wait  npth_cond_wait
#define PTHREAD_cond_signal npth_cond_signal
#define PTHREAD_cond_init npth_cond_init
#define PTHREAD_cond_broadcast npth_cond_broadcast
#define PTHREAD_cond_destroy npth_cond_destroy

#define PTHREAD_create npth_create
#define PTHREAD_join npth_join

#define PTHREAD_cond_t npth_cond_t
#define PTHREAD_mutex_t npth_mutex_t
#define PTHREAD_t npth_t

#define PTHREAD_init npth_init

#elif defined(_WIN32)

#define PTHREAD_mutex_lock pthread_mutex_lock
#define PTHREAD_mutex_unlock pthread_mutex_unlock
#define PTHREAD_mutex_init pthread_mutex_init
#define PTHREAD_mutex_destroy pthread_mutex_destroy

#define PTHREAD_cond_wait  pthread_cond_wait
#define PTHREAD_cond_signal pthread_cond_signal
#define PTHREAD_cond_init pthread_cond_init
#define PTHREAD_cond_broadcast pthread_cond_broadcast
#define PTHREAD_cond_destroy pthread_cond_destroy

#define PTHREAD_create pthread_create
#define PTHREAD_join pthread_join

#define PTHREAD_cond_t pthread_cond_t
#define PTHREAD_mutex_t pthread_mutex_t
#define PTHREAD_t pthread_t

#else  /* not _WIN32 */

#define PTHREAD_mutex_lock pthread_mutex_lock
#define PTHREAD_mutex_unlock pthread_mutex_unlock
#define PTHREAD_mutex_init pthread_mutex_init
#define PTHREAD_mutex_destroy pthread_mutex_destroy

#define PTHREAD_cond_wait  pthread_cond_wait
#define PTHREAD_cond_signal pthread_cond_signal
#define PTHREAD_cond_init pthread_cond_init
#define PTHREAD_cond_broadcast pthread_cond_broadcast
#define PTHREAD_cond_destroy pthread_cond_destroy

#define PTHREAD_create pthread_create
#define PTHREAD_join pthread_join

#define PTHREAD_cond_t pthread_cond_t
#define PTHREAD_mutex_t pthread_mutex_t
#define PTHREAD_t pthread_t

#endif
#endif /* _HAVE_THREADS_HH_ */
