#include <stdio.h>
#ifdef WIN32
#include <malloc.h>
#else
#include <stdlib.h>
#endif
#include <setjmp.h>
#include <string.h>

#include "mj2.h"
#include <cio.h>
#include <j2k.h>
#include <int.h>

#define MJ2_MJ2   0x6d6a7032
#define MJ2_MJ2S  0x6d6a3273
#define JP2_JP2C  0x6a703263
#define MJ2_MDAT  0x6d646174

//MEMORY LEAK
#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>  // Must be included first
#include <crtdbg.h>
#endif
//MEM

jmp_buf j2k_error;

void j2k_read_siz_marker(FILE *file, j2k_image_t *j2k_img)
{
  int len,i;
  char buf, buf2[2];
  char *siz_buffer;
  
  fseek(file, 0, SEEK_SET);
  do {
    fread(&buf,1,1, file);
    if (buf==(char)0xff)
      fread(&buf,1,1, file);
  }
  while (!(buf==(char)0x51));
  
  fread(buf2,2,1,file);		/* Lsiz                */
  len = ((buf2[0])<<8) + buf2[1];
  
  siz_buffer = (char*) malloc(len * sizeof(char));
  fread(siz_buffer,len, 1, file);
  cio_init(siz_buffer,len);
  
  cio_read(2);			/* Rsiz (capabilities) */
  j2k_img->x1 = cio_read(4);	/* Xsiz                */
  j2k_img->y1 = cio_read(4);	/* Ysiz                */
  j2k_img->x0 = cio_read(4);	/* X0siz               */
  j2k_img->y0 = cio_read(4);	/* Y0siz               */
  cio_skip(16);			/* XTsiz, YTsiz, XT0siz, YT0siz        */
  
  j2k_img->numcomps = cio_read(2);	/* Csiz                */
  j2k_img->comps =
    (j2k_comp_t *) malloc(j2k_img->numcomps * sizeof(j2k_comp_t));
  for (i = 0; i < j2k_img->numcomps; i++) {
    int tmp;
    tmp = cio_read(1);		/* Ssiz_i          */
    j2k_img->comps[i].prec = (tmp & 0x7f) + 1;
    j2k_img->comps[i].sgnd = tmp >> 7;
    j2k_img->comps[i].dx = cio_read(1);	/* XRsiz_i         */
    j2k_img->comps[i].dy = cio_read(1);	/* YRsiz_i         */
    j2k_img->comps[i].resno_decoded = 0;	/* number of resolution decoded */
    j2k_img->comps[i].factor = 0;	/* reducing factor by component */
  }
  free(siz_buffer);
  fseek(file, 0, SEEK_SET);
}

void setparams(mj2_movie_t *movie, j2k_image_t *img) {
  int i, depth_0, depth, sign;
  
  movie->tk[0].sample_rate = 25;
  movie->tk[0].w = int_ceildiv(img->x1 - img->x0, img->comps[0].dx);
  movie->tk[0].h = int_ceildiv(img->y1 - img->y0, img->comps[0].dy);
  mj2_init_stdmovie(movie);
  
  movie->tk[0].depth = img->comps[0].prec;

  if (img->numcomps==3) {
    if ((img->comps[0].dx == 1) && (img->comps[1].dx == 1) && (img->comps[1].dx == 1)) 
      movie->tk[0].CbCr_subsampling_dx = 1;
    else if ((img->comps[0].dx == 1) && (img->comps[1].dx == 2) && (img->comps[1].dx == 2))
      movie->tk[0].CbCr_subsampling_dx = 2;
    else
      fprintf(stderr,"Image component sizes are incoherent\n");
    
    if ((img->comps[0].dy == 1) && (img->comps[1].dy == 1) && (img->comps[1].dy == 1)) 
      movie->tk[0].CbCr_subsampling_dy = 1;
    else if ((img->comps[0].dy == 1) && (img->comps[1].dy == 2) && (img->comps[1].dy == 2))
      movie->tk[0].CbCr_subsampling_dy = 2;
    else
      fprintf(stderr,"Image component sizes are incoherent\n");
  }
  
  movie->tk[0].sample_rate = 25;
  
  movie->tk[0].jp2_struct.numcomps = img->numcomps;	// NC  
  jp2_init_stdjp2(&movie->tk[0].jp2_struct);
  
  movie->tk[0].jp2_struct.w = int_ceildiv(img->x1 - img->x0, img->comps[0].dx);
  movie->tk[0].jp2_struct.h = int_ceildiv(img->y1 - img->y0, img->comps[0].dy);
  
  depth_0 = img->comps[0].prec - 1;
  sign = img->comps[0].sgnd;
  movie->tk[0].jp2_struct.bpc = depth_0 + (sign << 7);
  
  for (i = 1; i < img->numcomps; i++) {
    depth = img->comps[i].prec - 1;
    sign = img->comps[i].sgnd;
    if (depth_0 != depth)
      movie->tk[0].jp2_struct.bpc = 255;
  }
  
  for (i = 0; i < img->numcomps; i++)
    movie->tk[0].jp2_struct.comps[i].bpcc =
    img->comps[i].prec - 1 + (img->comps[i].sgnd << 7);
  
  if ((img->numcomps == 1 || img->numcomps == 3)
    && (movie->tk[0].jp2_struct.bpc != 255))
    movie->tk[0].jp2_struct.meth = 1;
  else
    movie->tk[0].jp2_struct.meth = 2;
    
  if (img->numcomps == 1)
    movie->tk[0].jp2_struct.enumcs = 17;  // Grayscale
  
  else   if ((img->comps[0].dx == 1) && (img->comps[1].dx == 1) && (img->comps[1].dx == 1) &&
    (img->comps[0].dy == 1) && (img->comps[1].dy == 1) && (img->comps[1].dy == 1)) 
    movie->tk[0].jp2_struct.enumcs = 16;    // RGB
  
  else   if ((img->comps[0].dx == 1) && (img->comps[1].dx == 2) && (img->comps[1].dx == 2) &&
    (img->comps[0].dy == 1) && (img->comps[1].dy == 2) && (img->comps[1].dy == 2)) 
    movie->tk[0].jp2_struct.enumcs = 18;  // YUV
  
  else
    movie->tk[0].jp2_struct.enumcs = 0;	// Unkown profile */
}

int main(int argc, char *argv[]) {
  
  unsigned int snum;
  mj2_movie_t movie;
  mj2_sample_t *sample;
  unsigned char* frame_codestream;
  FILE *mj2file, *j2kfile;
  char j2kfilename[50];
  char *buf;
  int offset, mdat_initpos;
  j2k_image_t img;
  int i;
  
  if (argc != 3) {
    printf("Bad syntax: Usage: MJ2_Wrapper source_location mj2_filename\n");
    printf("Example: MJ2_Wrapper input/input output.mj2\n");
    return 1;
  }
  
  mj2file = fopen(argv[2], "wb");
  
  if (!mj2file) {
    fprintf(stderr, "failed to open %s for writing\n", argv[2]);
    return 1;
  }
  
  // Initialing the movie (parameters used in the JP and FTYP boxes  
  movie.num_htk = 0;	  // No hint tracks
  movie.num_stk = 0;	  // No sound tracks
  movie.num_vtk = 1;	  // One video track  
  movie.tk = (mj2_tk_t*) malloc (sizeof(mj2_tk_t)); //Memory allocation for the video track
  movie.tk[0].sample = (mj2_sample_t*) malloc (sizeof(mj2_sample_t));
  movie.tk[0].chunk = (mj2_chunk_t *) malloc(sizeof(mj2_chunk_t));  
  movie.tk[0].track_type = 0;	  // Video track
  movie.tk[0].track_ID = 1;	  // Track ID = 1 
  movie.brand = MJ2_MJ2;  // One brand: MJ2
  movie.num_cl = 2;	  // Two compatible brands: MJ2 and MJ2S
  movie.cl = (unsigned int *) malloc(movie.num_cl * sizeof(unsigned int));
  movie.cl[0] = MJ2_MJ2;
  movie.cl[1] = MJ2_MJ2S;
  movie.minversion = 0;	  // Minimum version: 0
  
  // Writing JP, FTYP and MDAT boxes 
  buf = (char*) malloc (300 * sizeof(char)); // Assuming that the JP and FTYP
					     // boxes won't be longer than 300 bytes
  cio_init(buf , 300);
  mj2_write_jp();
  mj2_write_ftyp(&movie);
  mdat_initpos = cio_tell();
  cio_skip(4);
  cio_write(MJ2_MDAT, 4);	
  fwrite(buf,cio_tell(),1,mj2file);
  free(buf);
    
  // Insert each j2k codestream in a JP2C box  
  snum=0;
  offset = 0;  
  while(1)
  {
    sample = &movie.tk[0].sample[snum];
    sprintf(j2kfilename,"%s_%d.j2k",argv[1],snum);
    j2kfile = fopen(j2kfilename, "rb");
    if (!j2kfile) {
      if (snum==0) {  // Could not open a single codestream
	fprintf(stderr, "failed to open %s for reading\n",j2kfilename);
	return 1;
      }
      else {	      // Tried to open a inexistant codestream
	fprintf(stdout,"%d frames created\n",snum);
	break;
      }
    }
    // Calculating offset for samples and chunks
    offset += cio_tell();     
    sample->offset = offset;
    movie.tk[0].chunk[snum].offset = offset;  // There will be one sample per chunk
    
    // Calculating sample size
    fseek(j2kfile,0,SEEK_END);	
    sample->sample_size = ftell(j2kfile) + 8; // Sample size is codestream + JP2C box header
    fseek(j2kfile,0,SEEK_SET);
    
    // Reading siz marker of j2k image for the first codestream
    if (snum==0)	      
      j2k_read_siz_marker(j2kfile, &img);
    
    // Writing JP2C box header			    
    frame_codestream = (unsigned char*) malloc (sample->sample_size+8); 
    cio_init(frame_codestream, sample->sample_size);
    cio_write(sample->sample_size, 4);  // Sample size
    cio_write(JP2_JP2C, 4);	// JP2C
    
    // Writing codestream from J2K file to MJ2 file
    fread(frame_codestream+8,sample->sample_size-8,1,j2kfile);
    fwrite(frame_codestream,sample->sample_size,1,mj2file);
    cio_skip(sample->sample_size-8);
    
    // Ending loop
    fclose(j2kfile);
    snum++;
    movie.tk[0].sample = realloc(movie.tk[0].sample, (snum+1) * sizeof(mj2_sample_t));
    movie.tk[0].chunk = realloc(movie.tk[0].chunk, (snum+1) * sizeof(mj2_chunk_t));
    free(frame_codestream);
  }
  
  // Writing the MDAT box length in header
  offset += cio_tell();
  buf = (char*) malloc (4 * sizeof(char));
  cio_init(buf,4);
  cio_write(offset-mdat_initpos,4); // Write MDAT length in MDAT box header
  fseek(mj2file,(long)mdat_initpos,SEEK_SET);
  fwrite(buf,4,1,mj2file);
  fseek(mj2file,0,SEEK_END);
  free(buf);

  // Setting movie parameters
  movie.tk[0].num_samples=snum;
  movie.tk[0].num_chunks=snum;
  setparams(&movie, &img);

  // Writing MOOV box 
  i=1;
  buf = (char*) malloc (10000 * sizeof(char));
  cio_init(buf , i*10000);
  if (setjmp(j2k_error)) {
    i++;
    buf = realloc(buf,i*10000* sizeof(char));
    cio_init(buf , i*10000);
  }
  mj2_write_moov(&movie);
  fwrite(buf,cio_tell(),1,mj2file);

  // Ending program
  fclose(mj2file);
  free(img.comps);
  free(buf);
  mj2_memory_free(&movie);


  //MEMORY LEAK
  #ifdef _DEBUG
    _CrtDumpMemoryLeaks();
  #endif
  //MEM

  return 0;
}