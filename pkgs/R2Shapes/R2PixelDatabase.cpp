/* Source file for the R2 pixel database class */



////////////////////////////////////////////////////////////////////////
// INCLUDE FILES
////////////////////////////////////////////////////////////////////////

#include "R2Shapes.h"



////////////////////////////////////////////////////////////////////////
// Namespace
////////////////////////////////////////////////////////////////////////

namespace gaps {



////////////////////////////////////////////////////////////////////////
// Versioning variables
////////////////////////////////////////////////////////////////////////

static unsigned int current_major_version = 0;
static unsigned int current_minor_version = 1;



////////////////////////////////////////////////////////////////////////
// R2PixelDatabaseEntry class definition
////////////////////////////////////////////////////////////////////////

struct R2PixelDatabaseEntry {
public:
  R2PixelDatabaseEntry(const char *key = NULL, unsigned long long offset = 0, unsigned int size = 0)
    : offset(offset), size(size) { this->key[0] = '\0'; if (key) strncpy(this->key, key, 127); this->key[127]='\0'; };
public:
  char key[128];
  unsigned long long offset;
  unsigned int size;
};



////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS/DESTRUCTORS
////////////////////////////////////////////////////////////////////////

R2PixelDatabase::
R2PixelDatabase(void)
  : fp(NULL),
    name(NULL),
    filename(NULL),
    rwaccess(NULL),
    major_version(current_major_version),
    minor_version(current_minor_version),
    swap_endian(0),
    entries_count(0),
    entries_offset(0),
    map()
{
}



R2PixelDatabase::
R2PixelDatabase(const R2PixelDatabase& database)
  : fp(NULL),
    name(NULL),
    filename(NULL),
    rwaccess(NULL),
    major_version(current_major_version),
    minor_version(current_minor_version),
    swap_endian(0),
    entries_count(0),
    entries_offset(0),
    map()
{
  RNAbort("Not implemented");
}



R2PixelDatabase::
~R2PixelDatabase(void)
{
  // Delete name
  if (name) free(name);

  // Delete filename
  if (filename) free(filename);

  // Delete rwaccess
  if (rwaccess) free(rwaccess);
}



////////////////////////////////////////////////////////////////////////
// PROPERTY MANIPULATION FUNCTIONS
////////////////////////////////////////////////////////////////////////

void R2PixelDatabase::
SetName(const char *name)
{
  // Set node name
  if (this->name) delete this->name;
  this->name = RNStrdup(name);
}
  


////////////////////////////////////////////////////////////////////////
// ENTRY ACCESS FUNCTIONS
////////////////////////////////////////////////////////////////////////

int R2PixelDatabase::
Find(const char *key, R2Grid *grid) const
{
  // Find entry
  R2PixelDatabaseEntry entry;
  if (!map.Find(key, &entry)) return FALSE;

  // Read grid
  if (grid) {
    // Seek to start of entry
    RNFileSeek(fp, entry.offset, RN_FILE_SEEK_SET);
  
    // Read entry
    if (!grid->ReadPNGStream(fp)) {
      RNFail("Error reading %s from pixel database\n", key);
      return FALSE;
    }
  }

  // Return success
  return TRUE;
}



////////////////////////////////////////////////////////////////////////
// ENTRY MANIPULATION FUNCTIONS
////////////////////////////////////////////////////////////////////////

int R2PixelDatabase::
Insert(const char *key, const R2Grid& pixels)
{
  // Seek to end of entries
  unsigned long long offset = entries_offset;
  RNFileSeek(fp, offset, RN_FILE_SEEK_SET);

  // Write pixels to file
  if (!pixels.WritePNGStream(fp)) return FALSE;

  // Update entries offset
  entries_offset = RNFileTell(fp);
  unsigned int size = entries_offset - offset;
  
  // Insert entry into map
  R2PixelDatabaseEntry entry(key, offset, size);
  map.Insert(key, entry);
  
  // Increment number of entries
  entries_count++;

  // Return success
  return 1;
}



int R2PixelDatabase::
Remove(const char *key)
{
  // Remove entry from map
  map.Remove(key);

  // Leaves hole in file where pixels were :(

  // Decrement number of entries
  entries_count--;

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// FILE I/O FUNCTIONS
////////////////////////////////////////////////////////////////////////

int R2PixelDatabase::
OpenFile(const char *filename, const char *rwaccess)
{
  // Remember file name
  if (this->filename) free(this->filename);
  this->filename = RNStrdup(filename);

  // Parse rwaccess
  if (this->rwaccess) free(this->rwaccess);
  if (!rwaccess) this->rwaccess = RNStrdup("w+b");
  else if (strstr(rwaccess, "w")) this->rwaccess = RNStrdup("w+b");
  else if (strstr(rwaccess, "+")) this->rwaccess = RNStrdup("r+b");
  else this->rwaccess = RNStrdup("rb"); 

  // Open file
  fp = fopen(filename, this->rwaccess);
  if (!fp) {
    RNFail("Unable to open database file %s with rwaccess %s\n", filename, rwaccess);
    return 0;
  }

  // Check if file is new
  if (!strcmp(this->rwaccess, "w+b")) {
    // Just checking ...
    assert(entries_count == 0);

    // File is new -- write header
    if (!WriteHeader(fp, 0)) return 0;

    // Update entries info
    entries_offset = RNFileTell(fp);
  }
  else {
    // Read header
    if (!ReadHeader(fp)) return 0;

    // Read entries
    if (!ReadEntries(fp, swap_endian)) return 0;
  }

  // Return success
  return 1;
}



int R2PixelDatabase::
CloseFile(void)
{
  // Check if writing file
  if (strcmp(rwaccess, "rb")) {
    // Write entries
    if (!WriteEntries(fp, swap_endian)) return 0;

    // Write header again (now that the offset values have been filled in)
    if (!WriteHeader(fp, swap_endian)) return 0;
  }

  // Close file
  fclose(fp);
  fp = NULL;
  
  // Reset filename
  if (filename) free(filename);
  filename = NULL;

  // Reset rwaccess
  if (rwaccess) free(rwaccess);
  rwaccess = NULL;

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// INTERNAL I/O FUNCTIONS
////////////////////////////////////////////////////////////////////////

int R2PixelDatabase::
ReadHeader(FILE *fp)
{
  // Seek to beginning of file
  RNFileSeek(fp, 0, RN_FILE_SEEK_SET);

  // Read magic string
  char buffer[1024]; 
  if (!RNReadChar(fp, buffer, 32, 0)) return 0;
  if (strcmp(buffer, "R2PixelDatabase")) {
    RNFail("Incorrect header (%s) in database file %s\n", buffer, filename);
    return 0;
  }

  // Read endian test
  unsigned int endian_test1, endian_test2;
  if (!RNReadUnsignedInt(fp, &endian_test1, 1, 0)) return 0;
  if (endian_test1 != 1) swap_endian = 1;
  if (!RNReadUnsignedInt(fp, &endian_test2, 1, swap_endian)) return 0;
  if (endian_test2 != 1) {
    RNFail("Incorrect endian (%x) in database file %s\n", endian_test1, filename);
    return 0;
  }

  // Read version
  if (!RNReadUnsignedInt(fp, &major_version, 1, swap_endian)) return 0;
  if (!RNReadUnsignedInt(fp, &minor_version, 1, swap_endian)) return 0;
  
  // Read entry info
  if (!RNReadUnsignedLongLong(fp, &entries_offset, 1, swap_endian)) return 0;
  if (!RNReadUnsignedInt(fp, &entries_count, 1, swap_endian)) return 0;

  // Read extra at end of header
  if (!RNReadChar(fp, buffer, 1024, swap_endian)) return 0;
  
  // Return success
  return 1;
}

 
int R2PixelDatabase::
WriteHeader(FILE *fp, int swap_endian)
{
  // Seek to beginning of file
  RNFileSeek(fp, 0, RN_FILE_SEEK_SET);

  // Get convenient variables
  unsigned int endian_test = 1;
  char magic[32] = { '\0' };
  strncpy(magic, "R2PixelDatabase", 32);
  char buffer[1024] = { '\0' };

  // Write header
  if (!RNWriteChar(fp, magic, 32, swap_endian)) return 0;
  if (!RNWriteUnsignedInt(fp, &endian_test, 1, swap_endian)) return 0;
  if (!RNWriteUnsignedInt(fp, &endian_test, 1, swap_endian)) return 0;
  if (!RNWriteUnsignedInt(fp, &major_version, 1, swap_endian)) return 0;
  if (!RNWriteUnsignedInt(fp, &minor_version, 1, swap_endian)) return 0;
  if (!RNWriteUnsignedLongLong(fp, &entries_offset, 1, swap_endian)) return 0;
  if (!RNWriteUnsignedInt(fp, &entries_count, 1, swap_endian)) return 0;
  if (!RNWriteChar(fp, buffer, 1024, swap_endian)) return 0;
  
  // Return success
  return 1;
}


 
int R2PixelDatabase::
ReadEntries(FILE *fp, int swap_endian)
{
  // Seek to entries offset
  RNFileSeek(fp, entries_offset, RN_FILE_SEEK_SET);

  // Read entries
  for (unsigned int i = 0; i < entries_count; i++) {
    R2PixelDatabaseEntry entry;
    if (!RNReadChar(fp, entry.key, 128, swap_endian)) return 0;
    if (!RNReadUnsignedLongLong(fp, &entry.offset, 1, swap_endian)) return 0;
    if (!RNReadUnsignedInt(fp, &entry.size, 1, swap_endian)) return 0;
    map.Insert(entry.key, entry);
  }

  // Return success
  return 1;
}


 
int R2PixelDatabase::
WriteEntries(FILE *fp, int swap_endian)
{
  // Seek to entries offset
  RNFileSeek(fp, entries_offset, RN_FILE_SEEK_SET);

  // Write entries
  char buffer[128] = { '\0' };
  std::map<std::string, R2PixelDatabaseEntry, RNMapComparator<std::string>>::iterator it;
  for (it = map.m->begin(); it != map.m->end(); ++it) {
    R2PixelDatabaseEntry entry = it->second;
    strncpy(buffer, entry.key, 127);
    if (!RNWriteChar(fp, buffer, 128, swap_endian)) return 0;
    if (!RNWriteUnsignedLongLong(fp, &entry.offset, 1, swap_endian)) return 0;
    if (!RNWriteUnsignedInt(fp, &entry.size, 1, swap_endian)) return 0;
  }

  // Return success
  return 1;
}

 
 
} // namespace gaps
