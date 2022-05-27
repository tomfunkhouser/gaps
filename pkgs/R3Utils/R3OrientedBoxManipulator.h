// Include file for interactive oriented box manipulator
#ifndef __R3__ORIENTED__BOX__MANNIPULATOR__H__
#define __R3__ORIENTED__BOX__MANNIPULATOR__H__



////////////////////////////////////////////////////////////////////////
// Include files
////////////////////////////////////////////////////////////////////////

#include "R3Graphics/R3Graphics.h"



////////////////////////////////////////////////////////////////////////
// Namespace
////////////////////////////////////////////////////////////////////////

namespace gaps {



////////////////////////////////////////////////////////////////////////
// Class definition
////////////////////////////////////////////////////////////////////////

class R3OrientedBoxManipulator {  
public:
  // Constructor/destructor
  R3OrientedBoxManipulator(void);
  R3OrientedBoxManipulator(const R3OrientedBox& oriented_box);

  // Oriented box query functions
  const R3OrientedBox& OrientedBox(void) const;
  
  // Manipulation status query functions
  int ManipulationType(void) const;
  RNBoolean IsManipulating(void) const;
  RNBoolean IsRotating(void) const;
  RNBoolean IsScaling(void) const;
  RNBoolean IsTranslating(void) const;
  RNBoolean IsRotatingAllowed(void) const;
  RNBoolean IsScalingAllowed(void) const;
  RNBoolean IsTranslatingAllowed(void) const;
  RNBoolean IsDirty(void) const;
  
  // High-level manipulation functions
  int BeginManipulation(const R3Viewer& viewer, int x, int y);
  int UpdateManipulation(const R3Viewer& viewer, int x, int y);
  void EndManipulation(void);

  // Property setting functions
  void SetOrientedBox(const R3OrientedBox& oriented_box);
  void RotateOrientedBox(RNAngle theta);
  void SwapOrientedBoxAxes(void);
  void SetRotatingAllowed(RNBoolean allowed);
  void SetScalingAllowed(RNBoolean allowed);
  void SetTranslatingAllowed(RNBoolean allowed);
  void Reset(void);
  
  // Draw functions
  void Draw(void) const;
  void DrawOrientedBox(void) const;
  void DrawNose(void) const;
  void DrawAnchor(void) const;

public:
  // Reset functions
  void ResetOrientedBox(void);
  void ResetManipulation(void);
  void ResetSelection(void);
  
  // Pick functions
  int Pick(const R3Viewer& viewer, int x, int y);

  // Low-level manipulation functions
  int UpdateRotation(const R3Viewer& viewer, int x, int y);
  int UpdateScale(const R3Viewer& viewer, int x, int y);
  int UpdateTranslation(const R3Viewer& viewer, int x, int y);

  // Low-level geometry query functions
  R3Point AnchorPosition(void) const;

private:
  R3OrientedBox oriented_box;
  RNBoolean rotating_allowed;
  RNBoolean scaling_allowed;
  RNBoolean translating_allowed;
  int selection_corner0;
  int selection_corner1;
  double selection_t;
  int manipulation_type;
  int manipulation_counter;
};


  
////////////////////////////////////////////////////////////////////////
// Manipulation types
////////////////////////////////////////////////////////////////////////

enum {
  R3_NO_MANIPULATION,
  R3_ROTATION_MANIPULATION,
  R3_SCALE_MANIPULATION,
  R3_TRANSLATION_MANIPULATION,
  R3_NUM_MANIPULATIONS
};

  
  
////////////////////////////////////////////////////////////////////////
// Inline functions
////////////////////////////////////////////////////////////////////////


inline const R3OrientedBox& R3OrientedBoxManipulator::
OrientedBox(void) const
{
  // Return oriented box
  return oriented_box;
}


  
inline int R3OrientedBoxManipulator::
ManipulationType(void) const
{
  // Return manipulation type
  return manipulation_type;
}


  
inline RNBoolean R3OrientedBoxManipulator::
IsRotatingAllowed(void) const
{
  // Return whether rotating is allowed
  return rotating_allowed;
}

  

inline RNBoolean R3OrientedBoxManipulator::
IsScalingAllowed(void) const
{
  // Return whether scaling is allowed
  return scaling_allowed;
}

  

inline RNBoolean R3OrientedBoxManipulator::
IsTranslatingAllowed(void) const
{
  // Return whether translating is allowed
  return translating_allowed;
}

  

inline RNBoolean R3OrientedBoxManipulator::
IsManipulating(void) const
{
  // Return whether oriented box is being manipulated
  if (manipulation_type == R3_NO_MANIPULATION) return FALSE;
  return TRUE;
}


  
inline RNBoolean R3OrientedBoxManipulator::
IsRotating(void) const
{
  // Return whether oriented box is being rotated
  if (manipulation_type != R3_ROTATION_MANIPULATION) return FALSE;
  return TRUE;
}


  
inline RNBoolean R3OrientedBoxManipulator::
IsScaling(void) const
{
  // Return whether oriented box is being resized
  if (manipulation_type != R3_SCALE_MANIPULATION) return FALSE;
  return TRUE;
}


  
inline RNBoolean R3OrientedBoxManipulator::
IsTranslating(void) const
{
  // Return whether oriented box is being translated
  if (manipulation_type != R3_TRANSLATION_MANIPULATION) return FALSE;
  return TRUE;
}


  
inline RNBoolean R3OrientedBoxManipulator::
IsDirty(void) const
{
  // Return whether oriented box has changed since BeginManipulation
  return (manipulation_counter > 0) ? TRUE : FALSE;
}


  
}; // end namespace



#endif
