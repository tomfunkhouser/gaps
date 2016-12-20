#ifndef __P5D__
#define __P5D__



////////////////////////////////////////////////////////////////////////
// Dependency include files
////////////////////////////////////////////////////////////////////////

#include <vector>



////////////////////////////////////////////////////////////////////////
// P5D Type definitions
////////////////////////////////////////////////////////////////////////

struct P5DMaterial {
public:
  P5DMaterial(void);
  ~P5DMaterial(void);
  void Print(FILE *fp = NULL) const;
public:
  char *name;
  char *texture_name;
  char *color;
  char *tcolor;
  void *data;
};

struct P5DWall {
public:
  P5DWall(struct P5DRoom *room = NULL);
  ~P5DWall(void);
  void Print(FILE *fp = NULL) const;
public:
  struct P5DRoom *room;
  int room_index;
  double x1, y1, z1;
  double x2, y2, z2;
  double w;
  bool hidden;
  int idx_index;
  void *data;
};

struct P5DPrimitive {
public:
  P5DPrimitive(struct P5DFloor *floor = NULL);
  ~P5DPrimitive(void);
  int NMaterials(void) const;
  P5DMaterial *Material(int k) const;
  void Print(FILE *fp = NULL) const;
public:
  struct P5DFloor *floor;
  int floor_index;
  struct P5DRoom *room;
  int room_index;
  std::vector<P5DMaterial *> materials;
  char *className;
  char *t;
  char *puid;
  double tm;
  double x;
  double y;
  double z;
  double sX;
  double sY;
  double sZ;
  double aX;
  double aY;
  double aZ;
  int idx_index;
  void *data;
};

struct P5DObject {
public:
  P5DObject(struct P5DFloor *floor = NULL);
  ~P5DObject(void);
  int NMaterials(void) const;
  P5DMaterial *Material(int k) const;
  void Print(FILE *fp = NULL) const;
public:
  struct P5DFloor *floor;
  int floor_index;
  struct P5DRoom *room;
  int room_index;
  std::vector<P5DMaterial *> materials;
  char *className;
  char *id;
  double x;
  double y;
  double z;
  double sX;
  double sY;
  double sZ;
  double a;
  int aframe;
  int fX;
  int fY;
  int otf;
  int idx_index;
  void *data;
};

struct P5DRoom {
  P5DRoom(struct P5DFloor *floor = NULL);
  ~P5DRoom(void);
  int NWalls(void) const;
  P5DWall *Wall(int k) const;
  int NObjects(void) const;
  P5DObject *Object(int k) const;
  int NPrimitives(void) const;
  P5DPrimitive *Primitive(int k) const;
  void Print(FILE *fp = NULL) const;
public:
  struct P5DFloor *floor;
  int floor_index;
  std::vector<P5DWall *> walls;
  std::vector<P5DObject *> objects;
  std::vector<P5DPrimitive *> primitives;
  char *className;
  double h;
  double x, y;
  double sX, sY;
  char *rtype;
  char *texture;
  char *otexture;
  char *rtexture;
  char *wtexture;
  int idx_index;
  void *data;
};

struct P5DFloor {
  P5DFloor(struct P5DProject *project = NULL);
  ~P5DFloor(void);
  int NRooms(void) const;
  P5DRoom *Room(int k) const;
  int NObjects(void) const;
  P5DObject *Object(int k) const;
  int NPrimitives(void) const;
  P5DPrimitive *Primitive(int k) const;
  void Print(FILE *fp = NULL) const;
public:
  struct P5DProject *project;
  int project_index;
  std::vector<P5DRoom *> rooms;
  std::vector<P5DObject *> objects;
  std::vector<P5DPrimitive *> primitives;
  double h;
  int idx_index;
  void *data;
};

struct P5DProject {
  P5DProject(void);
  ~P5DProject(void);
  int NFloors(void) const;
  P5DFloor *Floor(int k) const;
  int ReadFile(const char *filename);
  void Print(FILE *fp = NULL) const;
public:
  char *name;
  std::vector<P5DFloor *> floors;
  void *data;
};



////////////////////////////////////////////////////////////////////////
// INLINE FUNCTIONS
////////////////////////////////////////////////////////////////////////

inline int P5DObject::
NMaterials(void) const
{
  // Return number of materials
  return materials.size();
}


inline P5DMaterial *P5DObject::
Material(int k) const
{
  // Return kth material
  return materials[k];
}



inline int P5DPrimitive::
NMaterials(void) const
{
  // Return number of materials
  return materials.size();
}


inline P5DMaterial *P5DPrimitive::
Material(int k) const
{
  // Return kth material
  return materials[k];
}



inline int P5DRoom::
NWalls(void) const
{
  // Return number of walls
  return walls.size();
}


inline P5DWall *P5DRoom::
Wall(int k) const
{
  // Return kth wall
  return walls[k];
}



inline int P5DRoom::
NObjects(void) const
{
  // Return number of objects
  return objects.size();
}


inline P5DObject *P5DRoom::
Object(int k) const
{
  // Return kth object
  return objects[k];
}



inline int P5DRoom::
NPrimitives(void) const
{
  // Return number of primitives
  return primitives.size();
}


inline P5DPrimitive *P5DRoom::
Primitive(int k) const
{
  // Return kth primitive
  return primitives[k];
}



inline int P5DFloor::
NRooms(void) const
{
  // Return number of rooms
  return rooms.size();
}


inline P5DRoom *P5DFloor::
Room(int k) const
{
  // Return kth room
  return rooms[k];
}



inline int P5DFloor::
NObjects(void) const
{
  // Return number of objects
  return objects.size();
}


inline P5DObject *P5DFloor::
Object(int k) const
{
  // Return kth object
  return objects[k];
}



inline int P5DFloor::
NPrimitives(void) const
{
  // Return number of primitives
  return primitives.size();
}


inline P5DPrimitive *P5DFloor::
Primitive(int k) const
{
  // Return kth primitive
  return primitives[k];
}



inline int P5DProject::
NFloors(void) const
{
  // Return number of floors
  return floors.size();
}


inline P5DFloor *P5DProject::
Floor(int k) const
{
  // Return kth floor
  return floors[k];
}



#endif
