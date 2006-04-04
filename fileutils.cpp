////////////////////////////////////////////////////////////////////////
// FILE:        fileutils.cpp
// AUTHOR:      Johannes Winkelmann, jw@tks6.net
// COPYRIGHT:   (c) 2002-2005 by Johannes Winkelmann
// ---------------------------------------------------------------------
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
////////////////////////////////////////////////////////////////////////

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <iostream>

#include "md5.h"
#include "httpup.h"
#include "fileutils.h"

using namespace std;

int FileUtils::deltree(const char* directory)
{
    int ret = 0;

    struct stat info;
    if (stat(directory, &info)) {
        // already removed
        return 0;
    }
    if (!S_ISDIR(info.st_mode)) {
        return unlink(directory);
    }

    DIR* dir = opendir(directory);
    struct dirent* entry;
    while ((entry = readdir(dir)) != 0) {
        if (entry->d_name[0] == '.' &&
            (entry->d_name[1] == '.' || entry->d_name[1] == '\0')) {
            continue;
        }
        struct stat info;
        if (stat(entry->d_name, &info) != 0) {
            return -1;
        }
        string pathName = string(directory) + "/" + string(entry->d_name);
        if (S_ISDIR(info.st_mode)) {
            if (deltree(pathName.c_str())) {
                ret = -1;
            }
            rmdir(pathName.c_str());
        } else {
            if (unlink(pathName.c_str())) {
                ret = -1;
            }
        }
    }
    closedir(dir);
    if (rmdir(directory)) {
        ret = -1;
    }

    return ret;
}

int FileUtils::mktree(const string& directory)
{
    int ret = 0;
    size_t pos = 0;
    string fName;
    while ((pos = directory.find( '/', pos+1)) != string::npos ) {
        fName = directory.substr(0, pos);
        struct stat info;
        if (stat(fName.c_str(), &info)) {
            if (mkdir(fName.c_str(), 0755)) {
                ret = -1;
            }
        }
    }

    return ret;
}


bool FileUtils::fmd5sum(const string& fileName, unsigned char* result)
{
    struct md5_context ctx;
    unsigned char buffer[1000];

    FILE* f = fopen(fileName.c_str(), "r");
    if (!f) {
        return false;
    }
    md5_starts( &ctx );
    int i = 0;
    while( ( i = fread( buffer, 1, sizeof( buffer ), f ) ) > 0 ) {
        md5_update( &ctx, buffer, i );
    }
    fclose(f);

    md5_finish( &ctx, result );
    return true;
}

void FileUtils::listFiles(const string& target)
{
    list<string> files;
    string newTarget = target;

    if (newTarget != ".") {
        if (newTarget[newTarget.length()-1] != '/') {
            newTarget += "/";
        }
    }

    string repoFile = newTarget + "/" + HttpUp::REPOCURRENTFILE;
    FILE* fp = fopen(repoFile.c_str(), "r");
    if (fp) {
        char line[512];
        while (fgets(line, 512, fp)) {
            line[strlen(line)-1] = '\0';
            files.push_back(line);
        }
        fclose(fp);

        listFilesRec(newTarget, "", files);
    } else {
        cerr << "Failed to open " << repoFile << endl;
    }
}

void FileUtils::listFilesRec(const string& base,
                             const string& offset,
                             list<string>& files)
{
    string newOff = offset;
    if (newOff.length() > 0) {
        newOff += "/";
    }

    DIR* dir = opendir((base + newOff).c_str());
    if (dir) {
        struct dirent* d;
        string name;
        while ((d = readdir(dir))) {

            name = d->d_name;
            if (name == HttpUp::REPOCURRENTFILE ||
                name == HttpUp::URLINFO ||
                name == "." || name == "..") {
                continue;
            }

            if (find(files.begin(), files.end(),
                     newOff + d->d_name) == files.end()) {
                cout << "? ";
            } else {
                cout << "= ";
            }
            cout << newOff
                 << d->d_name << endl;

            struct stat buf;
            if (stat(((base + newOff) + d->d_name).c_str(), &buf) == 0 &&
                S_ISDIR(buf.st_mode)) {
                listFilesRec(base, newOff + d->d_name, files);
            }
        }

        closedir(dir);
    }
}

