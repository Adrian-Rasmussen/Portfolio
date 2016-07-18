using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Collections;

namespace DAA
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        private String decompressedText = "";
        private String compressedText = ""; //Replace with DAABitArray?
        private String frequencyText = "";
        private Dictionary<String, int> frequencyTable = new Dictionary<string, int>(); //Gives frequency for each symbol
        private Dictionary<String, String> symbolTable = new Dictionary<string, string>(); //Gives compressed code for each symbol
                                                        //e.g. A, 00 B, 10 C, 011
        private Huffman tree;

        public MainWindow()
        {
            InitializeComponent();
        }
               
        ///////////////
        ////BUTTONS////
        ///////////////

        //Frequency button pressed
        private void btnFreq_Click(object sender, RoutedEventArgs e)
        {
            //Check if there is any content in textbox to generate frequency table
            //If there is, get frequency for each character
            //If not, MessageBox.show()
            txtFreqTbl.Clear();

            if (txtPlain.Text == null || txtPlain.Text.Equals(""))
            {
                MessageBox.Show("No plain text to calculate frequency for. Please enter text first.");
            }
            else
            {
                decompressedText = txtPlain.Text;
                createFrequencyTable();
            }
        }

        //Compress Button Pressed
        private void btnCompress_Click(object sender, RoutedEventArgs e)
        {

            //Check if there is plain text to compress
            if (txtPlain.Text == null || txtPlain.Text.Equals(""))
            {
                MessageBox.Show("No text to compress, please enter text.");
            }
            else
            {
                decompressedText = txtPlain.Text;

                //Check if user has entered or generated a frequency table
                //If not, generate the optimal one
                if (txtFreqTbl.Text == null || txtFreqTbl.Text.Equals(""))
                {
                    createFrequencyTable();
                }
                else
                {
                    textToFreqTable();
                }
                
                Heap pQ = new Heap();
                
                //Add each symbol and frequency into the priority queue of HuffNodes
                foreach (var pair in frequencyTable)
                {
                    HuffNode node = new HuffNode(pair.Key, pair.Value);
                    pQ.insert(node);
                }

                tree = new Huffman();

                tree.buildTree(pQ);
             
                try
                {
                    compressedText = tree.encode(decompressedText);

                    txtCompressed.Text = compressedText;
                }
                catch (KeyNotFoundException exception)
                {
                    MessageBox.Show(exception.Message);
                }
            }
                //If not, MessageBox.show()
            //Check if frequency table exists, if not, generate optimal one
            //Compression Algorithm
                //Check no symbols missing from frequency table
                    //If there is, specify which symbol is at fault
        }

        //Decompress Button Pressed
        private void btnDecompress_Click(object sender, RoutedEventArgs e)
        {
            compressedText = txtCompressed.Text;
        }

        private void createFrequencyTable()
        {
            frequencyTable.Clear();

            //Add each unique character to table, update frequency if not unique
            foreach (char c in decompressedText)
            {
                String symbol = c.ToString();
                
                if (frequencyTable.ContainsKey(symbol))
                {
                    //Increment frequency of that symbol by 1
                    frequencyTable[symbol]++;
                }
                else
                {
                    frequencyTable.Add(symbol, 1);
                }
            }
            //Display frequency table in text box
            frequencyText = "";
            foreach (var pair in frequencyTable)
            {
                //Add in symbol and frequency to text
                if (pair.Key.Equals("\n"))
                {
                    frequencyText += "\\n" + ":" + pair.Value.ToString() + "\n";
                }
                else if (pair.Key.Equals("\r"))
                {
                    frequencyText += "\\r" + ":" + pair.Value.ToString() + "\n";
                }
                else
                {
                    frequencyText += pair.Key.ToString() + ":" + pair.Value.ToString() + "\n";
                }
            }

            //Set displayed text to frequencyText
            txtFreqTbl.Text = frequencyText;
        }

        private void textToFreqTable()
        {
            //Update frequency table used to reflect change
            //Clear table and make new one
            //Otherwise need to update same symbols and delete ones in table not in text
            frequencyTable.Clear();
            frequencyText = txtFreqTbl.Text;
            String[] lines = frequencyText.Split("\n".ToArray(), System.StringSplitOptions.RemoveEmptyEntries);
            //Go through each line in text and add to frequency table
            foreach (String line in lines)
            {
                //Validate each line, if not in correct format, output error and stop?
                //Including checking frequency must be >= 0
                String[] symbolFreq = line.Split(':');
                int freq = Convert.ToInt32(symbolFreq[1]);
                
                //Convert user typed \r or \n to actual carriage return/line feed characters
                if (symbolFreq[0].Equals("\\n"))
                {
                    frequencyTable.Add("\n", Convert.ToInt32(symbolFreq[1]));
                }
                else if (symbolFreq[0].Equals("\\r"))
                {
                    frequencyTable.Add("\r", Convert.ToInt32(symbolFreq[1]));
                }
                frequencyTable.Add(symbolFreq[0], Convert.ToInt32(symbolFreq[1]));
            }
        }
    
    }
}
