#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "Huffman.h"
#include "Heap.h"

/**
 * Method:    buildTree
 * FullName:  buildTree
 * Access:    public 
 * @brief     Builds a Huffman tree from a priority queue of HuffNodes.
 *			  Removes first two elements from the queue, adds them as the left and right children of a new node,
 *			  and then inserts the new tree into the queue in the correct place according to its new total frequency
 * @param 	  pQ - priority queue, with minimum frequency = highest priority
 * @param 	  num - number of elements in the queue
 * @return    the Huffman tree
 **/
HuffNode* buildTree( HuffNode** pQ, int *num)
{
    /*pQ is a priority queue of tree nodes*/
    HuffNode *node1, *node2, *newNode, *root;
    
    while ( *num > 1)
    {
        /*Remove first two elements from pQ*/
        node1 = removeNode(pQ, num);
        node2 = removeNode(pQ, num);
        
        /*Make them left and right child of new tree*/
        newNode = (HuffNode*) malloc(sizeof(HuffNode));
		if ( newNode == NULL)
		{
			printf("Could not allocate memory for newNode\n");
			break;
		}
        newNode->symbol = -1;
        newNode->left = node1;
        newNode->right = node2;
        newNode->freq = node1->freq + node2->freq;

        /*Insert new tree back into pQ*/
        insert( pQ, num, newNode);
        
        /*Element at num should be nulled*/
        pQ[*num] = NULL;
    }
	
	if ( newNode == NULL)
	{
		*num = 0;
		root = NULL;
	}
	else
	{
		*num = 1;
		root = removeNode(pQ, num);
	}

	free(pQ);
    pQ = NULL;

    return root;
}

/*Recursive function*/
/**
 * Method:    buildCodeTable
 * FullName:  buildCodeTable
 * Access:    public 
 * @brief     Recursively builds the table of codes for each symbol. 
 *			  Appends '0' for left or '1' for right until leaf is found, in which case it adds the code to the codeTable
 *			  with the symbol at that location
 * @param 	  codeTable - single char array where index represents symbol, contents = code
 * @param 	  node - current node, originally root
 * @param 	  code - current built up string of 0s and 1s
 * @param 	  level - current depth in tree
 **/
void buildCodeTable( char* codeTable[], HuffNode *node, char* code, int level)
{
    /*Code Table is an array of chars, where index = symbol and
     values are the code. */
    if ( node->left == NULL && node->right == NULL)
    {
        code[level]= '\0';
        /*Make a copy of the code so that it is separate and won't be changed*/
		codeTable[(int)node->symbol] = (char*) malloc( strlen(code) + 1);
		if ( codeTable[(int)node->symbol] == NULL)
		{
			printf("Could not allocate memory for codeTable[%d]\n", (int) node->symbol);
		}
		else
		{
	        strcpy(codeTable[(int)node->symbol], code);
    
		}
	}
    else
    {
        code[level] = '0';
        buildCodeTable( codeTable, node->left, code, level + 1);
        
        code[level] = '1';
        buildCodeTable( codeTable, node->right, code, level + 1);
    }
}

/*Serialize tree for storing in .ar file*/
/**
 * Method:    compressTree
 * FullName:  compressTree
 * Access:    public 
 * @brief     Serializes the Huffman tree into an array with two additional data fields to hold indexes of left and right child
 * @param 	  root - root node of the Huffman tree
 * @param 	  numElements - number of symbols in the tree (Not the total number of nodes)
 * @param 	  compressedSize - location to save the size of the serialized tree to, in bytes
 * @return    the serialized tree
 **/
char* compressTree(HuffNode *root, int numElements, int* compressedSize)
{
    /*Number of nodes will be 2*elements - 1*/
    int i = 0;
    char *compressed;
    compressed = (char*) calloc( (10 * numElements - 1), sizeof(char));
    serializeRecurse( compressed, root, &i);
    
    *compressedSize = (10*numElements-1)*sizeof(char);

	return compressed;
}

/**
 * Method:    serializeRecurse
 * FullName:  serializeRecurse
 * Access:    public 
 * @brief     Recursively serializes the tree, passing the index as a pointer to keep its value
 * @param 	  compressed - serialized tree
 * @param 	  node - current node
 * @param 	  i - current index
 **/
void serializeRecurse(char *compressed, HuffNode* node, int *i)
{
	/*If leaf-node: Output 1-bit + N-bit character/byte
		If not leaf-node, output 0-bit. Then encode both child nodes (left first then right) the same way*/
	if ( node->left == NULL && node->right == NULL)
	{
		compressed[(*i)++] = 1;
		compressed[(*i)++] = node->symbol;
	}
	else
	{
		compressed[(*i)++] = 0;
		serializeRecurse( compressed, node->left, i);
		serializeRecurse( compressed, node->right, i);
	}

}

/**
 * Method:    decompressTree
 * FullName:  decompressTree
 * Access:    public 
 * @brief     Recreates the original tree from its serialized form.
 * @param 	  treeSerial - the serialized tree
 * @return    the original tree using pointers
 **/
HuffNode* decompressTree( HuffNodeSerial* treeSerial)
{
    HuffNode* root = (HuffNode*) malloc( sizeof(HuffNode));
	if ( root == NULL)
	{
		printf("Could not allocate memory for root\n");
		return NULL;
	}
    root->symbol = treeSerial[0].symbol;
	root->freq = -1;
	root->left = NULL;
	root->right = NULL;
    
    deserializeRecurse( treeSerial, root, 0);

    return root;
}

/**
 * Method:    deserializeRecurse
 * FullName:  deserializeRecurse
 * Access:    public 
 * @brief     Recursively deserializes the tree, doing the inverse of the {@link serializeTree} function
 * @param 	  node - current node
 * @param 	  treeSerial - serialized tree
 * @param 	  i - current index, not a pointer and so will use original values when recursive calls ends
 **/
void deserializeRecurse( HuffNodeSerial* treeSerial, HuffNode* node, int i )
{
    HuffNode* newNode;
    int temp;

    node->left = NULL;
	node->right = NULL;
    /*Left*/
    if ( treeSerial[i].left != -1)
    {
        newNode = (HuffNode*) malloc( sizeof(HuffNode));
		if ( newNode == NULL)
		{
			printf("Could not allocate memory for newNode\n");
		}
		else
		{
			temp = treeSerial[i].left;
			newNode->freq = -1;
			newNode->symbol = treeSerial[temp].symbol;
			node->left = newNode;
			deserializeRecurse( treeSerial, node->left, temp);
		}
    }
	
    /*Right*/
    if ( treeSerial[i].right != -1)
    {
        newNode = (HuffNode*) malloc( sizeof(HuffNode));
		if ( newNode == NULL)
		{
			printf("Could not allocate memory for newNode\n");
		}
		else
		{
			temp = treeSerial[i].right;
			newNode->freq = -1;
			newNode->symbol = treeSerial[temp].symbol;
			node->right = newNode;
			deserializeRecurse( treeSerial, node->right, temp);
		}
    }
}

/**
 * Method:    decode
 * FullName:  decode
 * Access:    public 
 * @brief     Converts the binary string of 1s and 0s to the original symbols
 * @param 	  binary - array of 0s and 1s
 * @param 	  sizeBits - size in bits of binary, i.e. number of 1s and 0s
 * @param 	  uncompressed - size of the file when uncompressed
 * @param 	  root - root node of the Huffman tree
 * @return    the decoded symbols
 **/
char* decode( char *binary, int sizeBits, int uncompressed, HuffNode *root)
{
    /*Binary is a single long string of 1s and 0s*/
    int i, j;
    char ch;
    char *decoded;
    HuffNode* node;

    /*+1 for null terminator space*/
    decoded = (char*) malloc( uncompressed + 1);
	if ( decoded == NULL)
	{
		printf("Could not allocate memory for decoded\n");
		return NULL;
	}
    j = 0;
    node = root;
    /*Size in bits excludes padding*/
    for (i = 0; i < sizeBits; i++)
    {
        ch = binary[i];
        
        if ( ch == '0')
        {
            node = node->left;
        }
        else /*ch == '1'*/ 
        {
            node = node->right;
        }
        
        if ( node->symbol != -1) /*Leaf node*/
        {
            decoded[j++] = node->symbol;
            node = root;
        }
    }
	decoded[j] = '\0';
    return decoded;
}