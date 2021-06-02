////////////////////////////////////////////////////////////////////////
// Source file for MP category
////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////
// Include files
////////////////////////////////////////////////////////////////////////

#include "MP.h"



////////////////////////////////////////////////////////////////////////
// Namespace
////////////////////////////////////////////////////////////////////////

namespace gaps {




////////////////////////////////////////////////////////////////////////
// Member functions
////////////////////////////////////////////////////////////////////////

MPCategory::
MPCategory(void)
  : house(NULL),
    house_index(-1),
    objects(),
    label_id(-1),
    label_name(NULL),
    mpcat40_id(-1),
    mpcat40_name(NULL)
{
}



MPCategory::
~MPCategory(void)
{
  // Remove all objects
  while (!objects.IsEmpty()) RemoveObject(objects.Tail());
  
  // Remove from house
  if (house) house->RemoveCategory(this);
}



void MPCategory::
InsertObject(MPObject *object)
{
  // Insert object
  object->category = this;
  object->category_index = objects.NEntries();
  objects.Insert(object);
}



void MPCategory::
RemoveObject(MPObject *object)
{
  // Remove object
  MPObject *tail = objects.Tail();
  tail->category_index = object->category_index;
  objects[object->category_index] = tail;
  objects.RemoveTail();
  object->category = NULL;
  object->category_index = -1;
}

  

// End namespace
};
