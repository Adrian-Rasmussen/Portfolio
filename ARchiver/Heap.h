/* 
 * File:   Heap.h
 * Author: adrian
 *
 * Created on 25 November 2012, 12:20 PM
 */

#ifndef HEAP_H
#define	HEAP_H
void insert( HuffNode** heap, int *size, HuffNode* node);
HuffNode* removeNode( HuffNode* heap[], int *size);
void freeTree( HuffNode *node);
#endif	/* HEAP_H */

