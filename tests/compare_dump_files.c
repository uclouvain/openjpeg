/*
 * compare_dump_files.c
 *
 *  Created on: 25 juil. 2011
 *      Author: mickael
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "getopt.h"

typedef struct test_cmp_parameters
{
  /**  */
  char* base_filename;
  /**  */
  char* test_filename;
} test_cmp_parameters;

/*******************************************************************************
 * Command line help function
 *******************************************************************************/
void compare_dump_files_help_display() {
  fprintf(stdout,"\nList of parameters for the compare_dump_files function  \n");
  fprintf(stdout,"\n");
  fprintf(stdout,"  -b \t REQUIRED \t filename to the reference/baseline dump file \n");
  fprintf(stdout,"  -t \t REQUIRED \t filename to the test dump file image\n");
  fprintf(stdout,"\n");
}
/*******************************************************************************
 * Parse command line
 *******************************************************************************/
int parse_cmdline_cmp(int argc, char **argv, test_cmp_parameters* param)
{
  int sizemembasefile, sizememtestfile;
  int index;
  const char optlist[] = "b:t:";
  int c;

  // Init parameters
  param->base_filename = NULL;
  param->test_filename = NULL;

  opterr = 0;

  while ((c = getopt(argc, argv, optlist)) != -1)
    switch (c)
      {
      case 'b':
        sizemembasefile = (int)strlen(optarg)+1;
        param->base_filename = (char*) malloc(sizemembasefile);
        param->base_filename[0] = '\0';
        strncpy(param->base_filename, optarg, strlen(optarg));
        param->base_filename[strlen(optarg)] = '\0';
        //printf("param->base_filename = %s [%d / %d]\n", param->base_filename, strlen(param->base_filename), sizemembasefile );
        break;
      case 't':
        sizememtestfile = (int) strlen(optarg) + 1;
        param->test_filename = (char*) malloc(sizememtestfile);
        param->test_filename[0] = '\0';
        strncpy(param->test_filename, optarg, strlen(optarg));
        param->test_filename[strlen(optarg)] = '\0';
        //printf("param->test_filename = %s [%d / %d]\n", param->test_filename, strlen(param->test_filename), sizememtestfile);
       break;
      case '?':
        if ( (optopt == 'b') || (optopt == 't') )
          fprintf(stderr, "Option -%c requires an argument.\n", optopt);
        else
          if (isprint(optopt)) fprintf(stderr, "Unknown option `-%c'.\n", optopt);
          else fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
        return 1;
      default:
        fprintf(stderr, "WARNING -> this option is not valid \"-%c %s\"\n", c, optarg);
        break;
      }

  if (optind != argc)
    {
    for (index = optind; index < argc; index++)
      fprintf(stderr,"Non-option argument %s\n", argv[index]);
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
/*******************************************************************************
 * MAIN
 *******************************************************************************/
int main(int argc, char **argv)
{
  test_cmp_parameters inParam;
  FILE *fbase=NULL, *ftest=NULL;
  char chbase, chtest;
  int same = 1;
  unsigned long l=1, pos;

  if( parse_cmdline_cmp(argc, argv, &inParam) == EXIT_FAILURE )
    {
    compare_dump_files_help_display();
    if (!inParam.base_filename) free(inParam.base_filename);
    if (!inParam.test_filename) free(inParam.test_filename);
    return EXIT_FAILURE;
    }

  // Display Parameters
  printf("******Parameters********* \n");
  printf(" base_filename = %s\n"
          " test_filename = %s\n",
          inParam.base_filename, inParam.test_filename);
  printf("************************* \n");

  /* open base file */
  printf("Try to open: %s for reading ... ", inParam.base_filename);
  if((fbase = fopen(inParam.base_filename, "rb"))==NULL)
    {
    printf("Failed.\n");
    free(inParam.base_filename);
    free(inParam.test_filename);
    return EXIT_FAILURE;
    }
  printf("Ok.\n");

  /* open test file */
  printf("Try to open: %s for reading ... ", inParam.test_filename);
  if((ftest = fopen(inParam.test_filename, "rb"))==NULL)
    {
    printf("Failed.\n");
    fclose(fbase);
    free(inParam.base_filename);
    free(inParam.test_filename);
    return EXIT_FAILURE;
    }
  printf("Ok.\n");

  pos=ftell(fbase);

  while(!feof(fbase))
    {
    chbase = fgetc(fbase);
    if(ferror(fbase))
      {
      printf("Error reading base file.\n");
      return EXIT_FAILURE;
      }

    chtest = fgetc(ftest);
    if(ferror(ftest))
      {
      printf("Error reading test file.\n");
      return EXIT_FAILURE;
      }

    if(chbase != chtest)
      {
      size_t nbytes = 2048;
      char *strbase, *strtest, *strbase_d, *strtest_d;

      printf("Files differ at line %lu:\n", l);
      fseek(fbase,pos,SEEK_SET);
      fseek(ftest,pos,SEEK_SET);

      strbase = (char *) malloc(nbytes + 1);
      strtest = (char *) malloc(nbytes + 1);
      fgets(strbase, nbytes, fbase);
      fgets(strtest, nbytes, ftest);
      strbase_d = (char *) malloc(strlen(strbase));
      strtest_d = (char *) malloc(strlen(strtest));
      strncpy(strbase_d, strbase, strlen(strbase)-1);
      strncpy(strtest_d, strtest, strlen(strtest)-1);
      strbase_d[strlen(strbase)] = '\0';
      strtest_d[strlen(strtest)] = '\0';
      printf("<%s> vs. <%s>\n", strbase_d, strtest_d);

      free(strbase);free(strtest);
      free(strbase_d);free(strtest_d);
      same = 0;
      break;
      }
    else
      {
      if (chbase == '\n')
        {
        l++;
        pos = ftell(fbase);
        }
      }
    }

  //Close File
  fclose(fbase);
  fclose(ftest);

  // Free memory
  free(inParam.base_filename);
  free(inParam.test_filename);

  if(same)
    {
      printf("\n***** TEST SUCCEED: Files are the same. *****\n");
      return EXIT_SUCCESS;
    }
  else return EXIT_FAILURE;
}
