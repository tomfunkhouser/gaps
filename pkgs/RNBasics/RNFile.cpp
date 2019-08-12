// Source file for file management utilities


// Include files
#include "RNBasics.h"



// Namespace

namespace gaps {



////////////////////////////////////////////////////////////////////////
// FILE EXISTENCE FUNCTIONS
////////////////////////////////////////////////////////////////////////

RNBoolean
RNFileExists(const char *filename)
{
    // Return whether or not file exists (and is readable)
    FILE *fp = fopen(filename, "rb");
    if (!fp) return FALSE;
    fclose(fp); 
    return TRUE;
}



////////////////////////////////////////////////////////////////////////
// FILE SIZE FUNCTIONS
////////////////////////////////////////////////////////////////////////

unsigned long long
RNFileSize(const char *filename)
{
#if 1
    // Return size of file in bytes
    FILE *fp = fopen(filename, "r");
    if (!fp) return 0;
    RNFileSeek(fp, RN_FILE_SEEK_END, 0);
    unsigned long long file_size = RNFileTell(fp);
    fclose(fp);
    return file_size;
#else
    // Requires "unistd.h"
    struct stat stat_buf;
    int rc = fstat(filename, &stat_buf);
    if (rc == 0) return 0;
    else return stat_buf.st_size;
#endif
}



////////////////////////////////////////////////////////////////////////
// FILE SEEK FUNCTIONS
////////////////////////////////////////////////////////////////////////

int 
RNFileSeek(FILE *fp, unsigned long long offset, int whence)
{
#if (RN_OS == RN_WINDOWS)
    // Windows
    if (_fseek64(fp, offset, whence) == 0) return 1;
    else return 0;
#elif (RN_CC_VER == RN_C11)
    // Linux/unix/cygwin etc.
    if (fseek(fp, offset, whence) == 0) return 1;
    else return 0;
#else
    // Linux/unix/cygwin etc.
    if (fseeko(fp, offset, whence) == 0) return 1;
    else return 0;
#endif
}



unsigned long long 
RNFileTell(FILE *fp)
{
#if (RN_OS == RN_WINDOWS)
    return _ftell64(fp);
#elif (RN_CC_VER == RN_C11)
    return ftell(fp);
#else
    // Linux/unix/cygwin etc.
    return ftello(fp);
#endif
}
  


} // namespace gaps
