#include <config.h>
/* 
 * author(s) and license
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#ifdef _WIN32
#include <windows.h>
#include <io.h>
#else
#define __STDC_FORMAT_MACROS
#include <stdint.h>
#include <unistd.h>
#endif

#ifdef HAVE_THREADS

#if defined(HAVE_NPTH_THREADS) && defined(_WIN32)
#include <npth.h>
#elif defined(HAVE_WINPORT)
#include <WinBase.h>
#include <process.h>

#define WAIT_FOR_CONDVARS 5
#else
#include <pthread.h>
#endif

#include "have_threads.hh"

#endif /* HAVE_THREADS */

/* #define DEBUG_NEXT_FRAME */

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#include "openjpeg.h" /* of version 2 */
       }
#else
#include "openjpeg.h" /* of version 2 */
#endif /* __cplusplus */

#include <FL/Fl.H>
#include <FL/fl_ask.H>

#include "flviewer.hh"
#include "tree.hh"
#include "viewerdefs.hh"
#include "lang/mj2_lang.h_utf8"

#include "JP2.hh"
#include "OPENMJ2.hh"


/* FORWARD */
typedef struct mj2_extract_info
{
    int cur_track;
    unsigned int width;
    unsigned int height;
    unsigned int duration;
    int cur_sample;
    int max_samples;
    int max_tracks;
    unsigned int channels;
#ifdef HAVE_FSEEKO
	off_t start_offset;
#elif HAVE_FSEEKI64
	int64_t start_offset;
#else
    long start_offset;
#endif
    double frame_rate;

    unsigned char *sample_buf;

    int sample_nr;

    FILE *reader;
    const char *read_idf;
} MJ2Extract;

typedef struct sample_info
{
    struct sample_info *next;

    unsigned char *sample_buf;
    unsigned char *rgb_buf;
    unsigned int  sample_size;
    int sample_nr;
    int inserted;

    unsigned char *cur_buf;
}SampleInfo;


static void next_frame(void);
static void stop_timeout();
static void load(Canvas *canvas, const char *read_idf);

static void free_sample_info(SampleInfo *sinfo);
static unsigned char* extract_single_frame(MJ2Extract *exinfo);
static void decode_frame(MJ2Extract *exinfo, SampleInfo *sinfo);

static void extract_frames_close(MJ2Extract *exinfo);
static MJ2Extract *extract_frames_open(const char *read_idf);

#ifdef HAVE_THREADS

static void threads_prelude(MJ2Extract *exinfo);
static void threads_postlude(MJ2Extract *exinfo);

static int threads_stopped;
static int stop_threads;
static int sample_list_busy;

static int elems_in_list;
#ifdef _WIN32
#define MAX_ELEMS_IN_LIST 30
#else
#define MAX_ELEMS_IN_LIST 20
#endif

static SampleInfo *sample_list_head, *sample_list_tail;

#ifdef HAVE_WINPORT
static CRITICAL_SECTION sample_list_mutex;

static CONDITION_VARIABLE sample_list_condGET;
static CONDITION_VARIABLE sample_list_condINSERTED;
#else
static PTHREAD_cond_t sample_list_condGET;
static PTHREAD_cond_t sample_list_condINSERTED;

static PTHREAD_mutex_t sample_list_mutex;
#endif

static unsigned char *get_rgb_buffer(void);

typedef struct decoder_info
{
	MJ2Extract *exinfo;
	SampleInfo *sample_info;
#ifdef HAVE_WINPORT
	HANDLE tid;
#else
	PTHREAD_t tid;
#endif
	int ID;

}Decoder;

static Decoder *decoder;

#endif /* HAVE_THREADS */

typedef struct mj2_info
{
	MJ2Extract *exinfo;
	Canvas *canvas;
	double delay;
#ifdef HAVE_THREADS
	int nr_threads;
#endif
	char paused, single_step_possible;
	char single_step_mode, single_step_served;

} OPENMJ2Info;

static OPENMJ2Info play;

/* RGB[A] buffers: */
static unsigned char *openmj2_buf;
static unsigned char *play_buf;
static int play_buf_nr;

static int mj2_stopped;

#ifdef HAVE_THREADS

static SampleInfo *new_sample_info(void)
{
	SampleInfo *sinfo;

	sinfo = (SampleInfo*)calloc(1,sizeof(SampleInfo));

	if(sinfo == NULL)
   {
	fprintf(stderr,"OPENMJ2.cxx:%d:\n\tmemory out\n",__LINE__);
	exit(1);
   }
	if(sample_list_head == NULL)
	 sample_list_head = sinfo;
	else
	 sample_list_tail->next = sinfo;
	sample_list_tail = sinfo;

	return sinfo;
}
#endif /* HAVE_THREADS */


static void OPENMJ2_postlude(void)
{
	mj2_stopped = 1;

	FLViewer_movie_runs(0);
	FLViewer_frames_animation(0);

	if(play.exinfo)
   {
	stop_timeout();
#ifdef HAVE_WINPORT
    Fl::wait(0.5);
#endif
#ifdef HAVE_THREADS
	if(!threads_stopped)
  {
	stop_threads = 1;
	threads_postlude(play.exinfo);
  }
#endif
	extract_frames_close(play.exinfo);
   }
	memset(&play, 0, sizeof(OPENMJ2Info));

	if(openmj2_buf)
   {
	free(openmj2_buf); openmj2_buf = NULL;
   }
	else
	if(play_buf)
   {
	free(play_buf); play_buf = NULL;
   }
#ifdef HAVE_THREADS
	stop_threads = 0;
	play_buf_nr = 1;
#endif
}

static void wait_cb(void *v)
{
	if(play.paused
	|| play.single_step_possible
	  )
   {
	return;
   }
	if(play_buf == NULL)
   {
	fprintf(stderr,"%s:%d:### wait_cb : play_buf == NULL ###\n",
	 __FILE__,__LINE__);

	exit(1);

	Fl::repeat_timeout(play.delay/10., wait_cb, v);
	return;
   }
	next_frame();
}

static void next_frame(void)
{
	int i;

	if(mj2_stopped) return;

#ifdef HAVE_THREADS
	if(stop_threads) return;

	if( !play.single_step_mode)
	 i = play_buf_nr;
	else
	 i = play.exinfo->cur_sample-1;
#else
	i = play.exinfo->cur_sample-1;
#endif

	if(i <= 0) i = 1;

	FLViewer_show_frame(i,play.exinfo->max_samples-1);

	play.single_step_served = play.single_step_mode;
/*---------------------------------------------------------------------*/
	FLViewer_use_buffer(play_buf, play.exinfo->width, play.exinfo->height,
		play.exinfo->channels);
/*---------------------------------------------------------------------*/
	if(openmj2_buf) free(openmj2_buf);

	openmj2_buf = play_buf; play_buf = NULL;

#ifdef HAVE_THREADS
	if(stop_threads) return;
#endif
/* forward or backward: */
	if( !play.single_step_mode)
   {
	Fl::repeat_timeout(play.delay, wait_cb, play.exinfo);
   }
	if((i = FLViewer_frame_selected()) > 0)
   {
	play.exinfo->cur_sample = i;
   }
#ifdef HAVE_THREADS
	if(stop_threads) return;

	if( !play.single_step_mode && (play.nr_threads > 0))
   {
	play_buf = get_rgb_buffer();
	assert(play_buf);
   }
	else
   {
	play_buf = extract_single_frame(play.exinfo);
	assert(play_buf);
	play_buf_nr = play.exinfo->sample_nr;
   }
#else /* not HAVE_THREADS */
	play_buf = extract_single_frame(play.exinfo);
	assert(play_buf);
	play_buf_nr = play.exinfo->sample_nr;
#endif /* HAVE_THREADS */

}/* next_frame() */

static void fin_cb(void)
{
	play.canvas->cleanup = NULL;

	OPENMJ2_postlude();

	FLViewer_movie_runs(0);
	FLViewer_frames_animation(0);
}

static void backward_cb(void)
{
	int i;

	if(mj2_stopped) return;

	if( !play.single_step_possible) return;

	play.paused = 0;
	play.single_step_served = 0;

	if(play_buf) 
   { 
	free(play_buf); play_buf = NULL; 
   }
	i = play.exinfo->cur_sample - 1;

	if(i == 0)
   {
	play.exinfo->cur_sample = play.exinfo->max_samples + 1;
   }
	play.exinfo->cur_sample -= 3;

	if((i = FLViewer_frame_selected()) > 0)
   {
	play.exinfo->cur_sample = i;
   }
	play_buf = extract_single_frame(play.exinfo);
	assert(play_buf);

	play_buf_nr = play.exinfo->sample_nr;
	play.single_step_mode = 1;

	next_frame();

}/* backward_cb() */

static void forward_cb(void)
{
	int i;

	if(mj2_stopped) return;

	if( !play.single_step_possible) return;

	play.paused = 0;
	play.single_step_mode = 1;
	play.single_step_served = 0;

	if((i = FLViewer_frame_selected()) > 0)
   {
	if(play_buf) { free(play_buf); play_buf = NULL; }

	play.exinfo->cur_sample = i;

	play_buf = extract_single_frame(play.exinfo);
	assert(play_buf);

	play_buf_nr = play.exinfo->sample_nr;
   }
	else
	if(!play_buf)
   {
	play_buf = extract_single_frame(play.exinfo);
	assert(play_buf);

	play_buf_nr = play.exinfo->sample_nr;
   }
	next_frame();

}/* forward_cb() */

static void pause_cb(void)
{
	if(mj2_stopped) return;

	if(play.paused) return;
	if(play.single_step_mode) return;

	stop_timeout();
	play.paused = 1;

	play.single_step_possible = 1;
	play.single_step_served = 0; play.single_step_mode = 0;

	FLViewer_movie_runs(0);

#ifdef HAVE_THREADS
	if(play.nr_threads)
   {
	stop_threads = 1;

	threads_postlude(play.exinfo);

	play.nr_threads = 0;
   }
#endif
}/* pause_cb() */

static void resume_cb(void)
{
	if(mj2_stopped) return;

	if(!play.paused
	&& !play.single_step_possible
	  )
	 return;

	play.paused = 0;
	play.single_step_possible = 0;
	play.single_step_mode = 0;
	play.single_step_served = 0;

	FLViewer_movie_runs(1);

#ifdef HAVE_THREADS
	if(threads_stopped || (play.nr_threads == 0))
   {
/*==========================================*/
	play.nr_threads = FLViewer_nr_threads();
/*==========================================*/
	if(play.nr_threads)
  {
	threads_prelude(play.exinfo);
  }
	if( !play_buf)
  {
	play_buf = get_rgb_buffer();
	assert(play_buf);
  }
   }
#endif

	next_frame();

}/* resume_cb() */

static void restart_cb(void)
{
	Canvas *canvas;
	const char *read_idf;

	canvas = play.canvas;
	read_idf = play.exinfo->read_idf;

	OPENMJ2_postlude();

	load(canvas, read_idf);

}/* restart_cb() */

static void stop_timeout(void)
{
	if(!play.exinfo) return;

	if(Fl::has_timeout(wait_cb, play.exinfo))
	 Fl::remove_timeout(wait_cb, play.exinfo);
}

static void load(Canvas *canvas, const char *read_idf)
{
	MJ2Extract *exinfo;
	int i;

	if((exinfo = extract_frames_open(read_idf)) == NULL)
   {
	fl_alert("%s\n %s", MJ2_NO_FRAMES_FROM_s, read_idf);
	return;
   }

	if(exinfo->max_samples == 0 || exinfo->width == 0 || exinfo->height == 0)
   {
	fl_alert(MJ2_SHOW_FAILS_s, read_idf,
		exinfo->max_samples, exinfo->width, exinfo->height);

	OPENMJ2_postlude();
	return;
   }

	mj2_stopped = 0;

	play.delay =
	((double)exinfo->duration/(double)(exinfo->max_samples-1)) * 0.001;
#ifdef _WIN32
	play.delay *= 1.10;
#endif
	play.exinfo = exinfo;

	FLViewer_url(read_idf, exinfo->width, exinfo->height);

	canvas->pause = &pause_cb;
	canvas->resume = &resume_cb;
	canvas->restart = &restart_cb;
	canvas->forward = &forward_cb;
	canvas->backward = &backward_cb;
	canvas->fin = &fin_cb;
	canvas->cleanup = &OPENMJ2_postlude;

#ifdef HAVE_THREADS
	FLViewer_threads_activate(1);
#endif
	FLViewer_mj2_animation(1);
	FLViewer_set_max_mj2_tracks(Tracks[0].max_tracks);

	canvas->top_frame = exinfo->max_samples - 1;
	play.canvas = canvas;

	if((i = FLViewer_frame_selected()) > 0)
   {
	exinfo->cur_sample = i;
   }
	FLViewer_movie_runs(1);

#ifdef HAVE_THREADS
/*==========================================*/
	play.nr_threads = FLViewer_nr_threads();
/*==========================================*/
	if(play.nr_threads)
   {
	play_buf_nr = 0;

	threads_prelude(exinfo);

	play_buf = get_rgb_buffer();
	assert(play_buf);
   }
	else
   {
	play_buf_nr = 0;

	play_buf = extract_single_frame(exinfo);
	assert(play_buf);
   }
#else /* not HAVE_THREADS */
	play_buf_nr = 0;

	play_buf = extract_single_frame(exinfo);
	assert(play_buf);
#endif

	Fl::add_timeout(play.delay, wait_cb, exinfo);

}/* load() */

void OPENMJ2_load(Canvas *canvas, const char *read_idf, uint64_t fsize)
{
	load(canvas, read_idf);
}

#ifdef HAVE_THREADS

#ifdef HAVE_WINPORT
static DWORD WINAPI decoder_thread(PVOID v)
#else
static void *decoder_thread(void *v)
#endif
{
	SampleInfo *sinfo;

	while( !stop_threads && !mj2_stopped)
   {
	sinfo = NULL;

#ifdef HAVE_WINPORT
	EnterCriticalSection(&sample_list_mutex);
#else
	PTHREAD_mutex_lock(&sample_list_mutex);
#endif

	while( !stop_threads && !mj2_stopped
	&& (sample_list_busy || (elems_in_list > MAX_ELEMS_IN_LIST)) )
  {
#ifdef HAVE_WINPORT
	SleepConditionVariableCS(&sample_list_condGET, &sample_list_mutex,
	 INFINITE);
#else
	PTHREAD_cond_wait(&sample_list_condGET, &sample_list_mutex);
#endif
  }
	sample_list_busy = 1;

	if(!stop_threads && !mj2_stopped)
  {
	sinfo = new_sample_info();

	++elems_in_list;
  }
#ifdef HAVE_WINPORT
	WakeConditionVariable(&sample_list_condINSERTED);

	LeaveCriticalSection(&sample_list_mutex);
#else
	PTHREAD_cond_signal(&sample_list_condINSERTED);

	PTHREAD_mutex_unlock(&sample_list_mutex);
#endif

	sample_list_busy = 0;

	if(stop_threads) break;

	decode_frame(play.exinfo, sinfo);

   }/* while( !stop_threads && !mj2_stopped) */
#ifdef HAVE_WINPORT
	return 0;
#else
	return NULL;
#endif
}/* decoder_thread() */

static void threads_prelude(MJ2Extract *exinfo)
{
	int i;
#ifdef HAVE_WINPORT
	DWORD id;
#endif

#if defined(HAVE_NPTH_THREADS) && defined(_WIN32)
	PTHREAD_init();
#endif

#ifdef HAVE_WINPORT
	InitializeCriticalSection(&sample_list_mutex);

	InitializeConditionVariable(&sample_list_condINSERTED);
	InitializeConditionVariable(&sample_list_condGET);
#else
	PTHREAD_mutex_init(&sample_list_mutex, NULL);

	PTHREAD_cond_init(&sample_list_condINSERTED, NULL);
	PTHREAD_cond_init(&sample_list_condGET, NULL);
#endif

	exinfo->cur_sample = play_buf_nr;

	stop_threads = 0; threads_stopped = 0;

	decoder = (Decoder*)calloc(1, sizeof(Decoder) * play.nr_threads);

	if(decoder == NULL)
   {
	fprintf(stderr,"OPENMJ2.cxx:%d:\n\tmemory out\n",__LINE__);
	exit(1);
   }
	for(i = 0; i < play.nr_threads; ++i)
   {
	decoder[i].ID = i;
	decoder[i].exinfo = exinfo;
#ifdef HAVE_WINPORT
	decoder[i].tid =
	 CreateThread(NULL, 0, &decoder_thread, (PVOID)&decoder[i], 0, &id);
#else
	PTHREAD_create(&decoder[i].tid, NULL, &decoder_thread, 
		(void*)&decoder[i]);
#endif
   }
}/* threads_prelude() */

static void samples_postlude(void)
{
	SampleInfo *sinfo, *first;

	first = sample_list_head; sample_list_head = sample_list_tail = NULL;

	while((sinfo = first))
   {
	first = first->next;

	free_sample_info(sinfo);
   }
	elems_in_list = 0;
}

static void threads_postlude(MJ2Extract *exinfo)
{
	int i;

#ifdef HAVE_WINPORT
	WakeAllConditionVariable(&sample_list_condGET);
	WakeAllConditionVariable(&sample_list_condINSERTED);

	Sleep(WAIT_FOR_CONDVARS);
#else
	PTHREAD_cond_broadcast(&sample_list_condINSERTED);
	PTHREAD_cond_broadcast(&sample_list_condGET);

	PTHREAD_mutex_unlock(&sample_list_mutex);
#endif

	for(i = 0; i < play.nr_threads; ++i)
   {
#ifdef HAVE_WINPORT
	WaitForSingleObject(decoder[i].tid, INFINITE);
#else
	(void)PTHREAD_join(decoder[i].tid, NULL);
#endif
   }
	free(decoder);
	decoder = NULL;

#ifdef HAVE_WINPORT
#else
	PTHREAD_cond_destroy(&sample_list_condINSERTED);
	PTHREAD_cond_destroy(&sample_list_condGET);

	PTHREAD_mutex_destroy(&sample_list_mutex);
#endif
	threads_stopped = 1;

	exinfo->cur_sample = play_buf_nr + 1;

	samples_postlude();

#ifdef HAVE_WINPORT
	Sleep(1);
#else
	usleep(10000);
#endif

	stop_threads = 0;//single_step !

}/* threads_postlude() */

static unsigned char *get_rgb_buffer(void)
{
	SampleInfo *sinfo;
	unsigned char *s;
	int nr;

	s = NULL; nr = 0;

#ifdef HAVE_WINPORT
	EnterCriticalSection(&sample_list_mutex);
#else
	PTHREAD_mutex_lock(&sample_list_mutex);
#endif

	while( !stop_threads && !mj2_stopped
	&&( !sample_list_head || !sample_list_head->rgb_buf))
   {
#ifdef HAVE_WINPORT
	SleepConditionVariableCS(&sample_list_condINSERTED, &sample_list_mutex,
		INFINITE);
#else
	PTHREAD_cond_wait(&sample_list_condINSERTED, &sample_list_mutex);
#endif
   }
	sample_list_busy = 1;

	if(!stop_threads && !mj2_stopped)
   {
	sinfo = sample_list_head;

	if(sinfo)
  {
	sample_list_head = sample_list_head->next;

	if(sample_list_head == NULL) sample_list_tail = NULL;

	s = sinfo->rgb_buf; nr = sinfo->sample_nr;

	free(sinfo); --elems_in_list;
  }
	else
  {
	assert("INTERNAL ERROR" == NULL);
  }
   }
#ifdef HAVE_WINPORT
	WakeConditionVariable(&sample_list_condGET);

	LeaveCriticalSection(&sample_list_mutex);
#else
	PTHREAD_cond_signal(&sample_list_condGET);
	
	PTHREAD_mutex_unlock(&sample_list_mutex);
#endif

	sample_list_busy = 0;

	if(s)
   {
	play_buf_nr = nr;
   }

	return s;
}/* get_rgb_buffer() */

#endif /* HAVE_THREADS */

void OPENMJ2_free_tracks()
{
	int t, nr;

	if(Tracks == NULL) return;

	nr = Tracks[0].nr_tracks;

	for(t = 0; t < nr; ++t)
   {
	if(Tracks[t].samples)
  {
	free(Tracks[t].samples);
  }
   }
	free(Tracks);
	Tracks = NULL;
}

static MJ2Extract *extract_frames_open(const char *read_idf)
{
    FILE *reader;
    MJ2Extract *exinfo;
	track_t *track;

    if((reader = fopen(read_idf, "rb")) == NULL)
     return NULL;

    exinfo = (MJ2Extract*) calloc(1, sizeof(MJ2Extract));

	if(exinfo == NULL)
   {
	fprintf(stderr,"OPENMJ2.cxx:%d:\n\tmemory out\n",__LINE__);
	exit(1);
   }
	track = &Tracks[0];
    exinfo->duration = track->duration;
    exinfo->width = track->width;
    exinfo->height = track->height;
    exinfo->max_samples = track->max_samples;
	exinfo->max_tracks = track->max_tracks;
	exinfo->channels = 3;
    exinfo->reader = reader;
    exinfo->read_idf = read_idf;

#ifdef HAVE_FSEEKO
	exinfo->start_offset = ftello(reader);
#elif HAVE_FSEEKI64
	exinfo->start_offset = _ftelli64(reader);
#else
	exinfo->start_offset = ftell(reader);
#endif
    return exinfo;

}//extract_frames_open()

static void extract_frames_close(MJ2Extract *exinfo)
{
    if(exinfo->sample_buf) free(exinfo->sample_buf);

    fclose(exinfo->reader);

    free(exinfo);
}//extract_frames_close()

static void get_sample_buf(MJ2Extract *exinfo, SampleInfo *sinfo)
{
    sample_t *samples;
    unsigned char *sample_buf;
	const char *soc = "\xff\xd9";
	unsigned int size;
	int i;
	uint64_t pos;

	if(mj2_stopped) return;

    i = exinfo->cur_sample;

	if(i <= 0 || i == exinfo->max_samples)
   {
    i = 1;
#ifdef HAVE_FSEEKO
    fseeko(exinfo->reader, exinfo->start_offset, SEEK_SET);
#elif HAVE_FSEEKI64
    _fseeki64(exinfo->reader, exinfo->start_offset, SEEK_SET);
#else
    fseek(exinfo->reader, exinfo->start_offset, SEEK_SET);
#endif
   }
    exinfo->cur_sample = i + 1;
	
	samples = Tracks[exinfo->cur_track].samples;

	if(samples == NULL) return;

	size = samples[i].size;

	sample_buf = (unsigned char*)calloc(1, size+16);

	if(sample_buf == NULL)
   {
	fprintf(stderr,"OPENMJ2.cxx:%d:\n\tmemory out\n",__LINE__);
    return;
   }
//	Skip first two markers:
//
	pos = samples[i].pos + 8;

#ifdef HAVE_FSEEKO
	fseeko(exinfo->reader, pos, SEEK_SET);
#elif HAVE_FSEEKI64
	_fseeki64(exinfo->reader, pos, SEEK_SET);
#else
    fseek(exinfo->reader, (long)pos, SEEK_SET);
#endif
    fread(sample_buf, 1, size, exinfo->reader);
	memcpy(sample_buf + size, soc, 2);

    sinfo->cur_buf = sinfo->sample_buf = sample_buf;
    sinfo->sample_size = size + 2;
	sinfo->sample_nr = i;

}/* get_sample_buf() */

static void decode_frame(MJ2Extract *exinfo, SampleInfo *sinfo)
{
	int selected_component = 0;
	unsigned int width, height;
	int type;

	if(mj2_stopped) return;

    get_sample_buf(exinfo, sinfo);

	sinfo->rgb_buf = 
	JP2_decode(exinfo->read_idf, sinfo->sample_buf,
		(uint64_t)sinfo->sample_size, IS_MOVIE, J2K_CFMT,
		&width, &height, &type, &selected_component);

	(void)type;

	if(width != exinfo->width || height != exinfo->height)
   {
	FLViewer_url(exinfo->read_idf, width, height);
   }
	exinfo->width = width; exinfo->height = height;

	if(selected_component > 0)
	 exinfo->channels = 1;
	else
	 exinfo->channels = 3;

	free(sinfo->sample_buf); sinfo->sample_buf = NULL;

}/* decode_frame() */

static unsigned char* extract_single_frame(MJ2Extract *exinfo)
{
    SampleInfo sinfo;

	if(mj2_stopped) return NULL;

    memset(&sinfo, 0, sizeof(SampleInfo));

    decode_frame(exinfo, &sinfo);

    if(sinfo.rgb_buf)
   {
    exinfo->sample_nr = sinfo.sample_nr;
   }
    return sinfo.rgb_buf;

}/* extract_single_frame() */

static void free_sample_info(SampleInfo *sinfo)
{
    if(sinfo->sample_buf) free(sinfo->sample_buf);
    if(sinfo->rgb_buf) free(sinfo->rgb_buf);

	free(sinfo);
}
