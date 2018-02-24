#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <iomanip>
#include <set>
#include "tokenizer.h"

using namespace std;
//------------------------Constants---------------------
const int MACHINE_SIZE = 200;

//------------------------Global Variables--------------
map<string, int> symbolTable;   //symbolTable used to store symbol and its resolved abosolute address

map<string, int> symbolToModule;    //symbolToModule used to store symbol and the number of module where it defined

map<string, bool> usedSymbol;   //usedSymbol used to keep track of symbols to see if they are used

set<string> useListUsed;    //useListUsed is a set which contains symbols that has been used in current module

map<int, int> baseTable;    //baseTable is a map used to save the base address of each module

map<string, bool> multiDefined;     //multiDefined is map to keep track of symbols, if it is multidefined then the value is true

vector<int> moduleSize;     //moduleSize is a vector which save the size of the modules

vector<string> useList;     //useList is a vector which save the symbols appear in current module's uselist

vector<string> warningList;     //warningList is a vector which save warning message

vector<string> errorList;       //errorList is a vector which save error message

int moduleNum = 0;      //moduleNum is a int used to keep track of module number

//------------------------Methods-----------------------

//isDefined is used to check if current symbol is already defined, return true if it is defined.
bool isDefined(string token) {
    if (symbolTable.find(token) != symbolTable.end()) {
        return true;
    }
    return false;
}

//saveUnusedInList is used to check if there are symbols appeared in use list but were never used.
void saveUnusedInList() {
    for (vector<string>::iterator iter = useList.begin(); iter != useList.end(); ++iter) {
        if (useListUsed.find(*iter) == useListUsed.end()) {
            string warning = "Warning: In module " + to_string(moduleNum) + " "+ *iter +" appeared in the use list but was not actually used.";
            warningList.push_back(warning);
        }
    }
}

//printWarnningMessage is used to print warnning message saved in warnning list
void printWarnningMessage() {
    cout<<endl;
    for (vector<string>::iterator iter = warningList.begin(); iter != warningList.end(); ++iter) {
        cout<<*iter<<endl;
        cout<<endl;
    }
    for(map<string, int>::iterator iter = symbolTable.begin(); iter != symbolTable.end(); ++iter) {
        if(!usedSymbol[iter->first]) {
            cout<<"Warning: "<<iter->first<<" was defined in module "<<symbolToModule[iter->first]<<" but never used."<<endl;
            cout<<endl;
        }
    }
}

//printErrorMessage is used to print error message saved in Error list
void printErrorMessage() {
    for (vector<string>::iterator iter = errorList.begin(); iter != errorList.end(); ++iter) {
        cout<<*iter<<endl;
        cout<<endl;
    }
}

//printSymbolTable is used to compute the absolute addresses of symbols, check errors and print symbol table and error information of symbols
void printSymbolTable(){
    std::cout << "Symbol Table" << std::endl;
    for(map<string, int>::iterator iter = symbolTable.begin(); iter != symbolTable.end(); ++iter) {
        int module = symbolToModule[iter->first];
        if (iter->second > moduleSize[module]) {
            iter->second = baseTable[module];
            string warning = "Error: In module " + to_string(module) + " the def of "+ iter->first + " exceeds the module size; zero (relative) used.";
            errorList.push_back(warning);
        } else {
            iter->second += baseTable[module];
        }
        cout<<iter->first<<" = "<<iter->second;
        if (multiDefined[iter->first]) {
            cout<<"    Error: This variable is multiply defined; first value used."<<endl;
        } else {
            cout<<endl;
        }
    }
}

//printAdress is used to print addresses and check errors.
void printAdrress(string type, string addr, int count, int moduleNum) {
    const char* addrtype = type.c_str();
    int address = stoi(addr);
    int baseAddr = baseTable[moduleNum];
    int opcode = address / 1000;
    int relative = address % 1000;
    int absolute;
    string error="";
    switch (addrtype[0]) {
        case 'I':
            absolute = address;
            cout<<count<<":     I "<<address<<setw(10)<<absolute;
            break;
        case 'A':
            if (relative >= MACHINE_SIZE) {
                error = "Error: Absolute address exceeds machine size; zero used";
                absolute = opcode * 1000;
                cout<<count<<":     A "<<address<<setw(10)<<absolute;
            } else {
                absolute = address;
                cout << count << ":     A " << address << setw(10) << absolute;
            }
            break;
        case 'R':
            if (relative >= moduleSize[moduleNum]) {
                error = "Error: Relative address exceeds module size; zero used";
                absolute = opcode * 1000;
                cout<<count<<":     R "<<address<<setw(10)<<absolute;
            } else {
                absolute = address + baseAddr;
                cout<<count<<":     R "<<address<<setw(10)<<address<<"+"<<baseAddr<<" = "<<absolute;
            }
            break;
        case 'E':
            string symbolUsed = useList[relative];
            if (relative >= useList.size()) {
                error = "Error: External address exceeds length of uselist; treated as immediate";
                absolute = address;
                cout<<count<<":     I "<<address<<setw(10)<<absolute;
            }else if (!isDefined(symbolUsed)) {
                useListUsed.insert(symbolUsed);
                error = "Error: " + symbolUsed + " is not defined; zero used.";
                absolute = opcode * 1000;
                cout<<count<<":     E "<<address<<setw(10)<<absolute;
            } else {
                usedSymbol[symbolUsed] = true;
                useListUsed.insert(symbolUsed);
                auto symbolAdd = symbolTable.find(symbolUsed);
                absolute = opcode * 1000 + symbolAdd->second;
                cout<<count<<":     E "<<address<<setw(10)<<address<<"+"<<symbolAdd->second<<" = "<<absolute;
            }
            break;
    }
    if (error.length() > 0) {
        cout<<"    "<<error<<endl;
    } else {
        cout<<endl;
    }
}

//passOne is used to save symbols and meta information
void passOne(string file){
    tokenizer* parser = new tokenizer(file);
    parser->getToken();
    while(parser->nextToken()) {
        //get DefList
        int defNum = stoi(parser->getToken().tokenName);
        for (int i = 0; i < defNum; i++ ) {
            if (parser->nextToken()) {
                tokenizer::Token symbol = parser->getToken();
                tokenizer::Token add = parser->getToken();
                if (isDefined(symbol.tokenName)) {
                    multiDefined[symbol.tokenName] = true;
                } else {
                    symbolTable[symbol.tokenName] = stoi(add.tokenName);
                    symbolToModule[symbol.tokenName] = moduleNum;
                }
            }
        }
        //get UsedList
        int usedNum = stoi(parser->getToken().tokenName);
        for (int i = 0; i < usedNum; i++) {
            if (parser->nextToken()) {
                parser->getToken();
            }
        }
        //get Instructions
        int instrNum = stoi(parser->getToken().tokenName);
        moduleSize.push_back(instrNum);
        for (int i = 0; i < instrNum; i++) {
            if (parser->nextToken()) {
                parser->getToken();
                parser->getToken();
            }
        }
        baseTable[moduleNum + 1] = baseTable[moduleNum] + instrNum;
        moduleNum++;
    }
    parser->close();
}

//passTwo is used to resolve external addresses and relative addresses
void passTwo(string file) {
    cout<<endl<<"Memory Map"<<endl;
    tokenizer* parser2 = new tokenizer(file);
    parser2->getToken();
    moduleNum= 0;
    while(parser2->nextToken()) {
        cout<<"+"<<baseTable[moduleNum]<<endl;
        useList.clear();
        useListUsed.clear();
        //get DefList
        int defNum = stoi(parser2->getToken().tokenName);
        for (int i = 0; i < defNum; i++ ) {
            if (parser2->nextToken()) {
                parser2->getToken();
                parser2->getToken();
            }
        }
        //get UsedList
        int usedNum = stoi(parser2->getToken().tokenName);
        for (int i = 0; i < usedNum; i++) {
            if (parser2->nextToken()) {
                string symbol = parser2->getToken().tokenName;
                useList.push_back(symbol);
            }
        }
        //get Instructions
        int instrNum = stoi(parser2->getToken().tokenName);
        for (int i = 0; i < instrNum; i++) {
            if (parser2->nextToken()) {
                tokenizer::Token type = parser2->getToken();
                tokenizer::Token addr = parser2->getToken();
                printAdrress(type.tokenName, addr.tokenName, i, moduleNum);
            }
        }
        saveUnusedInList();
        moduleNum++;
    }
    parser2->close();
}


int main(int argc, char* argv[]) {
    string fileName(argv[1]);
    passOne(fileName);
    printSymbolTable();
    passTwo(fileName);
    printWarnningMessage();
    printErrorMessage();
    return 0;
}
