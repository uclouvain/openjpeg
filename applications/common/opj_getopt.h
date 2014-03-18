/* last review : october 29th, 2002 */

#include "opj_config.h"

#ifndef _GETOPT_H_
#define _GETOPT_H_

typedef struct opj_option
{
	const char *name;
	int has_arg;
	int *flag;
	int val;
} opj_option_t;

#define	NO_ARG	0
#define REQ_ARG	1
#define OPT_ARG	2

#ifdef USE_SYSTEM_GETOPT
#include <getopt.h>

#define opj_opterr opterr
#define opj_optind optind
#define opj_optopt optopt
#define opj_optreset optreset
#define opj_optarg optarg

#define opj_getopt getopt
#define opj_getopt_long getopt_long

#else
extern int opj_opterr;
extern int opj_optind;
extern int opj_optopt;
extern int opj_optreset;
extern char *opj_optarg;

extern int opj_getopt(int nargc, char *const *nargv, const char *ostr);
extern int opj_getopt_long(int argc, char * const argv[], const char *optstring,
			const opj_option_t *longopts, int totlen);
extern void reset_options_reading(void);

#endif /* USE_SYSTEM_GETOPT */
#endif				/* _GETOPT_H_ */
