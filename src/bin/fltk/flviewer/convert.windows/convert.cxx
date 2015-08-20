#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <FL/fl_utf8.h>

int main(int argc, char *argv[])
{
	const char *fname;
	char *write_idf;
	FILE *reader, *writer;
	unsigned int slen;
	char s[128];
	char d[512];
	
	if(argc == 1) return 1;
	fname = argv[1];
	reader = fopen(fname, "r");
	if(reader == NULL)
   {
	printf("Can not open src file %s\n",fname);
	return 1;
   }
	slen = strlen(fname);
	write_idf = (char*)malloc(slen+8);
	strcpy(write_idf, fname); strcpy(write_idf+slen,"_utf8");
	printf("RESULT in:%s\n",write_idf);

	writer = fopen(write_idf, "w");

	while(fgets(s, 127, reader))
   {
	fl_utf8froma(d,512,s,strlen(s));

	fprintf(writer,"%s", d);
   }

	fclose(reader); fclose(writer); free(write_idf);

	return 0;
}
