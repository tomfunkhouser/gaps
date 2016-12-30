/* Include file for the R3 scene reference class */



/* Class definitions */

class R3SceneReference {
public:
  // Constructor functions
  R3SceneReference(R3Scene *referenced_scene = NULL);
  R3SceneReference(R3Scene *referenced_scene, const RNArray<R3Material *>& materials);
  ~R3SceneReference(void);

  // Access functions
  int NMaterials(void) const;
  R3Material *Material(int k) const;
  const RNArray<R3Material *>& Materials(void) const;
  R3Scene *ReferencedScene(void) const;

  // Manipulation function
  void SetReferencedScene(R3Scene *scene);
  void InsertMaterial(R3Material *material);
  void ReplaceMaterial(int k, R3Material *material);

  // Draw functions
  void Draw(const R3DrawFlags draw_flags = R3_DEFAULT_DRAW_FLAGS) const;

private:
  R3Scene *referenced_scene;
  RNArray<R3Material *> materials;
};



/* Inline functions */

inline R3Scene *R3SceneReference::
ReferencedScene(void) const
{
  // Return referenced scene
  return referenced_scene;
}



inline int R3SceneReference::
NMaterials(void) const
{
  // Return number of materials
  return materials.NEntries();
}



inline R3Material *R3SceneReference::
Material(int k) const
{
  // Return material
  return materials.Kth(k);
}



inline const RNArray<R3Material *>& R3SceneReference::
Materials(void) const
{
  // Return materials
  return materials;
}



inline void R3SceneReference::
SetReferencedScene(R3Scene *scene)
{
  // Set referenced scene
  this->referenced_scene = scene;
}



inline void R3SceneReference::
InsertMaterial(R3Material *material)
{
  // Insert material
  materials.Insert(material);
}



inline void R3SceneReference::
ReplaceMaterial(int k, R3Material *material)
{
  // Replace material
  RNArrayEntry *entry = materials.KthEntry(k);
  materials.EntryContents(entry) = material;
}



