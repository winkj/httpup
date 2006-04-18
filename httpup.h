////////////////////////////////////////////////////////////////////////
// FILE:        httpup.h
// AUTHOR:      Johannes Winkelmann, jw@tks6.net
// COPYRIGHT:   (c) 2002-2005 by Johannes Winkelmann
// ---------------------------------------------------------------------
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
////////////////////////////////////////////////////////////////////////

#ifndef _HTTPUP_H_
#define _HTTPUP_H_

#include <string>
#include <list>
#include <map>

#include <curl/curl.h>
#include "httpupargparser.h"

class HttpUp
{
public:
    enum Action { NOP,
                  DIR_CREATE,
                  REPLACE_FILE_WITH_DIR,
                  FILE_GET,
                  NEW_FILE_GET,
                  REPLACE_DIR_WITH_FILE,
                  REMOVE };

    enum ExecType {
        TYPE_SYNC,
        TYPE_COPY,
        TYPE_MIRROR
    };

    HttpUp(const HttpupArgparser& argParser,
           const std::string& url,
           const std::string& target,
           const std::string& fragment,
           const std::string& repoFile,
           bool verifyMd5);
    int exec(ExecType execType);


    static const std::string DEFAULT_REPOFILE;
    static const std::string REPOCURRENTFILE;
    static const std::string REPOCURRENTFILEOLD;
    static const std::string URLINFO;

    static const int DEFAULT_TIMEOUT;

private:
    int syncOrReturn(CURL* curl, char* curlErrorBuffer);

    int getRemoteRepoFile(CURL* curl, char* curlErrorBuffer);
    int getChangedFiles(const std::string& collectionName,
                         CURL* curl, char* curlErrorBuffer);

    void saveRepoCurrent();

    int findDiff();
    int parseCurrent();

    static bool verifyMd5sum(const char* input, unsigned char result[16]);


    std::map<std::string, Action> m_actions;
    std::map<std::string, std::string> m_md5sums;
    std::list<std::string> m_remoteFiles;

    const std::string m_baseDirectory;
    const std::string m_remoteUrl;
    const std::string m_fragment;
    std::string m_repoFile;

    const HttpupArgparser& m_argParser;
    bool m_verifyMd5;
};


#endif /* _HTTPUP_H_ */
