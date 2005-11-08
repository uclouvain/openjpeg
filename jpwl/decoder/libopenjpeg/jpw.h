#ifndef __JPW_H
#define __JPW_H

int decode_JPWL(unsigned char *src, int len);

int read_EPC();

int read_EPB_2(int *j2k_state);

int read_EPB(int next,int *j2k_state);
//void read_EPB(int *j2k_state);

int read_EPB_PM(int *j2k_state);

void insert_RED(int pos, int lred, int redlenok);

void write_buff(unsigned char *buff,int pos,long cl);

void read_buff(unsigned char *buff,int pos,long cl);

void ResetCRC();

void UpdateCRC16(char x);

void UpdateCRC32(char x);

char reflectByte(char inbyte);

void reflectCRC32();

void generate_gf(int nn, int kk);

void gen_poly(int nn, int kk);

void encode_rs(int nn, int kk, int tt);

void decode_rs(int nn, int kk, int tt);








#endif
