////////////////////////////////////////////////////////////////////////
// Source file for object descriptor class
////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////
// Include files
////////////////////////////////////////////////////////////////////////

#include "object.h"



////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////

ObjectDescriptor::
ObjectDescriptor(const double *values, int nvalues)
  : nvalues(nvalues),
    values(NULL)
{ 
  // Initialize values
  if (nvalues > 0) {
    this->values = new double [ nvalues ];
    for (int i = 0; i < nvalues; i++) {
      this->values[i] = (values) ? values[i] : 0.0; 
    }
  }
}  

  

ObjectDescriptor::
ObjectDescriptor(const ObjectDescriptor& descriptor)
  : nvalues(descriptor.nvalues),
    values(NULL)
{ 
  // Initialize values
  if (nvalues > 0) {
    values = new double [ nvalues ];
    for (int i = 0; i < nvalues; i++) {
      values[i] = descriptor.values[i];
    }
  }
}  

  

ObjectDescriptor::
~ObjectDescriptor(void)
{
  // Delete values
  if (values) delete [] values;
}



ObjectDescriptor& ObjectDescriptor::
operator=(const ObjectDescriptor& descriptor)
{
  // Copy values
  Reset(descriptor.values, descriptor.nvalues);
  return *this;
}



void ObjectDescriptor::
Reset(const double *values, int nvalues)
{ 
  // Delete old values
  if ((this->nvalues > 0) && (this->values)) {
    delete [] this->values;
    this->values = NULL;
    this->nvalues = 0;
  }

  // Assign new values
  if (nvalues > 0) {
    // Allocate new values
    this->nvalues = nvalues;
    this->values = new double [ this->nvalues ];

    // Copy values
    for (int i = 0; i < nvalues; i++) {
      this->values[i] = (values) ? values[i] : 0.0;
    }
  }
}



double ObjectDescriptor::
SquaredDistance(const ObjectDescriptor& descriptor) const
{
  // Check dimensionality
  assert(nvalues == descriptor.nvalues);

  // Sum squared distances
  double sum = 0;
  for (int i = 0; i < nvalues; i++) {
    double delta = values[i] - descriptor.values[i];
    sum += delta * delta;
  }

  // Return squared distance
  return sum;
}

