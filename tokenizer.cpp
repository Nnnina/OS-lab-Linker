//
// Created by Nina Lei on 1/30/18.
//
#include <iostream>
#include <fstream>
#include "tokenizer.h"


//------------------------Methods--------------------

//constructor of tokenizer
tokenizer::tokenizer(string inputFile) {
    file.open(inputFile);
    this->lineNum = 0;
    this->offset = 0;
}

//getLineNum is used to return the line Number of current token
int tokenizer::getLineNum() {
    return lineNum;
}

//getOffset is used to return the offset of current token
int tokenizer::getOffset() {
    return offset;
}

//close is used to close the file stream
void tokenizer::close() {
    this->file.close();
}

//nextWord is used to check if there is next word and skip white spaces including ' ' and '\t'
bool tokenizer::nextToken() {
    if (offset >= line.length()) {
        if (getline(file, line)) {
            lineNum++;
            offset = 0;
        } else {
            return false;
        }
    }
    for (; offset < line.length(); offset++) {
        char cur = line.at(offset);
        if (cur != '\t' && cur != ' ') {
            break;
        }
    }
    return true;
}

//getToken is used to return next Token
tokenizer::Token tokenizer::getToken() {
    int start = 0;
    if (nextToken()) {
        start = offset;
        while (offset < line.length()) {
            if (line.at(offset) == ' ' || line.at(offset) == '\t') {
                break;
            }
            offset++;
        }
        Token token(line.substr(start, offset - start), lineNum, start);
        return token;
    }
    Token token("", lineNum, start);
    return token;
}
