using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DAA
{
    class Heap
    {
        private List<HuffNode> heap;

        public int Count { get { return heap.Count; } }

        public Heap()
        {
            heap = new List<HuffNode>();
        }
        
        /**
         * Method:    insert
         * Access:    public 
         * @brief:    Inserts the provided node into the correct position in the (min) heap
         * @param 	  node - node to insert into the heap
         **/
        public void insert( HuffNode node)
        {
            int currIdx, parentIdx;
            /*Add node to last position*/
            heap.Add(node);
            currIdx = heap.Count - 1;
            parentIdx = (currIdx - 1)/2;
    
            /*Trickle up to correct place*/
            while ( heap[parentIdx].Frequency > heap[currIdx].Frequency)
            {
                HuffNode temp = heap[parentIdx];
                heap[parentIdx] = heap[currIdx];
                heap[currIdx] = temp;
                currIdx = parentIdx;
                parentIdx = (currIdx - 1)/2;
            }
        }

        /**
         * Method:    remove
         * Access:    public 
         * @brief     Removes the highest priority (min value, index 0) from the heap and returns it
         * @return    highest-priority node
         **/
        public HuffNode remove()
        {
            int currIdx, left, right;
            HuffNode temp;
            
            //Get highest priority node
            HuffNode frontNode = heap[0];
            
            //Put last node in root position
            int lastIdx = heap.Count - 1;
            heap[0] = heap[lastIdx];

            //Remove duplicated node
            heap.RemoveAt(lastIdx);
            
            currIdx = 0;
            left = (currIdx * 2) + 1;
            right = (currIdx * 2) + 2;

            lastIdx--;
            
            /*Trickle down to correct place*/
            while ( currIdx < heap.Count/2 && (heap[currIdx].Frequency > heap[left].Frequency || 
                                        (right <= lastIdx && heap[currIdx].Frequency > heap[right].Frequency)))
            {
                temp = heap[currIdx];
                //Node may not have right child
                if (right > lastIdx || heap[left].Frequency < heap[right].Frequency)
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

            return frontNode;
        }

    }
}
