/* 
 * File:   ARchiver.h
 * Author: adrian
 *
 * Created on 15 November 2012, 9:29 PM
 */
#include "Huffman.h"
#ifndef ARCHIVER_H
#define	ARCHIVER_H

int compressFile( char* file);
int decompressFile( char* file);
HuffNode** createFreqTable( char* file, int* uncompressed);
HuffNode** sortPriority( HuffNode** freqTable, int *numElements);
void writeARFile( ARHeader *header, HuffNodeSerial *tree, int treeSize, char **compressed, int compressedDataSize);
void writeFile( char* uncompressed, int size);
char** encode(char *inputName, char *codeTable[], int *compressedSize, int uncompressedSize);
char* decompress( char* compressed, int compressedSize, int decompressedSize, HuffNode *tree);
char* toBinary( char* compressed, int size);
#endif
