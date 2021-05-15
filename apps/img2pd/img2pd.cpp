// Source file for loading images into a pixel database



////////////////////////////////////////////////////////////////////////
// Include files
////////////////////////////////////////////////////////////////////////

namespace gaps {}
using namespace gaps;
#include "R2Shapes/R2Shapes.h"
#include <dirent.h>



////////////////////////////////////////////////////////////////////////
// Program arguments
////////////////////////////////////////////////////////////////////////

static RNArray<const char *> input_image_filenames;
static RNArray<const char *> input_list_filenames;
static RNArray<const char *> input_image_directories;
static const char *output_pixel_database_filename = NULL;
static int print_verbose = 0;
static int print_debug = 0;



////////////////////////////////////////////////////////////////////////
// Statistics variables
////////////////////////////////////////////////////////////////////////

static int image_count = 0;



////////////////////////////////////////////////////////////////////////
// Processing functions
////////////////////////////////////////////////////////////////////////

static int
AddImageToPixelDatabase(R2PixelDatabase& pd, const char *image_filename)
{
  // Add image to pixel database
  R2Image image;
  if (!image.ReadFile(image_filename)) return 0;
  pd.InsertImage(image_filename, image);

  // Increment image counter
  image_count++;

  // Print message
  if (print_debug) {
    printf("  %s\n", image_filename);
    fflush(stdout);
  }
  
  // Return success
  return 1;
}



static int
AddListToPixelDatabase(R2PixelDatabase& pd, const char *list_filename)
{
  // Open list of images
  FILE *fp = fopen(list_filename, "r");
  if (!fp) {
    RNFail("Unable to open list of images: %s", list_filename);
    return 0;
  }

  // Add images to pixel database
  char image_filename[4096];
  while (fscanf(fp, "%s", image_filename) == (unsigned int) 1) {
    if (!AddImageToPixelDatabase(pd, image_filename)) return 0;
  }

  // Close list of images
  fclose(fp);

  // Return success
  return 1;
}



static int
AddDirectoryToPixelDatabase(R2PixelDatabase& pd, const char *directory_name)
{
  // Open directory
  DIR *dir = opendir(directory_name);
  if (!dir) {
    RNFail("Unable to open directory %s\n", directory_name);
    return 0;
  }

  // Read directory contents
  for (struct dirent *entry = readdir(dir); entry; entry = readdir(dir)) {
    char path_name[4096];
    const char *entry_name = entry->d_name;
    if (entry_name[0] == '.') continue;
    sprintf(path_name, "%s/%s", directory_name, entry_name);
    if (!AddImageToPixelDatabase(pd, path_name)) return 0;
   }

  // Close directory
  closedir(dir);

  // Return success
  return 1;
}  



////////////////////////////////////////////////////////////////////////
// Program argument parsing
////////////////////////////////////////////////////////////////////////

static int 
ParseArgs(int argc, char **argv)
{
  // Parse arguments
  argc--; argv++;
  while (argc > 0) {
    if ((*argv)[0] == '-') {
      if (!strcmp(*argv, "-v")) {
        print_verbose = 1;
      }
      else if (!strcmp(*argv, "-debug")) {
        print_debug = 1;
      }
      else if (!strcmp(*argv, "-list")) {
        argc--; argv++; input_list_filenames.Insert(*argv);
      }
      else if (!strcmp(*argv, "-image")) {
        argc--; argv++; input_image_filenames.Insert(*argv);
      }
      else if (!strcmp(*argv, "-directory")) {
        argc--; argv++; input_image_directories.Insert(*argv);
      }
      else {
        RNFail("Invalid program argument: %s", *argv);
        exit(1);
      }
      argv++; argc--;
    }
    else {
      if (strstr(*argv, ".pd")) output_pixel_database_filename = *argv;
      else if (strstr(*argv, ".png")) input_image_filenames.Insert(*argv);
      else if (strstr(*argv, ".PNG")) input_image_filenames.Insert(*argv);
      else if (strstr(*argv, ".jpeg")) input_image_filenames.Insert(*argv);
      else if (strstr(*argv, ".JPEG")) input_image_filenames.Insert(*argv);
      else if (strstr(*argv, ".jpg")) input_image_filenames.Insert(*argv);
      else if (strstr(*argv, ".JPG")) input_image_filenames.Insert(*argv);
      else if (strstr(*argv, ".txt")) input_list_filenames.Insert(*argv);
      else input_image_directories.Insert(*argv);
      argv++; argc--;
    }
  }

  // Check inputs
  RNBoolean input_found = FALSE;
  if (!input_image_filenames.IsEmpty()) input_found = TRUE;
  if (!input_list_filenames.IsEmpty()) input_found = TRUE;
  if (!input_image_directories.IsEmpty()) input_found = TRUE;
  
  // Check program arguments
  if (!output_pixel_database_filename || !input_found) {
    RNFail("Usage: img2pd output.pd [inputoptions].\n");
    return 0;
  }

  // Return OK status 
  return 1;
}



////////////////////////////////////////////////////////////////////////
// Main function
////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
  // Parse program arguments
  if (!ParseArgs(argc, argv)) exit(-1);

  // Print message
  RNTime start_time;
  start_time.Read();
  if (print_verbose) {
    printf("Adding images to %s\n", output_pixel_database_filename);
    fflush(stdout);
  }

  // Allocate pixel database
  R2PixelDatabase pd;
    
  // Open pixel database for writing
  if (!pd.OpenFile(output_pixel_database_filename, "w+")) {
    RNFail("Unable to open initial pixel database %s\n", output_pixel_database_filename);
    exit(-1);
  }

  // Add images to pixel database
  for (int i = 0; i < input_image_filenames.NEntries(); i++) {
    const char *image_filename = input_image_filenames.Kth(i);
    if (!AddImageToPixelDatabase(pd, image_filename)) exit(-1);
  }
  
  // Add lists of images to pixel database
  for (int i = 0; i < input_list_filenames.NEntries(); i++) {
    const char *list_filename = input_list_filenames.Kth(i);
    if (!AddListToPixelDatabase(pd, list_filename)) exit(-1);
  }
  
  // Add directories of images to pixel database
  for (int i = 0; i < input_image_directories.NEntries(); i++) {
    const char *image_directory = input_image_directories.Kth(i);
    if (!AddDirectoryToPixelDatabase(pd, image_directory)) exit(-1);
  }

  // Close pixel database
  if (!pd.CloseFile()) {
    RNFail("Unable to close initial pixel database %s\n", output_pixel_database_filename);
    exit(-1);
  }

  // Print message
  if (print_verbose) {
    printf("  Time = %.2f\n", start_time.Elapsed());
    printf("  # Images = %d\n", image_count);
    fflush(stdout);
  }

  // Return success
  return 0;
}






