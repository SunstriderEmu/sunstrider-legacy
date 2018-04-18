#ifndef __GITREVISION_H__
#define __GITREVISION_H__

#include <string>
#include "Define.h"

namespace GitRevision
{
    TC_COMMON_API char const* GetHash();
    TC_COMMON_API char const* GetDate();
    TC_COMMON_API char const* GetBranch();
    TC_COMMON_API char const* GetCMakeCommand();
    TC_COMMON_API char const* GetCMakeVersion();
    TC_COMMON_API char const* GetHostOSVersion();
    TC_COMMON_API char const* GetBuildDirectory();
    TC_COMMON_API char const* GetSourceDirectory();
    TC_COMMON_API char const* GetMySQLExecutable();
    TC_COMMON_API char const* GetFullDatabase();
    TC_COMMON_API char const* GetFullVersion();
    TC_COMMON_API char const* GetCompanyNameStr();
    TC_COMMON_API char const* GetLegalCopyrightStr();
    TC_COMMON_API char const* GetFileVersionStr();
    TC_COMMON_API char const* GetProductVersionStr();
}

#endif
