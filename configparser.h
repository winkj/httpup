////////////////////////////////////////////////////////////////////////
// FILE:        configparser.h
// AUTHOR:      Johannes Winkelmann, jw@tks6.net
// COPYRIGHT:   (c) 2002-2005 by Johannes Winkelmann
// ---------------------------------------------------------------------
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
////////////////////////////////////////////////////////////////////////

#ifndef _CONFIGPARSER_H_
#define _CONFIGPARSER_H_

#include <string>

struct Config
{
    Config() 
        : proxyHost(""), 
          proxyPort(""), 
          proxyUser(""), 
          proxyPassword(""),
          operationTimeout("")
        {}
    
    std::string proxyHost;
    std::string proxyPort;
    std::string proxyUser;
    std::string proxyPassword;
    std::string operationTimeout;
};

class ConfigParser
{
public:
    static std::string stripWhiteSpace(const std::string& input);
    static int parseConfig(const std::string& fileName,
                           Config& config);
};

#endif /* _CONFIGPARSER_H_ */
