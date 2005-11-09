////////////////////////////////////////////////////////////////////////
// FILE:        httpupargparser.h
// AUTHOR:      Johannes Winkelmann, jw@tks6.net
// COPYRIGHT:   (c) 2005 by Johannes Winkelmann
// ---------------------------------------------------------------------
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
////////////////////////////////////////////////////////////////////////


#ifndef _HTTPUPARGPARSER_H_
#define _HTTPUPARGPARSER_H_

#include "argparser.h"

class HttpupArgparser
    : public ArgParser
{
public:
    HttpupArgparser();
    virtual ~HttpupArgparser() {}


    static ArgParser::APCmd CMD_SYNC;
    // static ArgParser::APCmd CMD_MIRROR;
    static ArgParser::APCmd CMD_COPY;
    static ArgParser::APCmd CMD_LIST;

    static ArgParser::APOpt OPT_REPOFILE;
    static ArgParser::APOpt OPT_ENCODE;
    static ArgParser::APOpt OPT_VERIFY_MD5;

    std::string getAppIdentification() const
        { return std::string("httpup ")  + MF_VERSION + "\n"; }


};


#endif /* _HTTPUPARGPARSER_H_ */
