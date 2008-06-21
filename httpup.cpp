////////////////////////////////////////////////////////////////////////
// FILE:        httpup.cpp
// AUTHOR:      Johannes Winkelmann, jw@tks6.net
// COPYRIGHT:   (c) 2002-2005 by Johannes Winkelmann
// ---------------------------------------------------------------------
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
////////////////////////////////////////////////////////////////////////

#include <iostream>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libgen.h>
#include <dirent.h>
#include <cstring>
#include <cstdlib>

#include "fileutils.h"
#include "httpup.h"
#include "configparser.h"

using namespace std;

const string HttpUp::DEFAULT_REPOFILE = "REPO";
const string HttpUp::REPOCURRENTFILEOLD = "REPO.CURRENT";
const string HttpUp::REPOCURRENTFILE = ".httpup-repo.current";
const string HttpUp::URLINFO = ".httpup-urlinfo";

const int HttpUp::DEFAULT_TIMEOUT = 60;

HttpUp::HttpUp(const HttpupArgparser& argParser,
               const string& url, const string& target,
               const string& fragment, const string& repoFile,
               bool verifyMd5)
    : m_baseDirectory(target),
      m_remoteUrl(url),
      m_fragment(fragment),
      m_argParser(argParser),
      m_verifyMd5(verifyMd5)
{
    if (repoFile != "") {
        m_repoFile = repoFile;
    } else {
        m_repoFile = DEFAULT_REPOFILE;
    }
}


int HttpUp::parseCurrent()
{
    FILE* fp = fopen((m_baseDirectory+"/"+REPOCURRENTFILE).c_str(), "r");
    if (!fp) {
        // TODO: remove in 0.3.1
        fp = fopen((m_baseDirectory+"/"+REPOCURRENTFILEOLD).c_str(), "r");
        if (!fp) {
            return -1;
        }
    }
    char input[512];

    while (fgets(input, 512, fp)) {
        input[strlen(input)-1] = '\0';
        m_actions[string(input)] = REMOVE;
    }

    return 0;
}

int HttpUp::findDiff()
{
    FILE* fp = fopen((m_baseDirectory + m_repoFile).c_str(), "r");
    if (!fp) {
        cerr << "Couldn't open " << m_repoFile << endl;
        return -1;
    }
    char input[512];
    struct stat info;

    string fileToStat;
    while (fgets(input, 512, fp)) {
        input[strlen(input)-1] = '\0';
        if (input[0] == 'd') {

            string dir = input+2;

            if (m_fragment != "" &&
                dir.substr(0, m_fragment.length()) != m_fragment) {
                // doesn't start with fragment
                continue;
            }

            if (m_fragment == dir) {
                continue;
            }

            if (m_fragment != "") {
                if (dir.substr(0, m_fragment.length()) == m_fragment &&
                    dir.length() > m_fragment.length()+1 &&
                    dir[m_fragment.length()] == '/') {
                    // strip; matching but hierarchy

                    dir = dir.substr(m_fragment.length()+1);
                    if (dir.length() == 0) {
                        continue;
                    }
                } else {
                    // strip: fragment is only a substring of dir
                    continue;
                }
            }

            m_remoteFiles.push_back(dir);
            fileToStat = m_baseDirectory + (dir);
            if (stat(fileToStat.c_str(), &info) == 0) {
                // dir exists
                if (!S_ISDIR(info.st_mode)) {
                    m_actions[dir] = REPLACE_FILE_WITH_DIR;
                } else {
                    m_actions[dir] = NOP;
                }
            } else {
                m_actions[dir] = DIR_CREATE;
            }
        } else {
            int fileNameOffset = 2 + 32 + 1;
            // 0+2+32+1 means
            //   +2  skip the "f:" string
            //   +32 skip the md5 string
            //   +1  skip the separator (':') between fileName and md5

            string file = input+fileNameOffset;
            if (m_fragment != "" &&
                file.substr(0, m_fragment.length()) != m_fragment) {
                // doesn't start with fragment
                continue;
            }

            if (m_fragment != "") {

                if (file.substr(0, m_fragment.length()) == m_fragment &&
                    file.length() > m_fragment.length()+1 &&
                    file[m_fragment.length()] == '/') {

                    file = file.substr(m_fragment.length()+1);
                } else {
                    // skip; fragment is only a substring
                    continue;
                }
            }


            m_remoteFiles.push_back(file);
            fileToStat = m_baseDirectory + (file);
            if (stat(fileToStat.c_str(), &info) == 0) {

                if (S_ISDIR(info.st_mode)) {
                    m_actions[file] = REPLACE_DIR_WITH_FILE;
                } else {
                    // file exists
                    unsigned char result[16];
                    bool diff = false;
                    if (FileUtils::fmd5sum(fileToStat, result)) {
                        input[2+32] = '\0';
                        diff = verifyMd5sum(input+2, result);
                    }
                    if (diff) {
                        m_actions[file] = FILE_GET;
                    } else {
                        m_actions[file] = NOP;
                    }
                }
            } else {
                m_actions[file] = NEW_FILE_GET;
            }

            if (m_verifyMd5) {
                m_md5sums[file] = string(input+2);
            }
        }
    }
    fclose(fp);

    return 0;
}

bool HttpUp::verifyMd5sum(const char* input, unsigned char result[16])
{
    static char hexNumbers[] = {'0','1','2','3','4','5','6','7',
                                '8','9','a','b','c','d','e','f'};
    bool diff = false;

    unsigned char high, low;
    for (int i = 0; i < 16; ++i) {
        high = (result[i] & 0xF0) >> 4;
        low = result[i] & 0xF;
        if (*(input+2*i) - hexNumbers[high] ||
            *(input+2*i+1) - hexNumbers[low]) {
            diff = true;
            break;
        }
    }

    return diff;
}

int HttpUp::exec(ExecType type)
{
    struct stat info;
    if (stat(m_baseDirectory.c_str(), &info)) {
        if (FileUtils::mktree(m_baseDirectory.c_str())) {
            cerr << "Failed to create base directory "
                 << m_baseDirectory << endl;
            return -1;
        }
    }

    Config config;
    ConfigParser::parseConfig("/etc/httpup.conf", config);


    // TODO: check return values.
    CURL *curl;
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();

    char errorBuffer[CURL_ERROR_SIZE];
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorBuffer);
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);

    long timeout = DEFAULT_TIMEOUT;
    if (config.operationTimeout != "") {
        char* end = 0;
        long config_timeout = 0;
        config_timeout = strtol(config.operationTimeout.c_str(), &end, 10);
        if (*end == 0) {
            timeout = config_timeout;
        }
    }
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);


    // proxy, proxy auth
    if (config.proxyHost != "") {
        curl_easy_setopt(curl, CURLOPT_PROXY, config.proxyHost.c_str());
    }

    if (config.proxyPort != "") {
        long port = atol(config.proxyPort.c_str());
        curl_easy_setopt(curl, CURLOPT_PROXYPORT, port);
    }

    string usrpwd;
    if (config.proxyUser != "" || config.proxyPassword != "") {
        usrpwd = config.proxyUser + ":" + config.proxyPassword;
        curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD, usrpwd.c_str());
    }


#if 0
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
#endif

    if (!curl) {
        cerr << "Failed to initialize CURL engine" << endl;
        return -1;
    }

    cout << "Connecting to " << m_remoteUrl << endl;
    int ret = syncOrReturn(curl, errorBuffer);

    curl_easy_cleanup(curl);

    if (ret == 0) {

        if (type == TYPE_SYNC) {
            saveRepoCurrent();
        } else if (type == TYPE_COPY){
            unlink((m_baseDirectory+m_repoFile).c_str());
        }


        if (type == TYPE_SYNC) {
            FILE* fp = fopen((m_baseDirectory+"/"+URLINFO).c_str(), "w");
            if (fp) {
                fprintf(fp, "%s#%s",
                        m_remoteUrl.c_str(), m_fragment.c_str());
                fclose(fp);
            } else {
                cerr << "Failed to store urlinfo" << endl;
            }
        }
    }

    return ret;
}

int HttpUp::syncOrReturn(CURL* curl, char* curlErrorBuffer)
{

    if (getRemoteRepoFile(curl, curlErrorBuffer) != 0) {
        cerr << "Failed to get remote repo file" << endl;
        return -1;
    }

    string collectionName = 
        m_baseDirectory.substr(0, m_baseDirectory.length()-1);
    string::size_type pos = collectionName.rfind("/");
    if (pos != string::npos) {
        collectionName = collectionName.substr(pos+1);
    }

    cout << "Updating collection " << collectionName << endl;

    // compare with local directory
    if (parseCurrent() != 0) {
        // -- also "fails" the first time...
        // cerr << "Failed to parse local directory" << endl;
        // return -1;
    }

    if (findDiff() != 0) {
        cerr << "Failed to check for differences" << endl;
        return -1;
    }

#if 0
    if (m_actions.size() == 0) {
        cerr << "No matches found for fragment " << m_fragment << endl;
        return -1;
    }
#endif


    return getChangedFiles(collectionName, curl, curlErrorBuffer);;
}

void HttpUp::saveRepoCurrent()
{
    // save current
    FILE* current = fopen((m_baseDirectory + REPOCURRENTFILE).c_str(), "w");
    if (!current) {
        cerr << "Couldn't open "
             << m_baseDirectory << REPOCURRENTFILE << " for writing" << endl;
    } else {
        list<string>::iterator cit = m_remoteFiles.begin();
        for (; cit != m_remoteFiles.end(); ++cit) {
            fprintf(current, "%s\n", cit->c_str());
        }
        fclose(current);
    }

    // TODO: remove in 0.3.1
    FILE* fp = fopen((m_baseDirectory+REPOCURRENTFILEOLD).c_str(), "r");
    if (fp) {
        fclose(fp);
        unlink((m_baseDirectory+REPOCURRENTFILEOLD).c_str());
    }

    unlink((m_baseDirectory+m_repoFile).c_str());
    cout << "Finished successfully" << endl;
}

int HttpUp::getChangedFiles(const string& collectionName, CURL* curl,
                             char* curlErrorBuffer)
{
    int errors = 0;

    string fragment = m_fragment;
    if (fragment != "") {
        fragment += "/";
    }

    // synchronize
    map<string, Action>::iterator it = m_actions.begin();
    for (; it != m_actions.end(); ++it) {

        if (it->first.substr(0, 3) == "../" ||
            it->first.find("/../") != string::npos) {
            cerr << " WARNING: Malicious path in remote REPO file: "
                 << it->first << endl;
            continue;
        }

        if (it->second == DIR_CREATE) {
            cout << " Checkout: "
                 << collectionName << "/" << it->first << endl;

            mkdir((m_baseDirectory+it->first).c_str(), 0755);
        } else if (it->second == NEW_FILE_GET || it->second == FILE_GET) {
            if (it->second == NEW_FILE_GET) {
                cout << " Checkout: "
                     << collectionName << "/" << it->first << endl;
            } else if (it->second == FILE_GET) {
                cout << " Edit: "
                     << collectionName << "/" << it->first << endl;
            }

            string fileName = it->first;
            if (m_argParser.isSet(HttpupArgparser::OPT_ENCODE)) {
                char* p = curl_escape(fileName.c_str(), fileName.length());
                fileName = p;
                curl_free(p);
            }

            string fileURL = m_remoteUrl+fragment+fileName;
            curl_easy_setopt(curl, CURLOPT_URL, fileURL.c_str());

            FILE* dlFile = fopen((m_baseDirectory+it->first).c_str(), "w");
            if (!dlFile) {
                cout << "  Failed to open " << it->first
                     << " for writing" <<endl;
            } else {
                curl_easy_setopt(curl, CURLOPT_FILE, dlFile);
                CURLcode res = curl_easy_perform(curl);
                if (res) {
                    cout << "Failed to download " << fileURL
                         << ": " << curlErrorBuffer << endl;
                }
                fclose(dlFile);
            }

            if (m_verifyMd5) {
                unsigned char result[16];
                if (FileUtils::fmd5sum(m_baseDirectory+it->first, result)) {
                    bool diff =
                        verifyMd5sum(m_md5sums[it->first.c_str()].c_str(),
                                             result);

                    if (diff) {
                        cerr << "Bad md5sum after download for "
                             << it->first << endl;
                        ++errors;
                    }
                } else {
                    ++errors;
                }
            }

        } else if (it->second == REPLACE_DIR_WITH_FILE) {
            cout << " Cowardly refusing to overwrite directory '"
                 << m_baseDirectory+it->first
                 << "' with a file" << endl;
            continue;
        } else if (it->second == REPLACE_FILE_WITH_DIR) {
            cout << " Remove: "
                 << collectionName << "/" << it->first
                 << " (file)"
                 << endl;
            int ret = FileUtils::deltree((m_baseDirectory+it->first).c_str());

            if (ret == 0) {
                cout << " Checkout: "
                     << collectionName << "/" << it->first << endl;
                mkdir((m_baseDirectory+it->first).c_str(), 0755);
            }
        } else if (it->second == REMOVE) {
            cout << " Delete: "
                 << collectionName << "/" << it->first << endl;

            if (FileUtils::deltree((m_baseDirectory+it->first).c_str())) {
                cout << "  Failed to remove " << it->first << endl;
                m_remoteFiles.push_back(it->first);
            }
        }
    }

    return errors;
}

int HttpUp::getRemoteRepoFile(CURL* curl, char* curlErrorBuffer)
{
    // download repo
    FILE* dlFile = 0;
    string fileName = m_repoFile;
    if (m_argParser.isSet(HttpupArgparser::OPT_ENCODE)) {
        char* p = curl_escape(fileName.c_str(), fileName.length());
        fileName = p;
        curl_free(p);
    }
    string repoURL = m_remoteUrl + fileName;

    curl_easy_setopt(curl, CURLOPT_URL, repoURL.c_str());
    dlFile = fopen((m_baseDirectory+m_repoFile).c_str(), "w");
    if (!dlFile) {
        cout << "  Failed to open " << m_repoFile << " for writing" << endl;
    } else {

        curl_easy_setopt(curl, CURLOPT_FILE, dlFile);
        CURLcode res = curl_easy_perform(curl);
        if (res) {
            cerr << "  Failed to download " << m_repoFile
                 << ": " << curlErrorBuffer << endl;
            return -1;
        }
        fclose(dlFile);
    }


    return 0;
}


