using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DAA
{
    class Huffman
    {

        public HuffNode Root { get; set; }
        public Dictionary<String, String> CodeTable, SymbolTable; //Use DAABitArray instead of String?

        public Dictionary<String, String> getCodeTable()
        {
            return CodeTable;
        }

        public void addSymbolCode(String symbol, String code)
        {
            CodeTable.Add(symbol, code);
            SymbolTable.Add(code, symbol);
        }

        //codeTable? Is it a property of this class or its own?
        //tree? or is root enough? - can get whole tree from root, but should I still store tree?
            //Maybe if I want to access non-root node for some reason


     /**
     * Method:    buildTree
     * FullName:  buildTree
     * Access:    public 
     * @brief     Builds a Huffman tree from a priority queue of HuffNodes.
     *			  Removes first two elements from the queue, adds them as the left and right children of a new node,
     *			  and then inserts the new tree into the queue in the correct place according to its new total frequency
     * @param 	  pQ - priority queue, with minimum frequency = highest priority
     **/
        public void buildTree( Heap pQ)
        {
            /*pQ is a priority queue of tree nodes*/
            HuffNode node1, node2, newNode;
        
            /*Go until only one node is remaining, the root node for the whole huffman tree*/
            while ( pQ.Count > 1)
            {
                /*Remove first two elements from pQ*/
                node1 = pQ.remove();
                node2 = pQ.remove();
        
                /*Make them left and right child of new tree*/
                newNode = new HuffNode("", node1.Frequency + node2.Frequency, node1, node2);
		
                /*Insert new tree back into pQ*/
                pQ.insert(newNode);
            }

	        Root = pQ.remove();

            CodeTable = new Dictionary<String,String>();
            SymbolTable = new Dictionary<string, string>();
            buildCodeTable(Root, "", 0);
        }

        /*Recursive function*/
        /**
         * Method:    buildCodeTable
         * FullName:  buildCodeTable
         * Access:    public 
         * @brief     Recursively builds the table of codes for each symbol. 
         *			  Appends '0' for left or '1' for right until leaf is found, in which case it adds the code to the codeTable
         *			  with the symbol at that location
         * @param 	  codeTable - Dictionary with string symobl key and string code value
         * @param 	  node - current node, originally root
         * @param 	  code - current built up string of 0s and 1s
         * @param 	  level - current depth in tree
         **/
        private void buildCodeTable( HuffNode node, String code, int level)
        {
            /*Code Table is a dictionary with the key as the symbol and the value as the code. */
            if ( node.isLeaf())
            {
                addSymbolCode(node.Symbol, code);
	        }
            else
            {
                code += '0';
                buildCodeTable( node.LeftChild, code, level + 1);
        
                code += '1';
                buildCodeTable( node.RightChild, code, level + 1);
            }
        }

        //Need to return compressed size as well
            //If just return encoded text with no padding, then can use String.Length

        /**
         * Doesn't add padding
         * 
         * 
         **/
        public String encode(String text)
        {
            String compressed = "";

            //Encode each character according to code table and append to compressed string
            foreach (char c in text.ToArray())
            {
                String symbol = c.ToString();
                if (CodeTable.ContainsKey(symbol))
                {
                    compressed += CodeTable[symbol];
                }
                else
                {
                    throw new KeyNotFoundException("Missing symbol in frequency table: " + symbol);
                }
            }

            return compressed;
        }

        /**
         * Method:    decode
         * FullName:  decode
         * Access:    public 
         * @brief     Converts the binary string of 1s and 0s to the original symbols
         * @param 	  binary - array of 0s and 1s
         * @param 	  sizeBits - size in bits of binary, i.e. number of 1s and 0s (minus padding)
         * @param 	  root - root node of the Huffman tree
         * @return    the decoded symbols
         **/
        public String decode( String binary, int sizeBits)
        {
            /*Binary is a single long string of 1s and 0s*/
            String decoded = "";
            
            int j = 0;
            HuffNode node = Root;
            /*Size in bits excludes padding*/
            for (int i = 0; i < sizeBits; i++)
            {
                char ch = binary.ElementAt(i);
        
                if ( ch == '0')
                {
                    node = node.LeftChild;
                }
                else /*ch == '1'*/ 
                {
                    node = node.RightChild;
                }
        
                if ( ! node.Symbol.Equals("")) /*Leaf node*/
                {
                    decoded += node.Symbol;
                    node = Root;
                }
            }
	        decoded += '\0';
            return decoded;
        }
    }
}
