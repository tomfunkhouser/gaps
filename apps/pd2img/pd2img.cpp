// Source file for the pixel database image extraction program



////////////////////////////////////////////////////////////////////////
// Include files
////////////////////////////////////////////////////////////////////////

namespace gaps {}
using namespace gaps;
#include "R2Shapes/R2Shapes.h"



////////////////////////////////////////////////////////////////////////
// Program arguments
////////////////////////////////////////////////////////////////////////

static const char *input_pixel_database = NULL;
static const char *output_image_directory = NULL;
static int print_verbose = FALSE;
static int print_debug = FALSE;



////////////////////////////////////////////////////////////////////////
// Function to extract and write images
////////////////////////////////////////////////////////////////////////

static int
WriteImages(const R2PixelDatabase& pd, const char *output_image_directory)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();
  int count = 0;
  if (print_verbose) {
    printf("Writing images to %s ...\n", output_image_directory);
    fflush(stdout);
  }
  
  // Write images
  for (int i = 0; i < pd.NKeys(); i++) {
    const char *key = pd.Key(i);

    // Get format
    int format = pd.GetFormat(key);
    if (format < 0) {
      RNFail("Unable to get format of %s\n", key);
      continue;
    }
    
    // Print debug message
    if (print_debug) printf("  %s\n", key);

    // Get pathname
    char pathname[1024];
    sprintf(pathname, "%s/%s", output_image_directory, key);

    // Create directory
    char mkdir_cmd[1040];
    sprintf(mkdir_cmd, "mkdir -p %s", pathname);
    char *endp = strrchr(mkdir_cmd, '/');
    if (endp) *endp = '\0';
    system(mkdir_cmd);

    // Write image/grid of appropriate format
    if (format == R2_PIXEL_DATABASE_3_8_PNG_FORMAT) {
      R2Image image;
      if (pd.FindImage(key, &image)) {
        image.Write(pathname);
        count++;
      }
    }
    else if (format == R2_PIXEL_DATABASE_1_16_PNG_FORMAT) {
      R2Grid grid;
      if (pd.FindGrid(key, &grid, FALSE)) {
        grid.WriteFile(pathname);
        count++;
      }
    }
    else {
      RNFail("Unrecognized format %d for %s\n", format, key);
    }
  }

  // Print message
  if (print_verbose) {
    printf("  Time = %.2f\n", start_time.Elapsed());
    printf("  # Images = %d\n", count);
    fflush(stdout);
  }

  // Return succcess
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
      if (!strcmp(*argv, "-v")) print_verbose = 1;
      else if (!strcmp(*argv, "-debug")) print_debug = 1;
      else { RNFail("Invalid program argument: %s", *argv); exit(1); }
      argv++; argc--;
    }
    else {
      if (!input_pixel_database) input_pixel_database = *argv;
      else if (!output_image_directory) output_image_directory = *argv;
      else { RNFail("Invalid program argument: %s", *argv); exit(1); }
      argv++; argc--;
    }
  }

  // Check filenames
  if (!input_pixel_database || !output_image_directory) {
    RNFail("Usage: pd2img inputpixeldatabase outputimagedirectory\n");
    return 0;
  }

  // Return OK status 
  return 1;
}



////////////////////////////////////////////////////////////////////////
// Main
////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
  // Parse program arguments
  if (!ParseArgs(argc, argv)) exit(-1);
  
  // Open pixel database
  R2PixelDatabase pd;
  if (!pd.OpenFile(input_pixel_database, "r")) exit(-1);

  // Write images
  if (!WriteImages(pd, output_image_directory)) exit(-1);

  // Close pixel database
  if (!pd.CloseFile()) exit(-1);

  // Return success
  return 0;
}






