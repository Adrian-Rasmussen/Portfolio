/* 
 * File:   Huffman.h
 * Author: adrian
 *
 * Created on 20 November 2012, 11:34 PM
 */

#ifndef HUFFMAN_H
#define	HUFFMAN_H
typedef struct HuffNode
{
	char symbol;
    int freq;
    struct HuffNode* left;
    struct HuffNode* right;
} HuffNode;

typedef struct 
{
    char symbol;
    short left;
    short right;
} HuffNodeSerial;

HuffNode* buildTree( HuffNode** pQ, int *num);
void buildCodeTable( char* codeTable[], HuffNode *node, char* code, int level);
char* compressTree(HuffNode *root, int numElements, int* compressedSize);
void serializeRecurse(HuffNodeSerial *compressed, HuffNode* node, int *i);
HuffNode* decompressTree( HuffNodeSerial* treeSerial);
void deserializeRecurse( HuffNodeSerial* treeSerial, HuffNode* node, int i );
char* decode( char *binary, int compressed, int uncompressed, HuffNode *tree);
#endif	/* HUFFMAN_H */

