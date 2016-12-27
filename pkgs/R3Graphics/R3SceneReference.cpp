/* Source file for the R3 scene reference class */



/* Include files */

#include "R3Graphics.h"



/* Member functions */

R3SceneReference::
R3SceneReference(R3Scene *referenced_scene)
  : referenced_scene(referenced_scene),
    materials()
{
}



R3SceneReference::
R3SceneReference(R3Scene *referenced_scene, const RNArray<R3Material *>& materials)
  : referenced_scene(referenced_scene),
    materials()
{
  // Copy pointers to materials
  this->materials = materials;
}



R3SceneReference::
~R3SceneReference(void)
{
}



void R3SceneReference::
Draw(const R3DrawFlags draw_flags) const
{
  // Check scene
  if (!referenced_scene) return;
  
  // Draw scene with materials
  referenced_scene->Draw(draw_flags, &materials);
}



