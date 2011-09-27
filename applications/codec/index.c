/*
 * Copyright (c) 2002-2007, Communications and Remote Sensing Laboratory, Universite catholique de Louvain (UCL), Belgium
 * Copyright (c) 2002-2007, Professor Benoit Macq
 * Copyright (c) 2003-2007, Francois-Olivier Devaux 
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

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "openjpeg.h"
#include "index.h"

/* ------------------------------------------------------------------------------------ */

/**
Write a structured index to a file
@param cstr_info Codestream information 
@param index Index filename
@return Returns 0 if successful, returns 1 otherwise
*/
int write_index_file(opj_codestream_info_t *cstr_info, char *index) {
	int tileno, compno, layno, resno, precno, pack_nb, x, y;
	FILE *stream = NULL;
	double total_disto = 0;
/* UniPG>> */
	int tilepartno;
	char disto_on, numpix_on;

#ifdef USE_JPWL
	if (!strcmp(index, JPWL_PRIVATEINDEX_NAME))
		return 0;
#endif /* USE_JPWL */
/* <<UniPG */

	if (!cstr_info)		
		return 1;

	stream = fopen(index, "w");
	if (!stream) {
		fprintf(stderr, "failed to open index file [%s] for writing\n", index);
		return 1;
	}
	
	if (cstr_info->tile[0].distotile)
		disto_on = 1;
	else 
		disto_on = 0;

	if (cstr_info->tile[0].numpix)
		numpix_on = 1;
	else 
		numpix_on = 0;

	fprintf(stream, "%d %d\n", cstr_info->image_w, cstr_info->image_h);
	fprintf(stream, "%d\n", cstr_info->prog);
	fprintf(stream, "%d %d\n", cstr_info->tile_x, cstr_info->tile_y);
	fprintf(stream, "%d %d\n", cstr_info->tw, cstr_info->th);
	fprintf(stream, "%d\n", cstr_info->numcomps);
	fprintf(stream, "%d\n", cstr_info->numlayers);
	fprintf(stream, "%d\n", cstr_info->numdecompos[0]); /* based on component 0 */

	for (resno = cstr_info->numdecompos[0]; resno >= 0; resno--) {
		fprintf(stream, "[%d,%d] ", 
			(1 << cstr_info->tile[0].pdx[resno]), (1 << cstr_info->tile[0].pdx[resno]));	/* based on tile 0 and component 0 */
	}

	fprintf(stream, "\n");
/* UniPG>> */
	fprintf(stream, "%d\n", cstr_info->main_head_start);
/* <<UniPG */
	fprintf(stream, "%d\n", cstr_info->main_head_end);
	fprintf(stream, "%d\n", cstr_info->codestream_size);
	
	fprintf(stream, "\nINFO ON TILES\n");
	fprintf(stream, "tileno start_pos  end_hd  end_tile   nbparts");
	if (disto_on)
		fprintf(stream,"         disto");
	if (numpix_on)
		fprintf(stream,"     nbpix");
	if (disto_on && numpix_on)
		fprintf(stream,"  disto/nbpix");
	fprintf(stream, "\n");

	for (tileno = 0; tileno < cstr_info->tw * cstr_info->th; tileno++) {
		fprintf(stream, "%4d %9d %9d %9d %9d", 
			cstr_info->tile[tileno].tileno,
			cstr_info->tile[tileno].start_pos,
			cstr_info->tile[tileno].end_header,
			cstr_info->tile[tileno].end_pos,
			cstr_info->tile[tileno].num_tps);
		if (disto_on)
			fprintf(stream," %9e", cstr_info->tile[tileno].distotile);
		if (numpix_on)
			fprintf(stream," %9d", cstr_info->tile[tileno].numpix);
		if (disto_on && numpix_on)
			fprintf(stream," %9e", cstr_info->tile[tileno].distotile / cstr_info->tile[tileno].numpix);
		fprintf(stream, "\n");
	}
		
	for (tileno = 0; tileno < cstr_info->tw * cstr_info->th; tileno++) {
		int start_pos, end_ph_pos, end_pos;
		double disto = 0;
		int max_numdecompos = 0;
		pack_nb = 0;

		for (compno = 0; compno < cstr_info->numcomps; compno++) {
			if (max_numdecompos < cstr_info->numdecompos[compno])
				max_numdecompos = cstr_info->numdecompos[compno];
		}	

		fprintf(stream, "\nTILE %d DETAILS\n", tileno);	
		fprintf(stream, "part_nb tileno  start_pack num_packs  start_pos end_tph_pos   end_pos\n");
		for (tilepartno = 0; tilepartno < cstr_info->tile[tileno].num_tps; tilepartno++)
			fprintf(stream, "%4d %9d   %9d %9d  %9d %11d %9d\n",
				tilepartno, tileno,
				cstr_info->tile[tileno].tp[tilepartno].tp_start_pack,
				cstr_info->tile[tileno].tp[tilepartno].tp_numpacks,
				cstr_info->tile[tileno].tp[tilepartno].tp_start_pos,
				cstr_info->tile[tileno].tp[tilepartno].tp_end_header,
				cstr_info->tile[tileno].tp[tilepartno].tp_end_pos
				);

		if (cstr_info->prog == LRCP) {	/* LRCP */
			fprintf(stream, "LRCP\npack_nb tileno layno resno compno precno start_pos end_ph_pos end_pos");
			if (disto_on)
				fprintf(stream, " disto");
			fprintf(stream,"\n");

			for (layno = 0; layno < cstr_info->numlayers; layno++) {
				for (resno = 0; resno < max_numdecompos + 1; resno++) {
					for (compno = 0; compno < cstr_info->numcomps; compno++) {
						int prec_max;
						if (resno > cstr_info->numdecompos[compno])
							break;
						prec_max = cstr_info->tile[tileno].pw[resno] * cstr_info->tile[tileno].ph[resno];
						for (precno = 0; precno < prec_max; precno++) {
							start_pos = cstr_info->tile[tileno].packet[pack_nb].start_pos;
							end_ph_pos = cstr_info->tile[tileno].packet[pack_nb].end_ph_pos;
							end_pos = cstr_info->tile[tileno].packet[pack_nb].end_pos;
							disto = cstr_info->tile[tileno].packet[pack_nb].disto;
							fprintf(stream, "%4d %6d %7d %5d %6d  %6d    %6d     %6d %7d",
								pack_nb, tileno, layno, resno, compno, precno, start_pos, end_ph_pos, end_pos);
							if (disto_on)
								fprintf(stream, " %8e", disto);
							fprintf(stream, "\n");
							total_disto += disto;
							pack_nb++;
						}
					}
				}
			}
		} /* LRCP */

		else if (cstr_info->prog == RLCP) {	/* RLCP */			
			fprintf(stream, "RLCP\npack_nb tileno resno layno compno precno start_pos end_ph_pos end_pos\n");
			if (disto_on)
				fprintf(stream, " disto");
			fprintf(stream,"\n");

			for (resno = 0; resno < max_numdecompos + 1; resno++) {
				for (layno = 0; layno < cstr_info->numlayers; layno++) {
					for (compno = 0; compno < cstr_info->numcomps; compno++) {
						int prec_max; 
						if (resno > cstr_info->numdecompos[compno])
							break;
						prec_max = cstr_info->tile[tileno].pw[resno] * cstr_info->tile[tileno].ph[resno];
						for (precno = 0; precno < prec_max; precno++) {
							start_pos = cstr_info->tile[tileno].packet[pack_nb].start_pos;
							end_ph_pos = cstr_info->tile[tileno].packet[pack_nb].end_ph_pos;
							end_pos = cstr_info->tile[tileno].packet[pack_nb].end_pos;
							disto = cstr_info->tile[tileno].packet[pack_nb].disto;
							fprintf(stream, "%4d %6d %5d %7d %6d %6d %9d   %9d %7d",
								pack_nb, tileno, resno, layno, compno, precno, start_pos, end_ph_pos, end_pos);
							if (disto_on)
								fprintf(stream, " %8e", disto);
							fprintf(stream, "\n");
							total_disto += disto;
							pack_nb++;
						}
					}
				}
			}
		} /* RLCP */

		else if (cstr_info->prog == RPCL) {	/* RPCL */

			fprintf(stream, "RPCL\npack_nb tileno resno precno compno layno start_pos end_ph_pos end_pos"); 
			if (disto_on)
				fprintf(stream, " disto");
			fprintf(stream,"\n");

			for (resno = 0; resno < max_numdecompos + 1; resno++) {
				int numprec = cstr_info->tile[tileno].pw[resno] * cstr_info->tile[tileno].ph[resno];
				for (precno = 0; precno < numprec; precno++) {								
					/* I suppose components have same XRsiz, YRsiz */
					int x0 = cstr_info->tile_Ox + tileno - (int)floor((float)tileno/(float)cstr_info->tw ) * cstr_info->tw * cstr_info->tile_x;
					int y0 = cstr_info->tile_Ox + (int)floor( (float)tileno/(float)cstr_info->tw ) * cstr_info->tile_y;
					int x1 = x0 + cstr_info->tile_x;
					int y1 = y0 + cstr_info->tile_y;
					for (compno = 0; compno < cstr_info->numcomps; compno++) {					
						int pcnx = cstr_info->tile[tileno].pw[resno];
						int pcx = (int) pow( 2, cstr_info->tile[tileno].pdx[resno] + cstr_info->numdecompos[compno] - resno );
						int pcy = (int) pow( 2, cstr_info->tile[tileno].pdy[resno] + cstr_info->numdecompos[compno] - resno );
						int precno_x = precno - (int) floor( (float)precno/(float)pcnx ) * pcnx;
						int precno_y = (int) floor( (float)precno/(float)pcnx );
						if (resno > cstr_info->numdecompos[compno])
							break;
						for(y = y0; y < y1; y++) {							
							if (precno_y*pcy == y ) {
								for (x = x0; x < x1; x++) {									
									if (precno_x*pcx == x ) {
										for (layno = 0; layno < cstr_info->numlayers; layno++) {
											start_pos = cstr_info->tile[tileno].packet[pack_nb].start_pos;
											end_ph_pos = cstr_info->tile[tileno].packet[pack_nb].end_ph_pos;
											end_pos = cstr_info->tile[tileno].packet[pack_nb].end_pos;
											disto = cstr_info->tile[tileno].packet[pack_nb].disto;
											fprintf(stream, "%4d %6d %5d %6d %6d %7d %9d   %9d %7d",
												pack_nb, tileno, resno, precno, compno, layno, start_pos, end_ph_pos, end_pos); 
											if (disto_on)
												fprintf(stream, " %8e", disto);
											fprintf(stream, "\n");
											total_disto += disto;
											pack_nb++; 
										}
									}
								}/* x = x0..x1 */
							} 
						}  /* y = y0..y1 */
					} /* precno */
				} /* compno */
			} /* resno */
		} /* RPCL */

		else if (cstr_info->prog == PCRL) {	/* PCRL */
			/* I suppose components have same XRsiz, YRsiz */
			int x0 = cstr_info->tile_Ox + tileno - (int)floor( (float)tileno/(float)cstr_info->tw ) * cstr_info->tw * cstr_info->tile_x;
			int y0 = cstr_info->tile_Ox + (int)floor( (float)tileno/(float)cstr_info->tw ) * cstr_info->tile_y;
			int x1 = x0 + cstr_info->tile_x;
			int y1 = y0 + cstr_info->tile_y;

			// Count the maximum number of precincts 
			int max_numprec = 0;
			for (resno = 0; resno < max_numdecompos + 1; resno++) {
				int numprec = cstr_info->tile[tileno].pw[resno] * cstr_info->tile[tileno].ph[resno];
				if (numprec > max_numprec)
					max_numprec = numprec;
			}

			fprintf(stream, "PCRL\npack_nb tileno precno compno resno layno start_pos end_ph_pos end_pos"); 
			if (disto_on)
				fprintf(stream, " disto");
			fprintf(stream,"\n");

			for (precno = 0; precno < max_numprec; precno++) {
				for (compno = 0; compno < cstr_info->numcomps; compno++) {
					for (resno = 0; resno < cstr_info->numdecompos[compno] + 1; resno++) {
						int numprec = cstr_info->tile[tileno].pw[resno] * cstr_info->tile[tileno].ph[resno];
						int pcnx = cstr_info->tile[tileno].pw[resno];
						int pcx = (int) pow( 2, cstr_info->tile[tileno].pdx[resno] + cstr_info->numdecompos[compno] - resno );
						int pcy = (int) pow( 2, cstr_info->tile[tileno].pdy[resno] + cstr_info->numdecompos[compno] - resno );
						int precno_x = precno - (int) floor( (float)precno/(float)pcnx ) * pcnx;
						int precno_y = (int) floor( (float)precno/(float)pcnx );
						if (precno >= numprec)
							continue;
						for(y = y0; y < y1; y++) {							
							if (precno_y*pcy == y ) {
								for (x = x0; x < x1; x++) {									
									if (precno_x*pcx == x ) {
										for (layno = 0; layno < cstr_info->numlayers; layno++) {
											start_pos = cstr_info->tile[tileno].packet[pack_nb].start_pos;
											end_ph_pos = cstr_info->tile[tileno].packet[pack_nb].end_ph_pos;
											end_pos = cstr_info->tile[tileno].packet[pack_nb].end_pos;
											disto = cstr_info->tile[tileno].packet[pack_nb].disto;
											fprintf(stream, "%4d %6d %6d %6d %5d %7d %9d   %9d %7d",
												pack_nb, tileno, precno, compno, resno, layno, start_pos, end_ph_pos, end_pos); 
											if (disto_on)
												fprintf(stream, " %8e", disto);
											fprintf(stream, "\n");
											total_disto += disto;
											pack_nb++; 
										}
									}
								}/* x = x0..x1 */
							} 
						}  /* y = y0..y1 */
					} /* resno */
				} /* compno */
			} /* precno */
		} /* PCRL */

		else {	/* CPRL */
			// Count the maximum number of precincts 
			int max_numprec = 0;
			for (resno = 0; resno < max_numdecompos + 1; resno++) {
				int numprec = cstr_info->tile[tileno].pw[resno] * cstr_info->tile[tileno].ph[resno];
				if (numprec > max_numprec)
					max_numprec = numprec;
			}

			fprintf(stream, "CPRL\npack_nb tileno compno precno resno layno start_pos end_ph_pos end_pos"); 
			if (disto_on)
				fprintf(stream, " disto");
			fprintf(stream,"\n");

			for (compno = 0; compno < cstr_info->numcomps; compno++) {
				/* I suppose components have same XRsiz, YRsiz */
				int x0 = cstr_info->tile_Ox + tileno - (int)floor( (float)tileno/(float)cstr_info->tw ) * cstr_info->tw * cstr_info->tile_x;
				int y0 = cstr_info->tile_Ox + (int)floor( (float)tileno/(float)cstr_info->tw ) * cstr_info->tile_y;
				int x1 = x0 + cstr_info->tile_x;
				int y1 = y0 + cstr_info->tile_y;

				for (precno = 0; precno < max_numprec; precno++) {
					for (resno = 0; resno < cstr_info->numdecompos[compno] + 1; resno++) {
						int numprec = cstr_info->tile[tileno].pw[resno] * cstr_info->tile[tileno].ph[resno];
						int pcnx = cstr_info->tile[tileno].pw[resno];
						int pcx = (int) pow( 2, cstr_info->tile[tileno].pdx[resno] + cstr_info->numdecompos[compno] - resno );
						int pcy = (int) pow( 2, cstr_info->tile[tileno].pdy[resno] + cstr_info->numdecompos[compno] - resno );
						int precno_x = precno - (int) floor( (float)precno/(float)pcnx ) * pcnx;
						int precno_y = (int) floor( (float)precno/(float)pcnx );
						if (precno >= numprec)
							continue;

						for(y = y0; y < y1; y++) {
							if (precno_y*pcy == y ) {
								for (x = x0; x < x1; x++) {
									if (precno_x*pcx == x ) {
										for (layno = 0; layno < cstr_info->numlayers; layno++) {
											start_pos = cstr_info->tile[tileno].packet[pack_nb].start_pos;
											end_ph_pos = cstr_info->tile[tileno].packet[pack_nb].end_ph_pos;
											end_pos = cstr_info->tile[tileno].packet[pack_nb].end_pos;
											disto = cstr_info->tile[tileno].packet[pack_nb].disto;
											fprintf(stream, "%4d %6d %6d %6d %5d %7d %9d   %9d %7d",
												pack_nb, tileno, compno, precno, resno, layno, start_pos, end_ph_pos, end_pos); 
											if (disto_on)
												fprintf(stream, " %8e", disto);
											fprintf(stream, "\n");
											total_disto += disto;
											pack_nb++; 
										}
									}
								}/* x = x0..x1 */
							}
						} /* y = y0..y1 */
					} /* resno */
				} /* precno */
			} /* compno */
		} /* CPRL */   
	} /* tileno */
	
	if (disto_on) {
		fprintf(stream, "%8e\n", cstr_info->D_max); /* SE max */	
		fprintf(stream, "%.8e\n", total_disto);	/* SE totale */
	}
/* UniPG>> */
	/* print the markers' list */
	if (cstr_info->marknum) {
		fprintf(stream, "\nMARKER LIST\n");
		fprintf(stream, "%d\n", cstr_info->marknum);
		fprintf(stream, "type\tstart_pos    length\n");
		for (x = 0; x < cstr_info->marknum; x++)
			fprintf(stream, "%X\t%9d %9d\n", cstr_info->marker[x].type, cstr_info->marker[x].pos, cstr_info->marker[x].len);
	}
/* <<UniPG */
	fclose(stream);

	fprintf(stderr,"Generated index file %s\n", index);

	return 0;
}

/* ------------------------------------------------------------------------------------ */


/**
Dump the file info structure into a file
@param	stream		output stream
@param	file_info	informations read into the JPG2000 file
@return Returns 0 if successful, returns 1 otherwise
*/
int dump_file_info(FILE* stream, opj_file_info_t *file_info)
{
	/* IMAGE HEADER */
	if ( file_info->file_info_flag & OPJ_IMG_INFO ) {
		opj_image_header_t img_header = file_info->img_info;
		int compno;

		fprintf(stream, "Image info {\n");
		fprintf(stream, "\t x0=%d, y0=%d\n",img_header.x0, img_header.y0);
		fprintf(stream,	"\t x1=%d, y1=%d\n",img_header.x1, img_header.y1);
		fprintf(stream, "\t numcomps=%d\n", img_header.numcomps);
		for (compno = 0; compno < img_header.numcomps; compno++) {
			opj_image_comp_header_t comp = img_header.comps[compno];

			fprintf(stream, "\t component %d {\n", compno);
			fprintf(stream, "\t\t dx=%d, dy=%d\n", comp.dx, comp.dy);
			fprintf(stream, "\t\t prec=%d\n", comp.prec);
			fprintf(stream, "\t\t sgnd=%d\n", comp.sgnd);
			fprintf(stream, "\t}\n");
		}
		fprintf(stream, "}\n");
	}

	/* CODESTREAM INFO */
	if ( file_info->file_info_flag & OPJ_J2K_INFO ) {
		opj_codestream_info_v2_t cstr_info = file_info->codestream_info;
		int tileno, compno, layno, bandno, resno, numbands;

		fprintf(stream, "Codestream info {\n");
		fprintf(stream, "\t tx0=%d, ty0=%d\n", cstr_info.tx0, cstr_info.ty0);
		fprintf(stream, "\t tdx=%d, tdy=%d\n", cstr_info.tdx, cstr_info.tdy);
		fprintf(stream, "\t tw=%d, th=%d\n", cstr_info.tw, cstr_info.th);

		for (tileno = 0; tileno < cstr_info.tw * cstr_info.th; tileno++) {
			opj_tile_info_v2_t tile_info = cstr_info.tile[tileno];

			fprintf(stream, "\t tile %d {\n", tileno);
			fprintf(stream, "\t\t csty=%x\n", tile_info.csty);
			fprintf(stream, "\t\t prg=%d\n", tile_info.prg);
			fprintf(stream, "\t\t numlayers=%d\n", tile_info.numlayers);
			fprintf(stream, "\t\t mct=%d\n", tile_info.mct);
			fprintf(stream, "\t\t rates=");

			for (layno = 0; layno < tile_info.numlayers; layno++) {
				fprintf(stream, "%.1f ", tile_info.rates[layno]);
			}
			fprintf(stream, "\n");

			for (compno = 0; compno < cstr_info.numcomps; compno++) {
				opj_tccp_info_t tccp_info = tile_info.tccp_info[compno];

				fprintf(stream, "\t\t comp %d {\n", compno);
				fprintf(stream, "\t\t\t csty=%x\n", tccp_info.csty);
				fprintf(stream, "\t\t\t numresolutions=%d\n", tccp_info.numresolutions);
				fprintf(stream, "\t\t\t cblkw=%d\n", tccp_info.cblkw);
				fprintf(stream, "\t\t\t cblkh=%d\n", tccp_info.cblkh);
				fprintf(stream, "\t\t\t cblksty=%x\n", tccp_info.cblksty);
				fprintf(stream, "\t\t\t qmfbid=%d\n", tccp_info.qmfbid);
				fprintf(stream, "\t\t\t qntsty=%d\n", tccp_info.qntsty);
				fprintf(stream, "\t\t\t numgbits=%d\n", tccp_info.numgbits);
				fprintf(stream, "\t\t\t roishift=%d\n", tccp_info.roishift);

#ifdef TODO_MSD
				fprintf(stream, "\t\t\t stepsizes=");
				numbands = tccp_info->qntsty == J2K_CCP_QNTSTY_SIQNT ? 1 : tccp_info->numresolutions * 3 - 2;
				for (bandno = 0; bandno < numbands; bandno++) {
					fprintf(stream, "(%d,%d) ", tccp_info->stepsizes[bandno].mant,
						tccp_info->stepsizes[bandno].expn);
				}
				fprintf(stream, "\n");

				if (tccp_info->csty & J2K_CCP_CSTY_PRT) {
					fprintf(stream, "      prcw=");
					for (resno = 0; resno < tccp_info->numresolutions; resno++) {
						fprintf(stream, "%d ", tccp_info->prcw[resno]);
					}
					fprintf(stream, "\n");

					fprintf(stream, "      prch=");
					for (resno = 0; resno < tccp_info->numresolutions; resno++) {
						fprintf(stream, "%d ", tccp_info->prch[resno]);
					}
					fprintf(stream, "\n");
				}
#endif
				fprintf(stream, "\t\t\t }\n");
			} /*end of component*/
			fprintf(stream, "\t\t }\n");
		} /*end of tile */
		fprintf(stream, "\t }\n");
	}

	if ( file_info->file_info_flag & OPJ_JP2_INFO ) {
		// not yet coded
	}
	return EXIT_SUCCESS;
}
