/*
 * File:   ARHeader.h
 * Author: adrian
 *
 * Created on 15 November 2012, 9:27 PM
 */

#ifndef ARHEADER_H
#define	ARHEADER_H
typedef struct
{
	short arID; /*Is this an AR file?*/
	char arText[14];    /* Human-readable. Always "ARchiver file\0"*/
	int huffTreeSize;  /* Size of Huffman tree 'table', in bytes */
	int compressedDataSize;  /* Size of compressed data in bits - number of 1s and 0s*/
	int uncompressedDataSize; /* Size of data when uncompressed */
} ARHeader;
#endif

