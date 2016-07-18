/**
 * @file   ARchiver.c
 * @author Adrian Rasmussen
 *
 * @brief A program to compress text files using Huffman Coding, or to decompress .ar files.
 * Usage: ./ARchiver [file]    for compression
 *		  ./ARchiver -d [file] for decompression
 * @date 15 November 2012, 9:01 PM
 * @version 1.0 - Compression/Decompression of one file
 */
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include "ARHeader.h"
#include "ARchiver.h"
#include "Huffman.h"
#include "Heap.h"
#define _CRTDBG_MAP_ALLOC
#ifdef _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

int main(int argc, char* argv[])
{
	int status;
	FILE* file;

	status = EXIT_SUCCESS;
    /*Check command line parameters are either -d flag with file, or just file*/
    if (argc == 2) /*Compression*/
    {
        /*Check if file can be opened, otherwise exit*/
        file = fopen( argv[1], "r");
		if ( file == NULL)
        {
            perror(argv[1]);
        }
        else
        {
            fclose(file);
			status = compressFile(argv[1]);
        }
    }
    else if (argc == 3)/*Decompression*/
    {
        if ((strcmp("-d", argv[1]) == 0))
        {
            status = decompressFile(argv[2]);
        }
        else
        {
            printf("Invalid flag %s, must use -d to decompress", argv[1]);
        }
    }
    else
    {
        printf("Parameters must be either -d with the .ar file, or just the file to compress");
    }

#ifdef _CRTDBG_MAP_ALLOC
	_CrtDumpMemoryLeaks();
#endif
    return status;
}

/**
 * Method:    compressFile
 * FullName:  compressFile
 * Access:    public 
 * @brief   Used to compress the provided text file using Huffman Compression
 * @param 	  file - the name of the file to use
 * @return   return status of the function, either EXIT_SUCCESS or EXIT_FAILURE
 **/
int compressFile( char* file )
{
	int maxSize = 1048576, currSize;
    HuffNode **freqTable, **pQ, *root;
    int symbolSize = 1;
    char **codeTable, **compressed;
    char* code;
    ARHeader header;
    int num, i, numElements;
    HuffNodeSerial *treeSerial;

    /*Get frequency of each character in file and sort in lowest to highest*/
    freqTable = createFreqTable(file, &header.uncompressedDataSize);

	pQ = sortPriority(freqTable, &num);
	numElements = num;
	/*Create Huffman tree from frequencies and build code table for each char*/
	root = buildTree(pQ, &num);
	if ( root == NULL)
	{
		printf("Could not build tree, exiting");
		return EXIT_FAILURE;
	}

	codeTable = (char**) malloc(256 * symbolSize * sizeof(char*));
	if ( codeTable == NULL)
	{
		printf("Could not allocate memory for codeTable\n");
		return EXIT_FAILURE;
	}
		
	for (i = 0; i < 256 * symbolSize; i++)
	{
		codeTable[i] = NULL;
	}
	/*Max code length in worst case is 256*n - 1, so +1 for null terminator*/
	code = (char*) malloc(256 * symbolSize);
	buildCodeTable(codeTable, root, code, 0);

	/*Serialize tree for storage in .ar file*/
	treeSerial = compressTree(root, numElements, &header.huffTreeSize);

	/*Small file, handle normally*/
	if ( header.uncompressedDataSize <= maxSize)
	{
				
		/*Compress symbols*/
		/*************************
		*TODO:
		* Compressed data size may need to represent number of bits minus padding,
		* or otherwise need to include EOF character in tree.
		*
		*/
		compressed = encode(file, codeTable, &header.compressedDataSize, header.uncompressedDataSize);
		if ( compressed != NULL)
		{
			header.arID = 117;
			strcpy(header.arText, "ARchiver file");
	
			/*Create .ar file from the header, serialized tree and compressed data*/
			writeARFile(&header, treeSerial, header.huffTreeSize, compressed, header.compressedDataSize);
	
			/*Free serial tree*/
			free(treeSerial);
			treeSerial = NULL;
	
			/*free compressed*/
			for ( i = 0; i < header.uncompressedDataSize; i++)
			{
				free(compressed[i]);
				compressed[i] = NULL;
			}
			free(compressed);
			compressed = NULL;
		}
		else
		{
			printf("File could not be compressed, compressed size would be greater than original");
		}
		/*Free remaining allocated memory*/
		freeTree(root);
	
		for (i = 0; i < 256; i++)
		{
			/*Only free allocated memory, skip unused elements*/
			if (codeTable[i] != NULL)
			{
				free(codeTable[i]);
				codeTable[i] = NULL;
			}
		}
		free(code);
		code = NULL;
	
		free(codeTable);
		codeTable = NULL;
	}
	else /*File too large, handle in chunks*/
	{
		/*Compress symbols*/
		/*************************
		*TODO:
		* Compressed data size may need to represent number of bits minus padding,
		* or otherwise need to include EOF character in tree.
		*
		*/
		currSize = header.uncompressedDataSize;
		while ( currSize >= 0)
		{
			if ( currSize >= maxSize)
			{
				compressed = encode(file, codeTable, &header.compressedDataSize, maxSize);
				currSize -= maxSize;
			}
			else
			{
				compressed = encode(file, codeTable, &header.compressedDataSize, currSize);
			}

			if ( compressed != NULL)
			{
				header.arID = 117;
				strcpy(header.arText, "ARchiver file");
		
				/*Create .ar file from the header, serialized tree and compressed data*/
				writeARFile(&header, treeSerial, 0, compressed, 0);
		
				/*Free serial tree*/
				free(treeSerial);
				treeSerial = NULL;
		
				/*free compressed*/
				for ( i = 0; i < header.uncompressedDataSize; i++)
				{
					free(compressed[i]);
					compressed[i] = NULL;
				}
				free(compressed);
				compressed = NULL;
			}
			else
			{
				printf("File could not be compressed, compressed size would be greater than original");
			}
			/*Free remaining allocated memory*/
			freeTree(root);
		
			for (i = 0; i < 256; i++)
			{
				/*Only free allocated memory, skip unused elements*/
				if (codeTable[i] != NULL)
				{
					free(codeTable[i]);
					codeTable[i] = NULL;
				}
			}
			free(code);
			code = NULL;
		
			free(codeTable);
			codeTable = NULL;

			/*Seek  back to start and write header*/
			/*Create .ar file from the header, serialized tree and compressed data*/
			writeARFile(&header, treeSerial, header.huffTreeSize, compressed, header.compressedDataSize);	

		}
	}
	
	printf("Done\n"); 	
	return EXIT_SUCCESS;
}

/**
 * Method:    decompressFile
 * FullName:  decompressFile
 * Access:    public 
 * @brief   Decompresses a given .ar file, to create original file
 * @param 	  file name of compressed file with .ar extension
 * @return   return status of function, EXIT_SUCCESS or EXIT_FAILURE
 **/
int decompressFile( char* file )
{

    int compressedSizeBytes;
    ARHeader header;
    HuffNode *tree;
    HuffNodeSerial *treeSerial;
    char *compressed, *uncompressed, *binary;
    FILE* input = fopen(file, "rb");

    if ( input == NULL)
    {
        perror(file);
    }
    else
    {
        fread( &header, sizeof(header), 1, input);
        if ( header.arID != 117)
        {
            printf("Not a valid .ar file, wrong id %d", header.arID);
        }
        else
        {
            /*compressedDataSize is in bits, convert to bytes*/
            compressedSizeBytes = (header.compressedDataSize + 7)/ 8;
            compressed = (char*) malloc( compressedSizeBytes);
            if ( compressed == NULL)
            {
            	printf("Could not allocate memory for compressed\n");
            	return EXIT_FAILURE;
            }

            treeSerial = (HuffNodeSerial*) malloc( header.huffTreeSize);
			if ( treeSerial == NULL)
			{
				printf("Could not allocate memory for treeSerial\n");
				return EXIT_FAILURE;
			}

			fread( treeSerial, header.huffTreeSize, 1, input);	
			tree = decompressTree( treeSerial);
			fread( compressed, compressedSizeBytes, 1, input);

			/*Compressed is an array of chars, need to convert to binary representation*/
			binary = toBinary( compressed, header.compressedDataSize);
			uncompressed = decode( binary, header.compressedDataSize, header.uncompressedDataSize, tree);
			free(binary);
			
			writeFile( uncompressed, header.uncompressedDataSize);

			freeTree(tree);
			tree = NULL;
			
			free(treeSerial);
			treeSerial = NULL;

			free(uncompressed);
			uncompressed = NULL;

			free(compressed);
			compressed = NULL;
        }
		fclose(input);
    }

	return EXIT_SUCCESS;
}

/**
 * Method:    createFreqTable
 * FullName:  createFreqTable
 * Access:    public 
 * @brief   Generates a table of frequencies in the form of an array of HuffNodes with symbol, 
 * frequency and left and right pointers, for use in Huffman Coding 
 * @param 	  file - name of input file to be compressed, used to get the frequency of each symbol
 * @param 	  uncompressed - location to save uncompressed size in bytes to
 * @return    the frequency table
 **/
HuffNode** createFreqTable(char* file, int* uncompressed)
{
    int tableSize, i;
    unsigned char symbol;
    HuffNode** freqTable = NULL;
    FILE* input = fopen(file, "r");
    *uncompressed = 0;
    /*Check file is valid and openable*/
    if (input == NULL)
    {
        perror(file);
    }
    else
    {
        tableSize = 256;
        freqTable = (HuffNode**) malloc(tableSize * sizeof ( HuffNode*));
		if ( freqTable == NULL)
		{
			printf("Could not allocate memory for freqTable\n");
			return NULL;
		}

        for (i = 0; i < tableSize; i++)
        {
            freqTable[i] = (HuffNode*) malloc(sizeof (HuffNode));
			if ( freqTable[i] == NULL)
			{
				printf("Could not allocate memory for freqTable[%d]\n", i);
				return NULL;
			}
            freqTable[i]->freq = 0;
            freqTable[i]->left = NULL;
            freqTable[i]->right = NULL;
        }

        /*Go through file, add symbol each time a new one is found or otherwise increment frequency*/
        while (!feof(input))
        {
            symbol = fgetc(input);
            /**************************
            CHECK FOR SYMBOL SIZE =2 OR 3
             ***************************/
            if (!feof(input))
            {
                *uncompressed += sizeof (symbol);
                freqTable[(int) symbol]->freq++;
                freqTable[(int) symbol]->symbol = symbol;
            }
        }
        fclose(input);
    }
    return freqTable;
}

/**
 * Method:    sortPriority
 * FullName:  sortPriority
 * Access:    public 
 * @brief   Sorts the frequency table into priority order, where lowest frequency = highest priority. Implemented as a heap, 
 * see {@link Heap.c}
 * @param 	  freqTable - table of HuffNodes with symbol, frequency and null left and right pointers
 * @param 	  numElements - number of unique symbols in the frequency table
 * @return    the priority queue in heap form
 **/
HuffNode** sortPriority(HuffNode** freqTable, int *numElements)
{
    int i, size;
    HuffNode **pQ;

    *numElements = 0;
    for (i = 0; i < 256; i++)
    {
        if (freqTable[i]->freq > 0)
        {
            (*numElements)++;
        }
    }

    pQ = (HuffNode**) malloc(*numElements * sizeof (HuffNode*));

    /*Partially sort using a heap, only removing from front so doesn't need to be
     completely sorted*/
    size = 0;
    for (i = 0; i < 256; i++)
    {
        if (freqTable[i]->freq > 0)
        {
            insert(pQ, &size, freqTable[i]);
        }
        else
        {
            /*Free unused elements*/
            free(freqTable[i]);
            freqTable[i] = NULL;
        }
    }
    free(freqTable);

    return pQ;
}

/**
 * Method:    encode
 * FullName:  encode
 * Access:    public 
 * @brief   Reads in the symbols from the file and converts them to the compressed version,
 * saving them in a table along with the compressed size.
 * If compressed size exceeds (uncompressedSize - headerSize - treeSize), then
 * compression should not be done.
 * @param 	  inputName - name of the input file to get encoded form for each character
 * @param 	  codeTable - array of the codes corresponding to each symbol
 * @param 	  compressedSize - address to save compressed size of symbols to
 * @param 	  uncompressedSize - the size of the original data
 * @return    the compressed table of blocks of 8 chars (e.g. 10110101)
 **/
char** encode(char *inputName, char *codeTable[], int *compressedSize, int uncompressedSize)
{
    /*Create space for the compressed code
     Size will be a multiple of 8, as 8 bits = 1 byte, minimum size that can
     be simply written to a file*/

    int numBits, byte, i, j;
    char ch, *code, **compressed;
    FILE* input = fopen(inputName, "r");

    /*TODO: Work out size to malloc*/
    compressed = (char**) malloc(uncompressedSize * sizeof (char*));
	if ( compressed == NULL)
	{
		printf("Could not allocate memory for compressed\n");
		return NULL;
	}
    for ( j = 0; j < uncompressedSize; j++)
    {
        compressed[j] = (char*) malloc( 9);
		if ( compressed[j] == NULL)
		{
			printf("Could not allocate memory for compressed[%d]\n", j);
			return NULL;
		}
    }


    if (input != NULL)
    {
        *compressedSize = 0;
        byte = 0;
        numBits = 0;
        i = 0;
        ch = fgetc(input);
        code = codeTable[(int)ch];
        while (!feof(input) && byte < uncompressedSize)
        {
            if (!feof(input))
            {
                /*Build buffer until 8 bits or all parts of code is used*/
                /*Byte determines which block of 8 bits, numBits which individual bit*/
                while (numBits < 8 && i < (int)strlen(code))
                {
                    compressed[byte][numBits++] = code[i++];
                }
				
                /*Filled up single byte, move to next byte*/
                if (numBits == 8)
                {
                    compressed[byte][numBits] = '\0';
					*compressedSize += 8;
                    numBits = 0;
                    byte++;
                }

                /*Get next character from input file*/
                if (i >= (int)strlen(code))
                {
                    ch = fgetc(input);
                    if (!feof(input))
                    {
                        code = codeTable[(int)ch];
                        i = 0;
                    }
                }
            }
        }
        if (numBits < 8)
        {
            /*Exclude padding from count of total bits*/
            *compressedSize += numBits;
            /*Write final buffer with padding to file*/
            for (numBits = numBits; numBits < 8; numBits++)
            {
                compressed[byte][numBits] = '0';
            }
        }
        fclose(input);

        if ( byte >= uncompressedSize)
        {
            *compressedSize = 0;
            compressed = NULL;
        }
    }

    return compressed;
}

/**
 * Method:    writeARFile
 * FullName:  writeARFile
 * Access:    public 
 * @brief   Writes the header, tree and compressed data to a new .ar file
 * @param 	  header - struct containing size of tree, compressed and uncompressed data, as well as id and description
 * @param 	  tree - the created Huffman tree in a serialized form
 * @param 	  treeSize - size in bytes of the serialized tree
 * @param 	  compressed - the compressed data to write to the file
 * @param 	  compressedDataSize - size of compressed data
 **/
void writeARFile( ARHeader *header, char *tree, int treeSize, char **compressed, int compressedDataSize)
{
    int i, j;
    char file[101];
	unsigned char result;
    FILE* output;

    printf("Enter name of output file.\n");
    scanf("%97s", file);
    strcat(file, ".ar");
    output = fopen(file, "wb");

    if ( output == NULL)
    {
        perror( file);
    }
    else
    {
        fwrite(header, sizeof(*header), 1, output);
		/*Write each 8 bits of compressed data*/
		for ( i = 0; i < treeSize; i++)
		{
			if ( tree[i] == '0')
			{
				result |= 0 << (7 - j);
			}
			else
			{
				result |= 1 << (7 - j);
				
			}
			result = 0;
			for (j = 0; j < 8; j++)
			{
				result |= (compressed[i][j] == '1') << (7 - j);
			}
			fwrite( &result, 1, 1, output);
		}
		fwrite(tree, treeSize, 1, output);
        /*Round size to nearest multiple of 8, to get actual size with padding*/
        while ( compressedDataSize % 8 != 0)
        {
            compressedDataSize++;
        }
        compressedDataSize /= 8;
        /*Write each 8 bits of compressed data*/
        for ( i = 0; i < compressedDataSize; i++)
        {
            result = 0;
			for (j = 0; j < 8; j++)
            {
                result |= (compressed[i][j] == '1') << (7 - j);
            }
            fwrite( &result, 1, 1, output);
        }
        fclose(output);
    }
}

/**
 * Method:    toBinary
 * FullName:  toBinary
 * Access:    public 
 * @brief   Converts an array of compressed characters into their 'compressed binary' form, just using 0 and 1,
 *				as the compressed file must be read in bytes, and so the read data will consist of random symbols corresponding
 *				to the actual bits written
 * @param 	  compressed - array of characters in 'compressed' form from the .ar file
 * @param 	  sizeBits - size in bits of the compressed array
 * @return    array of characters which are either '1' or '0', to be decoded
 **/
char* toBinary( char* compressed, int sizeBits)
{
    int i, j, k, sizeBytes;
    unsigned char ch;
	char *binary;
	/*binary will need to be mallocd size as a multiple of 8, sizeBits doesn't include padding*/
	/*int division to get nearest multiple of 8*/
	sizeBytes = ( sizeBits + 7)/8;
    binary = (char*) malloc( sizeBytes * 8);/*Size????*/
	if ( binary == NULL)
	{
		printf("Could not allocate memory for binary\n");
		return NULL;
	}
    k = 0;
    for ( i = 0; i < sizeBytes; i++)
    {
        ch = compressed[i];
        for( j = 0; j < 8; j++)
        {
            if ( ((ch >> (7 - j)) & 1) == 0)
            {
                binary[k++] = '0';
            }
            else
            {
                binary[k++] = '1';
            }
        }
    }

    return binary;
}

/**
 * Method:    writeFile
 * FullName:  writeFile
 * Access:    public 
 * @brief   Writes the decompressed data to a new file, i.e., original data
 * @param 	  uncompressed - decompressed data in array
 * @param 	  size - size of decompressed data
 **/
void writeFile( char* uncompressed, int size)
{
    int i;
    char file[101];
    FILE* output;

    printf("Enter output file name\n");
    scanf("%99s", file);
    output = fopen(file,"w");


    if ( output != NULL)
    {
        for ( i = 0; i < size; i++)
        {
            fputc( uncompressed[i], output);
        }
        fclose(output);
    }
}
