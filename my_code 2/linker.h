



#include <string>
#include <vector>
using namespace std;





struct tokenReturn{

    char * token=new char[1024];
    int lineNum;
    int lineOffset;
    
};
int link (string filename);
tokenReturn *get_token (string filename);
int readInt (string filename);
string readSymbol (string filename);
string readAERI (string filenames);
void parseError(int errcode, bool eof);
void createModule(int instrNum,int moduleNum);
void createSymbol(string sym, int val);
int parser (string filename, int parseNumber);
string errorMessage(int ruleNum, string addiInfo, string instr);
void warningMsg(int warnNum,int modNum, string symName, int currSize, 
int maxSize);

vector<int> resInstr(string addrMod, int op,int modNumb);
