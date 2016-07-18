using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DAA
{
    class HuffNode
    {
        public String Symbol { get; set; }

        private double _Frequency;

        public double Frequency
        {
            get { return _Frequency; }
            set
            {
                if (value < 0)
                {
                    throw new ArgumentException("Frequency cannot be negative");
                }
                _Frequency = value;
            }
        }

        public HuffNode LeftChild { get; set; }
        
        public HuffNode RightChild { get; set; }

        public HuffNode()
        {
            Symbol = "";
            Frequency = 0;
            LeftChild = null;
            RightChild = null;
        }

        public HuffNode(String symbol, double frequency)
        {
            this.Symbol = symbol;
            this.Frequency = frequency;
            this.LeftChild = null;
            this.RightChild = null;
        }

        public HuffNode(String symbol, double frequency, HuffNode leftChild, HuffNode rightChild)
        {
            this.Symbol = symbol;
            this.Frequency = frequency;
            this.LeftChild = leftChild;
            this.RightChild = rightChild;
        }

        public bool isLeaf()
        {
            return LeftChild == null && RightChild == null;
        }
    }
}
