////////////////////////////////////////////////////////////////////////
// FILE:        configparser.cpp
// AUTHOR:      Johannes Winkelmann, jw@tks6.net
// COPYRIGHT:   (c) 2002-2005 by Johannes Winkelmann
// ---------------------------------------------------------------------
//  This program is free software; you can redistribute it and/or modify  
//  it under the terms of the GNU General Public License as published by  
//  the Free Software Foundation; either version 2 of the License, or     
//  (at your option) any later version.                                   
////////////////////////////////////////////////////////////////////////

#include <iostream>
#include "configparser.h"

using namespace std;

int ConfigParser::parseConfig(const std::string& fileName,
                              Config& config)
{
    FILE* fp = fopen(fileName.c_str(), "r");
    if (!fp) {
        return -1;
    }
    
    char line[512];
    string s;
    while (fgets(line, 512, fp)) {
        if (line[strlen(line)-1] == '\n') {
            line[strlen(line)-1] = '\0';
        }        
        s = line;
        
        string::size_type pos = s.find("#");
        if (pos != string::npos) {
            s = s.substr(0, pos);
        }

        if (s.length() > 10) {
            string key = s.substr(0, 10);
            string val = stripWhiteSpace(s.substr(10));
        
            if (key == "proxy_host") {
                config.proxyHost = val;
            } else if (s.substr(0, 10) == "proxy_port") {
                config.proxyPort = val;
            } else if (s.substr(0, 10) == "proxy_user") {
                config.proxyUser = val;
            } else if (s.substr(0, 10) == "proxy_pass") {
                config.proxyPassword = val;
            }
        }

    }
    
    fclose(fp);
    return 0;
}

string ConfigParser::stripWhiteSpace(const string& input)
{
    string output = input;
    while (isspace(output[0])) {
        output = output.substr(1);
    }
    while (isspace(output[output.length()-1])) {
        output = output.substr(0, output.length()-1);
    }
    
    return output;
}
