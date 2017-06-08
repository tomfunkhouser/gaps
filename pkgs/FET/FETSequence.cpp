////////////////////////////////////////////////////////////////////////
// Include files
////////////////////////////////////////////////////////////////////////

#include "FET.h"



////////////////////////////////////////////////////////////////////////
// Constructors
////////////////////////////////////////////////////////////////////////

FETSequence::
FETSequence(FETReconstruction *reconstruction)
  : reconstruction(NULL),
    reconstruction_index(-1),
    shapes(),
    sequence_type(0),
    bbox(FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX),
    name(NULL)
{
  // Insert into reconstruction
  if (reconstruction) reconstruction->InsertSequence(this);
}



FETSequence::
FETSequence(const FETSequence& sequence)
  : reconstruction(NULL),
    reconstruction_index(-1),
    shapes(),
    sequence_type(sequence.sequence_type),
    bbox(sequence.bbox),
    name((sequence.name) ? strdup(sequence.name) : NULL)
{
  // Copy shapes?
}



FETSequence::
~FETSequence(void) 
{
  // Remove shapes
  while (NShapes() > 0) {
    FETShape *shape = Shape(NShapes()-1);
    RemoveShape(shape);
  }

  // Remove from reconstruction
  if (reconstruction) reconstruction->RemoveSequence(this);

  // Delete name
  if (name) free(name);
}



////////////////////////////////////////////////////////////////////////
// Properties
////////////////////////////////////////////////////////////////////////

const R3Box& FETSequence::
BBox(void) const
{
  // Return bounding box
  if (bbox.IsEmpty()) ((FETSequence *) this)->UpdateBBox();
  return bbox;
}



////////////////////////////////////////////////////////////////////////
// Shape access
////////////////////////////////////////////////////////////////////////

FETShape *FETSequence::
Shape(const char *name) const
{
  // Check name
  if (!name) return NULL;

  // Return first shape found with same name
  for (int i = 0; i < NShapes(); i++) {
    FETShape *shape = Shape(i);
    if (!shape->Name()) continue;
    if (!strcmp(name, shape->Name())) return shape;
  }

  // None found
  return NULL;
}



////////////////////////////////////////////////////////////////////////
// Shape manipulation
////////////////////////////////////////////////////////////////////////

void FETSequence::
InsertShape(FETShape *shape)
{
  // Just checking
  assert(shape->sequence_index == -1);
  assert(shape->sequence == NULL);

  // Insert shape
  shape->sequence = this;
  shape->sequence_index = shapes.NEntries();
  shapes.Insert(shape);

  // Update bounding box
  if (!bbox.IsEmpty()) bbox.Union(shape->BBox());
}



void FETSequence::
RemoveShape(FETShape *shape)
{
  // Just checking
  assert(shape->sequence_index >= 0);
  assert(shape->sequence_index < shapes.NEntries());
  assert(shape->sequence == this);

  // Remove shape
  RNArrayEntry *entry = shapes.KthEntry(shape->sequence_index);
  FETShape *tail = shapes.Tail();
  tail->sequence_index = shape->sequence_index;
  shapes.EntryContents(entry) = tail;
  shapes.RemoveTail();
  shape->sequence_index = -1;
  shape->sequence = NULL;

  // Invalidate bounding box
  InvalidateBBox();
}



////////////////////////////////////////////////////////////////////////
// Update functions
////////////////////////////////////////////////////////////////////////

void FETSequence::
InvalidateBBox(void)
{
  // Invalidate bbox
  bbox.Reset(R3Point(FLT_MAX, FLT_MAX, FLT_MAX), R3Point(-FLT_MAX, -FLT_MAX, -FLT_MAX));
}



void FETSequence::
UpdateBBox(void)
{
  // Initialize bbox
  bbox = R3null_box;

  // Update bounding box from shapes
  for (int i = 0; i < NShapes(); i++) {
    FETShape *shape = Shape(i);
    bbox.Union(shape->BBox());
  }
}



////////////////////////////////////////////////////////////////////////
// Input / output functions
////////////////////////////////////////////////////////////////////////

int FETSequence::
ReadAscii(FILE *fp)
{
  // Read sequence
  int dummy = 0;
  int r, sequence_type, nshapes;
  char name_buffer[256];
  fscanf(fp, "%d", &r);
  fscanf(fp, "%s", name_buffer);
  fscanf(fp, "%d", &sequence_type);
  fscanf(fp, "%d", &nshapes);
  for (int k = 0; k < 8; k++) fscanf(fp, "%d", &dummy);

  // Assign name
  if (strcmp(name_buffer, "None") && (strcmp(name_buffer, "none"))) name = strdup(name_buffer);

  // Check reconstruction index
  assert(((r == -1) && !reconstruction) || (r == reconstruction_index));
  
  // Read shapes
  for (int i = 0; i < nshapes; i++) {
    int p;
    fscanf(fp, "%d", &p);
    if (reconstruction) {
      FETShape *shape = reconstruction->Shape(p);
      InsertShape(shape);
    }
  }

  // Return success
  return 1;
}



int FETSequence::
WriteAscii(FILE *fp) const
{
  // Write sequence
  int dummy = 0;
  int r = reconstruction_index;
  int nshapes = NShapes();
  char name_buffer[256] = { '\0' };
  if (name) strncpy(name_buffer, name, 255);
  else strncpy(name_buffer, "None", 255);
  fprintf(fp, "%d ", r);
  fprintf(fp, "%s ", name_buffer);
  fprintf(fp, "%d ", nshapes);
  fprintf(fp, "%d ", sequence_type);
  for (int k = 0; k < 8; k++) fprintf(fp, "%d ", dummy);

  // Write shapes
  for (int i = 0; i < nshapes; i++) {
    FETShape *shape = Shape(i);
    fprintf(fp, "%d ", shape->reconstruction_index);
  }

  // Return success
  return 1;
}

  

int FETSequence::
ReadBinary(FILE *fp)
{
  // Read sequence
  int r, sequence_type, nshapes, dummy;
  char name_buffer[256];
  fread(&r, sizeof(int), 1, fp);
  fread(name_buffer, sizeof(char), 256, fp);
  fread(&sequence_type, sizeof(int), 1, fp);
  fread(&nshapes, sizeof(int), 1, fp);
  for (int k = 0; k < 8; k++) fread(&dummy, sizeof(int), 1, fp);
  if (name_buffer[0] != '\0') name = strdup(name_buffer);

  // Check reconstruction index
  assert(((r == -1) && !reconstruction) || (r == reconstruction_index));
  
  // Read shapes
  for (int i = 0; i < nshapes; i++) {
    int p;
    fread(&p, sizeof(int), 1, fp);
    if (reconstruction) {
      FETShape *shape = reconstruction->Shape(p);
      InsertShape(shape);
    }
  }

  // Return success
  return 1;
}



int FETSequence::
WriteBinary(FILE *fp) const
{
  // Write sequence
  int dummy = 0;
  int nshapes = NShapes();
  char name_buffer[256] = { '\0' };
  if (name) strncpy(name_buffer, name, 255);
  fwrite(&reconstruction_index, sizeof(int), 1, fp);
  fwrite(name_buffer, sizeof(char), 256, fp);
  fwrite(&sequence_type, sizeof(int), 1, fp);
  fwrite(&nshapes, sizeof(int), 1, fp);
  for (int k = 0; k < 8; k++) fwrite(&dummy, sizeof(int), 1, fp);

  // Write shapes
  for (int i = 0; i < nshapes; i++) {
    FETShape *shape = Shape(i);
    fwrite(&shape->reconstruction_index, sizeof(int), 1, fp);
  }

  // Return success
  return 1;
}



