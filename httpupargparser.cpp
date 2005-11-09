////////////////////////////////////////////////////////////////////////
// FILE:        httpupargparser.cpp
// AUTHOR:      Johannes Winkelmann, jw@tks6.net
// COPYRIGHT:   (c) 2005 by Johannes Winkelmann
// ---------------------------------------------------------------------
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
////////////////////////////////////////////////////////////////////////


#include "httpupargparser.h"

ArgParser::APCmd HttpupArgparser::CMD_SYNC;
ArgParser::APCmd HttpupArgparser::CMD_COPY;
ArgParser::APCmd HttpupArgparser::CMD_LIST;

ArgParser::APOpt HttpupArgparser::OPT_REPOFILE;
ArgParser::APOpt HttpupArgparser::OPT_ENCODE;
ArgParser::APOpt HttpupArgparser::OPT_VERIFY_MD5;


HttpupArgparser::HttpupArgparser()
{
    // - sync
    addCommand(CMD_SYNC, "sync",
               "syncronize local copy with remote repository",
               ArgParser::MAX, 2, "[url] [target dir]");

    OPT_REPOFILE.init("repofile",
                      'r',
                      "alternative name for REPO file",
                      true,
                      "NAME");

    OPT_ENCODE.init("encode",
                    'e',
                    "encode special chars in URL");

    OPT_VERIFY_MD5.init("verify-md5",
                    'm',
                    "verify md5sum of downloaded files");

    addOption(CMD_SYNC, OPT_REPOFILE, false);
    addOption(CMD_SYNC, OPT_ENCODE, false);
    addOption(CMD_SYNC, OPT_VERIFY_MD5, false);


    // - copy
    addCommand(CMD_COPY, "copy",
               "copy a remote repository to a local directory",
               ArgParser::EQ, 2, "<url> <target dir>");
    addOption(CMD_COPY, OPT_REPOFILE, false);
    addOption(CMD_COPY, OPT_ENCODE, false);
    addOption(CMD_SYNC, OPT_VERIFY_MD5, false);

    // - list
    addCommand(CMD_LIST, "list",
               "list files in <directory> which are controlled by httpup",
               ArgParser::MAX, 1, "<directory>");
    addOption(CMD_LIST, OPT_REPOFILE, false);
}
