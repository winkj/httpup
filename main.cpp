////////////////////////////////////////////////////////////////////////
// FILE:        main.cpp
// AUTHOR:      Johannes Winkelmann, jw@tks6.net
// COPYRIGHT:   (c) 2002-2005 by Johannes Winkelmann
// ---------------------------------------------------------------------
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <string>
#include <cstdlib>
using namespace std;

#include <unistd.h>

#include "httpup.h"
#include "fileutils.h"
#include "httpupargparser.h"


int main(int argc, char** argv)
{
    HttpupArgparser htap;
    htap.parse(argc, argv);

    HttpUp::ExecType execType = HttpUp::TYPE_SYNC;

    if (htap.command() == HttpupArgparser::CMD_SYNC) {
        execType = HttpUp::TYPE_SYNC;
    } else if (htap.command() == HttpupArgparser::CMD_COPY) {
        execType = HttpUp::TYPE_COPY;
    } else if (htap.command() == HttpupArgparser::CMD_LIST) {
        string dir = ".";
        if (htap.otherArguments().size() > 0) {
            dir = htap.otherArguments()[0];
        }
        FileUtils::listFiles(dir);
        exit(0);
    } else {
        cerr << "Supported commands so far:\n"
             << "   sync  [<url#fragment>] [<target dir>]\n"
             << "   list  [<target dir>]\n"
             << "\n"
             << " if target dir is omitted, the "
             << " current working directory is used.\n"
             << " if url is omitted, it is read from the .httpup-urlinfo file"
             << endl;
        exit(-1);
    }

    string url = "";
    string fragment = "";
    if (htap.otherArguments().size() > 0) {
        url = htap.otherArguments()[0];
    } else {
        FILE* fp = fopen(HttpUp::URLINFO.c_str(), "r");
        if (!fp) {
            cerr << "Couldn't find " << HttpUp::URLINFO
                 << " in current working directory. "
                 << endl;
            exit(-1);
        }
        char urlBuf[512];
        fgets(urlBuf, 512, fp);
        url = urlBuf;
    }

    if (!htap.isSet(HttpupArgparser::OPT_ENCODE)) {
        string::size_type pos = url.find("#");
        if (pos != string::npos) {
            fragment = url.substr(pos+1);
            url = url.substr(0, pos);
        }

        if (fragment[fragment.size()-1] == '/') {
            fragment = fragment.substr(0, fragment.length()-1);
        }
    }
    if (url[url.size()-1] != '/') {
        url += '/';
    }

    string target = "";
    if (htap.otherArguments().size() > 1) {
        target = htap.otherArguments()[1];
    } else {
        char* pwd = new char[256];
        if (getcwd(pwd, 265) == NULL) {
            delete pwd;
            pwd = new char[1024];
            if (getcwd(pwd, 1024) == NULL) {
                cerr << "Path longer then 1024 characters; exiting" << endl;
                exit(-1);
            }
        }

        target = pwd;
        delete pwd;
    }
    if (target[target.size()-1] != '/') {
        target += '/';
    }

    string repoFile = "";
    if (htap.isSet(HttpupArgparser::OPT_REPOFILE)) {
        repoFile = htap.getOptionValue(HttpupArgparser::OPT_REPOFILE);
    }

    bool verifyMd5 = false;
    if (htap.isSet(HttpupArgparser::OPT_VERIFY_MD5)) {
        verifyMd5 = true;
    }

#if 0
    cout << "Sync "
         << (fragment==""?"all":fragment) << " from "
         << url << " to "
         << target << endl;
#endif

    HttpUp httpup(htap, url, target, fragment, repoFile, verifyMd5);
    return httpup.exec(execType);
}
