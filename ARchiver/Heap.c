#include <stdlib.h>
#include "Huffman.h"


/**
 * Method:    insert
 * FullName:  insert
 * Access:    public 
 * Purpose:   Inserts the provided node into the correct position in the (min) heap
 * @param 	  heap - to insert into, implemented as an array
 * @param 	  size - number of elements in the heap
 * @param 	  node - node to insert into the heap
 **/
void insert( HuffNode** heap, int *size, HuffNode* node)
{
    int currIdx, parentIdx;
    HuffNode *temp;
    /*Add node to last position*/
    heap[*size] = node;
    currIdx = *size;
    parentIdx = (currIdx - 1)/2;
    
    (*size)++;
    /*Trickle up to correct place*/
    while ( heap[parentIdx]->freq > heap[currIdx]->freq)
    {
        temp = heap[parentIdx];
        heap[parentIdx] = heap[currIdx];
        heap[currIdx] = temp;
        currIdx = parentIdx;
        parentIdx = (currIdx - 1)/2;
    }
}

/**
 * Method:    removeNode
 * FullName:  removeNode
 * Access:    public 
 * @brief   Removes the highest priority (min value, index 0) from the heap and returns it
 * @param 	  heap - heap to remove node from
 * @param 	  size - size of the heap
 * @return    highest-priority node
 **/
HuffNode* removeNode( HuffNode* heap[], int *size)
{
    int currIdx, left, right;
    HuffNode *temp;
    HuffNode *node = heap[0];
    (*size)--;
    heap[0] = heap[*size];
    
    currIdx = 0;
    left = (currIdx * 2) + 1;
    right = (currIdx * 2) + 2;
    /*Trickle down to correct place*/
    while ( currIdx < (*size)/2 && (heap[currIdx]->freq > heap[left]->freq || 
                                heap[currIdx]->freq > heap[right]->freq))
    {
        temp = heap[currIdx];
        if ( heap[left]->freq < heap[right]->freq)
        {
            heap[currIdx] = heap[left];
            heap[left] = temp;
            currIdx = left;
        }
        else
        {
            heap[currIdx] = heap[right];
            heap[right] = temp;
            currIdx = right;
        }
        left = ( currIdx * 2) + 1;
        right = (currIdx * 2) + 2;
    }
    return node;
}

/**
 * Method:    freeTree
 * FullName:  freeTree
 * Access:    public 
 * @brief     Deallocates all memory used by the tree, recursively freeing nodes
 * @param 	  node - root node initially, current node on recursive calls
 **/
void freeTree( HuffNode *node)
{
    if ( node != NULL)
    {
        freeTree( node->left);
        freeTree( node->right);
    }
    free(node);
	node = NULL;
}