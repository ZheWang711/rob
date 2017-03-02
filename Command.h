//
// Created by Zhe Wang on 3/1/17.
//


#ifndef COMMAND_H
#define COMMAND_H

#include <cstring>
#include <string>
#include <sstream>
#include <vector>
#include <iterator>
#include <map>
#include "Constants.h"
#include <cassert>

using std::string;
using std::stringstream;

// splict a string http://stackoverflow.com/questions/236129/split-a-string-in-c
template<typename Out>
void split(const string &s, char delim, Out result) {
    stringstream ss;
    ss.str(s);
    string item;
    while (std::getline(ss, item, delim)) {
        *(result++) = item;
    }
}

std::vector<string> split(const string &s, char delim) {
    std::vector<string> elems;
    split(s, delim, std::back_inserter(elems));
    return elems;
}


class Command{
    string cmd;
    std::map<string, string> param;

public:

    // A raw command is in the form of "<cmd>|<key:val>|...|<key:val>"
    Command(const char* rawCmd){
        std::vector<string> fields = split(string(rawCmd), '|');
        this->cmd = fields[0];

        string field;
        size_t colon;
        for (int i = 1; i < fields.size(); ++i){
            field = fields[i];
            colon = fields[i].find(":");
            this->param[field.substr(0, colon)] = field.substr(colon + 1, field.length() - (colon + 1));
        }
    }

    string toString(){
        char* buffer = new char[4096];
        sprintf(buffer, "----[COMMAND]----\n\t<command:> <%s>\n", this->cmd.c_str());
        for (std::map<string,string>::iterator it=this->param.begin(); it!=this->param.end(); ++it)
            sprintf(buffer, "%s\t<%s> : <%s>\n", buffer, it->first.c_str(), it->second.c_str());
        sprintf(buffer, "%s----[COMMAND END]----\n", buffer);
        return string(buffer);
    }

    int getCmdType(){
        if (cmd == Constants::CMDMOVE){
            return Constants::MOVE;
        }
        else if (cmd == Constants::CMDROTATE){
            return Constants::ROTATE;
        }
        else if (cmd == Constants::CMDSTOP){
            return Constants::STOP;
        }
        // illegal command
        assert(false);
    }

    string getPara(string key){
        return this->param[key];
    }
};


#endif
