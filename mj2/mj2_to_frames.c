#include <stdio.h>
#include <malloc.h>
#include <setjmp.h>

#include "mj2.h"
#include "mj2_convert.h"
#include <openjpeg.h>

//MEMORY LEAK
#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>  // Must be included first
#include <crtdbg.h>
#endif
//MEM

jmp_buf j2k_error;

int main(int argc, char *argv[]) {

  unsigned int tnum, snum;
  mj2_movie_t movie;
  mj2_tk_t *track;
  mj2_sample_t *sample;
  unsigned char* frame_codestream;
  FILE *file, *outfile;
  char outfilename[50];
  j2k_image_t img;
  j2k_cp_t cp;
  int i;

  if (argc != 3) {
    printf("Bad syntax: Usage: MJ2_decoder inputfile.mj2 outputfile.yuv\n"); 
    printf("Example: MJ2_decoder foreman.mj2 foreman.yuv\n");
    return 1;
  }
  
  file = fopen(argv[1], "rb");
  
  if (!file) {
    fprintf(stderr, "failed to open %s for reading\n", argv[1]);
    return 1;
  }

  // Checking output file
  outfile = fopen(argv[2], "w");
  if (!file) {
    fprintf(stderr, "failed to open %s for writing\n", argv[2]);
    return 1;
  }
  fclose(outfile);

  if (mj2_read_struct(file, &movie)) // Creating the movie structure
    return 1;


  // Decode first video track 
  tnum = 0;
  while (movie.tk[tnum].track_type != 0)
    tnum ++;

  track = &movie.tk[tnum];

  // Output info on first video tracl
  fprintf(stdout,"The first video track contains %d frames.\nWidth: %d, Height: %d \n\n",
    track->num_samples, track->w, track->h);

  for (snum=0; snum < track->num_samples; snum++)
  {
    fprintf(stdout,"Frame %d: ",snum+1);
    sample = &track->sample[snum];
    frame_codestream = (unsigned char*) malloc (sample->sample_size-8); // Skipping JP2C marker
    fseek(file,sample->offset+8,SEEK_SET);
    fread(frame_codestream,sample->sample_size-8,1, file);  // Assuming that jp and ftyp markers size do

    if (!j2k_decode(frame_codestream, sample->sample_size-8, &img, &cp)) // Decode J2K to image
      return 1;

    if (((img.numcomps == 3) && (img.comps[0].dx == img.comps[1].dx / 2) 
      && (img.comps[0].dx == img.comps[2].dx / 2 ) && (img.comps[0].dx == 1)) 
      || (img.numcomps == 1)) {
      
      if (imagetoyuv(&img, &cp, argv[2]))	// Convert image to YUV
	return 1;
    }
    else if ((img.numcomps == 3) && 
      (img.comps[0].dx == 1) && (img.comps[1].dx == 1)&&
      (img.comps[2].dx == 1))// If YUV 4:4:4 input --> to bmp
    {
      fprintf(stdout,"The frames will be output in a bmp format (output_1.bmp, ...)\n");
      sprintf(outfilename,"output_%d.bmp",snum);
      if (imagetobmp(&img, &cp, outfilename))	// Convert image to YUV
	return 1;
      
    }
    else {
      fprintf(stdout,"Image component dimensions are unknown. Unable to output image\n");
      fprintf(stdout,"The frames will be output in a j2k file (output_1.j2k, ...)\n");

      sprintf(outfilename,"output_%d.j2k",snum);
      outfile = fopen(outfilename, "wb");
      if (!outfile) {
	fprintf(stderr, "failed to open %s for writing\n",outfilename);
	return 1;
      }
      fwrite(frame_codestream,sample->sample_size-8,1,outfile);
      fclose(outfile);
    }
    for (i=0; i<img.numcomps; i++)
      free(img.comps[i].data);
    j2k_dec_release();
    free(frame_codestream);
  }

  fclose(file);
  fprintf(stdout, "%d frame(s) correctly extracted\n", snum);
  mj2_memory_free(&movie);


  //MEMORY LEAK
  #ifdef _DEBUG
    _CrtDumpMemoryLeaks();
  #endif
  //MEM

  return 0;
}