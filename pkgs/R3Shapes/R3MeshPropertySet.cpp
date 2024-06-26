// Source file for the GAPS mesh property set



////////////////////////////////////////////////////////////////////////
// Include files 
////////////////////////////////////////////////////////////////////////

#include "R3Shapes.h"



// Namespace

namespace gaps {



////////////////////////////////////////////////////////////////////////
// Constructor/destructor
////////////////////////////////////////////////////////////////////////

R3MeshPropertySet::
R3MeshPropertySet(R3Mesh *mesh)
  : mesh(mesh),
    properties()
{
}



R3MeshPropertySet::
R3MeshPropertySet(const R3MeshPropertySet& set)
  : mesh(set.mesh),
    properties()
{
  // Copy properties
  for (int i = 0; i < set.NProperties(); i++) {
    R3MeshProperty *property = set.Property(i);
    Insert(new R3MeshProperty(*property));
  }
}



R3MeshPropertySet::
~R3MeshPropertySet(void)
{
  // Empty all properties
  Empty();
}



////////////////////////////////////////////////////////////////////////
// Access functions
////////////////////////////////////////////////////////////////////////

int R3MeshPropertySet::
PropertyIndex(R3MeshProperty *query_property) const
{
  // Search through properties for matching name
  for (int i = 0; i < NProperties(); i++) {
    R3MeshProperty *property = Property(i);
    if (property == query_property) return i;
  }

  // Not found
  return -1;
}


int R3MeshPropertySet::
PropertyIndex(const char *property_name) const
{
  // Search through properties for matching name
  for (int i = 0; i < NProperties(); i++) {
    R3MeshProperty *property = Property(i);
    if (!strcmp(property->Name(), property_name)) return i;
  }

  // Not found
  return -1;
}



////////////////////////////////////////////////////////////////////////
// Manipulation functions
////////////////////////////////////////////////////////////////////////

void R3MeshPropertySet::
Empty(void)
{
  // Empty array of properties
  properties.Empty();
}



void R3MeshPropertySet::
SortByName(void)
{
  // Sort properties
  for (int i = 0; i < properties.NEntries(); i++) {
    for (int j = i+1; j < properties.NEntries(); j++) {
      if (strcmp(properties[i]->Name(), properties[j]->Name()) > 0) {
        R3MeshProperty *swap = properties[i];
        properties.EntryContents(properties.KthEntry(i)) = properties[j];
        properties.EntryContents(properties.KthEntry(j)) = swap;
      }
    }
  }
}



void R3MeshPropertySet::
Insert(R3MeshProperty *property) 
{
  // Insert reference to property 
  properties.Insert(property);
}



void R3MeshPropertySet::
Remove(R3MeshProperty *property) 
{
  // Remove reference to property 
  properties.Remove(property);
}



void R3MeshPropertySet::
Remove(int k) 
{
  // Remove kth property
  properties.RemoveKth(k);
}



void R3MeshPropertySet::
Reset(R3Mesh *mesh)
{
  // Empty array of properties
  properties.Empty();

  // Set mesh
  this->mesh = mesh;
}


R3MeshPropertySet& R3MeshPropertySet::
operator=(const R3MeshPropertySet& set)
{
  // Delete everything
  Empty();

  // Copy mesh
  mesh = set.mesh;
  
  // Copy properties
  for (int i = 0; i < set.NProperties(); i++) {
    R3MeshProperty *property = set.Property(i);
    Insert(new R3MeshProperty(*property));
  }

  // Return this
  return *this;
}


 
////////////////////////////////////////////////////////////////////////
// Input/output functions
////////////////////////////////////////////////////////////////////////

int R3MeshPropertySet::
Read(const char *filename)
{
  // Check mesh
  if (!mesh) {
    RNFail("Property set must be associated with mesh before file can be read: %s\n", filename);
    return 0;
  }
  
  // Parse input filename extension
  const char *extension;
  if (!(extension = strrchr(filename, '.'))) {
    printf("Filename %s has no extension (e.g., .arff)\n", filename);
    return 0;
  }

  // Read file of appropriate type
  if (!strcmp(extension, ".arff")) return ReadARFF(filename);
  else if (!strcmp(extension, ".npy")) return ReadNumpy(filename);
  else if (!strcmp(extension, ".prp")) return ReadBinary(filename);
  else if (!strcmp(extension, ".trt")) return ReadToronto(filename);
  else return ReadProperty(filename);
}



int R3MeshPropertySet::
ReadARFF(const char *filename)
{
  // Open file
  FILE *fp = fopen(filename, "r");
  if (!fp) {
    RNFail("Unable to open ARFF file: %s\n", filename);
    return 0;
  }

  // Allocate large buffer for reading ARFF lines
  const int max_line_size = 64 * 1024;
  char *buffer = new char [ max_line_size ];
  if (!buffer) {
    RNFail("Unable to allocate buffer for reading ARFF file\n");
    return 0;
  }
  
  // Read attribute names and allocate property for each one
  int property_count = 0;
  char token[1024], name[1024];
  while (fgets(buffer, max_line_size, fp)) {
    if (strstr(buffer, "@data")) break;
    else if (strstr(buffer, "@attribute")) {
      if (sscanf(buffer, "%s%s", token, name) == (unsigned int) 2) {
        if (!strcmp(token, "@attribute")) {
          R3MeshProperty *property = new R3MeshProperty(mesh, name);
          Insert(property);
          property_count++;
        }
      }
    }
  }

  // Check number of properties
  if (property_count == 0) { delete [] buffer; return 1; }

  // Read data and assign property values
  int vertex_index = 0;
  RNScalar property_value = 0;
  while (fgets(buffer, max_line_size, fp)) {
    // Check for header line
    if (buffer[0] == '#') continue;
    if (buffer[0] == '@') continue;
    if (buffer[0] == '\0') continue;
    if (buffer[0] == '\n') continue;
    char *bufferp = strtok(buffer, "\n\t ");
    for (int i = 0; i < property_count; i++) {
      R3MeshProperty *property = Property(NProperties()-property_count+i);
      if (!bufferp) { 
        RNFail("Unable to read property value %d for vertex %d\n", i, vertex_index);
        delete [] buffer;
        return 0;
      }
      if (sscanf(bufferp, "%lf", &property_value) != (unsigned int) 1) {
        RNFail("Unable to read property value %d for vertex %d\n", i, vertex_index);
        delete [] buffer;
        return 0;
      }
      if (vertex_index >= mesh->NVertices()) {
        RNFail("Too many data lines in %s\n", filename);
        delete [] buffer;
        return 0;
      }
      property->SetVertexValue(vertex_index, property_value);
      bufferp = strtok(NULL, "\n\t ");
    }
    vertex_index++;
  }

  // Check if read value for every vertex
  // if (vertex_index != mesh->NVertices()) {
  //   RNFail("Mismatching number of data lines (%d) and vertices (%d) in %s\n", vertex_index, mesh->NVertices(), filename);
  //   delete [] buffer;
  //   return 0;
  // }

  // Close file
  fclose(fp);

  // Delete buffer
  delete [] buffer;
  
  // Return success
  return 1;
}



static int
NumpyDataTypeSize(int data_type, int data_size)
{
  switch (data_type) {
  case 'U': return 4 * data_size;
  default: return data_size;
  }
}



int R3MeshPropertySet::
ReadNumpy(const char *filename)
{
  // Open file
  FILE *fp = fopen(filename, "rb");
  if (!fp) {
    fprintf(stderr, "Unable to open npy file %s\n", filename);
    return 0;
  }
  
  // Read magic string
  unsigned char magic[6];
  if (fread(magic, sizeof(unsigned char), 6, fp) != (unsigned int) 6) {
    fprintf(stderr, "Unable to read first character from npy file\n");
    fclose(fp);
    return 0;
  }

  // Check magic string
  if ((magic[0] != 0x93) || (magic[1] != 'N') || (magic[2] != 'U') ||
      (magic[3] != 'M')  || (magic[4] != 'P') || (magic[5] != 'Y')) {
    fprintf(stderr, "Unrecognized format in npy file\n");
    fclose(fp);
    return 0;
  }

  // Read version info
  unsigned char version[2];
  if (fread(version, sizeof(unsigned char), 2, fp) != (unsigned int) 2) {
    fprintf(stderr, "Unable to read version in npy file\n");
    fclose(fp);
    return 0;
  }
  
  // Read header length
  unsigned short int header_length;
  if (fread(&header_length, sizeof(unsigned short), 1, fp) != (unsigned int) 1) {
    fprintf(stderr, "Unable to read header length in npy file\n");
    fclose(fp);
    return 0;
  }

  // Check header length
  if (header_length <= 0) {
    fprintf(stderr, "Invalid header length in npy file\n");
    fclose(fp);
    return 0;
  }

  // Read header
  char *header = new char [ header_length + 1 ];
  if (fread(header, sizeof(char), header_length, fp) != (unsigned int) header_length) {
    fprintf(stderr, "Unable to read header in npy file\n");
    delete [] header;
    fclose(fp);
    return 0;
  }
  header[header_length] = '\0';

  // Extract data type and size
  int data_type = 0;
  int data_size = 0;
  char *start = strstr(header, "'descr'");
  if (start) {
    start = strchr(start, '<');
    if (start) {
      start++;
      while (*start == ' ') start++;
      data_type = *start;
      start++;
      char *end = strchr(start, '\'');
      if (end && (start < end)) {
        *end = '\0';
        data_size = atoi(start);
        *end = '\'';
      }
    }
  }

  // Extract fortrain order
  int fortran_order = 0;
  start = strstr(header, "'fortran_order'");
  if (start) {
    start = strchr(start, ':');
    if (start) {
      start++;
      while (*start == ' ') start++;
      char *end = strchr(start, ',');
      if (end && (start < end)) {
        *end = '\0';
        if (!strcmp(start, "True")) fortran_order = 1;
        if (!strcmp(start, "true")) fortran_order = 1;
        *end = ',';
      }
    }
  }

  // Extract width
  int width = 1;
  start = strstr(header, "'shape'");
  if (start) {
    start = strchr(start, '(');
    if (start) {
      start++;
      while (*start == ' ') start++;
      char *end = strchr(start, ',');
      if (end && (start < end)) {
        *end = '\0';
        width = atoi(start);
        *end = ',';
      }
    }
  }

  // Extract height
  int height = 1;
  start = strstr(header, "'shape'");
  if (start) {
    start = strchr(start, ',');
    if (start) {
      start++;
      while (*start == ' ') start++;
      char *end1 = strchr(start, ',');
      char *end2 = strchr(start, ')');
      char *end = (end1 < end2) ? end1 : end2;
      if (end && (start < end)) {
        *end = '\0';
        height = atoi(start);
        *end = ')';
      }
    }
  }

  // Delete header
  delete [] header;

  // Read values
  unsigned int nbytes = width * height * NumpyDataTypeSize(data_type, data_size);
  unsigned char *array = new unsigned char [ nbytes ];
  unsigned int read_nbytes = 0;
  while (read_nbytes < nbytes) {
    size_t k = fread(&array[read_nbytes], sizeof(unsigned char), nbytes - read_nbytes, fp);
    if (k <= 0) {
      fprintf(stderr, "Unable to read array in npy file (%lu %u %u)\n", k, read_nbytes, nbytes);
      if (k < 0) {
        delete [] array;
        fclose(fp);
        return 0;
      }
    }
    read_nbytes += k;
  }

  // Close file
  fclose(fp);

  // Create properties
  for (int j = 0; j < height; j++) {
    char property_name[128];
    sprintf(property_name, "P%d", j);
    Insert(new R3MeshProperty(mesh, property_name));
  }

  // Fill property values
  unsigned char *arrayp = array;
  if (fortran_order) {
    // Fortran order
    for (int j = 0; j < height; j++) {
      for (int i = 0; i < width; i++) {
        if (data_size == 4) {
          float *a = (float *) arrayp;
          Property(j)->SetVertexValue(i, (float) *a);
          arrayp += 4;
        }
        else if (data_size == 8) {
          double *a = (double *) arrayp;
          Property(j)->SetVertexValue(i, (float) *a);
          arrayp += 8;
        }
      }
    }
  }
  else {
    // C order
    for (int i = 0; i < width; i++) {
      for (int j = 0; j < height; j++) {
        if (data_size == 4) {
          float *a = (float *) arrayp;
          Property(j)->SetVertexValue(i, (float) *a);
          arrayp += 4;
        }
        else if (data_size == 8) {
          double *a = (double *) arrayp;
          Property(j)->SetVertexValue(i, (float) *a);
          arrayp += 8;
        }
      }
    }
  }

  // Delete array
  delete [] array;
  
  // Return success
  return 1;
}



int R3MeshPropertySet::
ReadBinary(const char *filename)
{
  // Just checking
  if (!mesh) return 0;
  
  // Open file
  FILE *fp = fopen(filename, "rb");
  if (!fp) {
    RNFail("Unable to open binary file: %s\n", filename);
    return 0;
  }

  // Read number of vertices
  int vertex_count = 0;
  if (fread(&vertex_count, sizeof(int), 1, fp) != (unsigned int) 1) {
    RNFail("Unable to read binary file: %s\n", filename);
    return 0;
  }

  // Read number of properties
  int property_count = 0;
  if (fread(&property_count, sizeof(int), 1, fp) != (unsigned int) 1) {
    RNFail("Unable to read binary file: %s\n", filename);
    return 0;
  }
  
  // Check number of vertices
  if (vertex_count != mesh->NVertices()) {
    RNFail("Mismatching number of vertices in %s\n", filename);
    return 0;
  }
  
  // Check number of properties
  if (property_count == 0) return 1;


  // Read property names and create properties
  char property_name[128];
  for (int j = 0; j < property_count; j++) {
    fread(property_name, sizeof(char), 128, fp);
    R3MeshProperty *property = new R3MeshProperty(mesh, property_name);
    Insert(property);
  }

  // Read data and assign property values
  RNScalar32 value;
  for (int i = 0; i < vertex_count; i++) {
    for (int j = 0; j < property_count; j++) {
      fread(&value, sizeof(RNScalar32), 1, fp);
      Property(j)->SetVertexValue(i, value);
    }
  }

  // Close file
  fclose(fp);

  // Return success
  return 1;
}



int R3MeshPropertySet::
ReadToronto(const char *filename)
{
  // Get base property name
  char *name = NULL;
  char buffer[512];
  strcpy(buffer, filename);
  name = strrchr(buffer, '/');
  if (name) name++; 
  else name = buffer;
  char *end = strrchr(name, '.');
  if (end) *end = '\0';

  // Open file
  FILE *fp = fopen(filename, "r");
  if (!fp) {
    RNFail("Unable to open toronto file: %s\n", filename);
    return 0;
  }

  // Read header
  int nprops, nfaces, dummy;
  if (fscanf(fp, "%d%d%d\n", &nprops, &nfaces, &dummy) != (unsigned int) 3) {
    RNFail("Unable to read toronto file: %s\n", filename);
    return 0;
  }

  // Check header
  if ((nprops == 0) || (nfaces == 0) || (nfaces != mesh->NFaces())) {
    RNFail("Invalid header in toronto file: %s\n", filename);
    return 0;
  }

  // Allocate memory for face values
  double *face_values = new double [ nprops * nfaces ];
  if (!face_values) {
    RNFail("Unable to allocate memory for toronto file: %s\n", filename);
    return 0;
  }

  // Read face values
  for (int j = 0; j < nfaces; j++) {
    for (int i = 0; i < nprops; i++) {
      if (fscanf(fp, "%lf", &face_values[i*nfaces+j]) != (unsigned int) 1) {
        RNFail("Unable to read value %d for face %d in %s\n", i, j, filename);
        return 0;
      }
    }
  }

  // Create properties
  for (int i = 0; i < nprops; i++) {
    // Allocate property
    char property_name[1024];
    sprintf(property_name, "%s%d", name, i);
    R3MeshProperty *property = new R3MeshProperty(mesh, property_name);

    // Compute property values by interpolation from attached faces
    for (int j = 0; j < mesh->NVertices(); j++) {
      R3MeshVertex *vertex = mesh->Vertex(j);
      RNScalar total_value = 0;
      RNScalar total_weight = 0;
      for (int k = 0; k < mesh->VertexValence(vertex); k++) {
        R3MeshEdge *edge = mesh->EdgeOnVertex(vertex, k);
        R3MeshFace *face = mesh->FaceOnVertex(vertex, edge);
        if (!face) continue;
        int face_index = mesh->FaceID(face);
        RNScalar value = face_values[i*nfaces + face_index];
        RNScalar weight = mesh->FaceArea(face);
        total_value += weight * value;
        total_weight += weight;
      }
      if (total_weight > 0) {
        property->SetVertexValue(j, total_value / total_weight);
      }
    }

    // Insert property
    Insert(property);
  }

  // Close file
  fclose(fp);

  // Delete memory for face values
  delete [] face_values;

  // Return success
  return 1;
}



int R3MeshPropertySet::
ReadProperty(const char *filename)
{
  // Create property name
  char name[1024];
  strcpy(name, filename);
  char *start = strrchr(name, '/');
  if (start) start++; 
  else start = name;
  char *end = strrchr(start, '.');
  if (end) *end = '\0';

  // Create mesh property
  R3MeshProperty *property = new R3MeshProperty(mesh, start);
  if (!property) {
    RNFail("Unable to create mesh property for %s\n", filename);
    return 0;
  }

  // Read data 
  if (!property->Read(filename)) {
    RNFail("Unable to read values from %s\n", filename);
    return 0;
  }

  // Add property to set
  Insert(property);

  // Return success
  return 1;
}



int R3MeshPropertySet::
Write(const char *filename) const
{
  // Check mesh
  if (!mesh) {
    RNFail("Property set must be associated with mesh before file can be written: %s\n", filename);
    return 0;
  }
  
  // Parse input filename extension
  const char *extension;
  if (!(extension = strrchr(filename, '.'))) {
    printf("Filename %s has no extension (e.g., .arff)\n", filename);
    return 0;
  }

  // Write file of appropriate type
  if (!strcmp(extension, ".arff")) return WriteARFF(filename);
  else if (!strcmp(extension, ".npy")) return WriteNumpy(filename);
  else if (!strcmp(extension, ".txt")) return WriteValues(filename);
  else if (!strcmp(extension, ".prp")) return WriteBinary(filename);
  else if (!strcmp(extension, ".val")) return WriteValues(filename);
  else if (!strcmp(extension, ".dat")) return WriteValues(filename);

  // None of the extensions matched
  RNFail("Unable to write file %s (unrecognized extension: %s)\n", filename, extension);
  return 0;
}



int R3MeshPropertySet::
WriteARFF(const char *filename) const
{
  // Open file
  FILE *fp = fopen(filename, "w");
  if (!fp) {
    RNFail("Unable to open ARFF file: %s\n", filename);
    return 0;
  }

  // Write header
  fprintf(fp, "@relation %s\n\n", mesh->Name());

  // Write names
  for (int i = 0; i < NProperties(); i++) {
    R3MeshProperty *property = Property(i);
    fprintf(fp, "@attribute %s real\n", property->Name());
  }

  // Write values
  fprintf(fp, "\n@data\n\n");
  for (int i = 0; i < mesh->NVertices(); i++) {
    for (int j = 0; j < NProperties(); j++) {
      fprintf(fp, "%g ", properties[j]->VertexValue(i));
    }
    fprintf(fp, "\n");
  }

  // Close file
  fclose(fp);

  // Return success
  return 1;
}



int R3MeshPropertySet::
WriteNumpy(const char *filename) const
{
  // Open file
  FILE *fp = fopen(filename, "wb");
  if (!fp) {
    RNFail("Unable to open npy file %s\n", filename);
    return 0;
  }

  // Write magic string
  unsigned char magic[] = { 0x93, 'N', 'U', 'M', 'P', 'Y' };
  unsigned int magic_length = sizeof(magic) / sizeof(unsigned char);
  if (fwrite(magic, 1, magic_length, fp) != magic_length) {
    RNFail("Unable to write first line to numpy file");
    return 0;
  }

  // Write version
  unsigned char version[] = { 0x01, 0x00 };
  unsigned int version_length = sizeof(version) / sizeof(unsigned char);
  if (fwrite(version, 1, version_length, fp) != version_length) {
    RNFail("Unable to write version to numpy file");
    return 0;
  }

  // Create header
  char header[1024];
  sprintf(header, "{'descr': '<f4', 'fortran_order': False, 'shape': (%d, %d), }", mesh->NVertices(), NProperties());
  unsigned int header_length = strlen(header);
 
  // Write header length
  if (fwrite(&header_length, sizeof(unsigned short), 1, fp) != (unsigned int) 1) {
    RNFail("Unable to write header length to numpy file");
    return 0;
  }

  // Write header
  if (fwrite(header, sizeof(char), header_length, fp) != header_length) {
    RNFail("Unable to write header to numpy file");
    return 0;
  }

  // Write values
  for (int i = 0; i < mesh->NVertices(); i++) {
    for (int j = 0; j < NProperties(); j++) {
      RNScalar32 value = Property(j)->VertexValue(i);
      if (fwrite(&value, sizeof(RNScalar32), 1, fp) != (unsigned int) 1) {
        RNFail("Unable to write value to binary file: %s\n", filename);
        return 0;
      }
    }
  }

  // Close file
  fclose(fp);

  // Return success
  return 1;
}



int R3MeshPropertySet::
WriteBinary(const char *filename) const
{
  // Just checking
  if (!mesh) return 0;
  
  // Open file
  FILE *fp = fopen(filename, "wb");
  if (!fp) {
    RNFail("Unable to open binary file: %s\n", filename);
    return 0;
  }

  // Write number of vertices
  int vertex_count = mesh->NVertices();
  if (fwrite(&vertex_count, sizeof(int), 1, fp) != (unsigned int) 1) {
    RNFail("Unable to write binary file: %s\n", filename);
    return 0;
  }
  
  // Write number of properties
  int property_count = NProperties();
  if (fwrite(&property_count, sizeof(int), 1, fp) != (unsigned int) 1) {
    RNFail("Unable to write binary file: %s\n", filename);
    return 0;
  }
  
  // Write property names
  for (int j = 0; j < NProperties(); j++) {
    R3MeshProperty *property = Property(j);
    char property_name[1024];
    memset(property_name, 0, 1024);
    if (property->Name()) strncpy(property_name, property->Name(), 1024);
    property_name[127] = '\0';
    if (fwrite(property_name, sizeof(char), 128, fp) != (unsigned int) 128) {
      RNFail("Unable to write property name to binary file: %s\n", filename);
      return 0;
    }    
  }

  // Write data and assign property values
  for (int i = 0; i < mesh->NVertices(); i++) {
    for (int j = 0; j < NProperties(); j++) {
      RNScalar32 value = Property(j)->VertexValue(i);
      if (fwrite(&value, sizeof(RNScalar32), 1, fp) != (unsigned int) 1) {
        RNFail("Unable to write value to binary file: %s\n", filename);
        return 0;
      }
    }
  }

  // Close file
  fclose(fp);

  // Return success
  return 1;
}



int R3MeshPropertySet::
WriteValues(const char *filename) const
{
  // Check number of properties
  for (int i = 0; i < NProperties(); i++) {
    R3MeshProperty *property = Property(i);

    // Determine file name
    char name[1024];
    strncpy(name, filename, 1023);
    if (NProperties() > 1) {
      char basename[512];
      strncpy(basename, filename, 511);
      char *extension = strrchr(basename, '.');
      if (extension) { *extension = '\0'; }
      else extension = (char *) ".val";
      sprintf(name, "%s_%d.%s", basename, i, extension);
    }

    // Open file
    FILE *fp = fopen(name, "w");
    if (!fp) {
      RNFail("Unable to open values file: %s\n", name);
      return 0;
    }

    // Write values
    for (int i = 0; i < mesh->NVertices(); i++) {
      fprintf(fp, "%g\n", property->VertexValue(i));
    }

    // Close file
    fclose(fp);
  }

  // Return success
  return 1;
}


} // namespace gaps
