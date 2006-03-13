// In questa versione si aggiunge la funzione che inserisce il marker RED

// Per ora si suppone che venga aggiunto un solo RED, nel main header, e che sia
// utilizzata la modalità byte-range mode. Osserviamo che ci sono problemi realizzativi
// per l'utilizzo delle modalità a pacchetto (non ci sono pacchetti negli header!).
// Decidiamo di aggiungere il marker RED subito prima di SOT (alla fine di MH!!!).
// Per stare sicuri, come address length utilizziamo 4 bytes (b1=1).

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
	id_tecn *tecn;   // array di strutture di tipo id_tecn!!!
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

unsigned char *cssrc;// Queste variabili servono per la gestione della codestream.
unsigned int cslen;  // Se voglio utilizzare le funzioni "cio" per gestire un buffer,
int cspos;			 // queste variabili consentono di reinizializzare cio per la CS!!
unsigned int csread; // Lunghezza della codestream letta...serve per uscire in caso di errori

int redpos; // Per la gestione del passaggio delle funzioni "cio" al e dal buffer RED
int decodeflag; // Vale 1 se RS è stato decodificato, 0 altrimenti
unsigned long redlen; // Lunghezza del buffer che contiene REDdata
int redmode; // Se vale 0 allora RED in MH, se vale 1 allora RED in MH e nei vari TPH
int redlenok; // Lunghezza del campo dati della RED che non necessita aggiornamento offset
int nepbrd; // Tiene conto del numero di EPB letti
int lastepb; // Se vale 1 l'EPB corrente è l'ultimo dell'header in questione!


// La funzione seguente cerca la presenza nella codestream del marker EPC
// in modo da determinare se si tratta di una codestream JPWL
// Ritorna il numero di EPC presenti nella codestream
int decode_JPWL(unsigned char *src, int len)
{
	unsigned int temp, nepb, j2k_state, pos, nepc, posi, mem, rest;
	//int flag; // se 0 vuol dire che ha trovato EPC dopo SIZ, se 1 dopo EPB
	int err; // se 1 vuol dire che EPC è corretto, se 0 vuol dire che contiene
	         // ancora errori!
	unsigned long psot, i;
	FILE *f,*g;

	cssrc = src;  //*********Aggiunta in questa versione 1.7
	cslen = len;  //*********Aggiunta in questa versione 1.7
	redpos = 0;   //*********Aggiunta in questa versione 1.7
	redlen = 0;   //*********Aggiunta in questa versione 1.7
	redlenok = 0;
	redmode = 0;  //*********Aggiunta in questa versione 1.7
				  // Per default si assume che la RED è scritta solo in MH!!!	

	csread = 0;	

	temp = cio_read(2);
    if (temp != J2K_MS_SOC)
	{
			printf("Expected marker SOT\n");
			return 0;
	}
	//csread+=2;

	//temp = cio_read(2);  // qui dovrebbe leggere SIZ
	//if (temp >> 8 != 0xff) {
	//	fprintf(stderr, "%.8x: expected a marker instead of %x\n",
	//        cio_tell() - 2, temp);
	//	return 0;
	//}
	//temp = cio_read(2); // qui dovrebbe leggere la lunghezza di SIZ: Lsiz
	//cio_skip(temp-2);

	
	j2k_state = 0; // inizializza j2k_state ad un valore indefinito
	nepc = 0; // inizializza a zero il numero di EPC finora trovati
	nepbrd = 0;
	lastepb = 0;


	//while ((j2k_state != J2K_STATE_MT)&&(csread < cslen))
	while (j2k_state != J2K_STATE_MT)
	{
	
		//nepc = find_EPC(nepc,&j2k_state);
		temp = cio_read(2);  // qui dovrebbe leggere SIZ o SOT
		if (temp >> 8 != 0xff) {
			fprintf(stderr, "%.8x: expected a marker instead of %x\n",
			    cio_tell() - 2, temp);
			return nepc;
		}
		//csread+=2;

		posi = cio_tell();  // memorizza la posizione a monte della lunghezza di SIZ o SOT
		
		//ncomp = 3;  // di default si assume che l'immagine abbia 3 componenti!!!
		if (temp == J2K_MS_SIZ) // Ha letto SIZ!!!
		{
			temp = cio_read(2); // legge Lsiz
			//csread+=2;
			//ncomp = (temp - 38)/3;  // calcola il numero di componenti dell'immagine
			// nbckpar serve per modificare il numero di blocchi di decodifica per la prima 
			// parte del primo EPB in base al numero di componenti utilizzate!!!
			lmex = temp + 17;  // lunghezza dei dati da proteggere;
			nbckpar = (int)ceil((double)lmex / 96); // 96 è nn-kk per il primo EPB
									// temp=Lsiz, 17 tiene conto di EPB,SIZ,SOC
			
		}
		else  // sta leggendo SOT oppure sta leggendo SIZ ma ci sono errori nel marker SIZ!!!
		{     // ...in tal caso il decoder assume che l'immagine sia composta da 3 componenti
			nbckpar = 1;
			temp = cio_read(2); // qui dovrebbe leggere la lunghezza di SIZ o SOT
		}
		cio_skip(temp-2);
		//csread += (temp - 2);
		


		temp = cio_read(2);  // qui dovrebbe leggere EPC o EPB, se ci sono
		//if (temp >> 8 != 0xff) {
		//	fprintf(stderr, "%.8x: expected a marker instead of %x\n",
		//		cio_tell() - 2, temp);
		//	return nepc;
		//}
		//csread += 2;
		if (temp != JPWL_MS_EPC)
		{
			temp = cio_read(2); // qui dovrebbe leggere la lunghezza di EPB, se c'è: Lepb
			cio_skip(temp-2);
			temp = cio_read(2); // qui dovrebbe leggere EPC, se c'è!!
			//if (temp >> 8 != 0xff) {
			//fprintf(stderr, "%.8x: expected a marker instead of %x\n",
			//	cio_tell() - 2, temp);
			//return nepc;
			//}
			//csread += temp + 2;

			pos = cio_tell();

			if ((temp != JPWL_MS_EPC)&&(nepc == 0))
			{
				cio_seek(0);
				return nepc;  // non ha trovato EPC => vede la codestream come NON JPWL
			}
			
			if ((temp != JPWL_MS_EPC)&&(nepc != 0))  //vuol dire che il TPH in questione non ha EPC
			{
				cio_seek(posi); // siamo a monte della lunghezza di SOT
				cio_skip(4);
				psot = cio_read(4);
				if (psot == 0)  // vuol dire che siamo nell'ultimo TPH
					j2k_state = J2K_STATE_MT;  // cosi' al passo seguente si esce dal ciclo
				cio_seek(posi-2);
				cio_skip(psot); // si pone a valle dei dati del tile corrente
				if (cio_read(2) == J2K_MS_EOC)
					j2k_state = J2K_STATE_MT;
				cio_skip(-2); // si pone a valle dei dati del tile corrente
				//csread += (psot - pos);
				//return nepc;

			}
			if (temp == JPWL_MS_EPC) // ha trovato l'EPC non subito dopo SIZ, quindi c'è EPB!
			{
				if (nepc == 0)
				{
					j2k_state = J2K_STATE_MHSOC;
					cio_seek(posi-4);   // si posiziona a monte di SOC
					next = cio_tell();  // assegna a next = 0!!!!
				}
				if (nepc != 0)
				{
					j2k_state = J2K_STATE_TPHSOT;
					cio_seek(posi-2); // si posiziona a monte di SOT
					next = cio_tell();
				}
				//printf("next: %x\n",next);

				red.reddata = (char *) malloc(len * sizeof(char));// Allochiamo lo spazio necessario per RED
																// Scegliamo len per "stare larghi"
				
				// ********Cio' che segue è un'aggiunta in jpwldec1.9!!!!**********
				mem = next;
				i = 0;
				if (!(rest = read_EPB_2(&j2k_state)))// legge il primo EPB(della CS o del tile,caso + EPC!)
					return nepc;
				i += rest;
				temp = cio_tell(); // Memorizza posizione a valle di EPB
				cio_seek(pos); // si posiziona a valle del marker EPC
				err = read_EPC();  // Legge il primo EPC, o comunque il primo EPC di un tile
				//if (err == 1)
				//	printf("CRC EPC corretto!\n");
				//else
				//	printf("CRC EPC errato!\n");
				nepc++;				// nel caso di più EPC usati nella codestream
				nepb = epc.tecn[0].lid / 4;  // calcola il numero di EPB presenti
				//printf("nepb: %d\n",nepb);
				/***********************************************************************
					Qui dovrà essere aggiunta la porzione di codice per la gestione
					della scrittura della RED anche nei tile!
				*************************************************************************/
				
				//while ((i<nepb)&&(csread < cslen))
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
						//while ((lastepb == 0)&&(csread < cslen));
					}
					if (j2k_state == J2K_STATE_TPHSOT)
					{
						cio_seek(temp); // Si posiziona all'inizio di SOT
						pos = cio_tell();
						//printf("pos: %x\n",pos);
						//system("pause");
						do
						{
							if (!(rest = read_EPB_2(&j2k_state)))
								return nepc;
							i += rest;
							//printf("state: %x\n",j2k_state);
						}
						while (lastepb == 0);
						//printf("ciao!\n");
						//while ((lastepb == 0)&&(csread < cslen));
						//printf("state: %x\n",j2k_state);
						//system("pause");
					}
					if (j2k_state == J2K_STATE_MH)
					{
						temp = cio_read(2);
						//if (temp >> 8 != 0xff) {
						//	fprintf(stderr, "%.8x: expected a marker instead of %x\n",
						//		cio_tell() - 2, temp);
						//	return nepc;
						//}

						//while ((temp != J2K_MS_SOT)&&(csread < cslen))
						while (temp != J2K_MS_SOT)
						{
							cio_skip(cio_read(2)-2);
							temp = cio_read(2);
							//if (temp >> 8 != 0xff) {
							//fprintf(stderr, "%.8x: expected a marker instead of %x\n",
							//	cio_tell() - 2, temp);
							//return nepc;
							//}
							//csread += 2;
						}
						cio_skip(-2);
					}
					temp = cio_read(2);
					//printf("mrk: %x\n",temp);
					//system("pause");
					//if (temp >> 8 != 0xff) {
					//fprintf(stderr, "%.8x: expected a marker instead of %x\n",
					//	cio_tell() - 2, temp);
					//return nepc;
					//}
					//csread += 2;
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
				// Ora sono stati letti tutti gli EPB associati all'ultimo EPC letto
				//printf("temp: %x\n",temp);

				//while ((temp != J2K_MS_EOC)&&(csread < cslen))
				while (temp != J2K_MS_EOC)
				{
					cio_seek(pos);
					cio_skip(6);
					psot = cio_read(4);
					cio_seek(pos);
					cio_skip(psot);
					temp = cio_read(2);
					//if (temp >> 8 != 0xff) {
					//fprintf(stderr, "%.8x: expected a marker instead of %x\n",
					//	cio_tell() - 2, temp);
					//return nepc;
					//}
					//csread += 2;
				}
				cio_skip(-2); // A questo punto siamo all'inizio di EOC
				j2k_state = J2K_STATE_MT;
				
			}
		}
		else // ho trovato EPC dopo SIZ o SOT
		{
			err = read_EPC();
			//if (err == 1)
			//		printf("CRC EPC corretto!\n");
			//else
			//		printf("CRC EPC errato!\n");
			if (nepc == 0)
			{
				cio_seek(posi); // siamo a monte della lunghezza di SIZ
				cio_skip(cio_read(2)-2);  // si pone a valle di SIZ
				temp = cio_read(2); // legge il marker successivo
				//if (temp >> 8 != 0xff) {
				//	fprintf(stderr, "%.8x: expected a marker instead of %x\n",
				//		cio_tell() - 2, temp);
				//	return nepc;
				//}

				//while ((temp != J2K_MS_SOT)&&(csread < cslen))
				while (temp != J2K_MS_SOT)
				{
					cio_skip(cio_read(2)-2); // si pone a valle del MS corrente
					temp = cio_read(2);  // legge il marker successivo
					//if (temp >> 8 != 0xff) {
					//fprintf(stderr, "%.8x: expected a marker instead of %x\n",
					//	cio_tell() - 2, temp);
					//return nepc;
					//}
					//csread += 2;
					//printf("MS: %x\n",temp);
				}
				cio_skip(-2); // si posiziona a valle del main header
			}
			if (nepc != 0)
			{
				cio_seek(posi); // siamo a monte della lunghezza di SOT
				cio_skip(4);
				psot = cio_read(4);
				if (psot == 0)  // vuol dire che siamo nell'ultimo TPH
					j2k_state = J2K_STATE_MT;  // cosi' al passo seguente si esce dal cilclo
				cio_seek(posi-2);
				cio_skip(psot); // si pone a valle dei dati del tile corrente
				if (cio_read(2) == J2K_MS_EOC)
				{
					j2k_state = J2K_STATE_MT;
				    cio_skip(-2); // si pone a valle dei dati del tile corrente
				}
			}
			//j2k_state = J2K_STATE_MT;
			nepc++;
		}
	} // fine while (j2k_state != J2K_STATE_MT)!!

	//printf("Eccomi!\n");
	//f = fopen("output","wb");
	//if (f==NULL)
	//	printf("Unable to open file!\n");
    //cio_seek(0);
	//printf("CL: %d\n",epc.cl);
	//for (i=0; i<epc.cl; i++)
	//	fputc(cio_read(1),f);
	//fclose(f);

	cio_seek(0);
	//printf("redlen: %d\n",redlen);
	if ((redlen != 0)&&(redmode==0))
		{
			red.lred = redlen + 3; // Tiene conto del campo Lred e del campo Pred
			red.pred = 0x43; // Pred = 01000011 , per i motivi specificati all'inizio!
			// Dobbiamo posizionarci alla fine del MH
			temp = cio_read(2); // Legge SOC
			
			//while ((temp != J2K_MS_SOT)&&(csread < cslen))
			while (temp != J2K_MS_SOT)
			{
				cio_skip(2);
				cio_skip(cio_read(2)-2);
				temp = cio_read(2);
				//if (temp >> 8 != 0xff) {
				//	fprintf(stderr, "%.8x: expected a marker instead of %x\n",
				//		cio_tell() - 2, temp);
				//	return nepc;
				//}
				cio_skip(-2);
			}
			//cio_skip(-2);
			//printf("sdfpo: %x\n",cio_read(2));
			// A questo punto ci troviamo a valle dell'ultimo marker del MH
			// Dobbiamo inserire il marker RED!!!
			insert_RED(cio_tell(),red.lred+2,redlenok);
			g = fopen("output","wb");
			if (g==NULL)
				printf("Unable to open file!\n");
			cio_seek(0);
			for (i=0; i<(epc.cl+redlen+5); i++)
				fputc(cio_read(1),g);
			fclose(g);
			cslen = epc.cl + redlen + 5;
			//free(red.reddata);
		}
	else
	{
		f = fopen("output","wb");
		if (f==NULL)
			printf("Unable to open file!\n");
		cio_seek(0);
		//printf("CL: %d\n",epc.cl);
		for (i=0; i< cslen; i++)
			fputc(cio_read(1),f);
		fclose(f);
	}
	//free(red.reddata);
	cio_seek(0);
	//printf("Eccomi qua!\n");
	return nepc;
	}

int read_EPC() // ritorna 1 se trova epc, 0 se non lo trova
{
	
	unsigned int id, lid;  
	//unsigned char *pid;  						   
	int i, h, pos, nid;
	unsigned int ltec, count;
	unsigned char *buff;

	//FILE *f;
	

	pos = cio_tell()-2; // memorizza la posizione a monte del marker EPC

	
	epc.lepc = cio_read(2);
	epc.pcrc = cio_read(2);
	epc.cl = cio_read(4);
	//printf("CL: %d\n",epc.cl);
	epc.pepc = cio_read(1);
	if ((epc.pepc>>4)&1)	// E' presente una o più ESD !!!
	{
		esd = (ESD_MS *) malloc(10 * sizeof(ESD_MS)); // ******Si puo' togliere!!!!!
		//printf("La codestream contiene il marker ESD!\n");
	}
	ltec = epc.lepc - 9; // lunghezza dell'EPC a partire dalla fine di pepc
	count = 0;
	nid = 0; // numero di tecniche id usate
	while (count<ltec)
	{
		id = cio_read(2);
		count += 2;
		lid = cio_read(2);
		count += 2 + lid;
		cio_skip(lid); // salta il campo Pid
		nid ++;
	} // fine while (count<ltec)
	// Ora nid contiene il numero totale di tecniche usate!!!
	epc.tecn = (id_tecn *) malloc(nid * sizeof(id_tecn));
	cio_seek(pos + 11); // si posiziona a valle di pepc!
	for (i=0; i<nid; i++)
	{
		epc.tecn[i].id = cio_read(2);
		epc.tecn[i].lid = cio_read(2);
		epc.tecn[i].pid = (char *) malloc(epc.tecn[i].lid * sizeof(char));
		for (h=0; h<epc.tecn[i].lid; h++)
				epc.tecn[i].pid[h] = cio_read(1);
	}

	/*f = fopen("epc.txt","w");
	fprintf(f,"ECP: \t%x\n",0xff97);
	fprintf(f,"Lepc:\t%x\n",epc.lepc);
	fprintf(f,"Pcrc:\t%x\n",epc.pcrc);
	fprintf(f,"CL:  \t%x\n",epc.cl);
	fprintf(f,"Pepc:\t%x\n",epc.pepc);
	fprintf(f,"ID:  \t%x\n",epc.tecn[0].id);
	fprintf(f,"Lid: \t%x\n",epc.tecn[0].lid);
	fprintf(f,"Pid: \tN.D.\n");
	fclose(f);*/

	/*
	// Facciamo riscrivere tutto l'EPC letto!
	printf("Lepc: %d\n",epc.lepc);
	printf("Pcrc: %d\n",epc.pcrc);
	printf("CL: %d\n",epc.cl);
	printf("Pepc: %d\n",epc.pepc);
	for (i=0; i<nid; i++)
	{
		printf("id[%d] : %d\n",i,epc.tecn[i].id);
		printf("lid[%d] : %d\n",i,epc.tecn[i].lid);
		for (h=0; h<epc.tecn[i].lid; h++)
			printf("pid[%d] : %x\t",i,epc.tecn[i].pid[h]);
		printf("\n");
	}
	*/
	//f = fopen("pepbs","w");
	//if (f==NULL)
	//	printf("Unable to open file 'pepbs'!\n");
	//for (i=0; i<(epc.tecn[0].lid/4); i++)
	//{	for (h=0; h<4; h++)
			//fputc(epc.tecn[0].pid[i+h],f);
	//		fprintf(f,"%x",epc.tecn[0].pid[i*4+h]);
	//	fprintf(f,"\n");
	//}
	//fclose(f);

	// Ora occorre verificare la correttezza del campo Pcrc
	buff = (char *) malloc(epc.lepc* sizeof(char));
	cio_seek(pos); // si posiziona all'inizio di EPC
	for (i=0; i<4; i++)
		buff[i] = cio_read(1);  // copia nel buffer epc fino a pcrc escluso
	//pcrc = cio_read(2); // ora abbiamo copiato in pcrc il campo corrispondente dell'EPC
	cio_skip(2);
	for (i=4; i<epc.lepc; i++)
		buff[i] = cio_read(1);
	//for (i=0; i<(epc.lepc); i++)
	//	printf("%x ",buff[i]);
	//printf("\n");
	// Ora buff contiene tutto l'epc a meno di pcrc!
	// Bisogna applicare la codifica crc a buff e verificare che il risultato coincida
	// con pcrc salvato!
	ResetCRC();
	for (i=0; i < epc.lepc; i++){
		UpdateCRC16(buff[i]);	
	}
	//printf("CRCSUM: %x\n",crcSum);
	if (crcSum == epc.pcrc)
		return 1;  // se la funzione read_EPC ritorna 1 vuol dire che CRC è corretto
	else
		return 0;  // vuol dire che il campo Pcrc indica la presenza di errori in EPC
}

int read_EPB_2(int *j2k_state) // ritorna il numero di EPB letti
{
	unsigned int lepb, lsiz, temp, ldata, lsot, lbuf;
	int posdata, posdata2, nblock, i,h, pos, lante, lpar;
	int nn, kk, tt, nn1, kk1; // parametri per la decodifica RS
	//int pos1, pos2, h; // utili per la gestione della decodifica della seconda parte di EPB
	int nepbpm, posfirst, posend, count;
	unsigned long lpack, ldpread;
	EPB_par *epb;
	unsigned long ldpepb, pepb, ndata, datacrc;  // ndata è utile per la decodifica della seconda parte EPB
	unsigned char depb;
	unsigned char *buff;
	int lparity, packall; // se packall = 1 allora tutti gli EPB del tile sono packed

	//FILE *f;

	if (*j2k_state == J2K_STATE_MHSOC)  
		{
			// Se siamo giunti a questo punto vuol dire che SOC e i primi due campi di SIZ non sono
			// errati!!...ora ci dobbiamo posizionare subito a valle di SIZ
			//printf("j2k_state: %x\n",*j2k_state);
			cio_skip(4); // si pone all'inizio del campo Lsiz
			lsiz = cio_read(2);
			cio_skip(lsiz-2); // ora siamo all'inizio dell'EPB MS
			pos = cio_tell(); // memorizza la posizione in cui inizia l'EPB
			temp = cio_read(2);  // ci si aspetta qui di trovare il marker EPB
			//*printf("EPB: %x\n",temp);
			nn = 160; kk = 64; tt = 48;  // inizializzazione per codice RS(160,64)
			lante = lsiz+4; 
			
		} // fine if (j2k_state == J2K_STATE_MHSOC)

		if (*j2k_state == J2K_STATE_TPHSOT)
		{
			
			//printf("j2k_state: %x\n",*j2k_state);
			startsot = cio_tell(); // memorizza nella variabile globale la posizione di SOT
			cio_skip(2); // si pone all'inizio del campo Lsot
			lsot = cio_read(2);
			cio_skip(2); // si posiziona all'inizio del campo Psot
			psot = cio_read(4); // Legge il campo Psot
			cio_skip(-6); // si riposiziona a valle del campo Lsot
			//*printf("lsot: %d\n",lsot);
			cio_skip(lsot-2); // ora siamo all'inizio dell'EPB MS
			pos = cio_tell(); // memorizza la posizione in cui inizia l'EPB
			temp = cio_read(2);  // ci si aspetta qui di trovare il marker EPB
			//*printf("EPB: %x\n",temp);
			nn = 80; kk = 25; tt = 28;  // inizializzazione per codice RS(80,25)
			lante = lsot+2;
		}

		if ((*j2k_state == J2K_STATE_MH)||(*j2k_state == J2K_STATE_TPH))
		{
			//printf("j2k_state: %x\n",*j2k_state);
			pos = cio_tell(); // memorizza la posizione in cui inizia l'EPB
			temp = cio_read(2); // ci si aspetta qui di trovare il marker EPB
			nn = 40; kk = 13; tt = 14;  // inizializzazione per codice RS(40,13)
			lante = 0;
		}

    // A questo punto possiamo decodificare la prima parte di dati tramite i codici di default

	//printf("state: %x\n",*j2k_state);
	//system("pause");

	//printf("nn,kk,tt: %d,%d,%d\n",nn,kk,tt);
	nn1 = 255; kk1 = kk + (nn1 - nn);
	alpha_to = (int *) malloc((nn1+1)*sizeof(int));
	index_of = (int *) malloc((nn1+1)*sizeof(int));
	gg = (int *) malloc((nn1-kk1+1)*sizeof(int));
	recd = (int *) malloc((nn1)*sizeof(int)); ///forse alcune di queste malloc possono
	data = (int *) malloc((kk1)*sizeof(int)); // essere eliminate, inutili per decodifica!!!
	bb = (int *) malloc((nn1-kk1)*sizeof(int));

	//next = cio_tell();

	//printf("COUNT: %d\n",count);
	lepb = cio_read(2);  // legge la lunghezza di EPB..si spera che questo campo non è errato!
	//*printf("LEPB: %x\n",lepb);
	cio_skip(9);  // si posiziona all'inizio del campo dati di EPB
	posdata = cio_tell();
	//printf("data: %x\n",cio_read(2));
	//cio_skip(-2);
	ldata = lepb - 11;  // determina la lunghezza del campo dati
	
	lpar = nn - kk; // determina la lunghezza dei bit di parità utilizzati per correggere la prima parte di EPB
	if (*j2k_state == J2K_STATE_MHSOC)  
	{
		lpar = nbckpar * (nn-kk);
	}
	//if (*j2k_state == J2K_STATE_TPHSOT)
	else
		nbckpar = 1;
	//lbuf = lante + 13 + lpar;  // lpar è la lunghezza dei bit di parità 
	//*printf("lbuf = %d\n",lbuf);
	buff = (char *) malloc(nn1 * sizeof(char)); // buffer che conterrà tutti i dati che precedono EPB
	          // e i parametri di EPB

	for (i=0; i<nbckpar; i++)
	{
		//buff = (char *) malloc(nn1 * sizeof(char)); // buffer che conterrà tutti i dati che precedono EPB
	                                           // e i parametri di EPB
		//printf("Ho inizializzato il buffer!\n");
		for (h=0; h<nn1; h++)
			buff[h] = 0; // inizializza il buffer tutto a zero
	
		// Bisognerà copiare tutto il contenuto da questo punto fino alla fine della prima parte dei dati EPB in buff
		// Per come lavora il decoder RS, i bytes di parità vanno posti all'inizio del buffer

		write_buff(buff,posdata+i*(nn-kk),(nn-kk)); // copia nel buffer i byte di parità del campo dati
		//printf("PROVA\n");
		//for (h=0; h<nn1; h++)
		//	printf(" %x\n",buff[h]);
		//system("pause");
	
		//printf("nbckpar: %d\n",nbckpar);
		//printf("nn: %d\n",nn);
		cio_seek(next + i*kk);  // si posiziona all'inizio dei dati protetti (SOC,SOT o EPB)
		if (i<(nbckpar -1))
		{
			for (h=(nn-kk); h<nn; h++)
			{
				buff[h] = cio_read(1);  // copia in buff i byte di messaggio (SOC,SIZ,ParEPB)
				//printf(" %x\n",buff[i]);
			}
		}
		else
		{
			if (*j2k_state == J2K_STATE_MHSOC)
			{
				ndata = lmex - ((nbckpar-1) * kk); // l'ultimo blocco dati non è in genere lungo 64!
				for (h=(nn-kk); h<((nn-kk)+ndata); h++)
				{
					buff[h] = cio_read(1);
				}
			}
			else
				for (h=(nn-kk); h<nn; h++)
					buff[h] = cio_read(1);  
		}
		//printf("Eccomi qua-1!\n");
		//for (h=0; h<nn1; h++)
		//	printf(" %x\n",buff[h]);
		//system("pause");

		for (h=0; h<nn1; h++)
			recd[h] = buff[h];  // copia in recd il contenuto di buff da decodificare

		//printf("Eccomi qua-1!\n");
		//if (*j2k_state == J2K_STATE_MHSOC)
		//if (i==0)
		//{
			
			/*f = fopen("debug","a");
			if (f==NULL)
				printf("Unable to open file!\n");
			fprintf(f,"\n");
			for (h=0; h<nn1; h++)
				fprintf(f,"%x ",recd[h]);
			fprintf(f,"\n");
			fclose(f);*/
		//}
		//else
		//{
		//f = fopen("debug","a");
		//if (f==NULL)
		//	printf("Unable to open file!\n");
		//fprintf(f,"\n");
		//for (h=0; h<nn1; h++)
		//	fprintf(f,"%x ",recd[h]);
		//fprintf(f,"\n");
		//fclose(f);
		//}
		//printf("Eccomi qua-1!\n");
		//for (h=0; h<nn1; h++)
		//	printf(" %x\n",recd[h]);
		//system("pause");

		generate_gf(nn1,kk1) ;
		gen_poly(nn1,kk1) ;
		for (h=0; h<nn1; h++)
			recd[h] = index_of[recd[h]] ; // a questo punto recd[] contiene i bytes decodificati
		decode_rs(nn1,kk1,tt); 

		if (decodeflag == 0) //*******Aggiunto in questa versione 1.7
		{
			//Inizializzo il buffer in cui vado a copiare la RED
			cio_init(red.reddata,cslen); //*******Aggiunta in questa versione 1.7
			cio_seek(redpos);

			//printf("Il blocco corrispondente non è stato decodificato!\n");
			cio_write(next + i*kk,4); // Scrive il byte di start del range considerato	
			redlen += 4;
			if (i<(nbckpar -1))
				cio_write(next + i*kk + kk - 1,4);  // Scrive il byte di end del range
			else
			{
				if (*j2k_state == J2K_STATE_MHSOC)
					cio_write(next + i*kk + ndata - 1,4);  // Scrive il byte di end del range
				else
					cio_write(next + i*kk + kk - 1,4);
			}
			redlen += 4;
			// Adesso segnaliamo la presenza di errori con 0xFFFF!!!
			cio_write(0xFFFF,2);
			redlen += 2;
			if ((*j2k_state == J2K_STATE_MHSOC)||(*j2k_state == J2K_STATE_MH))
				redlenok+=10;
			//cio_seek(redpos);
			//printf("START: %x\n",cio_read(4));
			//printf("END: %x\n",cio_read(4));
			//printf("VALUE: %x\n",cio_read(2));
			redpos = cio_tell(); // Memorizza la posizione attuale del buffer RED
			//printf("ciao\n");
		}

		//printf("Eccomi qua-2!\n");
		//for (i=0; i<nn1; i++)
		//	printf(" %x\n",recd[i]);
		//system("pause");

		// Adesso bisogna ricopiare il contenuto di recd[] nella codestream

		cio_init(cssrc, cslen); //******Aggiunto in questa versione 1.7
		
		cio_seek(posdata+i*(nn-kk)); // si posiziona all'inizio del blocco di parità corrispondente	
		for (h=0; h<(nn-kk); h++)
			cio_write(recd[h],1);  // copia i byte di parità corretti nel campo dati
		cio_seek(next + i*kk);
		//printf("next: %x\n",next);
		if (i<(nbckpar -1))
		{
			for (h=(nn-kk); h<nn; h++)
				cio_write(recd[h],1);  // copia i bytes di messaggio nella codestream
		}
		else
		{
			if (*j2k_state == J2K_STATE_MHSOC)
				for (h=(nn-kk); h<(nn-kk)+ndata; h++)
				{
					cio_write(recd[h],1);  // copia i bytes di messaggio nella codestream
				}
			else
				for (h=(nn-kk); h<nn; h++)
				cio_write(recd[h],1);  // copia i bytes di messaggio nella codestream
		}
	} // fine ciclo for (i=0; i<nbckpar; i++)

	// A questo punto la codestream è corretta fino alla fine della prima parte dei dati EPB
	// Possiamo leggere i parametri di EPB per condurre la codifica seguente

	
	cio_seek(pos);  // si posiziona all'inizio di EPB
	//printf("pos: %x\n",pos);
	//system("pause");
	temp = cio_read(2);  // ci si aspetta qui di trovare il marker EPB
			if (temp != JPWL_MS_EPB)
			{
				//*printf("Non ho decodificato l'EPB!\n"); 
				// Puo' succedere che l'EPC ha fornito informazione errata: in tal caso il
				// processo di decodifica effettuato perde di significato.
				// Puo' anche succedere pero' che il codice RS contenuto nell'EPB MS non
				// è stato in grado di correggere l'errore sul marker EPB!!
				
				return 0;
				//return;
				// Per adesso usciamo dalla procedura, ma la cosa migliore sarebbe
				// fare in modo che il decoder vada a cercare l'eventuale EPB successivo
			}

	//*count++; // se siamo a questo punto vuol dire che è stato letto effettivamente un EPB
	//printf("mark: %x\n",temp);
	cio_skip(2); // ora è all'inizio del campo depb
	depb = cio_read(1); // legge depb
	//printf("depb: %x\n",depb);
	ldpepb = cio_read(4); // legge ldpepb
	//*printf("ldpepb: %x\n",ldpepb);
	pepb = cio_read(4); // legge pepb

	/*f = fopen("epb.txt","a");
	fprintf(f,"EPB:   \t%x\n",0xff96);
	fprintf(f,"Lepb:  \t%x\n",lepb);
	fprintf(f,"Depb:  \t%x\n",depb);
	fprintf(f,"LDPepb:\t%x\n",ldpepb);
	fprintf(f,"Pepb:  \t%x\n",pepb);
	fclose(f);*/

	if (nepbrd!=0)
	{
		temp = cio_tell();
		cio_init(epc.tecn[0].pid, epc.tecn[0].lid);
		cio_seek(nepbrd*4);
		if (pepb!=cio_read(4))
		{
			cio_skip(-4);
			pepb=cio_read(4); // Copia nel campo pepb il corrispondente pepc contenuto in EPC
		}
		cio_init(cssrc, cslen);
		cio_seek(temp);
	}
	//*printf("pepb: %x\n",pepb);
	//*printf("ldata1: %d\n",ldata);
	ldata = ldata - nbckpar*(nn-kk);  // lunghezza della porzione rimanente del campo dati
	//*printf("ldata2: %d\n",ldata);
	cio_seek(posdata + nbckpar*(nn-kk)); 
	posdata2 = cio_tell(); // posizione inizio seconda parte dati EPB
	if (ldpepb == 0)
		next = cio_tell();
	//printf("pd2: %x\n",posdata2);
	//printf("nbckpar: %d\n",nbckpar);
	//printf("mark: %x\n",cio_read(2));
	//cio_skip(-2);

	if (!((depb >> 6)&1))  // quello corrente non è l'ultimo EPB del tile corrente
		lastepb = 0;
	if ((depb >> 6)&1)  // quello corrente è l'ultimo EPB del tile corrente
		lastepb = 1;

	if (!((depb >> 7)&1))  // EPB in modalità unpacked
	{
		//printf("Unpacked\n");
		//system("pause");
		if ((!((depb >> 6)&1))&&((*j2k_state == J2K_STATE_MHSOC)||(*j2k_state == J2K_STATE_MH)))
			*j2k_state = J2K_STATE_MH; // vuol dire che il prossimo EPB è in MH ma non è il primo
		//if (((depb >> 6)&1)&&((*j2k_state == J2K_STATE_MH)||(*j2k_state == J2K_STATE_MHSOC)))
		//	*j2k_state = J2K_STATE_TPHSOT; // vuol dire che il prossimo EPB è il primo di un TPH
		//if (((depb >> 6)&1)&&((*j2k_state == J2K_STATE_TPH)||(*j2k_state == J2K_STATE_TPHSOT)))
		//	*j2k_state = J2K_STATE_TPHSOT; // vuol dire che il prossimo EPB è il primo di un TPH
		if ((!((depb >> 6)&1))&&((*j2k_state == J2K_STATE_TPHSOT)||(*j2k_state == J2K_STATE_TPH)))
			*j2k_state = J2K_STATE_TPH; // vuol dire che il prossimo EPB è relativo ad un TPH

		nepbrd++;
	
		// Ora leggendo pepb il decoder deve capire quale codice applicare per la porzione di dati
		// cui fa riferimento la seconda parte del campo EPBdata

		if (pepb)  // se pepb=0 allora si usano i codici di default precedenti
		{
			if ((pepb>>28)==2)
			{
				// in questo caso deve effettuare la decodifica RS
				/***********/
				// liberiamo gli spazi allocati
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
				recd = (int *) malloc((nn1)*sizeof(int)); ///forse alcune di queste malloc possono
				data = (int *) malloc((kk1)*sizeof(int)); // essere eliminate, inutili per decodifica!!!
				bb = (int *) malloc((nn1-kk1)*sizeof(int));
				generate_gf(nn1,kk1) ;
				gen_poly(nn1,kk1) ;
			}
	
			if ((pepb>>28)==1)
			{
				// in questo caso deve effettuare le decodifica CRC
				free(buff);
				buff = (char *) malloc(ldpepb * sizeof(char));
				write_buff(buff,posdata2+ldata,ldpepb);
				if (pepb & 0x00000001)	// vuol dire che bisogna decodificare secondo CRC 32
				{
					/*per fare il crc32 occorre invertire i byte in ingresso, invertire il crc calcolato
				    e farne il complemento a 1*/
					ResetCRC();
					for (i=0; i < ldpepb; i++)
						UpdateCRC32(reflectByte(buff[i]));	
					reflectCRC32();
					crcSum ^= 0xffffffff;	// effettua il complemento a 1	
					cio_seek(posdata2);
					datacrc = cio_read(4);
					//printf("CRCSUM: %x\n",crcSum);
					//if (datacrc == crcSum)
					//	printf("CRC corretto!\n");
					//else
					//	printf("CRC errato!\n");
				}
				else	// vuol dire che bisogna decodificare secondo CRC 16
				{
					ResetCRC();
					for (i=0; i < ldpepb; i++)
						UpdateCRC16(buff[i]);	
					cio_seek(posdata2);
					datacrc = cio_read(2);
					//printf("CRCSUM: %x\n",crcSum);
					//if (datacrc == crcSum)
					//	printf("CRC corretto!\n");
					//else
					//	printf("CRC errato!\n");
				}
				free(buff);
				cio_seek(posdata2 + ldata + ldpepb);
				next = cio_tell();
				//printf("read: %x\n",cio_read(2));
				//cio_skip(-2);

				temp = cio_read(2);
				if (temp == J2K_MS_SOT)
					*j2k_state = J2K_STATE_TPHSOT;
				if (temp == J2K_MS_EOC)
					*j2k_state = J2K_STATE_MT;
				cio_skip(-2);
				//printf("state: %x\n",*j2k_state);

				return 1;
				//return (posdata2 + ldata + ldpepb);  // ritorna la posizione a valle dei dati successivi a EPB
				//return;
			}
	
			if (pepb>=0x30000000)
			{
				// tecniche registrate in RA
				cio_seek(posdata2);
				return 1;
				//return (posdata2 + ldata + ldpepb);
				// Per adesso prevede la semplice uscita dalla funzione
			}
	
			if (pepb==0xffffffff)
			{
				// non sono usati metodi per i dati seguenti
				cio_seek(posdata2);
				return 1;
				//return (posdata2 + ldata + ldpepb);
				//return;
			}
		}// Fine if (pepb)
	
		
	
		/*******************/
		// qui bisogna aggiungere la parte per la gestione della modalità packed/unpacked
		/*******************/
	
		
		//cio_seek(posdata + (nn-kk)); 
		//posdata2 = cio_tell(); // posizione inizio seconda parte dati EPB
		
		/********************/
		// Per adesso si suppone che il primo EPB di un header utilizza lo stesso codice
		// di default anche per la seconda parte dei dati...in seguito bisognerà aggiungere
		// la funzionalità che gestisce l'uso dei vari codici in base al campo pepb
		/********************/
	
		// Ora bisogna copiare in buff la seconda parte dei dati di EPB e i dati successivi all'epb
		// per una lunghezza pari a ldpepb
	    
		nblock = ldata / (nn-kk);  // numero di "blocchi di decodifica"
		//printf("nblock = %d\n",nblock);
		//*system("pause");
		//cio_seek(posdata2);  // si posiziona all'inizio della seconda parte dei dati EPB
		free(buff);
		buff = (char *) malloc(nn1 * sizeof(char));
		for (i=0; i<nblock; i++)
		{
			//free(buff);
			//buff = (char *) malloc(nn1 * sizeof(char));
			for (h=0; h<nn1; h++)
				buff[h] = 0; // inizializza il buffer tutto a zero
			write_buff(buff,posdata2+i*(nn-kk),(nn-kk)); // copia nel buff i bytes di parità
			cio_seek(posdata2+ldata+i*kk); // si posiziona nel blocco dati corrispondente
			
			//if (i==0) {
			//	printf("data: %x\n",cio_read(2));
			//    cio_skip(-2);
			//	system("pause");
			//}
			//pos1 = cio_tell(); // memorizza la posizione del blocco dati corrispondente
			if (i<(nblock-1))
			{
				//for (h=(posdata2+i*(2*tt)); h<(posdata2+i*(2*tt))+64; h++)
				for (h=(nn-kk); h<nn; h++)
				{
					buff[h] = cio_read(1);  // copia nel buff i bytes di messaggio
					//if (i==1)
					//  printf("Data: %x\n",buff[h]);  /**********/
				}
				//system("pause"); /***********/
			}
			else
			{
				ndata = ldpepb - ((nblock-1) * kk);  // l'ultimo blocco di dati non necessariamente è lungo 64!
				//*printf("ndata: %d\n",ndata);
				//*system("pause");
				//for (h=(posdata2+i*(2*tt)); h<(posdata2+i*(2*tt))+ndata; h++)
				for (h=(nn-kk); h<(nn-kk)+ndata; h++)
				{
					buff[h] = cio_read(1);  // copia nel buff i bytes di messaggio
					//printf("Data: %x\n",buff[h]);  /**********/
				}
				//system("pause"); /***********/
				next = cio_tell();  // posizione alla fine dei dati protetti (ldpepb)
				//printf("next: %x\n",next);
				if (cio_read(2) == 0xffd9)
					*j2k_state = J2K_STATE_MT;
				cio_skip(-2);
			}
			
			for (h=0; h<nn1; h++)
				recd[h] = buff[h];  // copia in recd il contenuto di buff da decodificare

			//if (*j2k_state == J2K_STATE_TPHSOT)
			//{
			//	f = fopen("debug","w");
			//	if (f==NULL)
			//		printf("Unable to open file!\n");
			//	fprintf(f,"\n");
			//	for (h=0; h<nn1; h++)
			//		fprintf(f,"%x ",recd[h]);
			//	fprintf(f,"\n");
			//	fclose(f);
			//}
			//else
			//{
			/*f = fopen("debug","a");
			if (f==NULL)
				printf("Unable to open file!\n");
			fprintf(f,"\n");
			for (h=0; h<nn1; h++)
				fprintf(f,"%x ",recd[h]);
			fprintf(f,"\n");
			fclose(f);*/
			//}
			//for (h=0; h<nn1; h++)
			//	printf("mess: %x\n",recd[h]);
			//system("pause");

			//printf("nn1: %d\n",nn1);
			//printf("kk1: %d\n",kk1);
	        
			generate_gf(nn1,kk1) ;
			gen_poly(nn1,kk1) ;
			for (h=0; h<nn1; h++)
				recd[h] = index_of[recd[h]] ;  // a questo punto recd[] contiene i bytes decodificati
			decode_rs(nn1,kk1,tt);  
			
			if (decodeflag == 0) //*******Aggiunto in questa versione 1.7
			{
				//Inizializzo il buffer in cui vado a copiare la RED
				cio_init(red.reddata,cslen); //*******Aggiunta in questa versione 1.7
				cio_seek(redpos);

				//printf("Il blocco corrispondente non è stato decodificato!\n");
				cio_write(posdata2+ldata+i*kk,4); // Scrive il byte di start del range considerato	
				redlen += 4;
				if (i<(nblock -1))
					cio_write(posdata2+ldata+i*kk + kk - 1,4);  // Scrive il byte di end del range
				else
					cio_write(posdata2+ldata+i*kk + ndata - 1,4);  // Scrive il byte di end del range
				redlen += 4;
				// Adesso segnaliamo la presenza di errori con 0xFFFF!!!
				cio_write(0xFFFF,2);
				redlen += 2;
				if ((*j2k_state == J2K_STATE_MH)||(*j2k_state == J2K_STATE_TPHSOT))
					redlenok+=10;
				redpos = cio_tell(); // Memorizza la posizione attuale del buffer RED
				//printf("ciao\n");
			}
			if ((redlen==0)&(redmode==1))
				free(red.reddata);
			
			//*printf("nciclo: %d\n\n",i);
			//for (h=0; h<nn1; h++)
			//	printf("mess: %x\n",recd[h]);
			//system("pause");

			// Adesso bisogna ricopiare il contenuto di recd[] nella codestream
			
			cio_init(cssrc, cslen); //*****Aggiunto in questa versione 1.7
			
			cio_seek(posdata2+i*(nn-kk));  // si posiziona all'inizio del blocco di parità corrispondente
			for (h=0; h<(nn-kk); h++)
			{
				cio_write(recd[h],1);  // copia nella codestream i bytes di parità corretti
				//printf("par: %x\n",recd[h]);
			}
			//system("pause");
			
			cio_seek(posdata2+ldata+i*kk);
			if (i<(nblock-1))
			{
				//for (h=(posdata2+i*(2*tt)); h<(posdata2+i*(2*tt))+64; h++)
				for (h=(nn-kk); h<nn; h++)
					cio_write(recd[h],1);  // copia nella codestream il blocco di dati corretti
			}
			else
			{
				ndata = ldpepb - (nblock-1) * kk;  // l'ultimo blocco di dati non necessariamente è lungo 64!			
				//for (h=(posdata2+i*(2*tt)); h<(posdata2+i*(2*tt))+ndata; h++)
				for (h=(nn-kk); h<(nn-kk)+ndata; h++)
					cio_write(recd[h],1);  // copia nella codestream il blocco di dati corretti
			}
		}//fine ciclo for (i=0; i<nblock; i++)

		temp = cio_read(2);
		if (temp == J2K_MS_SOT)
			*j2k_state = J2K_STATE_TPHSOT;
		if (temp == J2K_MS_EOC)
			*j2k_state = J2K_STATE_MT;
		cio_skip(-2);

		//csread += (lepb + ldpepb);

		free(alpha_to);
		free(index_of);
		free(gg);
		free(recd);
		free(data);
		free(bb);
		/***********/
		free(buff);
		return 1;
		//return next; // Ritorna la posizione subito a valle dell'EPB appena letto!!
	}
	else	// Gli EPB sono scritti in modalità packed 
	{
		if (*j2k_state == J2K_STATE_TPHSOT)
			packall = 1;
		else
			packall = 0;
		//printf("packall: %d\n",packall);

		posfirst = pos;
		cio_seek(pos+2); // Si posiziona nel campo lepb del primo EPB packed
		cio_skip(cio_read(2)-2);  // Si posiziona all'inizio del secondo EPB packed
		pos = cio_tell();
		cio_skip(2); // Si posiziona all'inizio del campo lepb del secondo EPB packed
		
		
		//printf("posfirst: %x\n",posfirst);
		//printf("mrk: %x\n",cio_read(2));
		//cio_skip(-2);
		//system("pause");

		//lastepb = 0; // Si suppone che l'EPB corrente non sia l'ultimo di un tile!
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
		recd = (int *) malloc((nn1)*sizeof(int)); ///forse alcune di queste malloc possono
		data = (int *) malloc((kk1)*sizeof(int)); // essere eliminate, inutili per decodifica!!!
		bb = (int *) malloc((nn1-kk1)*sizeof(int));

		
		//while ((lastepb == 0)&&(csread < cslen))
		while (lastepb == 0)
		{

		lepb = cio_read(2);  // legge la lunghezza di EPB..si spera che questo campo non è errato!
		cio_skip(9);  // si posiziona all'inizio del campo dati di EPB
		posdata = cio_tell();
		ldata = lepb - 11;  // determina la lunghezza del campo dati
		//printf("ldata: %d\n",ldata);
		//printf("lante: %d\n",lante);
		//lbuf = lante + 13 + (nn-kk);  // 2*tt è la lunghezza dei bit di parità 
		lbuf = 13 + (nn-kk);  // 2*tt è la lunghezza dei bit di parità 
		buff = (char *) malloc(nn1 * sizeof(char)); // buffer che conterrà tutti i dati che precedono EPB
												// e i parametri di EPB
		for (i=0; i<nn1; i++)
			buff[i] = 0; // inizializza il buffer tutto a zero
		
		// Bisognerà copiare tutto il contenuto da questo punto fino alla fine della prima parte dei dati EPB in buff
		// Per come lavora il decoder RS, i bytes di parità vanno posti all'inizio del buffer

		
		write_buff(buff,posdata,(nn-kk)); // copia nel buffer i byte di parità del campo dati
		
		cio_seek(pos);  // si posiziona all'inizio dei dati protetti (SOC,SOT o EPB)
		for (i=(nn-kk); i<lbuf; i++)
		{
			buff[i] = cio_read(1);  // copia in buff i byte di messaggio (SOC,SIZ,ParEPB)
		}

		for (i=0; i<nn1; i++)
			recd[i] = buff[i];  // copia in recd il contenuto di buff da decodificare

		/*f = fopen("debug","a");
		if (f==NULL)
			printf("Unable to open file!\n");
		fprintf(f,"EPB PAR: %d\n",nepbpm);
		for (h=0; h<nn1; h++)
			fprintf(f,"%x ",recd[h]);
		fprintf(f,"\n");
		fclose(f);*/

		generate_gf(nn1,kk1) ;
		gen_poly(nn1,kk1) ;
		for (i=0; i<nn1; i++)
			recd[i] = index_of[recd[i]] ; // a questo punto recd[] contiene i bytes decodificati     
		decode_rs(nn1,kk1,tt);  

		if (decodeflag == 0) //*******Aggiunto in questa versione 1.7
			{
				//Inizializzo il buffer in cui vado a copiare la RED
				cio_init(red.reddata,cslen); //*******Aggiunta in questa versione 1.7
				cio_seek(redpos);

				//printf("Il blocco corrispondente non è stato decodificato!\n");
				cio_write(pos,4); // Scrive il byte di start del range considerato	
				cio_write(pos + lbuf - (nn - kk) - 1,4);  // Scrive il byte di end del range
				// Adesso segnaliamo la presenza di errori con 0xFFFF!!!
				cio_write(0xFFFF,2);
				redlen += 10;
				redpos = cio_tell(); // Memorizza la posizione attuale del buffer RED
				//printf("ciao\n");
			}
		// Adesso bisogna ricopiare il contenuto di recd[] nella codestream

		cio_init(cssrc, cslen);

		cio_seek(posdata); // si posiziona all'inizio del campo dati della codestream	
		//printf("read: %x\n",cio_read(2));
		//cio_skip(-2);
		//if (packall == 1)
		//	for (i=0; i<(55); i++)
		//		cio_write(recd[i],1);  // copia i byte di parità corretti nel campo dati
		//else
		//if (nepbpm==1)
		//	for (i=0; i<(nn-kk); i++)
		//		printf("%x ",recd[i]);
		for (i=0; i<(nn-kk); i++)
			cio_write(recd[i],1);  // copia i byte di parità corretti nel campo dati
		cio_seek(pos);
		for (i=(nn-kk); i<lbuf; i++)
			cio_write(recd[i],1);  // copia i bytes di messaggio nella codestream
		

		// A questo punto la codestream è corretta fino alla fine della prima parte dei dati EPB
		// Possiamo leggere i parametri di EPB per condurre la codifica seguente

		
		cio_seek(pos);  // si posiziona all'inizio di EPB
		temp = cio_read(2);  // ci si aspetta qui di trovare il marker EPB
				if (temp != JPWL_MS_EPB)
				{
					// Puo' succedere che l'EPC ha fornito informazione errata: in tal caso il
					// processo di decodifica effettuato perde di significato.
					// Puo' anche succedere pero' che il codice RS contenuto nell'EPB MS non
					// è stato in grado di correggere l'errore sul marker EPB!!
					return 0;
					// Per adesso usciamo dalla procedura, ma la cosa migliore sarebbe
					// fare in modo che il decoder vada a cercare l'eventuale EPB successivo
				}

		cio_skip(2); // ora è all'inizio del campo depb
		depb = cio_read(1); // legge depb
		//printf("depb: %x\n",depb);
		//if ((depb >> 6)&1)  // quello corrente è l'ultimo EPB del tile corrente
		//{
		//	lastepb = 1; // l'epb corrente è l'ultimo del tile
			//nepbpm = ((depb << 2) >> 2); // numero di EPB accorpati in modalità packed
		//	nepbpm = (depb & 0x3f); // numero di EPB accorpati in modalità packed
			//printf("nepbpm: %d\n",nepbpm);
		//}
		if (!((depb >> 6)&1))  // quello corrente non è l'ultimo EPB del tile corrente
			nepbpm += 1;
		if ((depb >> 6)&1)  // quello corrente è l'ultimo EPB del tile corrente
		{
			nepbpm += 1;
			lastepb = 1;
		}

		//printf("nepbpm: %d\n",nepbpm);
		//printf("lastepb: %d\n",lastepb);
		//system("pause");

		cio_skip(-3); // si posiziona all'inizio del campo lepb
		cio_skip(cio_read(2)-2);  // si posiziona a valle dell'epb corrente
		pos = cio_tell(); // memorizza la posizione all'inizio dell'EPB successivo
		//printf("mrk: %x\n",cio_read(2));
		//cio_skip(-2);
		//system("pause");
		cio_skip(2);

		//conta++;

		//csread += lepb;

		} // Fine while (lastepb == 0)!!!!
		// A questo punto il decoder ha decodificato le porzioni iniziali di tutti gli EPB
		// del tile corrente
	

		// Ora dobbiamo decodificare le porzioni finali di tutti gli EPB!!!

		// pos contiene la posizione a valle dell'ultimo degli EPB packed!!!
		cio_skip(-2);
		posend = cio_tell();
		//printf("posend: %x\n",posend);
		//system("pause");
		lpack = posend-posfirst; // lunghezza totale della catena di EPB
		epb = (EPB_par *) malloc(nepbpm * sizeof(EPB_par));
		cio_seek(posfirst);
		//printf("posfirst: %x\n",posfirst);

		//printf("nepbpm: %d\n",nepbpm);
		for (count=0; count<nepbpm; count++)
		{
			cio_skip(2); // si posiziona all'inizio di lepb
			epb[count].lepb = cio_read(2); // legge lepb
			epb[count].depb = cio_read(1); // legge depb
			epb[count].ldpepb = cio_read(4); // legge ldpepb
			epb[count].pepb = cio_read(4); // legge pepb

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
				epb[count].ldata = (epb[count].lepb - 11) - (nn-kk);  // lunghezza della porzione rimanente del campo dati
			cio_skip(-11); // si posiziona all'inizio di lepb dell'EPB corrente
			cio_skip(epb[count].lepb); // si posiziona a valle dell'EPB corrente
		} // Abbiamo a questo punto memorizzato nella struttura epb i parametri degli EPB packed

		//for (count=0; count<nepbpm; count++)
		//{
		//	printf("EPB[%d]: %x\t%x\t%x\t%x\t%d\n",count,epb[count].lepb,epb[count].depb,
		//		epb[count].ldpepb,epb[count].pepb,epb[count].ldata);
		//}

		nepbrd+=nepbpm;

		cio_seek(posfirst);  // si posiziona all'inizio del primo degli EPB packed
		pos = cio_tell();
		ldpread = 0;
		lparity = nn - kk;
		//printf("lparity: %d\n",lparity);
		for (count=0; count<nepbpm; count++)
		{
			cio_seek(pos);
			//printf("mark: %x\n",cio_read(2));
			//printf("count: %d\n",count);
			//cio_skip(-2);
			//system("pause");
			cio_skip(13); // si posiziona all'inizio del campo dati
			posdata = cio_tell();
			//printf("tt: %d\n",nn-kk);
			if ((count == 0)&&(packall == 1))
				cio_seek(posdata + (80 - 25));
			else
				cio_seek(posdata + lparity); // si posiziona all'inizio seconda parte dati EPB corrente
			posdata2 = cio_tell(); // posizione inizio seconda parte dati EPB
			//printf("pd2: %x\n",posdata2);
			//system("pause");
			//cio_skip(-2);

			//if ((!((epb[count].depb >> 6)&1))&&((*j2k_state == J2K_STATE_MHSOC)||(*j2k_state == J2K_STATE_MH)))
			//	*j2k_state = J2K_STATE_MH; // vuol dire che il prossimo EPB è in MH ma non è il primo
			//if (((epb[count].depb >> 6)&1)&&((*j2k_state == J2K_STATE_MH)||(*j2k_state == J2K_STATE_MHSOC)))
			//	*j2k_state = J2K_STATE_TPHSOT; // vuol dire che il prossimo EPB è il primo di un TPH
			//if (((epb[count].depb >> 6)&1)&&((*j2k_state == J2K_STATE_TPH)||(*j2k_state == J2K_STATE_TPHSOT)))
			//	*j2k_state = J2K_STATE_TPHSOT; // vuol dire che il prossimo EPB è il primo di un TPH
			//if ((!((epb[count].depb >> 6)&1))&&((*j2k_state == J2K_STATE_TPHSOT)||(*j2k_state == J2K_STATE_TPH)))
			//	*j2k_state = J2K_STATE_TPH; // vuol dire che il prossimo EPB è relativo ad un TPH
			
			// Ora leggendo pepb il decoder deve capire quale codice applicare per la porzione di dati
			// cui fa riferimento la seconda parte del campo EPBdata
			
			if (epb[count].pepb)  // se pepb=0 allora si usano i codici di default precedenti
			{
				if ((epb[count].pepb>>28)==2)
				{
					// in questo caso deve effettuare la decodifica RS
					/***********/
					// liberiamo gli spazi allocati
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
					recd = (int *) malloc((nn1)*sizeof(int)); ///forse alcune di queste malloc possono
					data = (int *) malloc((kk1)*sizeof(int)); // essere eliminate, inutili per decodifica!!!
					bb = (int *) malloc((nn1-kk1)*sizeof(int));
					generate_gf(nn1,kk1) ;
					gen_poly(nn1,kk1) ;
				}

				if ((epb[count].pepb>>28)==1)
				{
					// in questo caso deve effettuare le decodifica CRC
					free(buff);
					buff = (char *) malloc(epb[count].ldpepb * sizeof(char));
					write_buff(buff,posdata2+epb[count].ldata,epb[count].ldpepb);//***Correggi!!!!!!!!!!!
					if (epb[count].pepb & 0x00000001)	// vuol dire che bisogna decodificare secondo CRC 32
					{
						/*per fare il crc32 occorre invertire i byte in ingresso, invertire il crc calcolato
						e farne il complemento a 1*/
						ResetCRC();
						cio_seek(posend+ldpread); // si posiziona nel blocco dati corrispondente
						for (i=0; i < epb[count].ldpepb; i++)
							UpdateCRC32(reflectByte(buff[i]));	
						reflectCRC32();
						if (lastepb==1)
						{
							next = startsot + psot; // **************Da correggere!!!!
							cio_seek(next);
							//printf("%x\n",cio_read(2));
							//cio_skip(-2);
						}
						if ((cio_read(2) == 0xffd9)||(psot == 0))
							*j2k_state = J2K_STATE_MT;
						cio_skip(-2);
						//else
						//{
						//	next = cio_tell();  // posizione alla fine dei dati protetti (ldpepb)
						//}
						crcSum ^= 0xffffffff;	// effettua il complemento a 1	
						cio_seek(posdata2);
						datacrc = cio_read(4);
						//if (datacrc == crcSum)
						//	printf("CRC corretto!\n");
						//else
						//	printf("CRC errato!\n");
						
					}
					else	// vuol dire che bisogna decodificare secondo CRC 16
					{
						ResetCRC();
						cio_seek(posend+ldpread); // si posiziona nel blocco dati corrispondente
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
						//if (datacrc == crcSum)
						//	printf("CRC corretto!\n");
						//else
						//	printf("CRC errato!\n");
					}
					//free(buff);
				}

				if (epb[count].pepb>=0x30000000)
				{
					next = cio_tell();
					// tecniche registrate in RA
				}

				if (epb[count].pepb==0xffffffff)
				{
					next = cio_tell();
					// non sono usati metodi per i dati seguenti
				}
			}
			
			if (((epb[count].pepb>>28)==2)||(epb[count].pepb==0))  // Vuol dire che si usano codici RS
			{
			// Ora bisogna copiare in buff la seconda parte dei dati di EPB e i dati successivi all'epb
			// per una lunghezza pari a ldpepb
	    
			//printf("count: %d\n",count);
			//printf("ldpread: %d\n",ldpread);
			//system("pause");
			//printf("nn: %d\n",nn);
			//printf("kk: %d\n",kk);
			//system("pause");
			//printf("posiz: %x\n",posdata2 + epb[count].ldata);
			nblock = epb[count].ldata / (nn-kk);  // numero di "blocchi di decodifica"
			free(buff);
			buff = (char *) malloc(nn1 * sizeof(char));
			//printf("ldata: %d\n",epb[count].ldata);
			//printf("nblock: %d\n",nblock);
			for (i=0; i<nblock; i++)
			{
				//free(buff);
				//buff = (char *) malloc(nn1 * sizeof(char));
				for (h=0; h<nn1; h++)
					buff[h] = 0; // inizializza il buffer tutto a zero
				write_buff(buff,posdata2+i*(nn-kk),(nn-kk)); // copia nel buff i bytes di parità
				//if (i==(nblock-1))
				//{
				//	for (h=0; h<(nn-kk); h++)
				//		printf("%x\n",buff[h]);
				//	system("pause");
				//}
				cio_seek(posend+ldpread+i*kk); // si posiziona nel blocco dati corrispondente
				if (i<(nblock-1))
				{
					for (h=(nn-kk); h<nn; h++)
					{
						buff[h] = cio_read(1);  // copia nel buff i bytes di messaggio
					}
				}
				else
				{
					ndata = epb[count].ldpepb - ((nblock-1) * kk);  // l'ultimo blocco di dati non necessariamente è lungo 64!
					for (h=(nn-kk); h<(nn-kk)+ndata; h++)
					{
						buff[h] = cio_read(1);  // copia nel buff i bytes di messaggio
					}
					if (lastepb==1)
					{
						next = cio_tell();
						//next = startsot + psot;
						//cio_seek(next);
					}
					//else
					//{
					//	next = cio_tell();  // posizione alla fine dei dati protetti (ldpepb)
					//}
					//next = cio_tell();  // posizione alla fine dei dati protetti (ldpepb)
					if ((cio_read(2) == 0xffd9)||(psot == 0))
						*j2k_state = J2K_STATE_MT;
					cio_skip(-2);
				}
			
				for (h=0; h<nn1; h++)
					recd[h] = buff[h];  // copia in recd il contenuto di buff da decodificare
	        
				/*f = fopen("debug","a");
				if (f==NULL)
					printf("Unable to open file!\n");
				fprintf(f,"EPB DATA: %d-%d\n",count,i);
				for (h=0; h<nn1; h++)
					fprintf(f,"%x ",recd[h]);
				fprintf(f,"\n");
				fclose(f);*/

				generate_gf(nn1,kk1) ;
				gen_poly(nn1,kk1) ;
				for (h=0; h<nn1; h++)
					recd[h] = index_of[recd[h]] ;// a questo punto recd[] contiene i bytes decodificati
				decode_rs(nn1,kk1,tt);  

				if (decodeflag == 0) //*******Aggiunto in questa versione 1.7
				{
					//Inizializzo il buffer in cui vado a copiare la RED
					cio_init(red.reddata,cslen); //*******Aggiunta in questa versione 1.7
					cio_seek(redpos);

					//printf("Il blocco corrispondente non è stato decodificato!\n");
					cio_write(posend+ldpread+i*kk,4); // Scrive il byte di start del range considerato	
					//printf("START: %x\n",posend+ldpread+i*kk);
					//printf("END: %x\n",posend+ldpread+i*kk+ndata);

					if (i<(nblock -1))
						cio_write(posend+ldpread+i*kk + kk - 1,4);  // Scrive il byte di end del range
					else
						cio_write(posend+ldpread+i*kk + ndata - 1,4);  // Scrive il byte di end del range
					// Adesso segnaliamo la presenza di errori con 0xFFFF!!!
					cio_write(0xFFFF,2);
					redlen+=10;
					redpos = cio_tell(); // Memorizza la posizione attuale del buffer RED
					//printf("ciao\n");
				}

				// Adesso bisogna ricopiare il contenuto di recd[] nella codestream

				cio_init(cssrc, cslen);

				cio_seek(posdata2+i*(nn-kk));  // si posiziona all'inizio del blocco di parità corrispondente
				for (h=0; h<(nn-kk); h++)
					cio_write(recd[h],1);  // copia nella codestream i bytes di parità corretti
			
				//cio_seek(posdata2+epb[count].ldata+i*kk);// si posiziona all'inizio del blocco dati corrispondente
				cio_seek(posend+ldpread+i*kk);// si posiziona all'inizio del blocco dati corrispondente
				if (i<(nblock-1))
				{
					for (h=(nn-kk); h<nn; h++)
						cio_write(recd[h],1);  // copia nella codestream il blocco di dati corretti
				}
				else
				{
					ndata = epb[count].ldpepb - (nblock-1) * kk;  // l'ultimo blocco di dati non necessariamente è lungo 64!			
					for (h=(nn-kk); h<(nn-kk)+ndata; h++)
						cio_write(recd[h],1);  // copia nella codestream il blocco di dati corretti
				}
			}//fine ciclo for (i=0; i<nblock; i++)

			} // fine if (!(epb[count]->pepb))
		
		// A questo punto abbiamo corretto anche la parte di dati cui fa riferimento la seconda
		// parte del campo EPBdata

			ldpread += epb[count].ldpepb;
			cio_seek(pos+2);
			cio_skip(epb[count].lepb);
			pos = cio_tell(); // posizione a valle dell'EPB corrente

		} // fine for (count=0; count<nepbpm; count++)

		
		cio_seek(next); // si posiziona alla fine dei dati corretti dall'EPB
		//printf("read: %x\n",cio_read(2));
		//cio_skip(-2);
		temp = cio_read(2);
		if (temp == J2K_MS_SOT)
			*j2k_state = J2K_STATE_TPHSOT;
		if (temp == J2K_MS_EOC)
			*j2k_state = J2K_STATE_MT;
		cio_skip(-2);

		//csread += ldpread;
		
		free(alpha_to);
		free(index_of);
		free(gg);
		free(recd);
		free(data);
		free(bb);
		free(buff);
		
		return nepbpm; // Ritorna il numero di EPB letti in modalità packed
	}	
		

}

int read_EPB(int next, int *j2k_state)  // funzione che ritorna la posizione subito a valle dell'EPB letto
//void read_EPB(int *j2k_state)  // funzione che ritorna la posizione subito a valle dell'EPB letto
{
	unsigned int lepb, lsiz, temp, ldata, lsot;
	int posdata, posdata2, nblock, i,h, pos, lante, lpar;
	int nn, kk, tt, nn1, kk1; // parametri per la decodifica RS
	//int pos1, pos2, h; // utili per la gestione della decodifica della seconda parte di EPB
	unsigned long ldpepb, pepb, ndata, datacrc;  // ndata è utile per la decodifica della seconda parte EPB
	unsigned char depb;
	unsigned char *buff;
	int prova;
	//FILE *f;

	//next = 0;
	//cio_seek(next); // si posiziona all'inizio della codestream
	//j2k_state = J2K_STATE_MHSOC;  // ci troviamo nel Main-Header e ci aspettiamo SOC
	//FIRST_epb = 1;  // vuol dire che bisogna usare il codice di default RS(160,64)
    
	//printf("nepb: %d\n",nepb);
    //count = 0;  // ancora non abbiamo letto nessun EPB
	//while (count < nepb)
	//{
	cio_seek(next);
	//next = cio_tell();
		if (*j2k_state == J2K_STATE_MHSOC)  
		{
			// Se siamo giunti a questo punto vuol dire che SOC e i primi due campi di SIZ non sono
			// errati!!...ora ci dobbiamo posizionare subito a valle di SIZ
			//*printf("j2k_state: %x\n",*j2k_state);
			cio_skip(4); // si pone all'inizio del campo Lsiz
			lsiz = cio_read(2);
			cio_skip(lsiz-2); // ora siamo all'inizio dell'EPB MS
			pos = cio_tell(); // memorizza la posizione in cui inizia l'EPB
			temp = cio_read(2);  // ci si aspetta qui di trovare il marker EPB
			//*printf("EPB: %x\n",temp);
			nn = 160; kk = 64; tt = 48;  // inizializzazione per codice RS(160,64)
			lante = lsiz+4; 
			
		} // fine if (j2k_state == J2K_STATE_MHSOC)

		if (*j2k_state == J2K_STATE_TPHSOT)
		{
			
			//*printf("j2k_state: %x\n",*j2k_state);
			startsot = cio_tell(); // memorizza nella variabile globale la posizione di SOT
			cio_skip(2); // si pone all'inizio del campo Lsot
			lsot = cio_read(2);
			cio_skip(2); // si posiziona all'inizio del campo Psot
			psot = cio_read(4); // Legge il campo Psot
			cio_skip(-6); // si riposiziona a valle del campo Lsot
			//*printf("lsot: %d\n",lsot);
			cio_skip(lsot-2); // ora siamo all'inizio dell'EPB MS
			pos = cio_tell(); // memorizza la posizione in cui inizia l'EPB
			temp = cio_read(2);  // ci si aspetta qui di trovare il marker EPB
			//*printf("EPB: %x\n",temp);
			nn = 80; kk = 25; tt = 28;  // inizializzazione per codice RS(80,25)
			lante = lsot+2;
		}

		if ((*j2k_state == J2K_STATE_MH)||(*j2k_state == J2K_STATE_TPH))
		{
			//*printf("j2k_state: %x\n",*j2k_state);
			pos = cio_tell(); // memorizza la posizione in cui inizia l'EPB
			temp = cio_read(2); // ci si aspetta qui di trovare il marker EPB
			nn = 40; kk = 13; tt = 14;  // inizializzazione per codice RS(40,13)
			lante = 0;
		}

    // A questo punto possiamo decodificare la prima parte di dati tramite i codici di default

	//printf("nn,kk,tt: %d,%d,%d\n",nn,kk,tt);
	nn1 = 255; kk1 = kk + (nn1 - nn);
	alpha_to = (int *) malloc((nn1+1)*sizeof(int));
	index_of = (int *) malloc((nn1+1)*sizeof(int));
	gg = (int *) malloc((nn1-kk1+1)*sizeof(int));
	recd = (int *) malloc((nn1)*sizeof(int)); ///forse alcune di queste malloc possono
	data = (int *) malloc((kk1)*sizeof(int)); // essere eliminate, inutili per decodifica!!!
	bb = (int *) malloc((nn1-kk1)*sizeof(int));

	//printf("COUNT: %d\n",count);
	lepb = cio_read(2);  // legge la lunghezza di EPB..si spera che questo campo non è errato!
	//*printf("LEPB: %x\n",lepb);
	cio_skip(9);  // si posiziona all'inizio del campo dati di EPB
	posdata = cio_tell();
	//printf("data: %x\n",cio_read(2));
	//cio_skip(-2);
	ldata = lepb - 11;  // determina la lunghezza del campo dati
	
	lpar = nn - kk; // determina la lunghezza dei bit di parità utilizzati per correggere la prima parte di EPB
	if (*j2k_state == J2K_STATE_MHSOC)  
	{
		lpar = nbckpar * (nn-kk);
	}
	if (*j2k_state == J2K_STATE_TPHSOT)
		nbckpar = 1;
	//lbuf = lante + 13 + lpar;  // lpar è la lunghezza dei bit di parità 
	//*printf("lbuf = %d\n",lbuf);
	buff = (char *) malloc(nn1 * sizeof(char)); // buffer che conterrà tutti i dati che precedono EPB
	          // e i parametri di EPB

	for (i=0; i<nbckpar; i++)
	{
		//buff = (char *) malloc(nn1 * sizeof(char)); // buffer che conterrà tutti i dati che precedono EPB
	                                           // e i parametri di EPB
		//printf("Ho inizializzato il buffer!\n");
		for (h=0; h<nn1; h++)
			buff[h] = 0; // inizializza il buffer tutto a zero
	
		// Bisognerà copiare tutto il contenuto da questo punto fino alla fine della prima parte dei dati EPB in buff
		// Per come lavora il decoder RS, i bytes di parità vanno posti all'inizio del buffer

		write_buff(buff,posdata+i*(nn-kk),(nn-kk)); // copia nel buffer i byte di parità del campo dati
		//printf("PROVA\n");
		//for (h=0; h<nn1; h++)
		//	printf(" %x\n",buff[h]);
		//system("pause");
	
		//printf("nbckpar: %d\n",nbckpar);
		//printf("nn: %d\n",nn);
		cio_seek(next + i*kk);  // si posiziona all'inizio dei dati protetti (SOC,SOT o EPB)
		if (i<(nbckpar -1))
		{
			for (h=(nn-kk); h<nn; h++)
			{
				buff[h] = cio_read(1);  // copia in buff i byte di messaggio (SOC,SIZ,ParEPB)
				//printf(" %x\n",buff[i]);
			}
		}
		else
		{
			if (*j2k_state == J2K_STATE_MHSOC)
			{
				ndata = lmex - ((nbckpar-1) * kk); // l'ultimo blocco dati non è in genere lungo 64!
				for (h=(nn-kk); h<((nn-kk)+ndata); h++)
				{
					buff[h] = cio_read(1);
				}
			}
			else
				for (h=(nn-kk); h<nn; h++)
					buff[h] = cio_read(1);  
				
			
		}
		//printf("Eccomi qua-1!\n");
		//for (h=0; h<nn1; h++)
		//	printf(" %x\n",buff[h]);
		//system("pause");

		for (h=0; h<nn1; h++)
			recd[h] = buff[h];  // copia in recd il contenuto di buff da decodificare

		//printf("Eccomi qua-1!\n");
		//if (*j2k_state == J2K_STATE_MHSOC)
		//if (i==0)
		//{
		//	f = fopen("debug","a");
		//	if (f==NULL)
		//		printf("Unable to open file!\n");
		//	fprintf(f,"\n");
		//	for (h=0; h<nn1; h++)
		//		fprintf(f,"%x ",recd[h]);
		//	fprintf(f,"\n");
		//	fclose(f);
		//}
		//else
		//{
		//f = fopen("debug","a");
		//if (f==NULL)
		//	printf("Unable to open file!\n");
		//fprintf(f,"\n");
		//for (h=0; h<nn1; h++)
		//	fprintf(f,"%x ",recd[h]);
		//fprintf(f,"\n");
		//fclose(f);
		//}
		//printf("Eccomi qua-1!\n");
		//for (h=0; h<nn1; h++)
		//	printf(" %x\n",recd[h]);
		//system("pause");

		generate_gf(nn1,kk1) ;
		gen_poly(nn1,kk1) ;
		for (h=0; h<nn1; h++)
			recd[h] = index_of[recd[h]] ; // a questo punto recd[] contiene i bytes decodificati
		decode_rs(nn1,kk1,tt); 

		if (decodeflag == 0) //*******Aggiunto in questa versione 1.7
		{
			//Inizializzo il buffer in cui vado a copiare la RED
			cio_init(red.reddata,cslen); //*******Aggiunta in questa versione 1.7
			cio_seek(redpos);

			//printf("Il blocco corrispondente non è stato decodificato!\n");
			cio_write(next + i*kk,4); // Scrive il byte di start del range considerato	
			redlen += 4;
			if (i<(nbckpar -1))
				cio_write(next + i*kk + kk,4);  // Scrive il byte di end del range
			else
			{
				if (*j2k_state == J2K_STATE_MHSOC)
					cio_write(next + i*kk + ndata,4);  // Scrive il byte di end del range
				else
					cio_write(next + i*kk + kk,4);
			}
			redlen += 4;
			// Adesso segnaliamo la presenza di errori con 0xFFFF!!!
			cio_write(0xFFFF,2);
			redlen += 2;
			if ((*j2k_state == J2K_STATE_MHSOC)||(*j2k_state == J2K_STATE_MH))
				redlenok+=10;
			//cio_seek(redpos);
			//printf("START: %x\n",cio_read(4));
			//printf("END: %x\n",cio_read(4));
			//printf("VALUE: %x\n",cio_read(2));
			redpos = cio_tell(); // Memorizza la posizione attuale del buffer RED
			//printf("ciao\n");
		}

		//printf("Eccomi qua-2!\n");
		//for (i=0; i<nn1; i++)
		//	printf(" %x\n",recd[i]);
		//system("pause");

		// Adesso bisogna ricopiare il contenuto di recd[] nella codestream

		cio_init(cssrc, cslen); //******Aggiunto in questa versione 1.7
		
		cio_seek(posdata+i*(nn-kk)); // si posiziona all'inizio del blocco di parità corrispondente	
		for (h=0; h<(nn-kk); h++)
			cio_write(recd[h],1);  // copia i byte di parità corretti nel campo dati
		cio_seek(next + i*kk);
		if (i<(nbckpar -1))
		{
			for (h=(nn-kk); h<nn; h++)
				cio_write(recd[h],1);  // copia i bytes di messaggio nella codestream
		}
		else
		{
			if (*j2k_state == J2K_STATE_MHSOC)
				for (h=(nn-kk); h<(nn-kk)+ndata; h++)
				{
					cio_write(recd[h],1);  // copia i bytes di messaggio nella codestream
				}
			else
				for (h=(nn-kk); h<nn; h++)
				cio_write(recd[h],1);  // copia i bytes di messaggio nella codestream
		}
	} // fine ciclo for (i=0; i<nbckpar; i++)

	// A questo punto la codestream è corretta fino alla fine della prima parte dei dati EPB
	// Possiamo leggere i parametri di EPB per condurre la codifica seguente

	
	cio_seek(pos);  // si posiziona all'inizio di EPB
	temp = cio_read(2);  // ci si aspetta qui di trovare il marker EPB
			if (temp != JPWL_MS_EPB)
			{
				//*printf("Non ho decodificato l'EPB!\n"); 
				// Puo' succedere che l'EPC ha fornito informazione errata: in tal caso il
				// processo di decodifica effettuato perde di significato.
				// Puo' anche succedere pero' che il codice RS contenuto nell'EPB MS non
				// è stato in grado di correggere l'errore sul marker EPB!!
				
				return 0;
				//return;
				// Per adesso usciamo dalla procedura, ma la cosa migliore sarebbe
				// fare in modo che il decoder vada a cercare l'eventuale EPB successivo
			}

	//*count++; // se siamo a questo punto vuol dire che è stato letto effettivamente un EPB
	//printf("mark: %x\n",temp);
	cio_skip(2); // ora è all'inizio del campo depb
	depb = cio_read(1); // legge depb
	//*printf("depb: %x\n",depb);
	ldpepb = cio_read(4); // legge ldpepb
	//*printf("ldpepb: %x\n",ldpepb);
	pepb = cio_read(4); // legge pepb
	if (nepbrd!=0)
	{
		temp = cio_tell();
		cio_init(epc.tecn[0].pid, epc.tecn[0].lid);
		cio_seek(nepbrd*4);
		if (pepb!=cio_read(4))
		{
			cio_skip(-4);
			pepb=cio_read(4); // Copia nel campo pepb il corrispondente pepc contenuto in EPC
		}
		cio_init(cssrc, cslen);
		cio_seek(temp);
	}
	//*printf("pepb: %x\n",pepb);
	//*printf("ldata1: %d\n",ldata);
	ldata = ldata - nbckpar*(nn-kk);  // lunghezza della porzione rimanente del campo dati
	//*printf("ldata2: %d\n",ldata);
	cio_seek(posdata + nbckpar*(nn-kk)); 
	posdata2 = cio_tell(); // posizione inizio seconda parte dati EPB
	//printf("nbckpar: %d\n",nbckpar);
	//printf("mark: %x\n",cio_read(2));
	//cio_skip(-2);

	if ((!((depb >> 6)&1))&&((*j2k_state == J2K_STATE_MHSOC)||(*j2k_state == J2K_STATE_MH)))
		*j2k_state = J2K_STATE_MH; // vuol dire che il prossimo EPB è in MH ma non è il primo
	if (((depb >> 6)&1)&&((*j2k_state == J2K_STATE_MH)||(*j2k_state == J2K_STATE_MHSOC)))
		*j2k_state = J2K_STATE_TPHSOT; // vuol dire che il prossimo EPB è il primo di un TPH
	if (((depb >> 6)&1)&&((*j2k_state == J2K_STATE_TPH)||(*j2k_state == J2K_STATE_TPHSOT)))
		*j2k_state = J2K_STATE_TPHSOT; // vuol dire che il prossimo EPB è il primo di un TPH
	if ((!((depb >> 6)&1))&&((*j2k_state == J2K_STATE_TPHSOT)||(*j2k_state == J2K_STATE_TPH)))
		*j2k_state = J2K_STATE_TPH; // vuol dire che il prossimo EPB è relativo ad un TPH

	if (!((depb >> 7)&1))
		epbpm = 1; // Gli EPB sono scritti in modalità unpacked (epbpm=0)
	else
		epbpm = 0; // Gli EPB sono scritti in modalità packed (epbpm=1)


	nepbrd++;
	
	// Ora leggendo pepb il decoder deve capire quale codice applicare per la porzione di dati
	// cui fa riferimento la seconda parte del campo EPBdata

	if (pepb)  // se pepb=0 allora si usano i codici di default precedenti
	{
		if ((pepb>>28)==2)
		{
			// in questo caso deve effettuare la decodifica RS
			/***********/
			// liberiamo gli spazi allocati
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
			recd = (int *) malloc((nn1)*sizeof(int)); ///forse alcune di queste malloc possono
			data = (int *) malloc((kk1)*sizeof(int)); // essere eliminate, inutili per decodifica!!!
			bb = (int *) malloc((nn1-kk1)*sizeof(int));
			generate_gf(nn1,kk1) ;
			gen_poly(nn1,kk1) ;
		}

		if ((pepb>>28)==1)
		{
			// in questo caso deve effettuare le decodifica CRC
			free(buff);
			buff = (char *) malloc(ldpepb * sizeof(char));
			write_buff(buff,posdata2+ldata,ldpepb);
			if (pepb & 0x00000001)	// vuol dire che bisogna decodificare secondo CRC 32
			{
				/*per fare il crc32 occorre invertire i byte in ingresso, invertire il crc calcolato
			    e farne il complemento a 1*/
				ResetCRC();
				for (i=0; i < ldpepb; i++)
					UpdateCRC32(reflectByte(buff[i]));	
				reflectCRC32();
				crcSum ^= 0xffffffff;	// effettua il complemento a 1	
				cio_seek(posdata2);
				datacrc = cio_read(4);
				if (datacrc == crcSum)
					printf("CRC corretto!\n");
				else
					printf("CRC errato!\n");
			}
			else	// vuol dire che bisogna decodificare secondo CRC 16
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
			return (posdata2 + ldata + ldpepb);  // ritorna la posizione a valle dei dati successivi a EPB
			//return;
		}

		if (pepb>=0x30000000)
		{
			// tecniche registrate in RA
			return (posdata2 + ldata + ldpepb);
			// Per adesso prevede la semplice uscita dalla funzione
		}

		if (pepb==0xffffffff)
		{
			// non sono usati metodi per i dati seguenti
			return (posdata2 + ldata + ldpepb);
			//return;
		}
	}

	

	/*******************/
	// qui bisogna aggiungere la parte per la gestione della modalità packed/unpacked
	/*******************/

	
	//cio_seek(posdata + (nn-kk)); 
	//posdata2 = cio_tell(); // posizione inizio seconda parte dati EPB
	
	/********************/
	// Per adesso si suppone che il primo EPB di un header utilizza lo stesso codice
	// di default anche per la seconda parte dei dati...in seguito bisognerà aggiungere
	// la funzionalità che gestisce l'uso dei vari codici in base al campo pepb
	/********************/

	// Ora bisogna copiare in buff la seconda parte dei dati di EPB e i dati successivi all'epb
	// per una lunghezza pari a ldpepb
    
	nblock = ldata / (nn-kk);  // numero di "blocchi di decodifica"
	//printf("nblock = %d\n",nblock);
	//*system("pause");
	//cio_seek(posdata2);  // si posiziona all'inizio della seconda parte dei dati EPB
	free(buff);
	buff = (char *) malloc(nn1 * sizeof(char));
	for (i=0; i<nblock; i++)
	{
		//free(buff);
		//buff = (char *) malloc(nn1 * sizeof(char));
		for (h=0; h<nn1; h++)
			buff[h] = 0; // inizializza il buffer tutto a zero
		write_buff(buff,posdata2+i*(nn-kk),(nn-kk)); // copia nel buff i bytes di parità
		cio_seek(posdata2+ldata+i*kk); // si posiziona nel blocco dati corrispondente
		
		//if (i==0) {
		//	printf("data: %x\n",cio_read(2));
		//    cio_skip(-2);
		//	system("pause");
		//}
		//pos1 = cio_tell(); // memorizza la posizione del blocco dati corrispondente
		if (i<(nblock-1))
		{
			//for (h=(posdata2+i*(2*tt)); h<(posdata2+i*(2*tt))+64; h++)
			for (h=(nn-kk); h<nn; h++)
			{
				buff[h] = cio_read(1);  // copia nel buff i bytes di messaggio
				//if (i==1)
				//  printf("Data: %x\n",buff[h]);  /**********/
			}
			//system("pause"); /***********/
		}
		else
		{
			ndata = ldpepb - ((nblock-1) * kk);  // l'ultimo blocco di dati non necessariamente è lungo 64!
			//*printf("ndata: %d\n",ndata);
			//*system("pause");
			//for (h=(posdata2+i*(2*tt)); h<(posdata2+i*(2*tt))+ndata; h++)
			for (h=(nn-kk); h<(nn-kk)+ndata; h++)
			{
				buff[h] = cio_read(1);  // copia nel buff i bytes di messaggio
				//printf("Data: %x\n",buff[h]);  /**********/
			}
			//system("pause"); /***********/
			next = cio_tell();  // posizione alla fine dei dati protetti (ldpepb)
			if (cio_read(2) == 0xffd9)
				*j2k_state = J2K_STATE_MT;
			cio_skip(-2);
		}
		
		for (h=0; h<nn1; h++)
			recd[h] = buff[h];  // copia in recd il contenuto di buff da decodificare

		//if (*j2k_state == J2K_STATE_TPHSOT)
		//{
		//	f = fopen("debug","w");
		//	if (f==NULL)
		//		printf("Unable to open file!\n");
		//	fprintf(f,"\n");
		//	for (h=0; h<nn1; h++)
		//		fprintf(f,"%x ",recd[h]);
		//	fprintf(f,"\n");
		//	fclose(f);
		//}
		//else
		//{
		//f = fopen("debug","a");
		//if (f==NULL)
		//	printf("Unable to open file!\n");
		//fprintf(f,"\n");
		//for (h=0; h<nn1; h++)
		//	fprintf(f,"%x ",recd[h]);
		//fprintf(f,"\n");
		//fclose(f);
		//}
		//for (h=0; h<nn1; h++)
		//	printf("mess: %x\n",recd[h]);
		//system("pause");

		//printf("nn1: %d\n",nn1);
		//printf("kk1: %d\n",kk1);
        
		generate_gf(nn1,kk1) ;
		gen_poly(nn1,kk1) ;
		for (h=0; h<nn1; h++)
			recd[h] = index_of[recd[h]] ;  // a questo punto recd[] contiene i bytes decodificati
		decode_rs(nn1,kk1,tt);  
		
		if (decodeflag == 0) //*******Aggiunto in questa versione 1.7
		{
			//Inizializzo il buffer in cui vado a copiare la RED
			cio_init(red.reddata,cslen); //*******Aggiunta in questa versione 1.7
			cio_seek(redpos);

			//printf("Il blocco corrispondente non è stato decodificato!\n");
			cio_write(posdata2+ldata+i*kk,4); // Scrive il byte di start del range considerato	
			redlen += 4;
			if (i<(nblock -1))
				cio_write(posdata2+ldata+i*kk + kk,4);  // Scrive il byte di end del range
			else
				cio_write(posdata2+ldata+i*kk + ndata,4);  // Scrive il byte di end del range
			redlen += 4;
			// Adesso segnaliamo la presenza di errori con 0xFFFF!!!
			cio_write(0xFFFF,2);
			redlen += 2;
			if ((*j2k_state == J2K_STATE_MH)||(*j2k_state == J2K_STATE_TPHSOT))
				redlenok+=10;
			redpos = cio_tell(); // Memorizza la posizione attuale del buffer RED
			//printf("ciao\n");
		}
		if ((redlen==0)&(redmode==1))
			free(red.reddata);
		
		//*printf("nciclo: %d\n\n",i);
		//for (h=0; h<nn1; h++)
		//	printf("mess: %x\n",recd[h]);
		//system("pause");

		// Adesso bisogna ricopiare il contenuto di recd[] nella codestream
		
		cio_init(cssrc, cslen); //*****Aggiunto in questa versione 1.7
		
		cio_seek(posdata2+i*(nn-kk));  // si posiziona all'inizio del blocco di parità corrispondente
		for (h=0; h<(nn-kk); h++)
		{
			cio_write(recd[h],1);  // copia nella codestream i bytes di parità corretti
			//printf("par: %x\n",recd[h]);
		}
		//system("pause");
		
		cio_seek(posdata2+ldata+i*kk);
		if (i<(nblock-1))
		{
			//for (h=(posdata2+i*(2*tt)); h<(posdata2+i*(2*tt))+64; h++)
			for (h=(nn-kk); h<nn; h++)
				cio_write(recd[h],1);  // copia nella codestream il blocco di dati corretti
		}
		else
		{
			ndata = ldpepb - (nblock-1) * kk;  // l'ultimo blocco di dati non necessariamente è lungo 64!			
			//for (h=(posdata2+i*(2*tt)); h<(posdata2+i*(2*tt))+ndata; h++)
			for (h=(nn-kk); h<(nn-kk)+ndata; h++)
				cio_write(recd[h],1);  // copia nella codestream il blocco di dati corretti
		}
	}//fine ciclo for (i=0; i<nblock; i++)
	

	free(alpha_to);
	free(index_of);
	free(gg);
	free(recd);
	free(data);
	free(bb);
	/***********/

	//} // fine ciclo while iniziale
	free(buff);
	// A questo punto abbiamo corretto anche la parte di dati cui fa riferimento la seconda
	// parte del campo EPBdata

	// Bisogna ora posizionarsi alla fine dei dati corretti
	//next = posdata2 + ldata + ldpepb;
    //cio_seek(next);
	//if (cio_read(2) == 0xffd9)
	//	*j2k_state = J2K_STATE_MT;
	//printf("next: %x\n",cio_read(2));
	//cio_skip(-2);
	
	return next; // Ritorna la posizione subito a valle dell'EPB appena letto!!!
	//return;
	
	//*ltemp=cio_tell();
	//if (cio_read(2)==0xffd9)
	//{
	//	cio_write(0xffd9,2);
	//	cio_skip(-2);
	//}
	//cio_skip(-2);
	//*if (cio_read(2)==0xffd9)
	//*{
	///*	cio_seek(0);
	//	f=fopen("output.txt","wb");
	//*	for (i=0; i<ltemp+2; i++)
	//*		fputc(cio_read(1),f);
    //*    cio_skip(-2);
	//*	printf("EOC: %x\n",cio_read(2));
	//*	fclose(f);
	//*}
	
    //*cio_skip(-2);
	//*printf("next: %x\n",cio_read(2));
	//*cio_skip(-2);
	//*system("pause");
	/***********/
	// liberiamo gli spazi allocati
	
	
	

	//return;
} // fine funzione read_EPB



// La funzione seguente inizializza a zero la struttura relativa ad EPC

//void init_EPC()
//{
//    epc.lepc = 0;
//	epc.pcrc = 0;
//	epc.cl = 0;
//	epc.pepc = 0;
//	epc.tecn = NULL;   
//}

int read_EPB_PM(int *j2k_state)  // Gestisce la lettura degli EPB packed mode
{
	unsigned int lepb, lsiz, temp, ldata, lbuf, lsot;
	int posdata, posdata2, nblock, i,h, pos, lante;
	int lastepb, nepbpm, posfirst, posend, count; // variabili per la gestione modalità packed
	unsigned long lpack, ldpread;	// variabili per la gestione modalità packed
	EPB_par *epb;	// variabili per la gestione modalità packed
	int nn, kk, tt, nn1, kk1; // parametri per la decodifica RS
	unsigned long ldpepb, pepb, ndata, datacrc;  // ndata è utile per la decodifica della seconda parte EPB
	unsigned char depb;
	unsigned char *buff;

	int lparity;
	// int conta;
	//FILE *f;
	
	//next = cio_tell();
	//printf("read: %x\n",cio_read(2));
	//cio_skip(-2);
		if (*j2k_state == J2K_STATE_MHSOC)  
		{
			// Se siamo giunti a questo punto vuol dire che SOC e i primi due campi di SIZ non sono
			// errati!!...ora ci dobbiamo posizionare subito a valle di SIZ
			
			cio_skip(4); // si pone all'inizio del campo Lsiz
			lsiz = cio_read(2);
			cio_skip(lsiz-2); // ora siamo all'inizio dell'EPB MS
			pos = cio_tell(); // memorizza la posizione in cui inizia l'EPB
			temp = cio_read(2);  // ci si aspetta qui di trovare il marker EPB
			nn = 160; kk = 64; tt = 48;  // inizializzazione per codice RS(160,64)
			lante = lsiz+4;
			
		} // fine if (j2k_state == J2K_STATE_MHSOC)

		if (*j2k_state == J2K_STATE_TPHSOT)
		{
			
			cio_skip(2); // si pone all'inizio del campo Lsot
			lsot = cio_read(2);
			cio_skip(lsot-2); // ora siamo all'inizio dell'EPB MS
			pos = cio_tell(); // memorizza la posizione in cui inizia l'EPB
			temp = cio_read(2);  // ci si aspetta qui di trovare il marker EPB
			nn = 80; kk = 25; tt = 28;  // inizializzazione per codice RS(80,25)
			lante = lsot+2;
		}

		if ((*j2k_state == J2K_STATE_MH)||(*j2k_state == J2K_STATE_TPH))
		{
			pos = cio_tell(); // memorizza la posizione in cui inizia l'EPB
			temp = cio_read(2); // ci si aspetta qui di trovare il marker EPB
			nn = 40; kk = 13; tt = 14;  // inizializzazione per codice RS(40,13)
			lante = 0;
		}
	// A questo punto possiamo decodificare la prima parte di dati tramite i codici di default

	//printf("state: %x\n",*j2k_state);
	//printf("tt: %d\n",nn-kk);
	posfirst = pos; // memorizza la posizione al'inizio della catena di EPB packed
	nn1 = 255; kk1 = kk + (nn1 - nn);
	alpha_to = (int *) malloc((nn1+1)*sizeof(int));
	index_of = (int *) malloc((nn1+1)*sizeof(int));
	gg = (int *) malloc((nn1-kk1+1)*sizeof(int));
	recd = (int *) malloc((nn1)*sizeof(int)); ///forse alcune di queste malloc possono
	data = (int *) malloc((kk1)*sizeof(int)); // essere eliminate, inutili per decodifica!!!
	bb = (int *) malloc((nn1-kk1)*sizeof(int));

	lastepb = 0; // Si suppone che l'EPB corrente non sia l'ultimo di un tile!
	nepbpm = 0;
	//conta = 0;

	while (lastepb == 0)
	{

	lepb = cio_read(2);  // legge la lunghezza di EPB..si spera che questo campo non è errato!
	cio_skip(9);  // si posiziona all'inizio del campo dati di EPB
	posdata = cio_tell();
	ldata = lepb - 11;  // determina la lunghezza del campo dati
	//printf("ldata: %d\n",ldata);
	lbuf = lante + 13 + (nn-kk);  // 2*tt è la lunghezza dei bit di parità 
	buff = (char *) malloc(nn1 * sizeof(char)); // buffer che conterrà tutti i dati che precedono EPB
	                                           // e i parametri di EPB
	for (i=0; i<nn1; i++)
		buff[i] = 0; // inizializza il buffer tutto a zero
	
	// Bisognerà copiare tutto il contenuto da questo punto fino alla fine della prima parte dei dati EPB in buff
	// Per come lavora il decoder RS, i bytes di parità vanno posti all'inizio del buffer

	
	write_buff(buff,posdata,(nn-kk)); // copia nel buffer i byte di parità del campo dati
	
	cio_seek(pos);  // si posiziona all'inizio dei dati protetti (SOC,SOT o EPB)
	for (i=(nn-kk); i<lbuf; i++)
	{
		buff[i] = cio_read(1);  // copia in buff i byte di messaggio (SOC,SIZ,ParEPB)
	}

	for (i=0; i<nn1; i++)
		recd[i] = buff[i];  // copia in recd il contenuto di buff da decodificare

	//f = fopen("debug","a");
	//if (f==NULL)
	//	printf("Unable to open file!\n");
	//fprintf(f,"EPB PAR: %d\n",conta);
	//for (h=0; h<nn1; h++)
	//	fprintf(f,"%x ",recd[h]);
	//fprintf(f,"\n");
	//fclose(f);

	generate_gf(nn1,kk1) ;
	gen_poly(nn1,kk1) ;
	for (i=0; i<nn1; i++)
     recd[i] = index_of[recd[i]] ; // a questo punto recd[] contiene i bytes decodificati     
	decode_rs(nn1,kk1,tt);  

	if (decodeflag == 0) //*******Aggiunto in questa versione 1.7
		{
			//Inizializzo il buffer in cui vado a copiare la RED
			cio_init(red.reddata,cslen); //*******Aggiunta in questa versione 1.7
			cio_seek(redpos);

			//printf("Il blocco corrispondente non è stato decodificato!\n");
			cio_write(pos,4); // Scrive il byte di start del range considerato	
			cio_write(pos + lbuf - (nn - kk),4);  // Scrive il byte di end del range
			// Adesso segnaliamo la presenza di errori con 0xFFFF!!!
			cio_write(0xFFFF,2);
			redlen += 10;
			redpos = cio_tell(); // Memorizza la posizione attuale del buffer RED
			//printf("ciao\n");
		}
	// Adesso bisogna ricopiare il contenuto di recd[] nella codestream

	cio_init(cssrc, cslen);

	cio_seek(posdata); // si posiziona all'inizio del campo dati della codestream	
	for (i=0; i<(nn-kk); i++)
		cio_write(recd[i],1);  // copia i byte di parità corretti nel campo dati
	cio_seek(pos);
	for (i=(nn-kk); i<lbuf; i++)
		cio_write(recd[i],1);  // copia i bytes di messaggio nella codestream

	// A questo punto la codestream è corretta fino alla fine della prima parte dei dati EPB
	// Possiamo leggere i parametri di EPB per condurre la codifica seguente

	
	cio_seek(pos);  // si posiziona all'inizio di EPB
	temp = cio_read(2);  // ci si aspetta qui di trovare il marker EPB
			if (temp != JPWL_MS_EPB)
			{
				// Puo' succedere che l'EPC ha fornito informazione errata: in tal caso il
				// processo di decodifica effettuato perde di significato.
				// Puo' anche succedere pero' che il codice RS contenuto nell'EPB MS non
				// è stato in grado di correggere l'errore sul marker EPB!!
				return 0;
				// Per adesso usciamo dalla procedura, ma la cosa migliore sarebbe
				// fare in modo che il decoder vada a cercare l'eventuale EPB successivo
			}

	cio_skip(2); // ora è all'inizio del campo depb
	depb = cio_read(1); // legge depb
	//if ((depb >> 6)&1)  // quello corrente è l'ultimo EPB del tile corrente
	//{
	//	lastepb = 1; // l'epb corrente è l'ultimo del tile
		//nepbpm = ((depb << 2) >> 2); // numero di EPB accorpati in modalità packed
	//	nepbpm = (depb & 0x3f); // numero di EPB accorpati in modalità packed
		//printf("nepbpm: %d\n",nepbpm);
	//}
	if (!((depb >> 6)&1))  // quello corrente non è l'ultimo EPB del tile corrente
		nepbpm += 1;
	if ((depb >> 6)&1)  // quello corrente è l'ultimo EPB del tile corrente
	{
		nepbpm += 1;
		lastepb = 1;
	}

	cio_skip(-3); // si posiziona all'inizio del campo lepb
	cio_skip(cio_read(2)-2);  // si posiziona a valle dell'epb corrente
	pos = cio_tell(); // memorizza la posizione all'inizio dell'EPB successivo
	cio_skip(2);

	//conta++;

	} // Fine while (lastepb == 0)!!!!
    // A questo punto il decoder ha decodificato le porzioni iniziali di tutti gli EPB
	// del tile corrente
	

	// Ora dobbiamo decodificare le porzioni finali di tutti gli EPB!!!

	// pos contiene la posizione a valle dell'ultimo degli EPB packed!!!
	cio_skip(-2);
	posend = cio_tell();
	lpack = posend-posfirst; // lunghezza totale della catena di EPB
	epb = (EPB_par *) malloc(nepbpm * sizeof(EPB_par));
	cio_seek(posfirst);
	//printf("nepbpm: %d\n",nepbpm);
	for (count=0; count<nepbpm; count++)
	{
		cio_skip(2); // si posiziona all'inizio di lepb
		epb[count].lepb = cio_read(2); // legge lepb
		epb[count].depb = cio_read(1); // legge depb
		epb[count].ldpepb = cio_read(4); // legge ldpepb
		epb[count].pepb = cio_read(4); // legge pepb

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

		epb[count].ldata = (epb[count].lepb - 11) - (nn-kk);  // lunghezza della porzione rimanente del campo dati
		cio_skip(-11); // si posiziona all'inizio di lepb dell'EPB corrente
	    cio_skip(epb[count].lepb); // si posiziona a valle dell'EPB corrente
	} // Abbiamo a questo punto memorizzato nella struttura epb i parametri degli EPB packed

	//for (count=0; count<nepbpm; count++)
	//{
	//	printf("EPB[%d]: %x\t%x\t%x\t%x\t%d\n",count,epb[count].lepb,epb[count].depb,
	//		epb[count].ldpepb,epb[count].pepb,epb[count].ldata);
	//}

	nepbrd+=nepbpm;

	cio_seek(posfirst);  // si posiziona all'inizio del primo degli EPB packed
	pos = cio_tell();
	ldpread = 0;
	lparity = nn - kk;
	//printf("lparity: %d\n",lparity);
	for (count=0; count<nepbpm; count++)
	{
		cio_seek(pos);
		//printf("mark: %x\n",cio_read(2));
		//cio_skip(-2);
		cio_skip(13); // si posiziona all'inizio del campo dati
		posdata = cio_tell();
		//printf("tt: %d\n",nn-kk);
		cio_seek(posdata + lparity); // si posiziona all'inizio seconda parte dati EPB corrente
		posdata2 = cio_tell(); // posizione inizio seconda parte dati EPB
		//printf("rd: %x\n",cio_read(2));
		//cio_skip(-2);

		if ((!((epb[count].depb >> 6)&1))&&((*j2k_state == J2K_STATE_MHSOC)||(*j2k_state == J2K_STATE_MH)))
			*j2k_state = J2K_STATE_MH; // vuol dire che il prossimo EPB è in MH ma non è il primo
		if (((epb[count].depb >> 6)&1)&&((*j2k_state == J2K_STATE_MH)||(*j2k_state == J2K_STATE_MHSOC)))
			*j2k_state = J2K_STATE_TPHSOT; // vuol dire che il prossimo EPB è il primo di un TPH
		if (((epb[count].depb >> 6)&1)&&((*j2k_state == J2K_STATE_TPH)||(*j2k_state == J2K_STATE_TPHSOT)))
			*j2k_state = J2K_STATE_TPHSOT; // vuol dire che il prossimo EPB è il primo di un TPH
		if ((!((epb[count].depb >> 6)&1))&&((*j2k_state == J2K_STATE_TPHSOT)||(*j2k_state == J2K_STATE_TPH)))
			*j2k_state = J2K_STATE_TPH; // vuol dire che il prossimo EPB è relativo ad un TPH
		
		// Ora leggendo pepb il decoder deve capire quale codice applicare per la porzione di dati
		// cui fa riferimento la seconda parte del campo EPBdata
		
		if (epb[count].pepb)  // se pepb=0 allora si usano i codici di default precedenti
		{
			if ((epb[count].pepb>>28)==2)
			{
				// in questo caso deve effettuare la decodifica RS
				/***********/
				// liberiamo gli spazi allocati
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
				recd = (int *) malloc((nn1)*sizeof(int)); ///forse alcune di queste malloc possono
				data = (int *) malloc((kk1)*sizeof(int)); // essere eliminate, inutili per decodifica!!!
				bb = (int *) malloc((nn1-kk1)*sizeof(int));
				generate_gf(nn1,kk1) ;
				gen_poly(nn1,kk1) ;
			}

			if ((epb[count].pepb>>28)==1)
			{
				// in questo caso deve effettuare le decodifica CRC
				free(buff);
				buff = (char *) malloc(epb[count].ldpepb * sizeof(char));
				write_buff(buff,posdata2+epb[count].ldata,epb[count].ldpepb);
				if (epb[count].pepb & 0x00000001)	// vuol dire che bisogna decodificare secondo CRC 32
				{
					/*per fare il crc32 occorre invertire i byte in ingresso, invertire il crc calcolato
					e farne il complemento a 1*/
					ResetCRC();
					cio_seek(posend+ldpread); // si posiziona nel blocco dati corrispondente
					for (i=0; i < epb[count].ldpepb; i++)
						UpdateCRC32(reflectByte(buff[i]));	
					reflectCRC32();
					if (lastepb==1)
					{
						next = startsot + psot;
						cio_seek(next);
						//printf("%x\n",cio_read(2));
						//cio_skip(-2);
					}
					if ((cio_read(2) == 0xffd9)||(psot == 0))
						*j2k_state = J2K_STATE_MT;
					cio_skip(-2);
					//else
					//{
					//	next = cio_tell();  // posizione alla fine dei dati protetti (ldpepb)
					//}
					crcSum ^= 0xffffffff;	// effettua il complemento a 1	
					cio_seek(posdata2);
					datacrc = cio_read(4);
					if (datacrc == crcSum)
						printf("CRC corretto!\n");
					else
						printf("CRC errato!\n");
					
				}
				else	// vuol dire che bisogna decodificare secondo CRC 16
				{
					ResetCRC();
					cio_seek(posend+ldpread); // si posiziona nel blocco dati corrispondente
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
				//free(buff);
			}

			if (epb[count].pepb>=0x30000000)
			{
				//if (lastepb==1)
				//		next = startsot + psot;
				// tecniche registrate in RA
			}

			if (epb[count].pepb==0xffffffff)
			{
				//if (lastepb==1)
				//		next = startsot + psot;
				// non sono usati metodi per i dati seguenti
			}
		}
		
		if (((epb[count].pepb>>28)==2)||(epb[count].pepb==0))  // Vuol dire che si usano codici RS
		{
		// Ora bisogna copiare in buff la seconda parte dei dati di EPB e i dati successivi all'epb
		// per una lunghezza pari a ldpepb
    
		//printf("posiz: %x\n",posdata2 + epb[count].ldata);
		nblock = epb[count].ldata / (nn-kk);  // numero di "blocchi di decodifica"
		free(buff);
		buff = (char *) malloc(nn1 * sizeof(char));
		//printf("ldata: %d\n",epb[count].ldata);
		//printf("nblock: %d\n",nblock);
		for (i=0; i<nblock; i++)
		{
			//free(buff);
			//buff = (char *) malloc(nn1 * sizeof(char));
			for (h=0; h<nn1; h++)
				buff[h] = 0; // inizializza il buffer tutto a zero
			write_buff(buff,posdata2+i*(nn-kk),(nn-kk)); // copia nel buff i bytes di parità
			//if (i==(nblock-1))
			//{
			//	for (h=0; h<(nn-kk); h++)
			//		printf("%x\n",buff[h]);
			//	system("pause");
			//}
			cio_seek(posend+ldpread+i*kk); // si posiziona nel blocco dati corrispondente
			if (i<(nblock-1))
			{
				for (h=(nn-kk); h<nn; h++)
				{
					buff[h] = cio_read(1);  // copia nel buff i bytes di messaggio
				}
			}
			else
			{
				ndata = epb[count].ldpepb - ((nblock-1) * kk);  // l'ultimo blocco di dati non necessariamente è lungo 64!
				for (h=(nn-kk); h<(nn-kk)+ndata; h++)
				{
					buff[h] = cio_read(1);  // copia nel buff i bytes di messaggio
				}
				if (lastepb==1)
				{
					next = startsot + psot;
					cio_seek(next);
				}
				//else
				//{
				//	next = cio_tell();  // posizione alla fine dei dati protetti (ldpepb)
				//}
				//next = cio_tell();  // posizione alla fine dei dati protetti (ldpepb)
				if ((cio_read(2) == 0xffd9)||(psot == 0))
					*j2k_state = J2K_STATE_MT;
				cio_skip(-2);
			}
		
			for (h=0; h<nn1; h++)
				recd[h] = buff[h];  // copia in recd il contenuto di buff da decodificare
        
			//f = fopen("debug","a");
			//if (f==NULL)
			//	printf("Unable to open file!\n");
			//fprintf(f,"EPB DATA: %d-%d\n",count,i);
			//for (h=0; h<nn1; h++)
			//	fprintf(f,"%x ",recd[h]);
			//fprintf(f,"\n");
			//fclose(f);

			generate_gf(nn1,kk1) ;
			gen_poly(nn1,kk1) ;
			for (h=0; h<nn1; h++)
				recd[h] = index_of[recd[h]] ;// a questo punto recd[] contiene i bytes decodificati
			decode_rs(nn1,kk1,tt);  

			if (decodeflag == 0) //*******Aggiunto in questa versione 1.7
			{
				//Inizializzo il buffer in cui vado a copiare la RED
				cio_init(red.reddata,cslen); //*******Aggiunta in questa versione 1.7
				cio_seek(redpos);

				//printf("Il blocco corrispondente non è stato decodificato!\n");
				cio_write(posend+ldpread+i*kk,4); // Scrive il byte di start del range considerato	
				if (i<(nblock -1))
					cio_write(posend+ldpread+i*kk + kk,4);  // Scrive il byte di end del range
				else
					cio_write(posend+ldpread+i*kk + ndata,4);  // Scrive il byte di end del range
				// Adesso segnaliamo la presenza di errori con 0xFFFF!!!
				cio_write(0xFFFF,2);
				redlen+=10;
				redpos = cio_tell(); // Memorizza la posizione attuale del buffer RED
				//printf("ciao\n");
			}

			// Adesso bisogna ricopiare il contenuto di recd[] nella codestream

			cio_init(cssrc, cslen);

			cio_seek(posdata2+i*(nn-kk));  // si posiziona all'inizio del blocco di parità corrispondente
			for (h=0; h<(nn-kk); h++)
				cio_write(recd[h],1);  // copia nella codestream i bytes di parità corretti
		
			//cio_seek(posdata2+epb[count].ldata+i*kk);// si posiziona all'inizio del blocco dati corrispondente
			cio_seek(posend+ldpread+i*kk);// si posiziona all'inizio del blocco dati corrispondente
			if (i<(nblock-1))
			{
				for (h=(nn-kk); h<nn; h++)
					cio_write(recd[h],1);  // copia nella codestream il blocco di dati corretti
			}
			else
			{
				ndata = epb[count].ldpepb - (nblock-1) * kk;  // l'ultimo blocco di dati non necessariamente è lungo 64!			
				for (h=(nn-kk); h<(nn-kk)+ndata; h++)
					cio_write(recd[h],1);  // copia nella codestream il blocco di dati corretti
			}
		}//fine ciclo for (i=0; i<nblock; i++)

		} // fine if (!(epb[count]->pepb))
	
	// A questo punto abbiamo corretto anche la parte di dati cui fa riferimento la seconda
	// parte del campo EPBdata

		ldpread += epb[count].ldpepb;
		cio_seek(pos+2);
		cio_skip(epb[count].lepb);
		pos = cio_tell(); // posizione a valle dell'EPB corrente

	} // fine for (count=0; count<nepbpm; count++)

	cio_seek(next); // si posiziona alla fine del tile
	//printf("read: %x\n",cio_read(2));
	//cio_skip(-2);
	
	free(alpha_to);
	free(index_of);
	free(gg);
	free(recd);
	free(data);
	free(bb);
	free(buff);
	
	return nepbpm; // Ritorna il numero di EPB letti in modalità packed
	
}

/*******************************************************************/
///////////////FUNZIONE AGGIUNTA IN QUESTA VERSIONE//////////////////
void insert_RED(int pos, int lred, int redlenok)
{
	unsigned long i;
	unsigned char *buff;
	int temp, mem;
	// Buffer in cui verrà salvata la codestream a partire dal primo SOT
	buff = (char *) malloc((epc.cl-pos) * sizeof(char));
	//printf("lung: %d\n",epc.cl-pos);
	for (i=0; i<(epc.cl-pos); i++)
	{
		buff[i] = cio_read(1);
		//printf("i: %d\n",i);
	}
	//printf("fine: %x\n",buff[epc.cl-pos-1]);
	// A questo punto andiamo a scrivere il marker segment RED
	cio_seek(pos);
	//cio_skip(-2);
	//printf("red: %x\n",cio_read(2));
	cio_write(JPWL_MS_RED,2); // Inserisce il marker
	cio_write(red.lred,2); // Inserisce il campo Lred
	cio_write(red.pred,1); // Inserisce il campo Pred
	//printf("redlen: %d\n",redlen);
	temp = cio_tell(); // Memorizza posizione corrente della CS
	
	//printf("redlenok: %d\n",redlenok);
	//for (i=0; i<redlen; i++)
	//	printf("%x",red.reddata[i]);
	//printf("\n");
	//printf("lred: %d\n",lred);
	cio_init(red.reddata,cslen); // Aggiorna le posizioni a causa dell'offset introdotto
	cio_seek(redlenok);			// dall'aggiunta del MS RED
	for (i=0; i<(redlen-redlenok)/10; i++)
	{
		mem = cio_read(4);
		//printf("mem: %x\n",mem);
		cio_skip(-4);
		cio_write(mem + lred,4); // Aggiorna il byte di inizio
		mem = cio_read(4);
		//printf("mem: %x\n",mem);
		cio_skip(-4);
		cio_write(mem + lred,4); // Aggiorna il byte di fine
	}

	cio_init(cssrc,epc.cl+redlen+5);
	cio_seek(temp);

	for (i=0; i<redlen; i++)
		cio_write(red.reddata[i],1); // Copio il buffer reddata nella codestream
	// Adesso andiamo a riaggiungere il resto della codestream
	//printf("cl: %d\n",epc.cl);
	//printf("pos: %d\n",pos);
	//printf("cl-pos: %d\n",epc.cl-pos);
	//printf("fine: %x\n",buff[epc.cl-pos]);
	cio_init(cssrc,epc.cl+redlen+5);
	cio_seek(pos + redlen + 5);
	for (i=0; i<(epc.cl-pos); i++)
	{
		cio_write(buff[i],1);
		//printf("i: %d\n",i);
	}
	cio_skip(-2);
	//printf("fine: %x\n",cio_read(2));
}
/*******************************************************************/

// La funzione seguente legge la codestream a partire da pos e la copia in buff
void write_buff(unsigned char *buff,int pos,long cl)
{
    long i;
	cio_seek(pos);
	for (i=0; i<cl; i++) 
		 buff[i] = cio_read(1);	 
}

// La funzione seguente copia il contenuto di buff a partire dalla posizione della
// stream p
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
/* funzioni per l'inversione dei byte e del crc32 */

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

// Codice relativo alla CODIFICA/DECODIFICA RS



//#define nn  255          /* nn=2**mm -1   length of codeword */
//#define tt  48           /* number of errors that can be corrected */
//#define kk  159           /* kk = nn-2*tt  */

//int pp [mm+1] = { 1, 1, 0, 0, 1} ; /* specify irreducible polynomial coeffts */



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



