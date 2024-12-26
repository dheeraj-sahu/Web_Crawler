#include <iostream>
#include <queue>
#include <map>
#include <climits> // for CHAR_BIT
#include <iterator>
#include <algorithm>
#include <fstream>
#include <string>

const int UniqueSymbols = 1 << CHAR_BIT;

typedef std::vector<bool> HuffCode;
typedef std::map<char, HuffCode> HuffCodeMap;

class INode
{
public:
    const int f;

    virtual ~INode() {}

protected:
    INode(int f) : f(f) {}
};
using namespace std;
void saveCodesToFile(const HuffCodeMap& codes, const string& outputFile) {
    ofstream outFile(outputFile);
    if (!outFile) {
        cerr << "Error: Unable to open output file: " << outputFile << endl;
        exit(1);
    }
    for (const auto& pair : codes) {
        outFile << pair.first << " ";
        for (bool bit : pair.second)
            outFile << bit;
        outFile << endl;
    }
    outFile.close();
}


class InternalNode : public INode
{
public:
    INode *const left;
    INode *const right;

    InternalNode(INode* c0, INode* c1) : INode(c0->f + c1->f), left(c0), right(c1) {}
    ~InternalNode()
    {
        delete left;
        delete right;
    }
};

class LeafNode : public INode
{
public:
    const char c;

    LeafNode(int f, char c) : INode(f), c(c) {}
};

struct NodeCmp
{
    bool operator()(const INode* lhs, const INode* rhs) const { return lhs->f > rhs->f; }
};

INode* BuildTree(const int (&frequencies)[UniqueSymbols])
{
    std::priority_queue<INode*, std::vector<INode*>, NodeCmp> trees;

    for (int i = 0; i < UniqueSymbols; ++i)
    {
        if(frequencies[i] != 0)
            trees.push(new LeafNode(frequencies[i], (char)i));
    }
    while (trees.size() > 1)
    {
        INode* childR = trees.top();
        trees.pop();

        INode* childL = trees.top();
        trees.pop();

        INode* parent = new InternalNode(childR, childL);
        trees.push(parent);
    }
    return trees.top();
}

void GenerateCodes(const INode* node, const HuffCode& prefix, HuffCodeMap& outCodes)
{
    if (const LeafNode* lf = dynamic_cast<const LeafNode*>(node))
    {
        outCodes[lf->c] = prefix;
    }
    else if (const InternalNode* in = dynamic_cast<const InternalNode*>(node))
    {
        HuffCode leftPrefix = prefix;
        leftPrefix.push_back(false);
        GenerateCodes(in->left, leftPrefix, outCodes);

        HuffCode rightPrefix = prefix;
        rightPrefix.push_back(true);
        GenerateCodes(in->right, rightPrefix, outCodes);
    }
}
using namespace std;
string readFileToString(const string &filename) {
    ifstream inFile(filename, ios::binary);
    if (!inFile) {
        cerr << "Error: Unable to open file: " << filename << endl;
        exit(1);
    }

    // Read file contents into a string
    string contents((istreambuf_iterator<char>(inFile)), istreambuf_iterator<char>());
    inFile.close();
    return contents;
}



int main()
{
    // Build frequency table
    std::string filename = "test.html"; // Replace with your file name
    std::string fileContents = readFileToString(filename);
    int frequencies[UniqueSymbols] = {0};
    const char* ptr = fileContents.c_str();
    while (*ptr != '\0')
        ++frequencies[*ptr++];

    INode* root = BuildTree(frequencies);
    
    HuffCodeMap codes;
    GenerateCodes(root, HuffCode(), codes);
    delete root;

    string codesFile = "huffman_codes.txt";
    saveCodesToFile(codes, codesFile);

    cout << "Huffman codes saved to " << codesFile << endl;
    return 0;

    return 0;
}