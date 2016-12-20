////////////////////////////////////////////////////////////////////////
// Include files
////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <limits.h>
#include "p5d.h"



////////////////////////////////////////////////////////////////////////
// Material functions
////////////////////////////////////////////////////////////////////////

P5DMaterial::
P5DMaterial(void)
  : name(NULL),
    texture_name(NULL),
    color(NULL),
    tcolor(NULL),
    data(NULL)
{
}



P5DMaterial::
~P5DMaterial(void)
{
  // Delete string
  if (name) free(name);
  if (texture_name) free(texture_name);
  if (color) free(color);
  if (tcolor) free(tcolor);
}



////////////////////////////////////////////////////////////////////////
// Wall functions
////////////////////////////////////////////////////////////////////////

P5DWall::
P5DWall(P5DRoom *room)
  : room(room),
    room_index(-1),
    x1(0),
    y1(0),
    z1(0),
    x2(0),
    y2(0),
    z2(0),
    w(0),
    hidden(false),
    idx_index(-1),
    data(NULL)
{
  // Insert wall into room
  if (room) {
    this->room_index = room->NWalls();
    room->walls.push_back(this);
  }
}



P5DWall::
~P5DWall(void)
{
}



void P5DWall::
Print(FILE *fp) const
{
  // Resolve output file
  if (!fp) fp = stdout;

  // Print attributes
  fprintf(fp, "      Wall\n");
  fprintf(fp, "      w = %g\n", w);
  fprintf(fp, "      x1 = %g\n", x1);
  fprintf(fp, "      y1 = %g\n", y1);
  fprintf(fp, "      z1 = %g\n", z1);
  fprintf(fp, "      x2 = %g\n", x2);
  fprintf(fp, "      y2 = %g\n", y2);
  fprintf(fp, "      z2 = %g\n", z2);
  fprintf(fp, "\n");
}



////////////////////////////////////////////////////////////////////////
// Primitive functions
////////////////////////////////////////////////////////////////////////

P5DPrimitive::
P5DPrimitive(P5DFloor *floor)
  : floor(floor),
    floor_index(-1),
    room(NULL),
    room_index(-1),
    materials(),
    className(NULL),
    t(NULL),
    puid(NULL),
    tm(0),
    x(0), y(0), z(0),
    sX(1), sY(1), sZ(1),
    aX(0), aY(0), aZ(0),
    idx_index(-1),
    data(NULL)
{
  // Insert primitive into floor
  if (floor) {
    this->floor_index = floor->NPrimitives();
    floor->primitives.push_back(this);
  }
}



P5DPrimitive::
~P5DPrimitive(void)
{
}



void P5DPrimitive::
Print(FILE *fp) const
{
  // Resolve output file
  if (!fp) fp = stdout;
  
  // Print attributes
  fprintf(fp, "    Primitive\n");
  fprintf(fp, "    className = %s\n", (className) ? className : "None");
  fprintf(fp, "    t = %s\n", (t) ? t : "None");
  fprintf(fp, "    puid = %s\n", (puid) ? puid : "None");
  fprintf(fp, "    tm = %g\n", tm);
  fprintf(fp, "    x = %g\n", x);
  fprintf(fp, "    y = %g\n", y);
  fprintf(fp, "    z = %g\n", z);
  fprintf(fp, "    sX = %g\n", sX);
  fprintf(fp, "    sY = %g\n", sY);
  fprintf(fp, "    sZ = %g\n", sZ);
  fprintf(fp, "    aX = %g\n", aX);
  fprintf(fp, "    aY = %g\n", aY);
  fprintf(fp, "    aZ = %g\n", aZ);
  fprintf(fp, "\n");
}



////////////////////////////////////////////////////////////////////////
// Object functions
////////////////////////////////////////////////////////////////////////

P5DObject::
P5DObject(P5DFloor *floor)
  : floor(floor),
    floor_index(-1),
    room(NULL),
    room_index(-1),
    materials(),
    className(NULL),
    id(NULL),
    x(0), y(0), z(0),
    sX(1), sY(1), sZ(1),
    a(0),
    aframe(0),
    fX(0), fY(0),
    otf(0),
    idx_index(-1),
    data(NULL)
{
  // Insert object into floor
  if (floor) {
    this->floor_index = floor->NObjects();
    floor->objects.push_back(this);
  }
}



P5DObject::
~P5DObject(void)
{
}



void P5DObject::
Print(FILE *fp) const
{
  // Resolve output file
  if (!fp) fp = stdout;
  
  // Print attributes
  fprintf(fp, "    Object\n");
  fprintf(fp, "    className = %s\n", (className) ? className : "None");
  fprintf(fp, "    id = %s\n", (id) ? id : "None");
  fprintf(fp, "    x = %g\n", x);
  fprintf(fp, "    y = %g\n", y);
  fprintf(fp, "    z = %g\n", z);
  fprintf(fp, "    sX = %g\n", sX);
  fprintf(fp, "    sY = %g\n", sY);
  fprintf(fp, "    sZ = %g\n", sZ);
  fprintf(fp, "    a = %g\n", a);
  fprintf(fp, "    fX = %d\n", fX);
  fprintf(fp, "    fY = %d\n", fY);
  fprintf(fp, "    otf = %d\n", otf);
  fprintf(fp, "\n");
}



////////////////////////////////////////////////////////////////////////
// Room functions
////////////////////////////////////////////////////////////////////////

P5DRoom::
P5DRoom(P5DFloor *floor)
  : floor(floor),
    floor_index(-1),
    walls(),
    objects(),
    className(NULL),
    h(0),
    x(0), y(0),
    sX(0), sY(0),
    rtype(NULL),
    texture(NULL),
    otexture(NULL),
    rtexture(NULL),
    wtexture(NULL),
    idx_index(-1),
    data(NULL)
{
  // Insert room into floor
  if (floor) {
    this->floor_index = floor->NRooms();
    floor->rooms.push_back(this);
  }
}



P5DRoom::
~P5DRoom(void)
{
}



void P5DRoom::
Print(FILE *fp) const
{
  // Resolve output file
  if (!fp) fp = stdout;

  // Print attributes
  fprintf(fp, "    Room\n");
  fprintf(fp, "    className = %s\n", (className) ? className : "None");
  fprintf(fp, "    h = %g\n", h);
  fprintf(fp, "    x = %g\n", x);
  fprintf(fp, "    y = %g\n", y);
  fprintf(fp, "    sX = %g\n", sX);
  fprintf(fp, "    sY = %g\n", sY);
  fprintf(fp, "\n");
  
  // Print walls
  for (int i = 0; i < NWalls(); i++) {
    P5DWall *wall = Wall(i);
    wall->Print(fp);
  }
}



////////////////////////////////////////////////////////////////////////
// Floor functions
////////////////////////////////////////////////////////////////////////

P5DFloor::
P5DFloor(P5DProject *project)
  : project(project),
    project_index(-1),
    rooms(),
    objects(),
    h(0),
    idx_index(-1),
    data(NULL)
{
  // Insert floor into project
  if (project) {
    this->project_index = project->NFloors();
    project->floors.push_back(this);
  }
}



P5DFloor::
~P5DFloor(void)
{
}



void P5DFloor::
Print(FILE *fp) const
{
  // Resolve output file
  if (!fp) fp = stdout;

  // Print attributes
  fprintf(fp, "  Floor\n");
  fprintf(fp, "  h = %g\n", h);
  
  // Print rooms
  for (int i = 0; i < NRooms(); i++) {
    P5DRoom *room = Room(i);
    room->Print(fp);
  }

  // Print objects
  for (int i = 0; i < NObjects(); i++) {
    P5DObject *object = Object(i);
    object->Print(fp);
  }

  // Print primitives
  for (int i = 0; i < NPrimitives(); i++) {
    P5DPrimitive *primitive = Primitive(i);
    primitive->Print(fp);
  }
}



///////////////////////////////////////////////////////////////////////
// Project functions
////////////////////////////////////////////////////////////////////////

P5DProject::
P5DProject(void)
  : name(NULL),
    floors(),
    data(NULL)
{
}



P5DProject::
~P5DProject(void)
{
  // Delete name
  if (name) free(name);
}



void P5DProject::
Print(FILE *fp) const
{
  // Resolve output file
  if (!fp) fp = stdout;

  // Print attributes
  fprintf(fp, "Project %s\n", (name) ? name : "");

  // Print floors
  for (int i = 0; i < NFloors(); i++) {
    P5DFloor *floor = Floor(i);
    floor->Print(fp);
  }
}


////////////////////////////////////////////////////////////////////////
// PARSING FUNCTIONS
////////////////////////////////////////////////////////////////////////

#include "json.h"


static int
GetJsonObjectMember(Json::Value *&result, Json::Value *object, const char *str, int expected_type = 0)
{
  // Check object type
  if (object->type() != Json::objectValue) {
    // fprintf(stderr, "P5D: not an object\n");
    return 0;
  }

  // Check object member
  if (!object->isMember(str)) {
    // fprintf(stderr, "P5D object has no member named %s\n", str);
    return 0;
  }

  // Get object member
  result = &((*object)[str]);
  if (result->type() == Json::nullValue) {
    // fprintf(stderr, "P5D object has null member named %s\n", str);
    return 0;
  }

  // Check member type
  if (expected_type > 0) {
    if (result->type() != expected_type) {
      // fprintf(stderr, "P5D object member %s has unexpected type %d (rather than %d)\n", str, result->type(), expected_type);
      return 0;
    }
  }
  
  // Check for empty strings
  if (result->type() == Json::stringValue) {
    if (result->asString().length() == 0) {
      // fprintf(stderr, "P5D object has zero length string named %s\n", str);
      return 0;
    }
  }

  // Return success
  return 1;
}



static int
GetJsonArrayEntry(Json::Value *&result, Json::Value *array, unsigned int k, int expected_type = -1)
{
  // Check array type
  if (array->type() != Json::arrayValue) {
    fprintf(stderr, "P5D: not an array\n");
    return 0;
  }

  // Check array size
  if (array->size() <= k) {
    // fprintf(stderr, "P5D array has no member %d\n", k);
    return 0;
  }

  // Get entry
  result = &((*array)[k]);
  if (result->type() == Json::nullValue) {
    // fprintf(stderr, "P5D array has null member %d\n", k);
    return 0;
  }

  // Check entry type
  if (expected_type > 0) {
    if (result->type() != expected_type) {
      // fprintf(stderr, "P5D array entry %d has unexpected type %d (rather than %d)\n", k, result->type(), expected_type);
      return 0;
    }
  }
  
  // Return success
  return 1;
}



static int 
ParseObject(P5DObject *object, Json::Value *json_object, int idx_index)
{
  // Parse attributes
  Json::Value *json_value;
  if (GetJsonObjectMember(json_value, json_object, "className", Json::stringValue)) 
    object->className = strdup(json_value->asString().c_str());
  if (GetJsonObjectMember(json_value, json_object, "id", Json::stringValue)) 
    object->id = strdup(json_value->asString().c_str());
  if (GetJsonObjectMember(json_value, json_object, "x")) 
    object->x = 0.01 * json_value->asDouble();
  if (GetJsonObjectMember(json_value, json_object, "y")) 
    object->y = 0.01 * json_value->asDouble();
  if (GetJsonObjectMember(json_value, json_object, "z")) 
    object->z = 0.01 * json_value->asDouble();
  if (GetJsonObjectMember(json_value, json_object, "sX")) 
    object->sX = 0.01 * json_value->asDouble();
  if (GetJsonObjectMember(json_value, json_object, "sY")) 
    object->sY = 0.01 * json_value->asDouble();
  if (GetJsonObjectMember(json_value, json_object, "sZ")) 
    object->sZ = 0.01 * json_value->asDouble();
  if (GetJsonObjectMember(json_value, json_object, "a")) 
    object->a = 3.14159265358979323846 * json_value->asDouble() / 180.0;
  if (GetJsonObjectMember(json_value, json_object, "aframe", Json::intValue)) 
    object->aframe = json_value->asInt();
  if (GetJsonObjectMember(json_value, json_object, "fX", Json::intValue)) 
    object->fX = json_value->asInt();
  if (GetJsonObjectMember(json_value, json_object, "fY", Json::intValue)) 
    object->fY = json_value->asInt();
  // if (GetJsonObjectMember(json_value, json_object, "otf", Json::intValue)) 
  //   object->otf = json_value->asInt();
  object->idx_index = idx_index;
  
  // Fix id
  if (object->id) {
    if (strchr(object->id, '/') >= 0) {
      char *copy = strdup(object->id);
      free(object->id);
      object->id = (char *) malloc(2*strlen(copy));
      char *idp = object->id;
      char *copyp = copy;
      while (*copyp) {
        if (*copyp == '/') {
          *idp++ = '_';
          *idp++ = '_';
        }
        else {
          *idp++ = *copyp;
        }
        copyp++;
      }
      *idp = '\0';
      free(copy);
    }
  }

  // Parse materials
  Json::Value *json_materials, *json_material;
  if (GetJsonObjectMember(json_materials, json_object, "materials", Json::arrayValue)) {
    for (Json::ArrayIndex index = 0; index < json_materials->size(); index++) {
      if (!GetJsonArrayEntry(json_material, json_materials, index)) continue;
      if (json_material->type() != Json::objectValue) continue;
      P5DMaterial *material = new P5DMaterial();
      if (GetJsonObjectMember(json_value, json_material, "name", Json::stringValue)) 
        material->name = strdup(json_value->asString().c_str());
      if (GetJsonObjectMember(json_value, json_material, "texture", Json::stringValue)) 
        material->texture_name = strdup(json_value->asString().c_str());
      if (GetJsonObjectMember(json_value, json_material, "color")) 
        material->color = strdup(json_value->asString().c_str());
      if (GetJsonObjectMember(json_value, json_material, "tcolor")) 
        material->tcolor = strdup(json_value->asString().c_str());
      object->materials.push_back(material);
    }
  }

  // Return success
  return 1;
}



static int 
ParsePrimitive(P5DPrimitive *primitive, Json::Value *json_primitive, int idx_index)
{
  // Parse attributes
  Json::Value *json_value;
  if (GetJsonObjectMember(json_value, json_primitive, "className", Json::stringValue)) 
    primitive->className = strdup(json_value->asString().c_str());
  if (GetJsonObjectMember(json_value, json_primitive, "t", Json::stringValue)) 
    primitive->t = strdup(json_value->asString().c_str());
  if (GetJsonObjectMember(json_value, json_primitive, "puid", Json::stringValue)) 
    primitive->puid = strdup(json_value->asString().c_str());
  if (GetJsonObjectMember(json_value, json_primitive, "tm")) 
    primitive->tm = 0.01 * json_value->asDouble();
  if (GetJsonObjectMember(json_value, json_primitive, "x")) 
    primitive->x = 0.01 * json_value->asDouble();
  if (GetJsonObjectMember(json_value, json_primitive, "y")) 
    primitive->y = 0.01 * json_value->asDouble();
  if (GetJsonObjectMember(json_value, json_primitive, "z")) 
    primitive->z = 0.01 * json_value->asDouble();
  if (GetJsonObjectMember(json_value, json_primitive, "sX")) 
    primitive->sX = 0.01 * json_value->asDouble();
  if (GetJsonObjectMember(json_value, json_primitive, "sY")) 
    primitive->sY = 0.01 * json_value->asDouble();
  if (GetJsonObjectMember(json_value, json_primitive, "sZ")) 
    primitive->sZ = 0.01 * json_value->asDouble();
  if (GetJsonObjectMember(json_value, json_primitive, "aX")) 
    primitive->aX = 3.14159265358979323846 * json_value->asDouble() / 180.0;
  if (GetJsonObjectMember(json_value, json_primitive, "aY")) 
    primitive->aY = 3.14159265358979323846 * json_value->asDouble() / 180.0;
  if (GetJsonObjectMember(json_value, json_primitive, "aZ")) 
    primitive->aZ = 3.14159265358979323846 * json_value->asDouble() / 180.0;
  primitive->idx_index = idx_index;
  
  // Parse materials
  Json::Value *json_materials, *json_material;
  if (GetJsonObjectMember(json_materials, json_primitive, "materials", Json::arrayValue)) {
    for (Json::ArrayIndex index = 0; index < json_materials->size(); index++) {
      if (!GetJsonArrayEntry(json_material, json_materials, index)) continue;
      if (json_material->type() != Json::objectValue) continue;
      P5DMaterial *material = new P5DMaterial();
      if (GetJsonObjectMember(json_value, json_material, "name", Json::stringValue)) 
        material->name = strdup(json_value->asString().c_str());
      if (GetJsonObjectMember(json_value, json_material, "texture", Json::stringValue)) 
        material->texture_name = strdup(json_value->asString().c_str());
      if (GetJsonObjectMember(json_value, json_material, "color")) 
        material->color = strdup(json_value->asString().c_str());
      if (GetJsonObjectMember(json_value, json_material, "tcolor")) 
        material->tcolor = strdup(json_value->asString().c_str());
      primitive->materials.push_back(material);
    }
  }

  // Return success
  return 1;
}



static int 
ParseWall(P5DWall *wall, Json::Value *json_wall, int idx_index)
{
  // Parse attributes
  Json::Value *json_value;
  if (GetJsonObjectMember(json_value, json_wall, "w"))
    wall->w = 0.01 * json_value->asDouble();
  if (GetJsonObjectMember(json_value, json_wall, "hidden"))
    wall->hidden = json_value->asBool();
  wall->idx_index = idx_index;

  // Check/fix stuff
  if (wall->w <= 0) wall->w = 0.1;

  // Parse points
  Json::Value *json_items, *json_item, *json_className, *json_x, *json_y;
  if (!GetJsonObjectMember(json_items, json_wall, "items", Json::arrayValue)) return 0;
  for (Json::ArrayIndex index = 0; index < json_items->size(); index++) {
    if (!GetJsonArrayEntry(json_item, json_items, index)) continue; // return 0;
    if (json_item->type() != Json::objectValue) continue;
    if (!GetJsonObjectMember(json_className, json_item, "className", Json::stringValue)) continue; // return 0;
    if (json_className->asString().compare(std::string("Point"))) continue;
    if (!GetJsonObjectMember(json_x, json_item, "x")) continue; // return 0;
    if (!GetJsonObjectMember(json_y, json_item, "y")) continue; // return 0;
    if (index == 0) { wall->x1 = 0.01 * json_x->asDouble(); wall->y1 = 0.01 * json_y->asDouble(); }
    else if (index == 1) { wall->x2 = 0.01 * json_x->asDouble(); wall->y2 = 0.01 * json_y->asDouble(); }
  }

  // Return success
  return 1;
}



static int 
ParseRoom(P5DRoom *room, Json::Value *json_room, int idx_index)
{
  // Parse attributes
  Json::Value *json_value;
  if (GetJsonObjectMember(json_value, json_room, "className", Json::stringValue)) 
    room->className = strdup(json_value->asString().c_str());
  if (GetJsonObjectMember(json_value, json_room, "h"))
    room->h = 0.01 * json_value->asDouble();
  if (GetJsonObjectMember(json_value, json_room, "x"))
    room->x = 0.01 * json_value->asDouble();
  if (GetJsonObjectMember(json_value, json_room, "y"))
    room->y = 0.01 * json_value->asDouble();
  if (GetJsonObjectMember(json_value, json_room, "sX"))
    room->sX = 0.01 * json_value->asDouble();
  if (GetJsonObjectMember(json_value, json_room, "sY"))
    room->sY = 0.01 * json_value->asDouble();
  if (GetJsonObjectMember(json_value, json_room, "rtype")) 
    room->rtype = strdup(json_value->asString().c_str());
  if (GetJsonObjectMember(json_value, json_room, "texture", Json::stringValue)) 
    room->texture = strdup(json_value->asString().c_str());
  if (GetJsonObjectMember(json_value, json_room, "otexture", Json::stringValue)) 
    room->otexture = strdup(json_value->asString().c_str());
  if (GetJsonObjectMember(json_value, json_room, "rtexture", Json::stringValue)) 
    room->rtexture = strdup(json_value->asString().c_str());
  if (GetJsonObjectMember(json_value, json_room, "wtexture", Json::stringValue))
    room->wtexture = strdup(json_value->asString().c_str());
  room->idx_index = idx_index;
  
  // Check/fix stuff
  if (room->h <= 0) room->h = 2.7;

  // Parse walls
  Json::Value *json_items, *json_item, *json_className;
  if (!GetJsonObjectMember(json_items, json_room, "items", Json::arrayValue)) return 0;
  for (Json::ArrayIndex index = 0; index < json_items->size(); index++) {
    if (!GetJsonArrayEntry(json_item, json_items, index)) continue; // return 0;
    if (json_item->type() != Json::objectValue) continue;
    if (!GetJsonObjectMember(json_className, json_item, "className", Json::stringValue)) continue; // return 0;
    if (!json_className->asString().compare(std::string("Wall"))) {
      P5DWall *wall = new P5DWall(room);
      if (!ParseWall(wall, json_item, index)) continue; // return 0;
    }
  }

  // Return success
  return 1;
}



static int 
ParseFloor(P5DFloor *floor, Json::Value *json_floor, int idx_index)
{
  // Parse attributes
  Json::Value *json_value;
  if (GetJsonObjectMember(json_value, json_floor, "h")) 
    floor->h = 0.01 * json_value->asDouble();
  floor->idx_index = idx_index;
  
  // Parse rooms
  std::vector<P5DRoom *> tmp_rooms;
  std::vector<P5DObject *> tmp_objects;
  std::vector<P5DPrimitive *> tmp_primitives;
  Json::Value *json_items, *json_item, *json_className;
  if (!GetJsonObjectMember(json_items, json_floor, "items", Json::arrayValue)) return 0;
  for (Json::ArrayIndex index = 0; index < json_items->size(); index++) {
    tmp_objects.push_back(NULL); tmp_primitives.push_back(NULL);
    if (!GetJsonArrayEntry(json_item, json_items, index)) continue; 
    if (json_item->type() != Json::objectValue) continue;
    if (!GetJsonObjectMember(json_className, json_item, "className", Json::stringValue)) continue; 
    if (!json_className->asString().compare(std::string("Ground"))) {
      P5DRoom *room = new P5DRoom(floor);
      if (ParseRoom(room, json_item, index)) tmp_rooms.push_back(room);
    }
    else if (!json_className->asString().compare(std::string("Room"))) {
      P5DRoom *room = new P5DRoom(floor);
      if (ParseRoom(room, json_item, index)) tmp_rooms.push_back(room);
      if (room->h > floor->h) floor->h = room->h;
    }
    else if (!json_className->asString().compare(std::string("Door"))) {
      P5DObject *object = new P5DObject(floor);
      if (ParseObject(object, json_item, index)) tmp_objects[index] = object;
    }
    else if (!json_className->asString().compare(std::string("Window"))) {
      P5DObject *object = new P5DObject(floor);
      if (ParseObject(object, json_item, index)) tmp_objects[index] = object;
    }
    else if (!json_className->asString().compare(std::string("Ns"))) {
      P5DObject *object = new P5DObject(floor);
      if (ParseObject(object, json_item, index)) tmp_objects[index] = object;
    }
    else if (!json_className->asString().compare(std::string("Pr"))) {
      P5DPrimitive *primitive = new P5DPrimitive(floor);
      if (ParsePrimitive(primitive, json_item, index)) tmp_primitives[index] = primitive;
    }
  }

  // Check/fix stuff
  if (floor->h <= 0) floor->h = 2.7;

  // Parse which objects are in which rooms
  Json::Value *array_items = NULL, *array_item = NULL;
  if (GetJsonObjectMember(json_items, json_floor, "roomObjectMapping", Json::arrayValue)) {
    for (Json::ArrayIndex room_index = 0; room_index < json_items->size(); room_index++) {
      if (room_index >= tmp_rooms.size()) break;
      GetJsonArrayEntry(array_items, json_items, room_index, Json::arrayValue);
      if (array_items->isArray()) {
        for (Json::ArrayIndex array_index = 0; array_index < array_items->size(); array_index++) {
          GetJsonArrayEntry(array_item, array_items, array_index);
          if (array_item->isNumeric()) { // XXX ASK SHURAN XXX
            int object_index = array_item->asInt() - 1;
            if ((object_index >= 0) && ((unsigned int) object_index < tmp_objects.size())) {
              P5DRoom *room = tmp_rooms[room_index];
              P5DObject *object = tmp_objects[object_index];
              if (object && !object->room) {
                object->room = room;
                object->room_index = room->objects.size();
                room->objects.push_back(object);
              }
            }
          }
        }
      }
      else{ // single object in the room 
        int object_index = array_items->asInt() - 1;
        if ((object_index >= 0) && ((unsigned int) object_index < tmp_objects.size())) {
          P5DRoom *room = tmp_rooms[room_index];
          P5DObject *object = tmp_objects[object_index];
          if (object && !object->room) {
            object->room = room;
            object->room_index = room->objects.size();
            room->objects.push_back(object);
          }
        }
      }
    }
  }
  
  // Return success
  return 1;
}



static int
ParseProject(P5DProject *project, Json::Value *json_project)
{
  // Parse 
  Json::Value *json_items, *json_item, *json_className;
  if (!GetJsonObjectMember(json_items, json_project, "items", Json::arrayValue)) return 0;
  for (Json::ArrayIndex index = 0; index < json_items->size(); index++) {
    if (!GetJsonArrayEntry(json_item, json_items, index)) continue; // return 0;
    if (json_item->type() != Json::objectValue) continue;
    if (!GetJsonObjectMember(json_className, json_item, "className", Json::stringValue)) continue; // return 0;
    if (json_className->asString().compare(std::string("Floor"))) continue;
    P5DFloor *floor = new P5DFloor(project);
    if (!ParseFloor(floor, json_item, index)) continue; // return 0;
  }

  // Return success
  return 1;
}



int P5DProject::
ReadFile(const char *filename)
{
  // Open file
  FILE* fp = fopen(filename, "rb");
  if (!fp) {
    fprintf(stderr, "Unable to open Planner5D file %s\n", filename);
    return 0;
  }

  // Read file 
  std::string text;
  fseek(fp, 0, SEEK_END);
  long const size = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  char* buffer = new char[size + 1];
  unsigned long const usize = static_cast<unsigned long const>(size);
  if (fread(buffer, 1, usize, fp) != usize) { fprintf(stderr, "Unable to read %s\n", filename); return 0; }
  else { buffer[size] = 0; text = buffer; }
  delete[] buffer;

  // Close file
  fclose(fp);

  // Parse file
  Json::Value json_root;
  Json::Reader json_reader;
  if (!json_reader.parse(text, json_root, false)) {
    fprintf(stderr, "Unable to parse %s\n", filename);
    return 0;
  }

  // Interpret file
  Json::Value *json_items, *json_item, *json_data, *json_className;
  if (!GetJsonObjectMember(json_items, &json_root, "items", Json::arrayValue)) return 0;
  for (Json::ArrayIndex index = 0; index < json_items->size(); index++) {
    if (!GetJsonArrayEntry(json_item, json_items, index)) return 0;
    if (json_item->type() != Json::objectValue) continue;
    if (!GetJsonObjectMember(json_data, json_item, "data", Json::objectValue)) continue;
    if (!GetJsonObjectMember(json_className, json_data, "className", Json::stringValue)) return 0;
    if (json_className->asString().compare(std::string("Project"))) continue;

    // Parse project
    if (!ParseProject(this, json_data)) {
      fprintf(stderr, "Unable to read planner5d project %s\n", filename);
      return 0;
    }

    // Return success after reading one project
    return 1;
  }

  // No project in file
  fprintf(stderr, "Planner5d project %s has no projects\n", filename);
  return 0;
}



