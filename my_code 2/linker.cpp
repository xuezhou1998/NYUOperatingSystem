#include "linker.h"
#include <iostream>
#include <fstream>
#include <string>
#include <locale>
#include <sstream>
#include <cctype>
#include <map>
#include <cstring>
#include <vector>
#include<iomanip>

#include <algorithm>
using namespace std;




map <int , int> modules;
map <int, int >modulesLen;

map <string, int> symbolTab;
map <string, int> symbolTabRel;
map <string, int> symbolTabModNum;
map <string, int> symbolErr;
vector <string> useLst;
vector <string> symLst;
vector<string> used_Lst;

int usecount=-1;

int lineNum = 1;
int lineNumNext = 1;
int lineOffset = -1;


void createModule(int instrNum,int moduleNum){


  modules.insert(pair<int, int>(moduleNum, instrNum));

}
void createSymbol(string sym, int absAddr,int relAddr,int modNum){
  if (symbolTab.empty()){
    symbolTab.insert(pair<string, int>(sym,absAddr ));
    symbolTabRel.insert(pair<string, int>(sym,relAddr ));
    symbolTabModNum.insert(pair<string, int>(sym,modNum ));
    symbolErr.insert(pair<string, int>(sym,-1 ));

  }
  else if (symbolTab.count(sym)>0){

    symbolErr[sym]=1;
  }
  else{
    symbolTab.insert(pair<string, int>(sym,absAddr ));
    symbolTabRel.insert(pair<string, int>(sym,relAddr ));
    symbolTabModNum.insert(pair<string, int>(sym,modNum ));
    symbolErr.insert(pair<string, int>(sym,-1 ));
  }
}
void createUse(string sym){
}

int parser (string filename, int parseNumber){
  int returnNum=-1;
  int errorCode=-1;
  int ruleNum=-1;
  if(parseNumber==1){
    bool eof=false;
    int instrNum=0;
    int moduleNum=1;


    while (!eof) {

      int defcount = readInt(filename);
      if (defcount==-2){
        if (moduleNum==1){
          return returnNum;
        }
        else {
          break;
        }

      }
      else if(defcount==-1){
        parseError(0,false);
        return returnNum;
      }
      else if (defcount>16){
        parseError(4,false);
        return returnNum;
      }
      createModule(instrNum,moduleNum);

      for (int i=0;i<defcount;i++) {
        string sym= readSymbol(filename);
        if (sym.length()>16){
          parseError(3,false);
          return returnNum;
        }
        else if (sym=="end of the file"){

          parseError(1,true);
          return returnNum;
        }
        else if (sym=="not symbol"){

          parseError(1,false);
          return returnNum;
        }

        int val = readInt(filename);
        if (val==-2){

          parseError(0,true);
          return returnNum;
        }
        else if (val==-1){

          parseError(0,false);
          return returnNum;
        }

        createSymbol(sym,instrNum+val,val,moduleNum);

      }
      usecount = readInt(filename);
      if (usecount>16){
        parseError(5,false);
        return returnNum;
      }
      else if (usecount==-2){
        parseError(0, true);
        return returnNum;
      }
      else if (usecount==-1){
        parseError(0, false);
        return returnNum;
      }
      for (int i=0;i<usecount;i++) {
        string sym = readSymbol(filename);
        if (sym=="not symbol" ){
          parseError(1,false);
          return returnNum;
        }
        else if (sym=="end of the file"){
          parseError(1, true);
          return returnNum;
        }

        useLst.push_back(sym);

      }

      int currInstrNum=readInt(filename);
      if (currInstrNum==-2){
        parseError(0, true);
        return returnNum;
      }
      else if (currInstrNum==-1){
        parseError(0, false);
        return returnNum;
        }
      modulesLen.insert(pair<int, int>(moduleNum, currInstrNum));

      instrNum = instrNum+currInstrNum;
      if (instrNum>512){
        parseError(6,false);
        return returnNum;
      }
      for (int i=0;i<currInstrNum;i++) {
        string addressmode = readAERI(filename);
        if (addressmode=="not AERI"){

          parseError(2,false);
          return returnNum;
        }
        else if (addressmode=="end of the file"){
          parseError(2, true);
          return returnNum;
        }

        int operand = readInt(filename);
        if (operand==-2){
          parseError(0, true);
          return returnNum;
        }
        else if (operand==-1){
          parseError(0, false);
          return returnNum;
        }

      }
      moduleNum+=1;

    }


    for (std::map<string,int>::iterator it=symbolTabRel.begin(); it!=symbolTabRel.end(); ++it){
      if (it->second > modulesLen[symbolTabModNum[it->first]] ) {

        warningMsg(0,symbolTabModNum[it->first], it->first , it->second,modulesLen[symbolTabModNum[it->first]] );
        symbolTabRel[it->first]=0;
        symbolTab[it->first]=modules[symbolTabModNum[it->first]];
      }
    }
    returnNum=0;
    cout<<"Symbol Table"<<endl;
    for (std::map<string,int>::iterator it=symbolErr.begin(); it!=symbolErr.end(); ++it){

      if (it->second==-1){
        cout<<it->first<<"="<<symbolTab[it->first]<<endl;
      }
      else if (it->second==1){
        string err=errorMessage(3,"",it->first+"="+to_string(symbolTab[it->first]));
        cout<<err<<endl;
      }
    }
  }
  else if (parseNumber==2){
    lineNum = 1;
    lineNumNext = 1;
    lineOffset = -1;

    bool eof=false;
    int instrNum=0;
    int moduleNum=1;
    cout<<""<<endl;
    cout<<"Memory Map"<<endl;
    while (!eof) {

      int defcount = readInt(filename);
      if (defcount==-2){
        break;
      }


      for (int i=0;i<defcount;i++) {
        string sym= readSymbol(filename);
        int val = readInt(filename);

      }
      usecount = readInt(filename);


      for (int i=0;i<usecount;i++) {
        string sym = readSymbol(filename);
        symLst.push_back(sym);
        used_Lst.push_back(sym);
      }
      int currInstrNum=readInt(filename);


      int prevInstr=instrNum;
      instrNum = instrNum+currInstrNum;

      for (int i=0;i<currInstrNum;i++) {
        string addressmode = readAERI(filename);
        int operand = readInt(filename);
        vector<int> resV(5);
        resV=resInstr(addressmode,operand,moduleNum);
        string instrStr="";


        string addiInfo="";
        if (resV[4]>=0){
          addiInfo=symLst[resV[4]];
        }

        string errStr=errorMessage(resV[1],addiInfo, instrStr);
        cout << setw(3) << setfill('0') << (prevInstr+i)<<": "<<setw(4) << setfill('0') << resV[0]<<" "
        <<errStr<<endl;
      }
      for (unsigned i=0; i<used_Lst.size(); ++i){
        warningMsg(1, moduleNum, used_Lst[i], -1, -1);
      }
      moduleNum+=1;
      symLst.clear();
      used_Lst.clear();
    }
    cout<<""<<endl;
    for (std::map<string,int>::iterator it=symbolErr.begin(); it!=symbolErr.end(); ++it){
      if (it->second>=-1){
          warningMsg(2, symbolTabModNum[it->first], it->first, -1, -1);
      }

    }
    returnNum=0;
  }

  return returnNum;
}

vector<int> resInstr(string addrMod, int op, int modNumb){
  int result=0;
  int opcode=op/1000;
  int operand=op % 1000;
  int err=-1;
  int baseAddr=modules[modNumb];
  int unDef=-1;

   if (addrMod=="I"){
    if (op<10000){
      result=op;
    }
    else{
      err=4;
      result=9999;
    }

  }
  else if (addrMod=="E"){
    int newOperand=0;

    if (operand<usecount){

      result=opcode*1000;
      if (symbolTab.count(symLst[operand])<=0){
        result+=0;
        err=6;
        unDef=operand;
        std::vector<string>::iterator position =
        std::find(used_Lst.begin(), used_Lst.end(), symLst[operand]);
        if (position != used_Lst.end()){
          used_Lst.erase(position);
        }
      }
      else{
        newOperand=symbolTab[symLst[operand]];

        result+=newOperand;
        std::vector<string>::iterator position = std::find(used_Lst.begin(), used_Lst.end(), symLst[operand]);
        if (position != used_Lst.end()){
          used_Lst.erase(position);
        }

        symbolErr[symLst[operand]]-=1;
      }
    }
    else {
      err=2;
      result=op;
    }
  }
  else if (addrMod=="R"){
    if (modulesLen[modNumb]>operand){
      result=opcode*1000;
      result+=operand+baseAddr;
    }
    else{
      result=opcode*1000;
      result+=baseAddr;
      err=1;
    }

  }
  else if (addrMod=="A"){
    if (operand<512){
      result=opcode*1000;
      result+=operand;
    }
    else if (operand>=512){
      err=0;
      result=opcode*1000;
      result+=0;
    }
  }
  if (opcode>=10 && err==-1){
      err=5;
      result=9999;
    }
  vector<int> resV={result,err,operand,opcode,unDef};
  return resV;
}

void parseError(int errcode, bool eof) {
    static const char* errstr[] = {
        "NUM_EXPECTED",
        "SYM_EXPECTED",
        "ADDR_EXPECTED",
        "SYM_TOO_LONG",
        "TOO_MANY_DEF_IN_MODULE",
        "TOO_MANY_USE_IN_MODULE",
        "TOO_MANY_INSTR",

    };
    if (eof==false){
      printf("Parse Error line %d offset %d: %s\n", lineNum, lineOffset+1, errstr[errcode]);
    }
    else{
      printf("Parse Error line %d offset %d: %s\n", lineNum, lineOffset+1, errstr[errcode]);
    }

}
string errorMessage(int ruleNum, string addiInfo, string instr){
  string result="";
  static const char* errstr[] = {
      "Error: Absolute address exceeds machine size; zero used",
      "Error: Relative address exceeds module size; zero used",
      "Error: External address exceeds length of uselist; treated as immediate",

      "Error: This variable is multiple times defined; first value used",
      "Error: Illegal immediate value; treated as 9999",
      "Error: Illegal opcode; treated as 9999"
    };
  if (ruleNum<0){
    result="";
  }
  else if (ruleNum==3){
    result=instr+" "+errstr[ruleNum];
  }
  else if (ruleNum==6){
    result= "Error: "+ addiInfo +" is not defined; zero used";

  }

  else{
    result= errstr[ruleNum];

  }
  return result;
}

void warningMsg(int warnNum,int modNum, string symName, int currSize,
int maxSize){
  char* symChar= new char[1024];

    symChar = &symName[0];
  static const char* warnstr[]={
    "Warning: Module %d: %s too big %d (max=%d) assume zero relative\n",
 "Warning: Module %d: %s appeared in the uselist but was not actually used\n",
 "Warning: Module %d: %s was defined but never used\n"

  };
  printf(warnstr[warnNum], modNum, symChar, currSize,maxSize-1);

}


int readInt (string filename)
{

  tokenReturn* myInt=get_token(filename);
  if (myInt->token==NULL){
    return -2;
  }

  int result=-1;

  char * strr= new char[strlen(myInt->token)+1];
  strcpy (strr, myInt->token);

  for (int i = 0; i < 1024; i++)
    {


          if ( isdigit (strr[i]))
     {
        result=1;

     }
      else if(strr[i]=='\0'){
        break;
      }
          else
     {

        result=-1;
        break;
     }
    }
  string intString(myInt->token);
  if (result==1){
    sscanf(intString.data(), "%d", &result);

  }

  delete myInt;
  delete [] strr;

  return result;
}

string
readSymbol (string filename)
{
  tokenReturn* mySym=get_token(filename);
  if (mySym->token==NULL){
    return "end of the file";
  }

  string result="not symbol";

  char * strr= new char[strlen(mySym->token)+1];
  strcpy (strr, mySym->token);

  for (int i = 0; i < 1024; i++)
    {

          if (i==0)
     {
        if (isalpha(strr[i])){
          result="symbol";

        }
        else{
          result="not symbol";
          break;
        }

     }
      else if (i>0){
        if (isalnum(strr[i])){
          result="symbol";

        }
        else if(strr[i]=='\0'){
          break;
        }
          else
       {
          result="not symbol";
          break;
       }
      }

    }
  string strString(mySym->token);
  if (result=="symbol"){



    result=strString;
  }

  delete mySym;

  delete [] strr;

  return result;
}

string
readAERI (string filename)
{


  tokenReturn* myAERI=get_token(filename);
  if (myAERI->token==NULL){
    return "end of the file";
  }



  string result="not AERI";

  char * strr= new char[strlen(myAERI->token)+1];
  strcpy (strr, myAERI->token);

  for (int i = 0; i < 1024; i++)
    {



        if (strr[i]=='A' || strr[i]=='E'|| strr[i]=='R'|| strr[i]=='I' ){
          result="AERI";

          break;
        }
        else{
          result="not AERI";
          break;
        }
    }
  string strString(myAERI->token);
  if (result=="AERI"){

    delete myAERI;
    return strString;
  }

  delete myAERI;

  delete [] strr;

  return result;
}


tokenReturn *
get_token_old (string filename)
{

  string resString = "empty";
  string lk_line;
  ifstream myfile (filename);
  int curLine = 1;
  int curOffset = 0;
  int startCharIdx = 0;
  int endCharIdx = 0;
  char ch_line[1024];
  char* token =new char[1024];
  bool nextLine = false;
  char ch_lineNext[1024];
  bool lastLine = false;
  char delimiter[4] = " \t\n";

  if (myfile.is_open ()){

      while (!myfile.eof()){
        getline (myfile, lk_line);
       if (curLine >= lineNum){


         strcpy (ch_line, lk_line.c_str ());
         lastLine = true;
         int count2=lineNum;
         while (!myfile.eof()){
            getline (myfile, lk_line);
            count2+=1;
            if (!lk_line.empty()){
              strcpy (ch_lineNext, lk_line.c_str ());
              lineNumNext = count2;
              lastLine=false;
              break;
            }
        }
          if (lastLine==true){
            lineNum=count2-1;


          }


         break;
       }
       else{
         curLine += 1;
       }
     }



    token = strtok (ch_line, delimiter);
    while (token != NULL){

      if (curOffset > lineOffset){
        lineOffset = curOffset;
        break;
      }
      token = strtok (NULL, delimiter);
      if (token == NULL){
          nextLine = true;
          break;
      }
      curOffset = token - ch_line;
    }
    if (lastLine==true){
      lineOffset=curOffset;
    }
    if (nextLine == true and lastLine == false){


      lineNum=lineNumNext;
      lineOffset = -1;
      curOffset=0;
      char delimiter[] = " \t\n";
      token = strtok (ch_lineNext, delimiter);

      while (token != NULL){

        if (curOffset > lineOffset){
          lineOffset = curOffset;
          break;
        }
        token = strtok (NULL, delimiter);
        if (token == NULL){
          nextLine = true;
          break;
        }
        curOffset = token - ch_lineNext;
      }
   }
  }
  else{
   std::cout << "cannot open the file" << std::endl;
 }

  tokenReturn *t1= new tokenReturn ;
  t1->token=token;
  t1->lineNum=lineNum;
  t1->lineOffset=lineOffset+1;
  return t1;
}

tokenReturn * get_token (string filename){
  string resString = "empty";
  string lk_line;
  ifstream myfile (filename);
  int curLine = 0;
  int curOffset = -1;
  int startCharIdx = 0;
  int endCharIdx = 0;
  char * ch_line=new char[1024];
  char* token =new char[1024];
  bool nextLine = false;
  
  bool lastLine = false;
  char delimiter[4] = " \t\n";
  int lineCount=-1;
  string lineCountStr="";
  if (myfile.is_open()){
    while (!myfile.eof()){
      getline(myfile, lineCountStr);
      lineCount+=1;
    }
  }

  myfile.close();
  ifstream myfile2 (filename);

  if (myfile2.is_open()){
    while (!myfile2.eof()){

      getline(myfile2, lk_line);
      curLine+=1;
      if (curLine==lineNum && curLine!=lineCount){
        strcpy (ch_line, lk_line.c_str ());
        break;
      }
      else if (curLine==lineNum && curLine==lineCount){
        strcpy (ch_line, lk_line.c_str ());
        lastLine=true;
        break;
      }
    }
    char lastChar=' ';
    int lastCharNum=0;
    if (lastLine==true){
        for (int i =0; i<lk_line.length();i++){
          if (lastChar!='\0'){
            lastChar=lk_line[lastCharNum];
            lastCharNum+=1;
          }
          else{
            break;
          }
        }
    }


    token = strtok(ch_line, delimiter);

    int tokenCount=0;
    while (token !=NULL){
      tokenCount+=1;
      curOffset=token-ch_line;

      if (curOffset>lineOffset){
        lineOffset=curOffset;
        break;
      }
      else{
        token=strtok(NULL, delimiter);
      }
    }
    if (lastLine==true && token==NULL){
      lineOffset=lastCharNum;
    }
    else if (lastLine==false && token==NULL){
      lineNum+=1;
      lineOffset=-1;
      tokenReturn *t2=get_token(filename);

      myfile2.close();
	//delete [] ch_line;
      return t2;
    }
    myfile2.close();
  }
  else {
    cout<<"cannot open the file"<<endl;
  }

  tokenReturn *t1= new tokenReturn ;
  t1->token=token;
  t1->lineNum=lineNum;
  t1->lineOffset=lineOffset+1;
	//delete [] ch_line;
  return t1;
}
int
main (int argc, char const *argv[])
{
  string fileName = "";
  if (argc > 1)
    {
      fileName = argv[1];
    }



  if (fileName != "")
    {
	
    int returnMsg=parser(fileName, 1);
    if (returnMsg==0){
      int returnMsg=parser(fileName,2);
  	 }
  }



  return 0;
}
