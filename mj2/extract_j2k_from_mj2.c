#include <stdio.h>
#include <malloc.h>
#include <setjmp.h>

#include "mj2.h"

//MEMORY LEAK
#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>  // Must be included first
#include <crtdbg.h>
#endif
//MEM

jmp_buf j2k_error;

int main(int argc, char *argv[]) {

  int tnum;
  unsigned int snum;
  mj2_movie_t movie;
  mj2_tk_t *track;
  mj2_sample_t *sample;
  unsigned char* frame_codestream;
  FILE *file, *outfile;
  char outfilename[50];

  if (argc != 3) {
    printf("Bad syntax: Usage: MJ2_extractor mj2filename output_location\n"); 
    printf("Example: MJ2_extractor foreman.mj2 output/foreman\n");
    return 1;
  }
  
  file = fopen(argv[1], "rb");
  
  if (!file) {
    fprintf(stderr, "failed to open %s for reading\n", argv[1]);
    return 1;
  }

  if (mj2_read_struct(file, &movie)) // Creating the movie structure
    return 1;

  mj2_init_stdmovie(&movie);

  // Decode first video track 
  tnum = 0;
  while (movie.tk[tnum].track_type != 0)
    tnum ++;

  track = &movie.tk[tnum];

  fprintf(stdout,"Extracting %d frames from file...\n",track->num_samples);

  for (snum=0; snum < track->num_samples; snum++)
  {
    sample = &track->sample[snum];
    frame_codestream = (unsigned char*) malloc (sample->sample_size-8); // Skipping JP2C marker
    fseek(file,sample->offset+8,SEEK_SET);
    fread(frame_codestream,sample->sample_size-8,1, file);  // Assuming that jp and ftyp markers size do

    sprintf(outfilename,"%s_%d.j2k",argv[2],snum);
    outfile = fopen(outfilename, "wb");
    if (!outfile) {
      fprintf(stderr, "failed to open %s for writing\n",outfilename);
      return 1;
    }
    fwrite(frame_codestream,sample->sample_size-8,1,outfile);
    fclose(outfile);
    free(frame_codestream);
    }
  fclose(file);
  fprintf(stdout, "%d frames correctly extracted\n", snum);
  mj2_memory_free(&movie);

  //MEMORY LEAK
  #ifdef _DEBUG
    _CrtDumpMemoryLeaks();
  #endif
  //MEM

  return 0;
}