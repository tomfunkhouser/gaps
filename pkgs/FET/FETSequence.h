////////////////////////////////////////////////////////////////////////
// FETSequence class definition
////////////////////////////////////////////////////////////////////////

struct FETSequence {
public:
  // Constructor
  FETSequence(FETReconstruction *reconstruction = NULL);
  FETSequence(const FETSequence& sequence);
  ~FETSequence(void);

  // Reconstruction access
  FETReconstruction *Reconstruction(void) const;
  int ReconstructionIndex(void) const;
  
  // Shape access
  int NShapes(void) const;
  FETShape *Shape(int k) const;
  FETShape *Shape(const char *name) const;
  
  // Properties
  int Type(void) const;
  R3Point Centroid(void) const;
  const R3Box& BBox(void) const;
  const char *Name(void) const;

  // Manipulation
  void InsertShape(FETShape *shape);
  void RemoveShape(FETShape *shape);
  void SetType(int sequence_type);
  void SetName(const char *name);

  // Input/output
  int ReadAscii(FILE *fp);
  int ReadBinary(FILE *fp);
  int WriteAscii(FILE *fp) const;
  int WriteBinary(FILE *fp) const;

public:
  // Internal updates
  void InvalidateBBox();
  void UpdateBBox();

public:
  // Internal data
  FETReconstruction *reconstruction;
  int reconstruction_index;
  RNArray<FETShape *> shapes;
  int sequence_type;
  R3Box bbox;
  char *name;
};



////////////////////////////////////////////////////////////////////////
// Sequence types
////////////////////////////////////////////////////////////////////////

#define TRAJECTORY_SEQUENCE_TYPE 0
#define PANORAMA_SEQUENCE_TYPE 1




////////////////////////////////////////////////////////////////////////
// Inline functions
////////////////////////////////////////////////////////////////////////

inline FETReconstruction *FETSequence::
Reconstruction(void) const
{
  // Return reconstruction
  return reconstruction;
}



inline int FETSequence::
ReconstructionIndex(void) const
{
  // Return index of this sequence in reconstruction
  return reconstruction_index;
}



inline int FETSequence::
NShapes(void) const
{
  // Return number of shapes
  return shapes.NEntries();
}



inline FETShape *FETSequence::
Shape(int k) const
{
  // Return kth shape
  return shapes.Kth(k);
}



inline int FETSequence::
Type(void) const
{
  // Return sequence type
  return sequence_type;
}



inline R3Point FETSequence::
Centroid(void) const
{
  // Return centroid
  return BBox().Centroid();
}



inline const char *FETSequence::
Name(void) const
{
  // Return name
  return name;
}



inline void FETSequence::
SetType(int sequence_type)
{
  // Set type
  this->sequence_type = sequence_type;
}



inline void FETSequence::
SetName(const char *name)
{
  // Set name
  if (this->name) free(this->name);
  if (name) this->name = strdup(name);
  else this->name = NULL;
}



