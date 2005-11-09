////////////////////////////////////////////////////////////////////////
// FILE:        fileutils.h
// AUTHOR:      Johannes Winkelmann, jw@tks6.net
// COPYRIGHT:   (c) 2002-2005 by Johannes Winkelmann
// ---------------------------------------------------------------------
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
////////////////////////////////////////////////////////////////////////

#ifndef _FILEUTILS_H_
#define _FILEUTILS_H_

#include <string>
#include <list>

class FileUtils
{
public:
    static bool fmd5sum(const std::string& fileName, unsigned char* result);

    static int deltree(const char* directory);
    static int mktree(const std::string& directory);

    static void listFiles(const std::string& target);
    static void listFilesRec(const std::string& base,
                             const std::string& offset,
                             std::list<std::string>& files);
};

#endif /* _FILEUTILS_H_ */
