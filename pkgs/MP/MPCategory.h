////////////////////////////////////////////////////////////////////////
// Include file for MP category
////////////////////////////////////////////////////////////////////////
#ifndef __MP__CATEGORY__H__
#define __MP__CATEGORY__H__



////////////////////////////////////////////////////////////////////////
// Namespace
////////////////////////////////////////////////////////////////////////

namespace gaps {



////////////////////////////////////////////////////////////////////////
// Class definition
////////////////////////////////////////////////////////////////////////

struct MPCategory {
  // Constructor/destructor
  MPCategory(void);
  ~MPCategory(void);

  // Manipulation stuff
  void InsertObject(MPObject *object);
  void RemoveObject(MPObject *object);

public:
  struct MPHouse *house;
  int house_index;
  RNArray<MPObject *> objects;
  int label_id;
  char *label_name;
  int mpcat40_id;
  char *mpcat40_name;  
};



// End namespace
}; 



// End include guard
#endif
