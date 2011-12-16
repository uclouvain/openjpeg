/*
 * Copyright (c) 2011, Mickael Savinaud, Communications & Systemes <mickael.savinaud@c-s.fr>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS `AS IS'
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * comparePGXimages.c
 *
 *  Created on: 8 juil. 2011
 *      Author: mickael
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>

#include "opj_config.h"
#include "opj_getopt.h"

#include "openjpeg.h"
#include "format_defs.h"
#include "convert.h"

double* parseToleranceValues( char* inArg, const int nbcomp);
void comparePGXimages_help_display(void);
opj_image_t* readImageFromFilePGX(char* filename, int nbFilenamePGX, char *separator);
#ifdef HAVE_LIBPNG
int imageToPNG(const opj_image_t* image, const char* filename, int num_comp_select);
#endif

typedef struct test_cmp_parameters
{
  /**  */
  char* base_filename;
  /**  */
  char* test_filename;
  /** Number of components */
  int nbcomp;
  /**  */
  double* tabMSEvalues;
  /**  */
  double* tabPEAKvalues;
  /**  */
  int nr_flag;
  /**  */
  char separator_base[2];
  /**  */
  char separator_test[2];

} test_cmp_parameters;

/*******************************************************************************
 * Command line help function
 *******************************************************************************/
void comparePGXimages_help_display(void) {
  fprintf(stdout,"\nList of parameters for the comparePGX function  \n");
  fprintf(stdout,"\n");
  fprintf(stdout,"  -b \t REQUIRED \t filename to the reference/baseline PGX image \n");
  fprintf(stdout,"  -t \t REQUIRED \t filename to the test PGX image\n");
  fprintf(stdout,"  -n \t REQUIRED \t number of component of the image (used to generate correct filename)\n");
  fprintf(stdout,"  -m \t OPTIONAL \t list of MSE tolerances, separated by : (size must correspond to the number of component) of \n");
  fprintf(stdout,"  -p \t OPTIONAL \t list of PEAK tolerances, separated by : (size must correspond to the number of component) \n");
  fprintf(stdout,"  -s \t OPTIONAL \t 1 or 2 filename separator to take into account PGX image with different components, "
                                      "please indicate b or t before separator to indicate respectively the separator "
                                      "for ref/base file and for test file.  \n");
  fprintf(stdout,"  -r \t OPTIONAL \t indicate if you want to run this function as conformance test or as non regression test\n");
  fprintf(stdout,"\n");
}

/*******************************************************************************
 * Parse command line
 *******************************************************************************/
int parse_cmdline_cmp(int argc, char **argv, test_cmp_parameters* param)
{
  char *MSElistvalues = NULL;  char *PEAKlistvalues= NULL;
  char *separatorList = NULL;
  int sizemembasefile, sizememtestfile;
  int index, flagM=0, flagP=0;
  const char optlist[] = "b:t:n:m:p:s:d";
  int c;

  /* Init parameters*/
  param->base_filename = NULL;
  param->test_filename = NULL;
  param->nbcomp = 0;
  param->tabMSEvalues = NULL;
  param->tabPEAKvalues = NULL;
  param->nr_flag = 0;

  opj_opterr = 0;

  while ((c = opj_getopt(argc, argv, optlist)) != -1)
    switch (c)
      {
      case 'b':
        sizemembasefile = (int)strlen(opj_optarg)+1;
        param->base_filename = (char*) malloc(sizemembasefile);
        param->base_filename[0] = '\0';
        strncpy(param->base_filename, opj_optarg, strlen(opj_optarg));
        param->base_filename[strlen(opj_optarg)] = '\0';
        /*printf("param->base_filename = %s [%d / %d]\n", param->base_filename, strlen(param->base_filename), sizemembasefile );*/
        break;
      case 't':
        sizememtestfile = (int) strlen(opj_optarg) + 1;
        param->test_filename = (char*) malloc(sizememtestfile);
        param->test_filename[0] = '\0';
        strncpy(param->test_filename, opj_optarg, strlen(opj_optarg));
        param->test_filename[strlen(opj_optarg)] = '\0';
        /*printf("param->test_filename = %s [%d / %d]\n", param->test_filename, strlen(param->test_filename), sizememtestfile);*/
       break;
      case 'n':
        param->nbcomp = atoi(opj_optarg);
        break;
      case 'm':
        MSElistvalues = opj_optarg;
        flagM = 1;
        break;
      case 'p':
        PEAKlistvalues = opj_optarg;
        flagP = 1;
        break;
      case 'd':
        param->nr_flag = 1;
        break;
      case 's':
        separatorList = opj_optarg;
        break;
      case '?':
        if ((opj_optopt == 'b') || (opj_optopt == 't') || (opj_optopt == 'n') || (opj_optopt == 'p') || (opj_optopt == 'm') || (opj_optopt
            == 's'))
          fprintf(stderr, "Option -%c requires an argument.\n", opj_optopt);
        else
          if (isprint(opj_optopt)) fprintf(stderr, "Unknown option `-%c'.\n", opj_optopt);
          else fprintf(stderr, "Unknown option character `\\x%x'.\n", opj_optopt);
        return 1;
      default:
        fprintf(stderr, "WARNING -> this option is not valid \"-%c %s\"\n", c, opj_optarg);
        break;
      }

  if (opj_optind != argc)
    {
    for (index = opj_optind; index < argc; index++)
      fprintf(stderr,"Non-option argument %s\n", argv[index]);
    return EXIT_FAILURE;
    }

  if (param->nbcomp == 0)
    {
    fprintf(stderr,"Need to indicate the number of components !\n");
    return EXIT_FAILURE;
    }
  else
    {
    if ( flagM && flagP )
      {
      param->tabMSEvalues = parseToleranceValues( MSElistvalues, param->nbcomp);
      param->tabPEAKvalues = parseToleranceValues( PEAKlistvalues, param->nbcomp);
      if ( (param->tabMSEvalues == NULL) || (param->tabPEAKvalues == NULL))
        {
        fprintf(stderr,"MSE and PEAK values are not correct (respectively need %d values)\n",param->nbcomp);
        return EXIT_FAILURE;
        }
      }
    /*else
      {

      }*/
    }

  /* Get separators after corresponding letter (b or t)*/
  if (separatorList != NULL)
    {
    if( (strlen(separatorList) ==2) || (strlen(separatorList) ==4) )
      {
      /* keep original string*/
      int sizeseplist = (int)strlen(separatorList)+1;
      char* separatorList2 = (char*)malloc( sizeseplist );
      separatorList2[0] = '\0';
      strncpy(separatorList2, separatorList, strlen(separatorList));
      separatorList2[strlen(separatorList)] = '\0';
      /*printf("separatorList2 = %s [%d / %d]\n", separatorList2, strlen(separatorList2), sizeseplist);*/

      if (strlen(separatorList) == 2) /* one separator behind b or t*/
        {
        char *resultT = NULL;
        resultT = strtok(separatorList2, "t");
        if (strlen(resultT) == strlen(separatorList)) /* didn't find t character, try to find b*/
          {
          char *resultB = NULL;
          resultB = strtok(resultT, "b");
          if (strlen(resultB) == 1)
            {
            param->separator_base[0] = separatorList[1];param->separator_base[1] = '\0';
            param->separator_test[0] ='\0';
            }
          else /* not found b*/
            {
            free(separatorList2);
            return EXIT_FAILURE;
            }
          }
        else /* found t*/
          {
          param->separator_base[0] ='\0';
          param->separator_test[0] = separatorList[1];param->separator_test[1] = '\0';
          }
        /*printf("sep b = %s [%d] and sep t = %s [%d]\n",param->separator_base, strlen(param->separator_base), param->separator_test, strlen(param->separator_test) );*/
        }
      else /* == 4 characters we must found t and b*/
        {
        char *resultT = NULL;
        resultT = strtok(separatorList2, "t");
        if (strlen(resultT) == 3) /* found t in first place*/
          {
          char *resultB = NULL;
          resultB = strtok(resultT, "b");
          if (strlen(resultB) == 1) /* found b after t*/
            {
            param->separator_test[0] = separatorList[1];param->separator_test[1] = '\0';
            param->separator_base[0] = separatorList[3];param->separator_base[1] = '\0';
            }
          else /* didn't find b after t*/
            {
            free(separatorList2);
            return EXIT_FAILURE;
            }
          }
        else /* == 2, didn't find t in first place*/
          {
          char *resultB = NULL;
          resultB = strtok(resultT, "b");
          if (strlen(resultB) == 1) /* found b in first place*/
            {
            param->separator_base[0] = separatorList[1]; param->separator_base[1] = '\0';
            param->separator_test[0] = separatorList[3]; param->separator_test[1] = '\0';
            }
          else /* didn't found b in first place => problem*/
            {
            free(separatorList2);
            return EXIT_FAILURE;
            }
          }
        }
      free(separatorList2);
      }
    else /* wrong number of argument after -s*/
      {
      return EXIT_FAILURE;
      }
    }
  else
    {
    if (param->nbcomp == 1)
      {
      param->separator_base[0] = '\0';
      param->separator_test[0] = '\0';
      }
    else
      {
      fprintf(stderr,"If number of component is > 1, we need separator\n");
      return EXIT_FAILURE;
      }
    }


  if ( (param->nr_flag) && (flagP || flagM) )
    {
    fprintf(stderr,"Wrong input parameters list: it is non-regression test or tolerance comparison\n");
    return EXIT_FAILURE;
    }
  if ( (!param->nr_flag) && (!flagP || !flagM) )
    {
    fprintf(stderr,"Wrong input parameters list: it is non-regression test or tolerance comparison\n");
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}

/*******************************************************************************
 * Parse MSE and PEAK input values (
 * separator = ":"
 *******************************************************************************/
double* parseToleranceValues( char* inArg, const int nbcomp)
{
  double* outArgs= malloc(nbcomp * sizeof(double));
  int it_comp = 0;
  char delims[] = ":";
  char *result = NULL;
  result = strtok( inArg, delims );

  while( (result != NULL) && (it_comp < nbcomp ))
    {
      outArgs[it_comp] = atof(result);
      result = strtok( NULL, delims );
      it_comp++;
    }

  if (it_comp != nbcomp)
  {
	free(outArgs);
    return NULL;
  }
  else
    return outArgs;
}
/*******************************************************************************
 * Create filenames from a filename by used separator and nb components
 * (begin to 0)
 *******************************************************************************/
char* createMultiComponentsFilename(const char* inFilename, const int indexF, const char* separator)
{
  char s[255];
  char *outFilename, *ptr;
  char token = '.';
  int posToken = 0;

  /*printf("inFilename = %s\n", inFilename);*/
  if ((ptr = strrchr(inFilename, token)) != NULL)
    {
    posToken = (int) (strlen(inFilename) - strlen(ptr));
    /*printf("Position of %c character inside inFilename = %d\n", token, posToken);*/
    }
  else
    {
    /*printf("Token %c not found\n", token);*/
    outFilename = (char*)malloc(1);
    outFilename[0] = '\0';
    return outFilename;
    }

  outFilename = (char*)malloc((posToken + 7) * sizeof(char)); /*6*/

  strncpy(outFilename, inFilename, posToken);

  outFilename[posToken] = '\0';

  strcat(outFilename, separator);

  sprintf(s, "%i", indexF);
  strcat(outFilename, s);

  strcat(outFilename, ".pgx");

  /*printf("outfilename: %s\n", outFilename);*/
  return outFilename;
}
/*******************************************************************************
 *
 *******************************************************************************/
opj_image_t* readImageFromFilePGX(char* filename, int nbFilenamePGX, char *separator)
{
  int it_file;
  opj_image_t* image_read = NULL;
  opj_image_t* image = NULL;
  opj_cparameters_t parameters;
  opj_image_cmptparm_t* param_image_read;
  int** data;

  /* If separator is empty => nb file to read is equal to one*/
  if ( strlen(separator) == 0 )
      nbFilenamePGX = 1;

  /* set encoding parameters to default values */
  opj_set_default_encoder_parameters(&parameters);
  parameters.decod_format = PGX_DFMT;
  strncpy(parameters.infile, filename, sizeof(parameters.infile)-1);

  /* Allocate memory*/
  param_image_read = malloc(nbFilenamePGX * sizeof(opj_image_cmptparm_t));
  data = malloc(nbFilenamePGX * sizeof(*data));

  it_file = 0;
  for (it_file = 0; it_file < nbFilenamePGX; it_file++)
    {
    /* Create the right filename*/
    char *filenameComponentPGX;
    if (strlen(separator) == 0)
      {
      filenameComponentPGX = malloc((strlen(filename) + 1) * sizeof(*filenameComponentPGX));
      strcpy(filenameComponentPGX, filename);
      }
    else
      filenameComponentPGX = createMultiComponentsFilename(filename, it_file, separator);

    /* Read the pgx file corresponding to the component */
    image_read = pgxtoimage(filenameComponentPGX, &parameters);
    if (!image_read)
    {
    	int it_free_data;
		fprintf(stderr, "Unable to load pgx file\n");

		free(param_image_read);

		for (it_free_data = 0; it_free_data < it_file; it_free_data++) {
			free(data[it_free_data]);
		}
		free(data);

		free(filenameComponentPGX);

		return NULL;
	}

    /* Set the image_read parameters*/
    param_image_read[it_file].x0 = 0;
    param_image_read[it_file].y0 = 0;
    param_image_read[it_file].dx = 0;
    param_image_read[it_file].dy = 0;
    param_image_read[it_file].h = image_read->comps->h;
    param_image_read[it_file].w = image_read->comps->w;
    param_image_read[it_file].bpp = image_read->comps->bpp;
    param_image_read[it_file].prec = image_read->comps->prec;
    param_image_read[it_file].sgnd = image_read->comps->sgnd;

    /* Copy data*/
    data[it_file] = malloc(param_image_read[it_file].h * param_image_read[it_file].w * sizeof(int));
    memcpy(data[it_file], image_read->comps->data, image_read->comps->h * image_read->comps->w * sizeof(int));

    /* Free memory*/
    opj_image_destroy(image_read);
    free(filenameComponentPGX);
    }

  image = opj_image_create(nbFilenamePGX, param_image_read, CLRSPC_UNSPECIFIED);
  for (it_file = 0; it_file < nbFilenamePGX; it_file++)
    {
    /* Copy data into output image and free memory*/
    memcpy(image->comps[it_file].data, data[it_file], image->comps[it_file].h * image->comps[it_file].w * sizeof(int));
    free(data[it_file]);
    }

  /* Free memory*/
  free(param_image_read);
  free(data);

  return image;
}

/*******************************************************************************
 *
 *******************************************************************************/
#ifdef HAVE_LIBPNG
int imageToPNG(const opj_image_t* image, const char* filename, int num_comp_select)
{
  opj_image_cmptparm_t param_image_write;
  opj_image_t* image_write = NULL;

  param_image_write.x0 = 0;
  param_image_write.y0 = 0;
  param_image_write.dx = 0;
  param_image_write.dy = 0;
  param_image_write.h = image->comps[num_comp_select].h;
  param_image_write.w = image->comps[num_comp_select].w;
  param_image_write.bpp = image->comps[num_comp_select].bpp;
  param_image_write.prec = image->comps[num_comp_select].prec;
  param_image_write.sgnd = image->comps[num_comp_select].sgnd;

  image_write = opj_image_create(1, &param_image_write, CLRSPC_GRAY);
  memcpy(image_write->comps->data, image->comps[num_comp_select].data, param_image_write.h * param_image_write.w * sizeof(int));

  imagetopng(image_write, filename);

  opj_image_destroy(image_write);

  return EXIT_SUCCESS;
}
#endif

/*******************************************************************************
 * MAIN
 *******************************************************************************/
int main(int argc, char **argv)
{
  test_cmp_parameters inParam;
  int it_comp, itpxl;
  int failed = 0;
  int nbFilenamePGXbase, nbFilenamePGXtest;
  char *filenamePNGtest= NULL, *filenamePNGbase = NULL, *filenamePNGdiff = NULL;
  int memsizebasefilename, memsizetestfilename, memsizedifffilename;
  int valueDiff = 0, nbPixelDiff = 0;
  double sumDiff = 0.0;
  /* Structures to store image parameters and data*/
  opj_image_t *imageBase = NULL, *imageTest = NULL, *imageDiff = NULL;
  opj_image_cmptparm_t* param_image_diff;

  /* Get parameters from command line*/
  if( parse_cmdline_cmp(argc, argv, &inParam) == EXIT_FAILURE )
    {
    comparePGXimages_help_display();
    if (inParam.tabMSEvalues) free(inParam.tabMSEvalues);
    if (inParam.tabPEAKvalues) free(inParam.tabPEAKvalues);
    if (inParam.base_filename) free(inParam.base_filename);
    if (inParam.test_filename) free(inParam.test_filename);
    return EXIT_FAILURE;
    }

  /* Display Parameters*/
  printf("******Parameters********* \n");
  printf(" base_filename = %s\n"
         " test_filename = %s\n"
         " nb of Components = %d\n"
         " Non regression test = %d\n"
         " separator Base = %s\n"
         " separator Test = %s\n",
         inParam.base_filename, inParam.test_filename, inParam.nbcomp,
         inParam.nr_flag, inParam.separator_base, inParam.separator_test);

  if ( (inParam.tabMSEvalues != NULL) && (inParam.tabPEAKvalues != NULL))
  {
    printf(" MSE values = [");
    for (it_comp = 0; it_comp < inParam.nbcomp; it_comp++)
      printf(" %f ", inParam.tabMSEvalues[it_comp]);
    printf("]\n");
    printf(" PEAK values = [");
    for (it_comp = 0; it_comp < inParam.nbcomp; it_comp++)
      printf(" %f ", inParam.tabPEAKvalues[it_comp]);
    printf("]\n");
    printf(" Non-regression test = %d\n", inParam.nr_flag);
    }

  if (strlen(inParam.separator_base) == 0)
    nbFilenamePGXbase = 0;
  else
    nbFilenamePGXbase = inParam.nbcomp;

  if (strlen(inParam.separator_test) == 0)
    nbFilenamePGXtest = 0;
  else
    nbFilenamePGXtest = inParam.nbcomp;

  printf(" NbFilename to generate from base filename = %d\n", nbFilenamePGXbase);
  printf(" NbFilename to generate from test filename = %d\n", nbFilenamePGXtest);
  printf("************************* \n");

  /*----------BASELINE IMAGE--------*/
  memsizebasefilename = (int)strlen(inParam.test_filename) + 1 + 5 + 2 + 4;
  memsizetestfilename = (int)strlen(inParam.test_filename) + 1 + 5 + 2 + 4;

  imageBase = readImageFromFilePGX( inParam.base_filename, nbFilenamePGXbase, inParam.separator_base);
  if ( imageBase != NULL)
    {
    filenamePNGbase = (char*) malloc(memsizebasefilename);
    filenamePNGbase[0] = '\0';
    strncpy(filenamePNGbase, inParam.test_filename, strlen(inParam.test_filename));
    filenamePNGbase[strlen(inParam.test_filename)] = '\0';
    strcat(filenamePNGbase, ".base");
    /*printf("filenamePNGbase = %s [%d / %d octets]\n",filenamePNGbase, strlen(filenamePNGbase),memsizebasefilename );*/
    }
  else
    {
    if (inParam.tabMSEvalues) free(inParam.tabMSEvalues);
    if (inParam.tabPEAKvalues) free(inParam.tabPEAKvalues);
    if (inParam.base_filename) free(inParam.base_filename);
    if (inParam.test_filename) free(inParam.test_filename);
    return EXIT_FAILURE;
    }

  /*----------TEST IMAGE--------*/

  imageTest = readImageFromFilePGX(inParam.test_filename, nbFilenamePGXtest, inParam.separator_test);
  if ( imageTest != NULL)
    {
    filenamePNGtest = (char*) malloc(memsizetestfilename);
    filenamePNGtest[0] = '\0';
    strncpy(filenamePNGtest, inParam.test_filename, strlen(inParam.test_filename));
    filenamePNGtest[strlen(inParam.test_filename)] = '\0';
    strcat(filenamePNGtest, ".test");
    /*printf("filenamePNGtest = %s [%d / %d octets]\n",filenamePNGtest, strlen(filenamePNGtest),memsizetestfilename );*/
    }
  else
    {
	if (imageBase) opj_image_destroy(imageBase);
    if (inParam.tabMSEvalues) free(inParam.tabMSEvalues);
    if (inParam.tabPEAKvalues) free(inParam.tabPEAKvalues);
    if (inParam.base_filename) free(inParam.base_filename);
    if (inParam.test_filename) free(inParam.test_filename);
    free(filenamePNGbase);
    return EXIT_FAILURE;
    }

  /*----------DIFF IMAGE--------*/

  /* Allocate memory*/
  param_image_diff = malloc( imageBase->numcomps * sizeof(opj_image_cmptparm_t));

  /* Comparison of header parameters*/
  printf("Step 1 -> Header comparison\n");

  for (it_comp = 0; it_comp < imageBase->numcomps; it_comp++)
    {
    param_image_diff[it_comp].x0 = 0;
    param_image_diff[it_comp].y0 = 0;
    param_image_diff[it_comp].dx = 0;
    param_image_diff[it_comp].dy = 0;

    if (imageBase->comps[it_comp].sgnd != imageTest->comps[it_comp].sgnd)
      {
      printf("ERROR: sign mismatch [comp %d] (%d><%d)\n", it_comp, ((imageBase->comps)[it_comp]).sgnd, ((imageTest->comps)[it_comp]).sgnd);
      failed = 1;
      }
    else
      param_image_diff[it_comp].sgnd = 0 ;

    if (((imageBase->comps)[it_comp]).prec != ((imageTest->comps)[it_comp]).prec)
      {
      printf("ERROR: prec mismatch [comp %d] (%d><%d)\n", it_comp, ((imageBase->comps)[it_comp]).prec, ((imageTest->comps)[it_comp]).prec);
      failed = 1;
      }
    else
      param_image_diff[it_comp].prec = 8 ;

    if (((imageBase->comps)[it_comp]).bpp != ((imageTest->comps)[it_comp]).bpp)
      {
      printf("ERROR: byte per pixel mismatch [comp %d] (%d><%d)\n", it_comp, ((imageBase->comps)[it_comp]).bpp, ((imageTest->comps)[it_comp]).bpp);
      failed = 1;
      }
    else
      param_image_diff[it_comp].bpp = 1 ;

    if (((imageBase->comps)[it_comp]).h != ((imageTest->comps)[it_comp]).h)
      {
      printf("ERROR: height mismatch [comp %d] (%d><%d)\n", it_comp, ((imageBase->comps)[it_comp]).h, ((imageTest->comps)[it_comp]).h);
      failed = 1;
      }
    else
      param_image_diff[it_comp].h = imageBase->comps[it_comp].h ;

    if (((imageBase->comps)[it_comp]).w != ((imageTest->comps)[it_comp]).w)
      {
      printf("ERROR: width mismatch [comp %d] (%d><%d)\n", it_comp, ((imageBase->comps)[it_comp]).w, ((imageTest->comps)[it_comp]).w);
      failed = 1;
      }
    else
      param_image_diff[it_comp].w = imageBase->comps[it_comp].w ;
    }

   /* If only one parameter is different, we stop the test*/
   if (failed)
     {
     free(inParam.tabMSEvalues);
     free(inParam.tabPEAKvalues);
     free(inParam.base_filename);
     free(inParam.test_filename);

     free(filenamePNGbase);
     free(filenamePNGtest);

     opj_image_destroy(imageBase);
     opj_image_destroy(imageTest);

     free(param_image_diff);

     return EXIT_FAILURE;
     }

   imageDiff = opj_image_create(imageBase->numcomps, param_image_diff, CLRSPC_UNSPECIFIED);
   /* Free memory*/
   free(param_image_diff);

   /* Measurement computation*/
   printf("Step 2 -> measurement comparison\n");

   memsizedifffilename = strlen(inParam.test_filename) + 1 + 5 + 2 + 4;
   filenamePNGdiff = (char*) malloc(memsizedifffilename);
   filenamePNGdiff[0] = '\0';
   strncpy(filenamePNGdiff, inParam.test_filename, strlen(inParam.test_filename));
   filenamePNGdiff[strlen(inParam.test_filename)] = '\0';
   strcat(filenamePNGdiff, ".diff");
   /*printf("filenamePNGdiff = %s [%d / %d octets]\n",filenamePNGdiff, strlen(filenamePNGdiff),memsizedifffilename );*/

   /* Compute pixel diff*/
   for (it_comp = 0; it_comp < imageDiff->numcomps; it_comp++)
     {
     double SE=0,PEAK=0;
     double MSE=0;
     char *filenamePNGbase_it_comp, *filenamePNGtest_it_comp, *filenamePNGdiff_it_comp;

     filenamePNGbase_it_comp = (char*) malloc(memsizebasefilename);
     filenamePNGbase_it_comp[0] = '\0';
     strncpy(filenamePNGbase_it_comp,filenamePNGbase,strlen(filenamePNGbase));
     filenamePNGbase_it_comp[strlen(filenamePNGbase)] = '\0';

     filenamePNGtest_it_comp = (char*) malloc(memsizetestfilename);
     filenamePNGtest_it_comp[0] = '\0';
     strncpy(filenamePNGtest_it_comp,filenamePNGtest,strlen(filenamePNGtest));
     filenamePNGtest_it_comp[strlen(filenamePNGtest)] = '\0';

     filenamePNGdiff_it_comp = (char*) malloc(memsizedifffilename);
     filenamePNGdiff_it_comp[0] = '\0';
     strncpy(filenamePNGdiff_it_comp,filenamePNGdiff,strlen(filenamePNGdiff));
     filenamePNGdiff_it_comp[strlen(filenamePNGdiff)] = '\0';

     for (itpxl = 0; itpxl < ((imageDiff->comps)[it_comp]).w * ((imageDiff->comps)[it_comp]).h; itpxl++)
       {
       if (abs( ((imageBase->comps)[it_comp]).data[itpxl] - ((imageTest->comps)[it_comp]).data[itpxl] ) > 0)
         {
         valueDiff = ((imageBase->comps)[it_comp]).data[itpxl] - ((imageTest->comps)[it_comp]).data[itpxl];
         ((imageDiff->comps)[it_comp]).data[itpxl] = abs(valueDiff);
         sumDiff += (double)valueDiff;
         nbPixelDiff++;

         SE += (double)(valueDiff * valueDiff);
         PEAK = (PEAK > abs(valueDiff)) ? PEAK : abs(valueDiff);
         }
       else
         ((imageDiff->comps)[it_comp]).data[itpxl] = 0;
       }/* h*w loop */

     MSE = SE / ( ((imageDiff->comps)[it_comp]).w * ((imageDiff->comps)[it_comp]).h );

     if (!inParam.nr_flag && (inParam.tabMSEvalues != NULL) && (inParam.tabPEAKvalues != NULL))
       { /* Conformance test*/
       printf("<DartMeasurement name=\"PEAK_%d\" type=\"numeric/double\"> %f </DartMeasurement> \n", it_comp, PEAK);
       printf("<DartMeasurement name=\"MSE_%d\" type=\"numeric/double\"> %f </DartMeasurement> \n", it_comp, MSE);

       if ( (MSE > inParam.tabMSEvalues[it_comp]) || (PEAK > inParam.tabPEAKvalues[it_comp]) )
         {
         printf("ERROR: MSE (%f) or PEAK (%f) values produced by the decoded file are greater "
                "than the allowable error (respectively %f and %f) \n",
                MSE, PEAK, inParam.tabMSEvalues[it_comp], inParam.tabPEAKvalues[it_comp]);
         failed = 1;
         }
       }
     else  /* Non regression-test */
       {
       if ( nbPixelDiff > 0)
         {
         char it_compc[255];
         it_compc[0] = '\0';

         printf("<DartMeasurement name=\"NumberOfPixelsWithDifferences_%d\" type=\"numeric/int\"> %d </DartMeasurement> \n", it_comp, nbPixelDiff);
         printf("<DartMeasurement name=\"ComponentError_%d\" type=\"numeric/double\"> %f </DartMeasurement> \n", it_comp, sumDiff);

#ifdef HAVE_LIBPNG
         sprintf(it_compc, "_%i", it_comp);
         strcat(it_compc,".png");
         strcat(filenamePNGbase_it_comp, it_compc);
         /*printf("filenamePNGbase_it = %s [%d / %d octets]\n",filenamePNGbase_it_comp, strlen(filenamePNGbase_it_comp),memsizebasefilename );*/
         strcat(filenamePNGtest_it_comp, it_compc);
         /*printf("filenamePNGtest_it = %s [%d / %d octets]\n",filenamePNGtest_it_comp, strlen(filenamePNGtest_it_comp),memsizetestfilename );*/
         strcat(filenamePNGdiff_it_comp, it_compc);
         /*printf("filenamePNGdiff_it = %s [%d / %d octets]\n",filenamePNGdiff_it_comp, strlen(filenamePNGdiff_it_comp),memsizedifffilename );*/

         if ( imageToPNG(imageBase, filenamePNGbase_it_comp, it_comp) == EXIT_SUCCESS )
           {
           printf("<DartMeasurementFile name=\"BaselineImage_%d\" type=\"image/png\"> %s </DartMeasurementFile> \n", it_comp, filenamePNGbase_it_comp);
           }

         if ( imageToPNG(imageTest, filenamePNGtest_it_comp, it_comp) == EXIT_SUCCESS )
           {
           printf("<DartMeasurementFile name=\"TestImage_%d\" type=\"image/png\"> %s </DartMeasurementFile> \n", it_comp, filenamePNGtest_it_comp);
           }

         if ( imageToPNG(imageDiff, filenamePNGdiff_it_comp, it_comp) == EXIT_SUCCESS )
           {
           printf("<DartMeasurementFile name=\"DiffferenceImage_%d\" type=\"image/png\"> %s </DartMeasurementFile> \n", it_comp, filenamePNGdiff_it_comp);
           }
#endif
         failed = 1;
         }
       }
     free(filenamePNGbase_it_comp);
     free(filenamePNGtest_it_comp);
     free(filenamePNGdiff_it_comp);
     } /* it_comp loop */

  /*-----------------------------*/
  /* Free memory */
  opj_image_destroy(imageBase);
  opj_image_destroy(imageTest);
  opj_image_destroy(imageDiff);

  free(filenamePNGbase);
  free(filenamePNGtest);
  free(filenamePNGdiff);

  free(inParam.tabMSEvalues);
  free(inParam.tabPEAKvalues);
  free(inParam.base_filename);
  free(inParam.test_filename);

  if (failed)
    return EXIT_FAILURE;
  else
    {
    printf("---- TEST SUCCEED ----\n");
    return EXIT_SUCCESS;
    }
}
