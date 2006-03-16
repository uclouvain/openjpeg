

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <math.h>
#include "j2k.h"
#include "cio.h"
#include "jpw.h"

#define J2K_MS_SOC 0xff4f
#define J2K_MS_SIZ 0xff51
#define J2K_MS_SOT 0xff90
#define JPWL_MS_EPC 0xff97
#define JPWL_MS_EPB 0xff96
#define JPWL_MS_RED 0xff99
#define J2K_MS_EOC 0xffd9

#define J2K_STATE_MHSOC 0x0001
#define J2K_STATE_TPHSOT 0x0008
#define J2K_STATE_MH 0x0004
#define J2K_STATE_TPH 0x0010
#define J2K_STATE_MT 0x0020

#define mm  8           /* RS code over GF(2**4) - change to suit */

int pp [mm+1] = { 1, 0, 1, 1, 1, 0, 0, 0, 1 };

int *alpha_to, *index_of, *gg;
int *recd, *data, *bb;


static long crcSum;

static int CrcT16[256] =
{0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0};

static long CrcT32[256] = {0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9,
0x130476dc, 0x17c56b6b, 0x1a864db2, 0x1e475005,
0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61,
0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd,
0x4c11db70, 0x48d0c6c7, 0x4593e01e, 0x4152fda9,
0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011,
0x791d4014, 0x7ddc5da3, 0x709f7b7a, 0x745e66cd,
0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5,
0xbe2b5b58, 0xbaea46ef, 0xb7a96036, 0xb3687d81,
0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49,
0xc7361b4c, 0xc3f706fb, 0xceb42022, 0xca753d95,
0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1,
0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d,
0x34867077, 0x30476dc0, 0x3d044b19, 0x39c556ae,
0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16,
0x018aeb13, 0x054bf6a4, 0x0808d07d, 0x0cc9cdca,
0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde,
0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02,
0x5e9f46bf, 0x5a5e5b08, 0x571d7dd1, 0x53dc6066,
0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e,
0xbfa1b04b, 0xbb60adfc, 0xb6238b25, 0xb2e29692,
0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6,
0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a,
0xe0b41de7, 0xe4750050, 0xe9362689, 0xedf73b3e,
0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686,
0xd5b88683, 0xd1799b34, 0xdc3abded, 0xd8fba05a,
0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637,
0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb,
0x4f040d56, 0x4bc510e1, 0x46863638, 0x42472b8f,
0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47,
0x36194d42, 0x32d850f5, 0x3f9b762c, 0x3b5a6b9b,
0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,
0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623,
0xf12f560e, 0xf5ee4bb9, 0xf8ad6d60, 0xfc6c70d7,
0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f,
0xc423cd6a, 0xc0e2d0dd, 0xcda1f604, 0xc960ebb3,
0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7,
0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b,
0x9b3660c6, 0x9ff77d71, 0x92b45ba8, 0x9675461f,
0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640,
0x4e8ee645, 0x4a4ffbf2, 0x470cdd2b, 0x43cdc09c,
0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8,
0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24,
0x119b4be9, 0x155a565e, 0x18197087, 0x1cd86d30,
0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088,
0x2497d08d, 0x2056cd3a, 0x2d15ebe3, 0x29d4f654,
0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0,
0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c,
0xe3a1cbc1, 0xe760d676, 0xea23f0af, 0xeee2ed18,
0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0,
0x9abc8bd5, 0x9e7d9662, 0x933eb0bb, 0x97ffad0c,
0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668,
0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4};

typedef struct {
	unsigned int id;
	unsigned int lid;
	unsigned char *pid;
} id_tecn;

typedef struct {
	unsigned int lepc;
	unsigned int pcrc;
	unsigned long cl;
	unsigned char pepc;
	id_tecn *tecn;   
} EPC_par;

typedef struct {
	unsigned int lepb;
	unsigned char depb;
	unsigned long ldpepb;
	unsigned long pepb;
	unsigned int ldata;
} EPB_par;
typedef struct	//***********Questa struttura non dovrebbe servire effettivamente!!!!
{
	unsigned int lesd;
	unsigned char cesd1;
	unsigned int cesd2;
	unsigned char pesd;
} ESD_MS;
typedef struct
{
	unsigned int lred;
	unsigned char pred;
	unsigned char *reddata;
} RED;


EPC_par epc;
ESD_MS *esd;
RED red;
int lmex, nbckpar, epbpm, next, startsot;
unsigned long psot;

unsigned char *cssrc;
unsigned int cslen;  
int cspos;			 
unsigned int csread; 

int redpos; 
int decodeflag; // 1 if there are not errors decoding RS codeword
unsigned long redlen; 
int redmode; // Se vale 0 allora RED in MH, se vale 1 allora RED in MH e nei vari TPH
int redlenok; 
int nepbrd; // umber of EPBs already read
int lastepb; // Is 1 if the current EPB is the last of an header


// This is the main function which parses the CS searching JPWL marker segments
int decode_JPWL(unsigned char *src, int len)
{
	unsigned int temp, nepb, j2k_state, pos, nepc, posi, mem, rest;
	int err; 
	unsigned long psot, i;
	FILE *f,*g;

	cssrc = src;  
	cslen = len;  
	redpos = 0;   
	redlen = 0;   
	redlenok = 0;
	redmode = 0;  
				  

	csread = 0;	

	temp = cio_read(2);
    if (temp != J2K_MS_SOC)
	{
			printf("Expected marker SOT\n");
			return 0;
	}
	
	j2k_state = 0; 
	nepc = 0; 
	nepbrd = 0;
	lastepb = 0;

	while (j2k_state != J2K_STATE_MT)
	{
		temp = cio_read(2);  // Read SIZ or SOT
		if (temp >> 8 != 0xff) {
			fprintf(stderr, "%.8x: expected a marker instead of %x\n",
			    cio_tell() - 2, temp);
			return nepc;
		}

		posi = cio_tell();  // Start of Lsiz or Lsot
		
		if (temp == J2K_MS_SIZ) 
		{
			temp = cio_read(2); // Read Lsiz
			
			lmex = temp + 17;  // Length of data to be protected
			nbckpar = (int)ceil((double)lmex / 96); 
									
			
		}
		else  
		{     
			nbckpar = 1;
			temp = cio_read(2); 
		}
		cio_skip(temp-2);

		temp = cio_read(2);  // Read EPB or EPC if there are
		if (temp != JPWL_MS_EPC)
		{
			temp = cio_read(2); // Lepb
			cio_skip(temp-2);
			temp = cio_read(2); // EPC
			pos = cio_tell();

			if ((temp != JPWL_MS_EPC)&&(nepc == 0))
			{
				cio_seek(0);
				return nepc;  // NOT JPWL CS
			}
			
			if ((temp != JPWL_MS_EPC)&&(nepc != 0))  // Current TPH doesn't have EPC
			{
				cio_seek(posi); // Lsot
				cio_skip(4);
				psot = cio_read(4);
				if (psot == 0)  // Last TPH
					j2k_state = J2K_STATE_MT;  // Next step skip out of the cicle
				cio_seek(posi-2);
				cio_skip(psot); // End of data of the current TP
				if (cio_read(2) == J2K_MS_EOC)
					j2k_state = J2K_STATE_MT;
				cio_skip(-2); 
			}
			if (temp == JPWL_MS_EPC) // There is EPB
			{
				if (nepc == 0)
				{
					j2k_state = J2K_STATE_MHSOC;
					cio_seek(posi-4);   // SOC
					next = cio_tell();  
				}
				if (nepc != 0)
				{
					j2k_state = J2K_STATE_TPHSOT;
					cio_seek(posi-2); // SOT
					next = cio_tell();
				}

				red.reddata = (char *) malloc(len * sizeof(char));
																
				
				
				mem = next;
				i = 0;
				if (!(rest = read_EPB_2(&j2k_state)))// First EPB
					return nepc;
				i += rest;
				temp = cio_tell(); // End of EPB
				cio_seek(pos); // End of EPC
				err = read_EPC();  // Read the first EPC
				nepc++;				// Handle the case of more then on EPC
				nepb = epc.tecn[0].lid / 4;  // Number of EPBs in the CS
				while (i<nepb)
				{
					if ((j2k_state == J2K_STATE_MH)&&(lastepb == 0))
					{	
						cio_seek(temp);
						do
						{
							if (!(rest = read_EPB_2(&j2k_state)))
								return nepc;
							i += rest;
						}
						while (lastepb == 0);
					}
					if (j2k_state == J2K_STATE_TPHSOT)
					{
						cio_seek(temp); // SOT
						pos = cio_tell();
						do
						{
							if (!(rest = read_EPB_2(&j2k_state)))
								return nepc;
							i += rest;
						
						}
						while (lastepb == 0);
						
					}
					if (j2k_state == J2K_STATE_MH)
					{
						temp = cio_read(2);
						
						while (temp != J2K_MS_SOT)
						{
							cio_skip(cio_read(2)-2);
							temp = cio_read(2);
						
						}
						cio_skip(-2);
					}
					temp = cio_read(2);
				
					if (temp != J2K_MS_EOC)
					{
						if ((j2k_state == J2K_STATE_TPH)||(temp != J2K_MS_SOT))
						{
							cio_seek(pos);
							cio_skip(6);
							psot = cio_read(4);
							cio_seek(pos);
							cio_skip(psot);
							j2k_state = J2K_STATE_TPHSOT;
						}
						if (temp == J2K_MS_SOT)
						{
							cio_skip(-2);
							temp = cio_tell();
							j2k_state = J2K_STATE_TPHSOT;
						}
					}
				}
				// All EPBs read
				
				while (temp != J2K_MS_EOC)
				{
					cio_seek(pos);
					cio_skip(6);
					psot = cio_read(4);
					cio_seek(pos);
					cio_skip(psot);
					temp = cio_read(2);
					
				}
				cio_skip(-2); // EOC
				j2k_state = J2K_STATE_MT;
				
			}
		}
		else // There is EPC after SIZ
		{
			err = read_EPC();
		
			if (nepc == 0)
			{
				cio_seek(posi); 
				cio_skip(cio_read(2)-2);  
				temp = cio_read(2); 
				
				while (temp != J2K_MS_SOT)
				{
					cio_skip(cio_read(2)-2); 
					temp = cio_read(2);  
				
				}
				cio_skip(-2); // End of Main-Header
			}
			if (nepc != 0)
			{
				cio_seek(posi); 
				cio_skip(4);
				psot = cio_read(4);
				if (psot == 0)  // Last TPH
					j2k_state = J2K_STATE_MT;  
				cio_seek(posi-2);
				cio_skip(psot); 
				if (cio_read(2) == J2K_MS_EOC)
				{
					j2k_state = J2K_STATE_MT;
				    cio_skip(-2); 
				}
			}
			
			nepc++;
		}
	} 

	
	cio_seek(0);

	if ((redlen != 0)&&(redmode==0))
		{
			red.lred = redlen + 3; 
			red.pred = 0x43; // Pred = 01000011 
			// Go to End of MH
			temp = cio_read(2); // SOC
			
		
			while (temp != J2K_MS_SOT)
			{
				cio_skip(2);
				cio_skip(cio_read(2)-2);
				temp = cio_read(2);
				
				cio_skip(-2);
			}
			
			insert_RED(cio_tell(),red.lred+2,redlenok);
			g = fopen("output","wb");
			if (g==NULL)
				printf("Unable to open file!\n");
			cio_seek(0);
			for (i=0; i<(epc.cl+redlen+5); i++)
				fputc(cio_read(1),g);
			fclose(g);
			cslen = epc.cl + redlen + 5;
		
		}
	else
	{
		f = fopen("output","wb");
		if (f==NULL)
			printf("Unable to open file!\n");
		cio_seek(0);
		
		for (i=0; i<cslen; i++)
			fputc(cio_read(1),f);
		fclose(f);
	}

	cio_seek(0);
	
	return nepc;
	}

int read_EPC() 
{
	
	unsigned int id, lid;  
							   
	int i, h, pos, nid;
	unsigned int ltec, count;
	unsigned char *buff;


	pos = cio_tell()-2; // EPC position

	
	epc.lepc = cio_read(2);
	epc.pcrc = cio_read(2);
	epc.cl = cio_read(4);
	epc.pepc = cio_read(1);
	if ((epc.pepc>>4)&1)	// One or more ESDs
	{
		esd = (ESD_MS *) malloc(10 * sizeof(ESD_MS)); 
	}
	ltec = epc.lepc - 9; 
	count = 0;
	nid = 0; // umber of techniques used
	while (count<ltec)
	{
		id = cio_read(2);
		count += 2;
		lid = cio_read(2);
		count += 2 + lid;
		cio_skip(lid); // Skip over Pid field
		nid ++;
	} 
	
	epc.tecn = (id_tecn *) malloc(nid * sizeof(id_tecn));
	cio_seek(pos + 11); 
	for (i=0; i<nid; i++)
	{
		epc.tecn[i].id = cio_read(2);
		epc.tecn[i].lid = cio_read(2);
		epc.tecn[i].pid = (char *) malloc(epc.tecn[i].lid * sizeof(char));
		for (h=0; h<epc.tecn[i].lid; h++)
				epc.tecn[i].pid[h] = cio_read(1);
	}

	
	
	// Check CRC
	buff = (char *) malloc(epc.lepc* sizeof(char));
	cio_seek(pos); 
	for (i=0; i<4; i++)
		buff[i] = cio_read(1);  
	
	cio_skip(2);
	for (i=4; i<epc.lepc; i++)
		buff[i] = cio_read(1);
	
	
	ResetCRC();
	for (i=0; i < epc.lepc; i++){
		UpdateCRC16(buff[i]);	
	}
	
	if (crcSum == epc.pcrc)
		return 1;  // CRC correct
	else
		return 0;  // Errors!
}

int read_EPB_2(int *j2k_state) // Returns the number of EPBs read
{
	unsigned int lepb, lsiz, temp, ldata, lsot, lbuf;
	int posdata, posdata2, nblock, i,h, pos, lante, lpar;
	int nn, kk, tt, nn1, kk1; 
	int nepbpm, posfirst, posend, count;
	unsigned long lpack, ldpread;
	EPB_par *epb;
	unsigned long ldpepb, pepb, ndata, datacrc;  
	unsigned char depb;
	unsigned char *buff;
	int lparity, packall; // If packall=1 all the following EPBs are packed


	if (*j2k_state == J2K_STATE_MHSOC)  
		{	
			cio_skip(4); //  Lsiz
			lsiz = cio_read(2);
			cio_skip(lsiz-2); // EPB
			pos = cio_tell(); 
			temp = cio_read(2);  
		
			nn = 160; kk = 64; tt = 48;  // RS(160,64)
			lante = lsiz+4; 
			
		} 

		if (*j2k_state == J2K_STATE_TPHSOT)
		{
			
			
			startsot = cio_tell(); // SOT
			cio_skip(2); //  Lsot
			lsot = cio_read(2);
			cio_skip(2); 
			psot = cio_read(4); 
			cio_skip(-6); 
			
			cio_skip(lsot-2); // EPB
			pos = cio_tell(); 
			temp = cio_read(2);
		
			nn = 80; kk = 25; tt = 28;  // RS(80,25)
			lante = lsot+2;
		}

		if ((*j2k_state == J2K_STATE_MH)||(*j2k_state == J2K_STATE_TPH))
		{
			
			pos = cio_tell(); // EPB
			temp = cio_read(2); 
			nn = 40; kk = 13; tt = 14;  // RS(40,13)
			lante = 0;
		}

    // Decode using default RS codes

	nn1 = 255; kk1 = kk + (nn1 - nn);
	alpha_to = (int *) malloc((nn1+1)*sizeof(int));
	index_of = (int *) malloc((nn1+1)*sizeof(int));
	gg = (int *) malloc((nn1-kk1+1)*sizeof(int));
	recd = (int *) malloc((nn1)*sizeof(int)); 
	data = (int *) malloc((kk1)*sizeof(int)); 
	bb = (int *) malloc((nn1-kk1)*sizeof(int));

	lepb = cio_read(2); 
	cio_skip(9);  // EPB data field
	posdata = cio_tell();
	ldata = lepb - 11;  // Length of data field
	
	lpar = nn - kk; // Length of parity bytes
	if (*j2k_state == J2K_STATE_MHSOC)  
	{
		lpar = nbckpar * (nn-kk);
	}
	else
		nbckpar = 1;
	buff = (char *) malloc(nn1 * sizeof(char)); 

	for (i=0; i<nbckpar; i++)
	{
		for (h=0; h<nn1; h++)
			buff[h] = 0; 
	
		// Put parity bytes at the beginning of the buffer

		write_buff(buff,posdata+i*(nn-kk),(nn-kk)); // Put in buffer parity bytes
		cio_seek(next + i*kk);  // Start of protected data
		if (i<(nbckpar -1))
		{
			for (h=(nn-kk); h<nn; h++)
			{
				buff[h] = cio_read(1);  // Put in buffer data bytes
			}
		}
		else
		{
			if (*j2k_state == J2K_STATE_MHSOC)
			{
				ndata = lmex - ((nbckpar-1) * kk); // Handle last block case
				for (h=(nn-kk); h<((nn-kk)+ndata); h++)
				{
					buff[h] = cio_read(1);
				}
			}
			else
				for (h=(nn-kk); h<nn; h++)
					buff[h] = cio_read(1);  
		}
		
		for (h=0; h<nn1; h++)
			recd[h] = buff[h];  

		

		generate_gf(nn1,kk1) ;
		gen_poly(nn1,kk1) ;
		for (h=0; h<nn1; h++)
			recd[h] = index_of[recd[h]] ; 
		decode_rs(nn1,kk1,tt); 

		if (decodeflag == 0) 
		{
			
			cio_init(red.reddata,cslen); 
			cio_seek(redpos);

			
			cio_write(next + i*kk,4); // Start byte
			redlen += 4;
			if (i<(nbckpar -1))
				cio_write(next + i*kk + kk - 1,4);  // End byte
			else
			{
				if (*j2k_state == J2K_STATE_MHSOC)
					cio_write(next + i*kk + ndata - 1,4);  // End byte
				else
					cio_write(next + i*kk + kk - 1,4);
			}
			redlen += 4;
			// 0xFFFF stands for errors occurred
			cio_write(0xFFFF,2);
			redlen += 2;
			if ((*j2k_state == J2K_STATE_MHSOC)||(*j2k_state == J2K_STATE_MH))
				redlenok+=10;
			
			redpos = cio_tell(); 
			
		}

		

		// Copy recd[] content in the CS

		cio_init(cssrc, cslen); 
		
		cio_seek(posdata+i*(nn-kk)); 
		for (h=0; h<(nn-kk); h++)
			cio_write(recd[h],1); 
		cio_seek(next + i*kk);
		
		if (i<(nbckpar -1))
		{
			for (h=(nn-kk); h<nn; h++)
				cio_write(recd[h],1);  
		}
		else
		{
			if (*j2k_state == J2K_STATE_MHSOC)
				for (h=(nn-kk); h<(nn-kk)+ndata; h++)
				{
					cio_write(recd[h],1); 
				}
			else
				for (h=(nn-kk); h<nn; h++)
				cio_write(recd[h],1);  
		}
	} 

	// Decode with the remaining EPB parity bytes

	
	cio_seek(pos);  // EPB
	
	temp = cio_read(2);  
			if (temp != JPWL_MS_EPB)
			{
				
				
				return 0;
				
				
			}

	
	cio_skip(2); 
	depb = cio_read(1); 
	
	ldpepb = cio_read(4); 

	pepb = cio_read(4); 

	

	if (nepbrd!=0)
	{
		temp = cio_tell();
		cio_init(epc.tecn[0].pid, epc.tecn[0].lid);
		cio_seek(nepbrd*4);
		if (pepb!=cio_read(4))
		{
			cio_skip(-4);
			pepb=cio_read(4); 
		}
		cio_init(cssrc, cslen);
		cio_seek(temp);
	}
	
	ldata = ldata - nbckpar*(nn-kk);  
	
	cio_seek(posdata + nbckpar*(nn-kk)); 
	posdata2 = cio_tell(); 
	if (ldpepb == 0)
		next = cio_tell();
	

	if (!((depb >> 6)&1))  // Not the last EPB
		lastepb = 0;
	if ((depb >> 6)&1)  // Last EPB of the TPH
		lastepb = 1;

	if (!((depb >> 7)&1))  // Unpacked mode
	{
		
		if ((!((depb >> 6)&1))&&((*j2k_state == J2K_STATE_MHSOC)||(*j2k_state == J2K_STATE_MH)))
			*j2k_state = J2K_STATE_MH; 
		
		if ((!((depb >> 6)&1))&&((*j2k_state == J2K_STATE_TPHSOT)||(*j2k_state == J2K_STATE_TPH)))
			*j2k_state = J2K_STATE_TPH; 

		nepbrd++;
	
		

		if (pepb)  // If Pepb=0 => default RS codes
		{
			if ((pepb>>28)==2)
			{
				
				free(alpha_to);
				free(index_of);
				free(gg);
				free(recd);
				free(data);
				free(bb);
				/***********/
				kk = (int) pepb & 0x000000ff;
			    nn = (int) (pepb>>8) & 0x000000ff;
				tt = (int) ceil((double)(nn-kk)/2);
				nn1 = 255; kk1 = kk + (nn1 - nn);
				alpha_to = (int *) malloc((nn1+1)*sizeof(int));
				index_of = (int *) malloc((nn1+1)*sizeof(int));
				gg = (int *) malloc((nn1-kk1+1)*sizeof(int));
				recd = (int *) malloc((nn1)*sizeof(int)); 
				data = (int *) malloc((kk1)*sizeof(int)); 
				bb = (int *) malloc((nn1-kk1)*sizeof(int));
				generate_gf(nn1,kk1) ;
				gen_poly(nn1,kk1) ;
			}
	
			if ((pepb>>28)==1)
			{
				// CRC decode
				free(buff);
				buff = (char *) malloc(ldpepb * sizeof(char));
				write_buff(buff,posdata2+ldata,ldpepb);
				if (pepb & 0x00000001)	// CRC 32
				{
					
					ResetCRC();
					for (i=0; i < ldpepb; i++)
						UpdateCRC32(reflectByte(buff[i]));	
					reflectCRC32();
					crcSum ^= 0xffffffff;	// 1's complement
					cio_seek(posdata2);
					datacrc = cio_read(4);
					
				}
				else	// CRC 16
				{
					ResetCRC();
					for (i=0; i < ldpepb; i++)
						UpdateCRC16(buff[i]);	
					cio_seek(posdata2);
					datacrc = cio_read(2);
					
				}
				free(buff);
				cio_seek(posdata2 + ldata + ldpepb);
				next = cio_tell();
				

				temp = cio_read(2);
				if (temp == J2K_MS_SOT)
					*j2k_state = J2K_STATE_TPHSOT;
				if (temp == J2K_MS_EOC)
					*j2k_state = J2K_STATE_MT;
				cio_skip(-2);
				
				return 1;
				
			}
	
			if (pepb>=0x30000000)
			{
				// RA techniques
				cio_seek(posdata2);
				return 1;
				
				
			}
	
			if (pepb==0xffffffff)
			{
				
				cio_seek(posdata2);
				return 1;
				
			}
		}
	
		
	
		
		
	    
		nblock = ldata / (nn-kk);  // Number of CW
		
		free(buff);
		buff = (char *) malloc(nn1 * sizeof(char));
		for (i=0; i<nblock; i++)
		{
			
			for (h=0; h<nn1; h++)
				buff[h] = 0; 
			write_buff(buff,posdata2+i*(nn-kk),(nn-kk)); // Parity bytes
			cio_seek(posdata2+ldata+i*kk); 
		
			if (i<(nblock-1))
			{
				for (h=(nn-kk); h<nn; h++)
				{
					buff[h] = cio_read(1);  // Data bytes
					
				}
				
			}
			else
			{
				ndata = ldpepb - ((nblock-1) * kk);  // Last CW
				
				for (h=(nn-kk); h<(nn-kk)+ndata; h++)
				{
					buff[h] = cio_read(1);  // Data
				
				}
				
				next = cio_tell();  
				
				if (cio_read(2) == 0xffd9)
					*j2k_state = J2K_STATE_MT;
				cio_skip(-2);
			}
			
			for (h=0; h<nn1; h++)
				recd[h] = buff[h];  

		
	        
			generate_gf(nn1,kk1) ;
			gen_poly(nn1,kk1) ;
			for (h=0; h<nn1; h++)
				recd[h] = index_of[recd[h]] ;  
			decode_rs(nn1,kk1,tt);  
			
			if (decodeflag == 0) 
			{
				
				cio_init(red.reddata,cslen); 
				cio_seek(redpos);

				
				cio_write(posdata2+ldata+i*kk,4); // Start byte
				redlen += 4;
				if (i<(nblock -1))
					cio_write(posdata2+ldata+i*kk + kk - 1,4);  //End byte
				else
					cio_write(posdata2+ldata+i*kk + ndata - 1,4);  // End byte
				redlen += 4;
	
				cio_write(0xFFFF,2);
				redlen += 2;
				if ((*j2k_state == J2K_STATE_MH)||(*j2k_state == J2K_STATE_TPHSOT))
					redlenok+=10;
				redpos = cio_tell(); 
				
			}
			if ((redlen==0)&(redmode==1))
				free(red.reddata);
			
			

			
			
			cio_init(cssrc, cslen); 
			
			cio_seek(posdata2+i*(nn-kk));  // Parity block
			for (h=0; h<(nn-kk); h++)
			{
				cio_write(recd[h],1);  // Copy parity bytes
				
			}
			
			
			cio_seek(posdata2+ldata+i*kk);
			if (i<(nblock-1))
			{
				
				for (h=(nn-kk); h<nn; h++)
					cio_write(recd[h],1);  // Copy data bytes
			}
			else
			{
				ndata = ldpepb - (nblock-1) * kk;  
				
				for (h=(nn-kk); h<(nn-kk)+ndata; h++)
					cio_write(recd[h],1);  
			}
		}

		temp = cio_read(2);
		if (temp == J2K_MS_SOT)
			*j2k_state = J2K_STATE_TPHSOT;
		if (temp == J2K_MS_EOC)
			*j2k_state = J2K_STATE_MT;
		cio_skip(-2);

		
		free(alpha_to);
		free(index_of);
		free(gg);
		free(recd);
		free(data);
		free(bb);
		free(buff);
		return 1;
		
	}
	else	// Packed mode
	{
		if (*j2k_state == J2K_STATE_TPHSOT)
			packall = 1;
		else
			packall = 0;
	
		posfirst = pos;
		cio_seek(pos+2); 
		cio_skip(cio_read(2)-2); 
		pos = cio_tell();
		cio_skip(2); 
		
		
		

	
		nepbpm = 1;

		free(alpha_to);
		free(index_of);
		free(gg);
		free(recd);
		free(data);
		free(bb);
		nn = 40; kk = 13;
		nn1 = 255; kk1 = kk + (nn1 - nn);
		alpha_to = (int *) malloc((nn1+1)*sizeof(int));
		index_of = (int *) malloc((nn1+1)*sizeof(int));
		gg = (int *) malloc((nn1-kk1+1)*sizeof(int));
		recd = (int *) malloc((nn1)*sizeof(int)); 
		data = (int *) malloc((kk1)*sizeof(int)); 
		bb = (int *) malloc((nn1-kk1)*sizeof(int));

		
	
		while (lastepb == 0)
		{

		lepb = cio_read(2); 
		cio_skip(9);  // EPB data field
		posdata = cio_tell();
		ldata = lepb - 11;  
		lbuf = 13 + (nn-kk);  
		buff = (char *) malloc(nn1 * sizeof(char)); 
		for (i=0; i<nn1; i++)
			buff[i] = 0; 
		
		

		
		write_buff(buff,posdata,(nn-kk)); // Parity bytes
		
		cio_seek(pos);  // Start of protected data
		for (i=(nn-kk); i<lbuf; i++)
		{
			buff[i] = cio_read(1);  // Data bytes
		}

		for (i=0; i<nn1; i++)
			recd[i] = buff[i];  

		
		generate_gf(nn1,kk1) ;
		gen_poly(nn1,kk1) ;
		for (i=0; i<nn1; i++)
			recd[i] = index_of[recd[i]] ; 
		decode_rs(nn1,kk1,tt);  

		if (decodeflag == 0) 
			{
				
				cio_init(red.reddata,cslen);
				cio_seek(redpos);

				
				cio_write(pos,4); // Start byte
				cio_write(pos + lbuf - (nn - kk) - 1,4);  // End byte
				
				cio_write(0xFFFF,2);
				redlen += 10;
				redpos = cio_tell(); 
			}
		

		cio_init(cssrc, cslen);

		cio_seek(posdata); // Start of data
		for (i=0; i<(nn-kk); i++)
			cio_write(recd[i],1);  // Parity bytes
		cio_seek(pos);
		for (i=(nn-kk); i<lbuf; i++)
			cio_write(recd[i],1);  // Data bytes
		

		

		
		cio_seek(pos);  // EPB
		temp = cio_read(2);  
				if (temp != JPWL_MS_EPB)
				{
					
					return 0;
				
				}

		cio_skip(2); 
		depb = cio_read(1); 
		
		if (!((depb >> 6)&1))  // Not the last EPB of the TPH
			nepbpm += 1;
		if ((depb >> 6)&1)  // Last EPB of the TPH
		{
			nepbpm += 1;
			lastepb = 1;
		}

		cio_skip(-3); 
		cio_skip(cio_read(2)-2); 
		pos = cio_tell(); // Next EPB
		
		cio_skip(2);

		

		} 
	

		// Decode EPB parity bytes protecting J2K data

		
		cio_skip(-2);
		posend = cio_tell();
		
		lpack = posend-posfirst; // Total EPBs length
		epb = (EPB_par *) malloc(nepbpm * sizeof(EPB_par));
		cio_seek(posfirst);
		
		for (count=0; count<nepbpm; count++)
		{
			cio_skip(2); 
			epb[count].lepb = cio_read(2); 
			epb[count].depb = cio_read(1); 
			epb[count].ldpepb = cio_read(4); 
			epb[count].pepb = cio_read(4); 

			temp = cio_tell();
			cio_init(epc.tecn[0].pid, epc.tecn[0].lid);
			cio_seek((nepbrd+count)*4);
			if (epb[count].pepb!=cio_read(4))
			{
				cio_skip(-4);
				epb[count].pepb=cio_read(4); 
			}
			cio_init(cssrc, cslen);
			cio_seek(temp);

			if ((count == 0)&&(packall == 1))
				epb[count].ldata = (epb[count].lepb - 11) - (80-25);
			else
				epb[count].ldata = (epb[count].lepb - 11) - (nn-kk);  
			cio_skip(-11); 
			cio_skip(epb[count].lepb); 
		} 

		
		nepbrd+=nepbpm;

		cio_seek(posfirst);  // First of the packed EPBs
		pos = cio_tell();
		ldpread = 0;
		lparity = nn - kk;
		
		for (count=0; count<nepbpm; count++)
		{
			cio_seek(pos);
			
			cio_skip(13); // Data field
			posdata = cio_tell();
			if ((count == 0)&&(packall == 1))
				cio_seek(posdata + (80 - 25));
			else
				cio_seek(posdata + lparity); 
			posdata2 = cio_tell(); 
			
			
			
			
			if (epb[count].pepb)  // If Pepb=0 => default RS codes
			{
				if ((epb[count].pepb>>28)==2)
				{
					
					free(alpha_to);
					free(index_of);
					free(gg);
					free(recd);
					free(data);
					free(bb);
					/***********/
					kk = (int) epb[count].pepb & 0x000000ff;
					nn = (int) (epb[count].pepb>>8) & 0x000000ff;
					tt = (int) ceil((double)(nn-kk)/2);
					nn1 = 255; kk1 = kk + (nn1 - nn);
					alpha_to = (int *) malloc((nn1+1)*sizeof(int));
					index_of = (int *) malloc((nn1+1)*sizeof(int));
					gg = (int *) malloc((nn1-kk1+1)*sizeof(int));
					recd = (int *) malloc((nn1)*sizeof(int)); 
					data = (int *) malloc((kk1)*sizeof(int)); 
					bb = (int *) malloc((nn1-kk1)*sizeof(int));
					generate_gf(nn1,kk1) ;
					gen_poly(nn1,kk1) ;
				}

				if ((epb[count].pepb>>28)==1)
				{
					// CRC decode
					free(buff);
					buff = (char *) malloc(epb[count].ldpepb * sizeof(char));
					write_buff(buff,posdata2+epb[count].ldata,epb[count].ldpepb);//***Correggi!!!!!!!!!!!
					if (epb[count].pepb & 0x00000001)	// CRC 32
					{
						
						ResetCRC();
						cio_seek(posend+ldpread); 
						for (i=0; i < epb[count].ldpepb; i++)
							UpdateCRC32(reflectByte(buff[i]));	
						reflectCRC32();
						if (lastepb==1)
						{
							next = startsot + psot; 
							cio_seek(next);
						}
						if ((cio_read(2) == 0xffd9)||(psot == 0))
							*j2k_state = J2K_STATE_MT;
						cio_skip(-2);
				
						crcSum ^= 0xffffffff;	
						cio_seek(posdata2);
						datacrc = cio_read(4);
						
						
					}
					else	// CRC 16
					{
						ResetCRC();
						cio_seek(posend+ldpread); 
						for (i=0; i < epb[count].ldpepb; i++)
							UpdateCRC16(buff[i]);
						if (lastepb==1)
						{
							next = startsot + psot;
							cio_seek(next);
						}
						if ((cio_read(2) == 0xffd9)||(psot == 0))
							*j2k_state = J2K_STATE_MT;
						cio_skip(-2);
						cio_seek(posdata2);
						datacrc = cio_read(2);
					}
				}

				if (epb[count].pepb>=0x30000000)
				{
					next = cio_tell();
					// RA
				}

				if (epb[count].pepb==0xffffffff)
				{
					next = cio_tell();
					
				}
			}
			
			if (((epb[count].pepb>>28)==2)||(epb[count].pepb==0))  // RS codes
			{
			
			nblock = epb[count].ldata / (nn-kk);  // Number of CW
			free(buff);
			buff = (char *) malloc(nn1 * sizeof(char));
			
			for (i=0; i<nblock; i++)
			{
				
				for (h=0; h<nn1; h++)
					buff[h] = 0;
				write_buff(buff,posdata2+i*(nn-kk),(nn-kk));
				
				cio_seek(posend+ldpread+i*kk); 
				if (i<(nblock-1))
				{
					for (h=(nn-kk); h<nn; h++)
					{
						buff[h] = cio_read(1); 
					}
				}
				else
				{
					ndata = epb[count].ldpepb - ((nblock-1) * kk); 
					for (h=(nn-kk); h<(nn-kk)+ndata; h++)
					{
						buff[h] = cio_read(1); 
					}
					if (lastepb==1)
					{
						next = cio_tell();
					
					}
					
					if ((cio_read(2) == 0xffd9)||(psot == 0))
						*j2k_state = J2K_STATE_MT;
					cio_skip(-2);
				}
			
				for (h=0; h<nn1; h++)
					recd[h] = buff[h]; 
	        
			

				generate_gf(nn1,kk1) ;
				gen_poly(nn1,kk1) ;
				for (h=0; h<nn1; h++)
					recd[h] = index_of[recd[h]] ;
				decode_rs(nn1,kk1,tt);  

				if (decodeflag == 0)
				{
					
					cio_init(red.reddata,cslen); 
					cio_seek(redpos);

					
					cio_write(posend+ldpread+i*kk,4);
					

					if (i<(nblock -1))
						cio_write(posend+ldpread+i*kk + kk - 1,4);  
					else
						cio_write(posend+ldpread+i*kk + ndata - 1,4); 
				
					cio_write(0xFFFF,2);
					redlen+=10;
					redpos = cio_tell(); 
					
				}

				

				cio_init(cssrc, cslen);

				cio_seek(posdata2+i*(nn-kk)); 
				for (h=0; h<(nn-kk); h++)
					cio_write(recd[h],1);
			
				
				cio_seek(posend+ldpread+i*kk);
				if (i<(nblock-1))
				{
					for (h=(nn-kk); h<nn; h++)
						cio_write(recd[h],1); 
				}
				else
				{
					ndata = epb[count].ldpepb - (nblock-1) * kk; 
					for (h=(nn-kk); h<(nn-kk)+ndata; h++)
						cio_write(recd[h],1); 
				}
			}

			} 
		
		

			ldpread += epb[count].ldpepb;
			cio_seek(pos+2);
			cio_skip(epb[count].lepb);
			pos = cio_tell(); 

		} 

		
		cio_seek(next);
		
		temp = cio_read(2);
		if (temp == J2K_MS_SOT)
			*j2k_state = J2K_STATE_TPHSOT;
		if (temp == J2K_MS_EOC)
			*j2k_state = J2K_STATE_MT;
		cio_skip(-2);

		free(alpha_to);
		free(index_of);
		free(gg);
		free(recd);
		free(data);
		free(bb);
		free(buff);
		
		return nepbpm; // Number of packed EPBs read
	}	
		

}

int read_EPB(int next, int *j2k_state)  
{
	unsigned int lepb, lsiz, temp, ldata, lsot;
	int posdata, posdata2, nblock, i,h, pos, lante, lpar;
	int nn, kk, tt, nn1, kk1;
	
	unsigned long ldpepb, pepb, ndata, datacrc;  
	unsigned char depb;
	unsigned char *buff;
	int prova;
	
	cio_seek(next);
	
		if (*j2k_state == J2K_STATE_MHSOC)  
		{
			
		
			cio_skip(4); 
			lsiz = cio_read(2);
			cio_skip(lsiz-2); 
			pos = cio_tell(); 
			temp = cio_read(2);
			
			nn = 160; kk = 64; tt = 48;  // RS(160,64)
			lante = lsiz+4; 
			
		}

		if (*j2k_state == J2K_STATE_TPHSOT)
		{
			
			
			startsot = cio_tell(); 
			cio_skip(2); 
			lsot = cio_read(2);
			cio_skip(2); 
			psot = cio_read(4);
			cio_skip(-6); 
			
			cio_skip(lsot-2); 
			pos = cio_tell(); 
			temp = cio_read(2);
			
			nn = 80; kk = 25; tt = 28;  // RS(80,25)
			lante = lsot+2;
		}

		if ((*j2k_state == J2K_STATE_MH)||(*j2k_state == J2K_STATE_TPH))
		{
			
			pos = cio_tell(); 
			temp = cio_read(2); 
			nn = 40; kk = 13; tt = 14;  // RS(40,13)
			lante = 0;
		}

   

	
	nn1 = 255; kk1 = kk + (nn1 - nn);
	alpha_to = (int *) malloc((nn1+1)*sizeof(int));
	index_of = (int *) malloc((nn1+1)*sizeof(int));
	gg = (int *) malloc((nn1-kk1+1)*sizeof(int));
	recd = (int *) malloc((nn1)*sizeof(int)); 
	data = (int *) malloc((kk1)*sizeof(int)); 
	bb = (int *) malloc((nn1-kk1)*sizeof(int));

	
	lepb = cio_read(2); 
	
	cio_skip(9);
	posdata = cio_tell();
	
	ldata = lepb - 11;  
	
	lpar = nn - kk;
	if (*j2k_state == J2K_STATE_MHSOC)  
	{
		lpar = nbckpar * (nn-kk);
	}
	if (*j2k_state == J2K_STATE_TPHSOT)
		nbckpar = 1;

	buff = (char *) malloc(nn1 * sizeof(char)); 

	for (i=0; i<nbckpar; i++)
	{
	
		for (h=0; h<nn1; h++)
			buff[h] = 0;
	
		

		write_buff(buff,posdata+i*(nn-kk),(nn-kk)); // Parity bytes
		cio_seek(next + i*kk);  // Start of protected data
		if (i<(nbckpar -1))
		{
			for (h=(nn-kk); h<nn; h++)
			{
				buff[h] = cio_read(1);  // Data bytes
			}
		}
		else
		{
			if (*j2k_state == J2K_STATE_MHSOC)
			{
				ndata = lmex - ((nbckpar-1) * kk); 
				for (h=(nn-kk); h<((nn-kk)+ndata); h++)
				{
					buff[h] = cio_read(1);
				}
			}
			else
				for (h=(nn-kk); h<nn; h++)
					buff[h] = cio_read(1);  
				
			
		}
		

		for (h=0; h<nn1; h++)
			recd[h] = buff[h];  

	
		

		generate_gf(nn1,kk1) ;
		gen_poly(nn1,kk1) ;
		for (h=0; h<nn1; h++)
			recd[h] = index_of[recd[h]] ; 
		decode_rs(nn1,kk1,tt); 

		if (decodeflag == 0) 
		{
			
			cio_init(red.reddata,cslen); 
			cio_seek(redpos);

			
			cio_write(next + i*kk,4); 
			redlen += 4;
			if (i<(nbckpar -1))
				cio_write(next + i*kk + kk,4);  
			else
			{
				if (*j2k_state == J2K_STATE_MHSOC)
					cio_write(next + i*kk + ndata,4); 
				else
					cio_write(next + i*kk + kk,4);
			}
			redlen += 4;
		
			cio_write(0xFFFF,2);
			redlen += 2;
			if ((*j2k_state == J2K_STATE_MHSOC)||(*j2k_state == J2K_STATE_MH))
				redlenok+=10;
			
			redpos = cio_tell(); 
			
		}

	

		

		cio_init(cssrc, cslen);
		
		cio_seek(posdata+i*(nn-kk));
		for (h=0; h<(nn-kk); h++)
			cio_write(recd[h],1); 
		cio_seek(next + i*kk);
		if (i<(nbckpar -1))
		{
			for (h=(nn-kk); h<nn; h++)
				cio_write(recd[h],1);  
		}
		else
		{
			if (*j2k_state == J2K_STATE_MHSOC)
				for (h=(nn-kk); h<(nn-kk)+ndata; h++)
				{
					cio_write(recd[h],1);  
				}
			else
				for (h=(nn-kk); h<nn; h++)
				cio_write(recd[h],1); 
		}
	} 

	

	
	cio_seek(pos);  // EPB
	temp = cio_read(2);  //
			if (temp != JPWL_MS_EPB)
			{
				
				
				return 0;
				
				
			}

	
	cio_skip(2);
	depb = cio_read(1); 
	
	ldpepb = cio_read(4); 
	
	pepb = cio_read(4);
	if (nepbrd!=0)
	{
		temp = cio_tell();
		cio_init(epc.tecn[0].pid, epc.tecn[0].lid);
		cio_seek(nepbrd*4);
		if (pepb!=cio_read(4))
		{
			cio_skip(-4);
			pepb=cio_read(4); 
		}
		cio_init(cssrc, cslen);
		cio_seek(temp);
	}
	
	ldata = ldata - nbckpar*(nn-kk);
	
	cio_seek(posdata + nbckpar*(nn-kk)); 
	posdata2 = cio_tell();


	if ((!((depb >> 6)&1))&&((*j2k_state == J2K_STATE_MHSOC)||(*j2k_state == J2K_STATE_MH)))
		*j2k_state = J2K_STATE_MH; 
	if (((depb >> 6)&1)&&((*j2k_state == J2K_STATE_MH)||(*j2k_state == J2K_STATE_MHSOC)))
		*j2k_state = J2K_STATE_TPHSOT;
	if (((depb >> 6)&1)&&((*j2k_state == J2K_STATE_TPH)||(*j2k_state == J2K_STATE_TPHSOT)))
		*j2k_state = J2K_STATE_TPHSOT; 
	if ((!((depb >> 6)&1))&&((*j2k_state == J2K_STATE_TPHSOT)||(*j2k_state == J2K_STATE_TPH)))
		*j2k_state = J2K_STATE_TPH;

	if (!((depb >> 7)&1))
		epbpm = 1; // Unpacked mode
	else
		epbpm = 0; // Packed mode


	nepbrd++;
	
	
	if (pepb)
	{
		if ((pepb>>28)==2)
		{
			
			free(alpha_to);
			free(index_of);
			free(gg);
			free(recd);
			free(data);
			free(bb);
			
			kk = (int) pepb & 0x000000ff;
		    nn = (int) (pepb>>8) & 0x000000ff;
			tt = (int) ceil((double)(nn-kk)/2);
			nn1 = 255; kk1 = kk + (nn1 - nn);
			alpha_to = (int *) malloc((nn1+1)*sizeof(int));
			index_of = (int *) malloc((nn1+1)*sizeof(int));
			gg = (int *) malloc((nn1-kk1+1)*sizeof(int));
			recd = (int *) malloc((nn1)*sizeof(int)); 
			data = (int *) malloc((kk1)*sizeof(int)); 
			bb = (int *) malloc((nn1-kk1)*sizeof(int));
			generate_gf(nn1,kk1) ;
			gen_poly(nn1,kk1) ;
		}

		if ((pepb>>28)==1)
		{
			//  CRC
			free(buff);
			buff = (char *) malloc(ldpepb * sizeof(char));
			write_buff(buff,posdata2+ldata,ldpepb);
			if (pepb & 0x00000001)	// CRC 32
			{
				
				ResetCRC();
				for (i=0; i < ldpepb; i++)
					UpdateCRC32(reflectByte(buff[i]));	
				reflectCRC32();
				crcSum ^= 0xffffffff;	
				cio_seek(posdata2);
				datacrc = cio_read(4);
				if (datacrc == crcSum)
					printf("CRC corretto!\n");
				else
					printf("CRC errato!\n");
			}
			else	// CRC 16
			{
				ResetCRC();
				for (i=0; i < ldpepb; i++)
					UpdateCRC16(buff[i]);	
				cio_seek(posdata2);
				datacrc = cio_read(2);
				if (datacrc == crcSum)
					printf("CRC corretto!\n");
				else
					printf("CRC errato!\n");
			}
			free(buff);
			return (posdata2 + ldata + ldpepb); 
			
		}

		if (pepb>=0x30000000)
		{
			//RA
			return (posdata2 + ldata + ldpepb);
			
		}

		if (pepb==0xffffffff)
		{
			
			return (posdata2 + ldata + ldpepb);
			
		}
	}

	

	

	
    
	nblock = ldata / (nn-kk);  // Number of CW
	
	free(buff);
	buff = (char *) malloc(nn1 * sizeof(char));
	for (i=0; i<nblock; i++)
	{
		
		for (h=0; h<nn1; h++)
			buff[h] = 0; 
		write_buff(buff,posdata2+i*(nn-kk),(nn-kk)); 
		cio_seek(posdata2+ldata+i*kk); 
		
		
		if (i<(nblock-1))
		{
			
			for (h=(nn-kk); h<nn; h++)
			{
				buff[h] = cio_read(1);  
			
			}
			
		}
		else
		{
			ndata = ldpepb - ((nblock-1) * kk);  
			
			for (h=(nn-kk); h<(nn-kk)+ndata; h++)
			{
				buff[h] = cio_read(1);  
				
			}
			
			next = cio_tell(); 
			if (cio_read(2) == 0xffd9)
				*j2k_state = J2K_STATE_MT;
			cio_skip(-2);
		}
		
		for (h=0; h<nn1; h++)
			recd[h] = buff[h];  

		
		generate_gf(nn1,kk1) ;
		gen_poly(nn1,kk1) ;
		for (h=0; h<nn1; h++)
			recd[h] = index_of[recd[h]] ;  
		decode_rs(nn1,kk1,tt);  
		
		if (decodeflag == 0) 
		{
			
			cio_init(red.reddata,cslen); 
			cio_seek(redpos);

			
			cio_write(posdata2+ldata+i*kk,4); 
			redlen += 4;
			if (i<(nblock -1))
				cio_write(posdata2+ldata+i*kk + kk,4); 
			else
				cio_write(posdata2+ldata+i*kk + ndata,4);  
			redlen += 4;
			
			cio_write(0xFFFF,2);
			redlen += 2;
			if ((*j2k_state == J2K_STATE_MH)||(*j2k_state == J2K_STATE_TPHSOT))
				redlenok+=10;
			redpos = cio_tell(); 
			
		}
		if ((redlen==0)&(redmode==1))
			free(red.reddata);
		
		

		
		
		cio_init(cssrc, cslen);
		
		cio_seek(posdata2+i*(nn-kk)); 
		for (h=0; h<(nn-kk); h++)
		{
			cio_write(recd[h],1); 
			
		}
		
		
		cio_seek(posdata2+ldata+i*kk);
		if (i<(nblock-1))
		{
			
			for (h=(nn-kk); h<nn; h++)
				cio_write(recd[h],1);  
		}
		else
		{
			ndata = ldpepb - (nblock-1) * kk;  
			
			for (h=(nn-kk); h<(nn-kk)+ndata; h++)
				cio_write(recd[h],1); 
		}
	}
	

	free(alpha_to);
	free(index_of);
	free(gg);
	free(recd);
	free(data);
	free(bb);
	
	free(buff);


	
	
	return next; // End of read EPB
	
	
	
	

	
}






int read_EPB_PM(int *j2k_state)  
{
	unsigned int lepb, lsiz, temp, ldata, lbuf, lsot;
	int posdata, posdata2, nblock, i,h, pos, lante;
	int lastepb, nepbpm, posfirst, posend, count; 
	unsigned long lpack, ldpread;	
	EPB_par *epb;	
	int nn, kk, tt, nn1, kk1;
	unsigned long ldpepb, pepb, ndata, datacrc;  
	unsigned char depb;
	unsigned char *buff;

	int lparity;
	
		if (*j2k_state == J2K_STATE_MHSOC)  
		{
			
			
			cio_skip(4);
			lsiz = cio_read(2);
			cio_skip(lsiz-2); 
			pos = cio_tell(); 
			temp = cio_read(2); 
			nn = 160; kk = 64; tt = 48;  // RS(160,64)
			lante = lsiz+4;
			
		} 

		if (*j2k_state == J2K_STATE_TPHSOT)
		{
			
			cio_skip(2); 
			lsot = cio_read(2);
			cio_skip(lsot-2); // EPB MS
			pos = cio_tell(); 
			temp = cio_read(2); 
			nn = 80; kk = 25; tt = 28;  // RS(80,25)
			lante = lsot+2;
		}

		if ((*j2k_state == J2K_STATE_MH)||(*j2k_state == J2K_STATE_TPH))
		{
			pos = cio_tell(); // EPB
			temp = cio_read(2); 
			nn = 40; kk = 13; tt = 14;  // RS(40,13)
			lante = 0;
		}
	

	
	posfirst = pos; 
	nn1 = 255; kk1 = kk + (nn1 - nn);
	alpha_to = (int *) malloc((nn1+1)*sizeof(int));
	index_of = (int *) malloc((nn1+1)*sizeof(int));
	gg = (int *) malloc((nn1-kk1+1)*sizeof(int));
	recd = (int *) malloc((nn1)*sizeof(int)); 
	data = (int *) malloc((kk1)*sizeof(int)); 
	bb = (int *) malloc((nn1-kk1)*sizeof(int));

	lastepb = 0; 
	nepbpm = 0;
	

	while (lastepb == 0)
	{

	lepb = cio_read(2); 
	cio_skip(9);  
	posdata = cio_tell();
	ldata = lepb - 11; 

	lbuf = lante + 13 + (nn-kk);  
	buff = (char *) malloc(nn1 * sizeof(char)); 
	for (i=0; i<nn1; i++)
		buff[i] = 0; 
	
	

	
	write_buff(buff,posdata,(nn-kk)); 
	
	cio_seek(pos);  
	for (i=(nn-kk); i<lbuf; i++)
	{
		buff[i] = cio_read(1); 
	}

	for (i=0; i<nn1; i++)
		recd[i] = buff[i]; 

	

	generate_gf(nn1,kk1) ;
	gen_poly(nn1,kk1) ;
	for (i=0; i<nn1; i++)
     recd[i] = index_of[recd[i]] ;
	decode_rs(nn1,kk1,tt);  

	if (decodeflag == 0) 
		{
			
			cio_init(red.reddata,cslen);
			cio_seek(redpos);

			
			cio_write(pos,4);
			cio_write(pos + lbuf - (nn - kk),4);
			
			cio_write(0xFFFF,2);
			redlen += 10;
			redpos = cio_tell(); 
			
		}
	

	cio_init(cssrc, cslen);

	cio_seek(posdata);
	for (i=0; i<(nn-kk); i++)
		cio_write(recd[i],1); 
	cio_seek(pos);
	for (i=(nn-kk); i<lbuf; i++)
		cio_write(recd[i],1); 

	

	
	cio_seek(pos);  
	temp = cio_read(2);  
			if (temp != JPWL_MS_EPB)
			{
				
				return 0;
				
			}

	cio_skip(2); 
	depb = cio_read(1); 
	
	if (!((depb >> 6)&1))
		nepbpm += 1;
	if ((depb >> 6)&1) 
	{
		nepbpm += 1;
		lastepb = 1;
	}

	cio_skip(-3); 
	cio_skip(cio_read(2)-2);
	pos = cio_tell(); 
	cio_skip(2);

	

	} 
	

	
	cio_skip(-2);
	posend = cio_tell();
	lpack = posend-posfirst; // Total EPBs length
	epb = (EPB_par *) malloc(nepbpm * sizeof(EPB_par));
	cio_seek(posfirst);
	
	for (count=0; count<nepbpm; count++)
	{
		cio_skip(2);
		epb[count].lepb = cio_read(2);
		epb[count].depb = cio_read(1); 
		epb[count].ldpepb = cio_read(4); 
		epb[count].pepb = cio_read(4); 

		temp = cio_tell();
		cio_init(epc.tecn[0].pid, epc.tecn[0].lid);
		cio_seek((nepbrd+count)*4);
		if (epb[count].pepb!=cio_read(4))
		{
			cio_skip(-4);
			epb[count].pepb=cio_read(4); 
		}
		cio_init(cssrc, cslen);
		cio_seek(temp);

		epb[count].ldata = (epb[count].lepb - 11) - (nn-kk);  
		cio_skip(-11); 
	    cio_skip(epb[count].lepb); 
	} 

	

	nepbrd+=nepbpm;

	cio_seek(posfirst);  // First of the packed EPBs
	pos = cio_tell();
	ldpread = 0;
	lparity = nn - kk;
	
	for (count=0; count<nepbpm; count++)
	{
		cio_seek(pos);
		
		cio_skip(13); 
		posdata = cio_tell();
		
		cio_seek(posdata + lparity); 
		posdata2 = cio_tell(); 
		

		if ((!((epb[count].depb >> 6)&1))&&((*j2k_state == J2K_STATE_MHSOC)||(*j2k_state == J2K_STATE_MH)))
			*j2k_state = J2K_STATE_MH; 
		if (((epb[count].depb >> 6)&1)&&((*j2k_state == J2K_STATE_MH)||(*j2k_state == J2K_STATE_MHSOC)))
			*j2k_state = J2K_STATE_TPHSOT; 
		if (((epb[count].depb >> 6)&1)&&((*j2k_state == J2K_STATE_TPH)||(*j2k_state == J2K_STATE_TPHSOT)))
			*j2k_state = J2K_STATE_TPHSOT; 
		if ((!((epb[count].depb >> 6)&1))&&((*j2k_state == J2K_STATE_TPHSOT)||(*j2k_state == J2K_STATE_TPH)))
			*j2k_state = J2K_STATE_TPH; 
		
		
		
		if (epb[count].pepb)  
		{
			if ((epb[count].pepb>>28)==2)
			{
				
				free(alpha_to);
				free(index_of);
				free(gg);
				free(recd);
				free(data);
				free(bb);
				
				kk = (int) epb[count].pepb & 0x000000ff;
				nn = (int) (epb[count].pepb>>8) & 0x000000ff;
				tt = (int) ceil((double)(nn-kk)/2);
				nn1 = 255; kk1 = kk + (nn1 - nn);
				alpha_to = (int *) malloc((nn1+1)*sizeof(int));
				index_of = (int *) malloc((nn1+1)*sizeof(int));
				gg = (int *) malloc((nn1-kk1+1)*sizeof(int));
				recd = (int *) malloc((nn1)*sizeof(int)); 
				data = (int *) malloc((kk1)*sizeof(int)); 
				bb = (int *) malloc((nn1-kk1)*sizeof(int));
				generate_gf(nn1,kk1) ;
				gen_poly(nn1,kk1) ;
			}

			if ((epb[count].pepb>>28)==1)
			{
				// CRC
				free(buff);
				buff = (char *) malloc(epb[count].ldpepb * sizeof(char));
				write_buff(buff,posdata2+epb[count].ldata,epb[count].ldpepb);
				if (epb[count].pepb & 0x00000001)	// CRC 32
				{
					
					ResetCRC();
					cio_seek(posend+ldpread); 
					for (i=0; i < epb[count].ldpepb; i++)
						UpdateCRC32(reflectByte(buff[i]));	
					reflectCRC32();
					if (lastepb==1)
					{
						next = startsot + psot;
						cio_seek(next);
						
					}
					if ((cio_read(2) == 0xffd9)||(psot == 0))
						*j2k_state = J2K_STATE_MT;
					cio_skip(-2);
					
					crcSum ^= 0xffffffff;	
					cio_seek(posdata2);
					datacrc = cio_read(4);
					if (datacrc == crcSum)
						printf("CRC corretto!\n");
					else
						printf("CRC errato!\n");
					
				}
				else	// CRC 16
				{
					ResetCRC();
					cio_seek(posend+ldpread);
					for (i=0; i < epb[count].ldpepb; i++)
						UpdateCRC16(buff[i]);
					if (lastepb==1)
					{
						next = startsot + psot;
						cio_seek(next);
					}
					if ((cio_read(2) == 0xffd9)||(psot == 0))
						*j2k_state = J2K_STATE_MT;
					cio_skip(-2);
					cio_seek(posdata2);
					datacrc = cio_read(2);
					if (datacrc == crcSum)
						printf("CRC corretto!\n");
					else
						printf("CRC errato!\n");
				}
				
			}

			if (epb[count].pepb>=0x30000000)
			{
				
			}

			if (epb[count].pepb==0xffffffff)
			{
				
			}
		}
		
		if (((epb[count].pepb>>28)==2)||(epb[count].pepb==0))  // RS codes
		{
		
    
		
		nblock = epb[count].ldata / (nn-kk);  // Number of CW
		free(buff);
		buff = (char *) malloc(nn1 * sizeof(char));
		
		for (i=0; i<nblock; i++)
		{
			
			for (h=0; h<nn1; h++)
				buff[h] = 0; 
			write_buff(buff,posdata2+i*(nn-kk),(nn-kk)); 
			
			cio_seek(posend+ldpread+i*kk);
			if (i<(nblock-1))
			{
				for (h=(nn-kk); h<nn; h++)
				{
					buff[h] = cio_read(1);  
				}
			}
			else
			{
				ndata = epb[count].ldpepb - ((nblock-1) * kk);  
				for (h=(nn-kk); h<(nn-kk)+ndata; h++)
				{
					buff[h] = cio_read(1);  
				}
				if (lastepb==1)
				{
					next = startsot + psot;
					cio_seek(next);
				}
				if ((cio_read(2) == 0xffd9)||(psot == 0))
					*j2k_state = J2K_STATE_MT;
				cio_skip(-2);
			}
		
			for (h=0; h<nn1; h++)
				recd[h] = buff[h]; 
        
			

			generate_gf(nn1,kk1) ;
			gen_poly(nn1,kk1) ;
			for (h=0; h<nn1; h++)
				recd[h] = index_of[recd[h]] ;
			decode_rs(nn1,kk1,tt);  

			if (decodeflag == 0)
			{
				
				cio_init(red.reddata,cslen); 
				cio_seek(redpos);

				
				cio_write(posend+ldpread+i*kk,4);
				if (i<(nblock -1))
					cio_write(posend+ldpread+i*kk + kk,4); 
				else
					cio_write(posend+ldpread+i*kk + ndata,4);  
				cio_write(0xFFFF,2);
				redlen+=10;
				redpos = cio_tell();
				
			}

		

			cio_init(cssrc, cslen);

			cio_seek(posdata2+i*(nn-kk)); 
			for (h=0; h<(nn-kk); h++)
				cio_write(recd[h],1);  
		
		
			cio_seek(posend+ldpread+i*kk);
			if (i<(nblock-1))
			{
				for (h=(nn-kk); h<nn; h++)
					cio_write(recd[h],1); 
			}
			else
			{
				ndata = epb[count].ldpepb - (nblock-1) * kk;  
				for (h=(nn-kk); h<(nn-kk)+ndata; h++)
					cio_write(recd[h],1); 
			}
		}

		} 
	
	

		ldpread += epb[count].ldpepb;
		cio_seek(pos+2);
		cio_skip(epb[count].lepb);
		pos = cio_tell(); 

	} 

	cio_seek(next); 
	
	
	free(alpha_to);
	free(index_of);
	free(gg);
	free(recd);
	free(data);
	free(bb);
	free(buff);
	
	return nepbpm;
	
}


void insert_RED(int pos, int lred, int redlenok)
{
	unsigned long i;
	unsigned char *buff;
	int temp, mem;
	
	buff = (char *) malloc((epc.cl-pos) * sizeof(char));

	for (i=0; i<(epc.cl-pos); i++)
	{
		buff[i] = cio_read(1);
		
	}
	
	
	cio_seek(pos);
	
	cio_write(JPWL_MS_RED,2); 
	cio_write(red.lred,2); 
	cio_write(red.pred,1); 
	
	temp = cio_tell(); 
	

	cio_init(red.reddata,cslen); 
	cio_seek(redlenok);			
	for (i=0; i<(redlen-redlenok)/10; i++)
	{
		mem = cio_read(4);
		
		cio_skip(-4);
		cio_write(mem + lred,4); // Start byte refresh
		mem = cio_read(4);
		
		cio_skip(-4);
		cio_write(mem + lred,4); // End byte refresh
	}

	cio_init(cssrc,epc.cl+redlen+5);
	cio_seek(temp);

	for (i=0; i<redlen; i++)
		cio_write(red.reddata[i],1); 
	
	cio_init(cssrc,epc.cl+redlen+5);
	cio_seek(pos + redlen + 5);
	for (i=0; i<(epc.cl-pos); i++)
	{
		cio_write(buff[i],1);
		
	}
	cio_skip(-2);

}

void write_buff(unsigned char *buff,int pos,long cl)
{
    long i;
	cio_seek(pos);
	for (i=0; i<cl; i++) 
		 buff[i] = cio_read(1);	 
}


void read_buff(unsigned char *buff,int pos,long cl)
{
	long i;
	cio_seek(pos);
	for (i=0; i<cl; i++)
		cio_write(buff[i],1);
}


void ResetCRC()
{
	crcSum = 0xffffffff;
}


void UpdateCRC16(char x)
{
	int tmp;
	tmp = ((x ^ (crcSum >> 8)) & 0xff);
	crcSum = ((crcSum << 8) ^ CrcT16[tmp]) & 0xffff;
}

void UpdateCRC32(char x)
{
	int tmp;
	tmp = ((x ^ (crcSum >> 24)) & 0xff);
	crcSum = ((crcSum << 8) ^ CrcT32[tmp]);
}

char reflectByte(char inbyte)
{
  // reflect one byte

  unsigned char outbyte=0;
  unsigned char i=0x01;
  unsigned char j;

  for (j=0x080; j; j>>=1) 
  {
    if (inbyte & i) outbyte|=j;
    i<<=1;
  }
  
  return outbyte;
}


void reflectCRC32()
{
 

  unsigned long outcrc=0;
  unsigned long i=0x00000001;
  unsigned long j;

  for (j=0x80000000; j; j>>=1) 
  {
    if (crcSum & i) 
		outcrc|=j;
    i=i<<1;
  }
  crcSum = outcrc;
}




void generate_gf(int nn, int kk)
/* generate GF(2**mm) from the irreducible polynomial p(X) in pp[0]..pp[mm]
   lookup tables:  index->polynomial form   alpha_to[] contains j=alpha**i;
                   polynomial form -> index form  index_of[j=alpha**i] = i
   alpha=2 is the primitive element of GF(2**mm)
*/
 {
   register int i, mask ;

  mask = 1 ;
  alpha_to[mm] = 0 ;
  for (i=0; i<mm; i++)
   { alpha_to[i] = mask ;
     index_of[alpha_to[i]] = i ;
     if (pp[i]!=0)
       alpha_to[mm] ^= mask ;
     mask <<= 1 ;
   }
  index_of[alpha_to[mm]] = mm ;
  mask >>= 1 ;
  for (i=mm+1; i<nn; i++)
   { if (alpha_to[i-1] >= mask)
        alpha_to[i] = alpha_to[mm] ^ ((alpha_to[i-1]^mask)<<1) ;
     else alpha_to[i] = alpha_to[i-1]<<1 ;
     index_of[alpha_to[i]] = i ;
   }
  index_of[0] = -1 ;
 }


void gen_poly(int nn, int kk)
/* Obtain the generator polynomial of the tt-error correcting, length
  nn=(2**mm -1) Reed Solomon code  from the product of (X+alpha**i), i=1..2*tt
*/
 {
   register int i,j ;

   gg[0] = 2 ;    /* primitive element alpha = 2  for GF(2**mm)  */
   gg[1] = 1 ;    /* g(x) = (X+alpha) initially */
   for (i=2; i<=nn-kk; i++)
    { gg[i] = 1 ;
      for (j=i-1; j>0; j--)
        if (gg[j] != 0)  gg[j] = gg[j-1]^ alpha_to[(index_of[gg[j]]+i)%nn] ;
        else gg[j] = gg[j-1] ;
      gg[0] = alpha_to[(index_of[gg[0]]+i)%nn] ;     /* gg[0] can never be zero */
    }
   /* convert gg[] to index form for quicker encoding */
   for (i=0; i<=nn-kk; i++)  gg[i] = index_of[gg[i]] ;
 }


void encode_rs(int nn, int kk, int tt)
/* take the string of symbols in data[i], i=0..(k-1) and encode systematically
   to produce 2*tt parity symbols in bb[0]..bb[2*tt-1]
   data[] is input and bb[] is output in polynomial form.
   Encoding is done by using a feedback shift register with appropriate
   connections specified by the elements of gg[], which was generated above.
   Codeword is   c(X) = data(X)*X**(nn-kk)+ b(X)          */
 {
   register int i,j ;
   int feedback ;

   for (i=0; i<nn-kk; i++)   bb[i] = 0 ;
   for (i=kk-1; i>=0; i--)
    {  feedback = index_of[data[i]^bb[nn-kk-1]] ;
       if (feedback != -1)
        { for (j=nn-kk-1; j>0; j--)
            if (gg[j] != -1)
              bb[j] = bb[j-1]^alpha_to[(gg[j]+feedback)%nn] ;
            else
              bb[j] = bb[j-1] ;
          bb[0] = alpha_to[(gg[0]+feedback)%nn] ;
        }
       else
        { for (j=nn-kk-1; j>0; j--)
            bb[j] = bb[j-1] ;
          bb[0] = 0 ;
        } ;
    } ;
 } ;

// La funzione seguente, leggermente modificata rispetto al sorgente in "eccpage", ritorna
// 1 se è riuscita a correggere gli errori o se non ci sono errori, 0 se il processo di 
// decodifica RS non è stato portato a termine.

void decode_rs(int nn, int kk, int tt)
/* assume we have received bits grouped into mm-bit symbols in recd[i],
   i=0..(nn-1),  and recd[i] is index form (ie as powers of alpha).
   We first compute the 2*tt syndromes by substituting alpha**i into rec(X) and
   evaluating, storing the syndromes in s[i], i=1..2tt (leave s[0] zero) .
   Then we use the Berlekamp iteration to find the error location polynomial
   elp[i].   If the degree of the elp is >tt, we cannot correct all the errors
   and hence just put out the information symbols uncorrected. If the degree of
   elp is <=tt, we substitute alpha**i , i=1..n into the elp to get the roots,
   hence the inverse roots, the error location numbers. If the number of errors
   located does not equal the degree of the elp, we have more than tt errors
   and cannot correct them.  Otherwise, we then solve for the error value at
   the error location and correct the error.  The procedure is that found in
   Lin and Costello. For the cases where the number of errors is known to be too
   large to correct, the information symbols as received are output (the
   advantage of systematic encoding is that hopefully some of the information
   symbols will be okay and that if we are in luck, the errors are in the
   parity part of the transmitted codeword).  Of course, these insoluble cases
   can be returned as error flags to the calling routine if desired.   */
 {
   register int i,j,u,q ;
   //int elp[nn-kk+2][nn-kk], d[nn-kk+2], l[nn-kk+2], u_lu[nn-kk+2], s[nn-kk+1] ;
   //int count=0, syn_error=0, root[tt], loc[tt], z[tt+1], err[nn], reg[tt+1] ;

   
   int **elp, *d, *l, *u_lu, *s;
   int count=0, syn_error=0, *root, *loc, *z, *err, *reg;


   elp = (int **) malloc((nn-kk+2)*sizeof(int));
   for (i=0; i<(nn-kk+2); i++)
	   elp[i] = (int *) malloc((nn-kk)*sizeof(int));
   d = (int *) malloc((nn-kk+2)*sizeof(int));
   l = (int *) malloc((nn-kk+2)*sizeof(int));
   u_lu = (int *) malloc((nn-kk+2)*sizeof(int));
   s = (int *) malloc((nn-kk+2)*sizeof(int));
   root = (int *) malloc(tt*sizeof(int));
   loc = (int *) malloc(tt*sizeof(int));
   z = (int *) malloc((tt+1)*sizeof(int));
   err = (int *) malloc(nn*sizeof(int));
   reg = (int *) malloc((tt+1)*sizeof(int));


/* first form the syndromes */
   for (i=1; i<=nn-kk; i++)
    { s[i] = 0 ;
      for (j=0; j<nn; j++)
        if (recd[j]!=-1)
          s[i] ^= alpha_to[(recd[j]+i*j)%nn] ;      /* recd[j] in index form */
/* convert syndrome from polynomial form to index form  */
      if (s[i]!=0)  syn_error=1 ;        /* set flag if non-zero syndrome => error */
      s[i] = index_of[s[i]] ;
    } ;

   if (syn_error)       /* if errors, try and correct */
    {
/* compute the error location polynomial via the Berlekamp iterative algorithm,
   following the terminology of Lin and Costello :   d[u] is the 'mu'th
   discrepancy, where u='mu'+1 and 'mu' (the Greek letter!) is the step number
   ranging from -1 to 2*tt (see L&C),  l[u] is the
   degree of the elp at that step, and u_l[u] is the difference between the
   step number and the degree of the elp.
*/
/* initialise table entries */
      d[0] = 0 ;           /* index form */
      d[1] = s[1] ;        /* index form */
      elp[0][0] = 0 ;      /* index form */
      elp[1][0] = 1 ;      /* polynomial form */
      for (i=1; i<nn-kk; i++)
        { elp[0][i] = -1 ;   /* index form */
          elp[1][i] = 0 ;   /* polynomial form */
        }
      l[0] = 0 ;
      l[1] = 0 ;
      u_lu[0] = -1 ;
      u_lu[1] = 0 ;
      u = 0 ;

      do
      {
        u++ ;
        if (d[u]==-1)
          { l[u+1] = l[u] ;
            for (i=0; i<=l[u]; i++)
             {  elp[u+1][i] = elp[u][i] ;
                elp[u][i] = index_of[elp[u][i]] ;
             }
          }
        else
/* search for words with greatest u_lu[q] for which d[q]!=0 */
          { q = u-1 ;
            while ((d[q]==-1) && (q>0)) q-- ;
/* have found first non-zero d[q]  */
            if (q>0)
             { j=q ;
               do
               { j-- ;
                 if ((d[j]!=-1) && (u_lu[q]<u_lu[j]))
                   q = j ;
               }while (j>0) ;
             } ;

/* have now found q such that d[u]!=0 and u_lu[q] is maximum */
/* store degree of new elp polynomial */
            if (l[u]>l[q]+u-q)  l[u+1] = l[u] ;
            else  l[u+1] = l[q]+u-q ;

/* form new elp(x) */
            for (i=0; i<nn-kk; i++)    elp[u+1][i] = 0 ;
            for (i=0; i<=l[q]; i++)
              if (elp[q][i]!=-1)
                elp[u+1][i+u-q] = alpha_to[(d[u]+nn-d[q]+elp[q][i])%nn] ;
            for (i=0; i<=l[u]; i++)
              { elp[u+1][i] ^= elp[u][i] ;
                elp[u][i] = index_of[elp[u][i]] ;  /*convert old elp value to index*/
              }
          }
        u_lu[u+1] = u-l[u+1] ;

/* form (u+1)th discrepancy */
        if (u<nn-kk)    /* no discrepancy computed on last iteration */
          {
            if (s[u+1]!=-1)
                   d[u+1] = alpha_to[s[u+1]] ;
            else
              d[u+1] = 0 ;
            for (i=1; i<=l[u+1]; i++)
              if ((s[u+1-i]!=-1) && (elp[u+1][i]!=0))
                d[u+1] ^= alpha_to[(s[u+1-i]+index_of[elp[u+1][i]])%nn] ;
            d[u+1] = index_of[d[u+1]] ;    /* put d[u+1] into index form */
          }
      } while ((u<nn-kk) && (l[u+1]<=tt)) ;

      u++ ;
      if (l[u]<=tt)         /* can correct error */
       {
/* put elp into index form */
         for (i=0; i<=l[u]; i++)   elp[u][i] = index_of[elp[u][i]] ;

/* find roots of the error location polynomial */
         for (i=1; i<=l[u]; i++)
           reg[i] = elp[u][i] ;
         count = 0 ;
         for (i=1; i<=nn; i++)
          {  q = 1 ;
             for (j=1; j<=l[u]; j++)
              if (reg[j]!=-1)
                { reg[j] = (reg[j]+j)%nn ;
                  q ^= alpha_to[reg[j]] ;
                } ;
             if (!q)        /* store root and error location number indices */
              { root[count] = i;
                loc[count] = nn-i ;
                count++ ;
              };
          } ;
         if (count==l[u])    /* no. roots = degree of elp hence <= tt errors */
          {
/* form polynomial z(x) */
           for (i=1; i<=l[u]; i++)        /* Z[0] = 1 always - do not need */
            { if ((s[i]!=-1) && (elp[u][i]!=-1))
                 z[i] = alpha_to[s[i]] ^ alpha_to[elp[u][i]] ;
              else if ((s[i]!=-1) && (elp[u][i]==-1))
                      z[i] = alpha_to[s[i]] ;
                   else if ((s[i]==-1) && (elp[u][i]!=-1))
                          z[i] = alpha_to[elp[u][i]] ;
                        else
                          z[i] = 0 ;
              for (j=1; j<i; j++)
                if ((s[j]!=-1) && (elp[u][i-j]!=-1))
                   z[i] ^= alpha_to[(elp[u][i-j] + s[j])%nn] ;
              z[i] = index_of[z[i]] ;         /* put into index form */
            } ;

  /* evaluate errors at locations given by error location numbers loc[i] */
           for (i=0; i<nn; i++)
             { err[i] = 0 ;
               if (recd[i]!=-1)        /* convert recd[] to polynomial form */
                 recd[i] = alpha_to[recd[i]] ;
               else  recd[i] = 0 ;
             }
           for (i=0; i<l[u]; i++)    /* compute numerator of error term first */
            { err[loc[i]] = 1;       /* accounts for z[0] */
              for (j=1; j<=l[u]; j++)
                if (z[j]!=-1)
                  err[loc[i]] ^= alpha_to[(z[j]+j*root[i])%nn] ;
              if (err[loc[i]]!=0)
               { err[loc[i]] = index_of[err[loc[i]]] ;
                 q = 0 ;     /* form denominator of error term */
                 for (j=0; j<l[u]; j++)
                   if (j!=i)
                     q += index_of[1^alpha_to[(loc[j]+root[i])%nn]] ;
                 q = q % nn ;
                 err[loc[i]] = alpha_to[(err[loc[i]]-q+nn)%nn] ;
                 recd[loc[i]] ^= err[loc[i]] ;  /*recd[i] must be in polynomial form */
               }
            }
			decodeflag = 1;
			//printf("Ho corretto gli errori!\n");
          }
         else    /* no. roots != degree of elp => >tt errors and cannot solve */
		 {  for (i=0; i<nn; i++)        /* could return error flag if desired */
               if (recd[i]!=-1)        /* convert recd[] to polynomial form */
                 recd[i] = alpha_to[recd[i]] ;
               else  recd[i] = 0 ;     /* just output received codeword as is */
			decodeflag = 0;
			//printf("Non ho corretto!\n");
		 }
       }
     else         /* elp has degree has degree >tt hence cannot solve */
	 {  for (i=0; i<nn; i++)       /* could return error flag if desired */
          if (recd[i]!=-1)        /* convert recd[] to polynomial form */
            recd[i] = alpha_to[recd[i]] ;
          else  recd[i] = 0 ;     /* just output received codeword as is */
		decodeflag = 0;
		//printf("Non ho corretto!\n");
	 }
    }
   else       /* no non-zero syndromes => no errors: output received codeword */
   { for (i=0; i<nn; i++)
       if (recd[i]!=-1)        /* convert recd[] to polynomial form */
         recd[i] = alpha_to[recd[i]] ;
       else  recd[i] = 0 ;
	 decodeflag = 1;	
	 //printf("La codestream non contiene errori!\n");
   }

   /******/
	//   int **elp, *d, *l, *u_lu, *s;
   //int count=0, syn_error=0, *root, *loc, *z, *err, *reg;

   //elp = (int *) malloc((nn-kk+2)*sizeof(int));
   
   for (i=0; i<(nn-kk+2); i++)
      free(elp[i]);
   free(elp);
   free(d);
   free(l); 
   free(u_lu);
   free(s);
   free(root);
   free(loc); 
   free(z); 
   free(err);
   free(reg);
 }



