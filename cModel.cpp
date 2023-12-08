//{{{  includes
#ifdef _WIN32
  #define _CRT_SECURE_NO_WARNINGS
#endif

#include <iostream>
#include <fstream>
#include <sstream>
#include <math.h>
#include <vector>
#include <string>
#include <thread>
#include <exception>
#include <limits>

#include <Eigen/Dense>

#include "../common/cLog.h"

#include "cModel.h"

using namespace std;
//}}}

namespace{
  // .obj loader
  constexpr bool kLoaderDebug = false;
  //{{{
  struct sVector2 {
    sVector2() {
      X = 0.0f;
      Y = 0.0f;
      }

    sVector2 (float X_, float Y_) {
      X = X_;
      Y = Y_;
      }

    bool operator == (const sVector2& other) const {
      return (this->X == other.X && this->Y == other.Y);
      }
    bool operator != (const sVector2& other) const {
      return !(this->X == other.X && this->Y == other.Y);
      }

    sVector2 operator + (const sVector2& right) const {
      return sVector2(this->X + right.X, this->Y + right.Y);
      }

    sVector2 operator - (const sVector2& right) const {
      return sVector2(this->X - right.X, this->Y - right.Y);
      }

    sVector2 operator * (const float& other) const {
      return sVector2(this->X *other, this->Y * other);
      }

    float X;
    float Y;
    };
  //}}}
  //{{{
  struct sVector3 {
    sVector3() {
      X = 0.0f;
      Y = 0.0f;
      Z = 0.0f;
      }

    sVector3(float X_, float Y_, float Z_) {
      X = X_;
      Y = Y_;
      Z = Z_;
      }

    bool operator == (const sVector3& other) const {
      return (this->X == other.X && this->Y == other.Y && this->Z == other.Z);
      }
    bool operator != (const sVector3& other) const {
      return !(this->X == other.X && this->Y == other.Y && this->Z == other.Z);
      }

    sVector3 operator + (const sVector3& right) const {
      return sVector3(this->X + right.X, this->Y + right.Y, this->Z + right.Z);
      }
    sVector3 operator - (const sVector3& right) const {
      return sVector3(this->X - right.X, this->Y - right.Y, this->Z - right.Z);
      }
    sVector3 operator * (const float& other) const {
      return sVector3(this->X * other, this->Y * other, this->Z * other);
      }
    sVector3 operator / (const float& other) const {
      return sVector3(this->X / other, this->Y / other, this->Z / other);
      }

    float X;
    float Y;
    float Z;
    };
  //}}}
  //{{{
  struct sVertex {
    sVector3 Position;
    sVector3 Normal;
    sVector2 TextureCoordinate;
    };
  //}}}
  //{{{
  struct sMaterial {
    sMaterial() {
      name;
      Ns = 0.0f;
      Ni = 0.0f;
      d = 0.0f;
      illum = 0;
      }

    // sMaterial Name
    string name;

    // Ambient Color
    sVector3 Ka;

    // Diffuse Color
    sVector3 Kd;

    // Specular Color
    sVector3 Ks;

    // Specular Exponent
    float Ns;

    // Optical Density
    float Ni;

    // Dissolve
    float d;

    // Illumination
    int illum;

    // Ambient Texture Map
    string map_Ka;

    // Diffuse Texture Map
    string map_Kd;

    // Specular Texture Map
    string map_Ks;

    // Specular Hightlight Map
    string map_Ns;

    // Alpha Texture Map
    string map_d;

    // Bump Map
    string map_bump;
    };
  //}}}
  //{{{
  struct sMesh {
    sMesh() { }

    sMesh (vector<sVertex>& _Vertices, vector<unsigned int>& _Indices) {
      Vertices = _Vertices;
      Indices = _Indices;
      }

    // Mesh Name
    string MeshName;

    // sVertex List
    vector<sVertex> Vertices;

    // Index List
    vector<unsigned int> Indices;

    // Material
    sMaterial MeshMaterial;
    };
  //}}}

  // vector utils
  //{{{
  sVector3 crossV3 (const sVector3 a, const sVector3 b) {

    return sVector3 (a.Y * b.Z - a.Z * b.Y,
                    a.Z * b.X - a.X * b.Z,
                    a.X * b.Y - a.Y * b.X);
    }
  //}}}
  //{{{
  float magnitudeV3 (const sVector3 in) {

    return (sqrtf (powf (in.X, 2) + powf (in.Y, 2) + powf (in.Z, 2)));
    }
  //}}}
  //{{{
  float dotV3 (const sVector3 a, const sVector3 b) {
    return (a.X * b.X) + (a.Y * b.Y) + (a.Z * b.Z);
    }
  //}}}
  //{{{
  float angleBetweenV3 (const sVector3 a, const sVector3 b) {

    float angle = dotV3 (a, b);
    angle /= (magnitudeV3 (a) * magnitudeV3 (b));
    return angle = acosf (angle);
    }
  //}}}
  //{{{
  sVector3 projV3 (const sVector3 a, const sVector3 b) {
  // Projection Calculation of a onto b

    sVector3 bn = b / magnitudeV3 (b);
    return bn * dotV3 (a, bn);
    }
  //}}}
  //{{{
  sVector3 operator * (const float& left, const sVector3& right) {

    return sVector3(right.X * left, right.Y * left, right.Z * left);
    }
  //}}}
  //{{{
  bool sameSide (sVector3 p1, sVector3 p2, sVector3 a, sVector3 b) {
  // A test to see if P1 is on the same side as P2 of a line segment ab

    sVector3 cp1 = crossV3 (b - a, p1 - a);
    sVector3 cp2 = crossV3 (b - a, p2 - a);

    if (dotV3(cp1, cp2) >= 0)
      return true;
    else
      return false;
    }
  //}}}
  //{{{
  sVector3 genTriNormal (sVector3 t1, sVector3 t2, sVector3 t3) {
  // Generate a cross produect normal for a triangle

    sVector3 u = t2 - t1;
    sVector3 v = t3 - t1;

    sVector3 normal = crossV3 (u,v);

    return normal;
    }
  //}}}
  //{{{
  bool inTriangle (sVector3 point, sVector3 tri1, sVector3 tri2, sVector3 tri3) {
  // Check to see if a sVector3 Point is within a 3 sVector3 Triangle

    // Test to see if it is within an infinite prism that the triangle outlines.
    bool within_tri_prisim = sameSide (point, tri1, tri2, tri3) &&
                             sameSide (point, tri2, tri1, tri3) &&
                             sameSide (point, tri3, tri1, tri2);

    // If it isn't it will never be on the triangle
    if (!within_tri_prisim)
      return false;

    // Calulate Triangle's Normal
    sVector3 n = genTriNormal (tri1, tri2, tri3);

    // Project the point onto this normal
    sVector3 proj = projV3 (point, n);

    // If the distance from the triangle to the point is 0
    //  it lies on the triangle
    if (magnitudeV3 (proj) == 0)
      return true;
    else
      return false;
    }
  //}}}

  // string utils
  //{{{
  void split (const string &in, vector<string> &out, string token) {
  // Split a String into a string array at a given token

    out.clear();

    string temp;

    for (int i = 0; i < int(in.size()); i++) {
      string test = in.substr (i, token.size());

      if (test == token) {
        if (!temp.empty()) {
          out.push_back (temp);
          temp.clear();
          i += (int)token.size() - 1;
          }
        else
          out.push_back("");
        }
      else if (i + token.size() >= in.size()) {
        temp += in.substr (i, token.size());
        out.push_back (temp);
        break;
        }
      else
        temp += in[i];
      }
    }
  //}}}
  //{{{
  string tail (const string &in) {
  // Get tail of string after first token and possibly following spaces

    size_t token_start = in.find_first_not_of (" \t");
    size_t space_start = in.find_first_of (" \t", token_start);

    size_t tail_start = in.find_first_not_of (" \t", space_start);
    size_t tail_end = in.find_last_not_of (" \t");

    if (tail_start != string::npos && tail_end != string::npos)
      return in.substr (tail_start, tail_end - tail_start + 1);
    else if (tail_start != string::npos)
      return in.substr (tail_start);

    return "";
    }
  //}}}
  //{{{
  string firstToken (const string &in) {
  // Get first token of string

    if (!in.empty()) {
      size_t token_start = in.find_first_not_of (" \t");
      size_t token_end = in.find_first_of (" \t", token_start);

      if (token_start != string::npos && token_end != string::npos)
        return in.substr (token_start, token_end - token_start);
      else if (token_start != string::npos)
        return in.substr (token_start);
      }

    return "";
    }
  //}}}
  //{{{
  template <class T> inline const T& getElement (const vector<T>& elements, string& index) {
  // Get element at given index position

    int idx = stoi (index);
    if (idx < 0)
      idx = int(elements.size()) + idx;
    else
      idx--;

    return elements[idx];
    }
  //}}}

  //{{{
  class cObjLoader {
  public:
    cObjLoader() {}
    ~cObjLoader() { mLoadedMeshes.clear(); }

    //{{{
    bool loadFile (string fileName) {

      ifstream file (fileName);
      if (!file.is_open())
        return false;

      //{{{  vars
      mLoadedMeshes.clear();
      mLoadedVertices.clear();
      mLoadedIndices.clear();

      vector<sVector3> Positions;
      vector<sVector2> TCoords;
      vector<sVector3> Normals;
      vector<sVertex> Vertices;
      vector<unsigned int> Indices;
      vector<string> MeshMatNames;

      bool listening = false;
      string meshname;
      sMesh tempMesh;
      //}}}

      const unsigned int outputEveryNth = 1000;
      unsigned int outputIndicator = outputEveryNth;

      string curline;
      while (getline (file, curline)) {
        if (kLoaderDebug)
          if ((outputIndicator = ((outputIndicator + 1) % outputEveryNth)) == 1)
            if (!meshname.empty())
               cout << "\r- " << meshname
                         << "\t| vertices > " << Positions.size()
                         << "\t| texcoords > " << TCoords.size()
                         << "\t| normals > " << Normals.size()
                         << "\t| triangles > " << (Vertices.size() / 3)
                         << (!MeshMatNames.empty() ? "\t| material: " + MeshMatNames.back() : "");

        //{{{  Generate a Mesh Object or Prepare for an object to be created
        if (firstToken (curline) == "o" ||
            firstToken (curline) == "g" || curline[0] == 'g') {
          if (!listening) {
            listening = true;
            if (firstToken (curline) == "o" || firstToken (curline) == "g")
              meshname = tail (curline);
            else
              meshname = "unnamed";
            }
          else {
            // Generate the mesh to put into the array

            if (!Indices.empty() && !Vertices.empty()) {
              // Create Mesh
              tempMesh = sMesh (Vertices, Indices);
              tempMesh.MeshName = meshname;

              // Insert Mesh
              mLoadedMeshes.push_back (tempMesh);

              // Cleanup
              Vertices.clear();
              Indices.clear();
              meshname.clear();

              meshname = tail (curline);
              }
            else {
              if (firstToken (curline) == "o" || firstToken (curline) == "g")
                meshname = tail (curline);
              else
                meshname = "unnamed";
              }
            }

          if (kLoaderDebug) {
            cout << endl;
            outputIndicator = 0;
            }
          }
        //}}}
        //{{{  Generate a sVertex Position
        if (firstToken (curline) == "v") {
          vector<string> spos;
          sVector3 vpos;
          split (tail(curline), spos, " ");

          vpos.X = stof (spos[0]);
          vpos.Y = stof (spos[1]);
          vpos.Z = stof (spos[2]);

          Positions.push_back (vpos);
          }
        //}}}
        //{{{  Generate a sVertex Texture Coordinate
        if (firstToken (curline) == "vt") {
          vector<string> stex;
          sVector2 vtex;
          split (tail (curline), stex, " ");

          vtex.X = stof (stex[0]);
          vtex.Y = stof (stex[1]);

          TCoords.push_back (vtex);
          }
        //}}}
        //{{{  Generate a sVertex Normal;
        if (firstToken (curline) == "vn") {
          vector<string> snor;
          sVector3 vnor;
          split (tail(curline), snor, " ");

          vnor.X = stof (snor[0]);
          vnor.Y = stof (snor[1]);
          vnor.Z = stof (snor[2]);

          Normals.push_back (vnor);
          }
        //}}}
        //{{{  Generate a Face (vertices & indices)
        if (firstToken (curline) == "f") {
          // Generate the vertices
          vector<sVertex> vVerts;
          genVerticesFromRawOBJ (vVerts, Positions, TCoords, Normals, curline);

          // Add Vertices
          for (int i = 0; i < int(vVerts.size()); i++) {
            Vertices.push_back (vVerts[i]);
            mLoadedVertices.push_back (vVerts[i]);
            }

          vector<unsigned int> iIndices;

          vertexTriangluation (iIndices, vVerts);

          // Add Indices
          for (int i = 0; i < int(iIndices.size()); i++) {
            unsigned int indnum = (unsigned int)((Vertices.size()) - vVerts.size()) + iIndices[i];
            Indices.push_back (indnum);

            indnum = (unsigned int)((mLoadedVertices.size()) - vVerts.size()) + iIndices[i];
            mLoadedIndices.push_back (indnum);

            }
          }
        //}}}
        //{{{  Get Mesh Material Name
        if (firstToken(curline) == "usemtl") {
          MeshMatNames.push_back(tail(curline));

          // Create new Mesh, if Material changes within a group
          if (!Indices.empty() && !Vertices.empty()) {
            // Create Mesh
            tempMesh = sMesh(Vertices, Indices);
            tempMesh.MeshName = meshname;
            int i = 2;
            while (true) {
              tempMesh.MeshName = meshname + "_" + to_string (i);

              for (auto &m : mLoadedMeshes)
                if (m.MeshName == tempMesh.MeshName)
                  continue;
              break;
              }

            // Insert Mesh
            mLoadedMeshes.push_back (tempMesh);

            // Cleanup
            Vertices.clear();
            Indices.clear();
            }

          if (kLoaderDebug)
            outputIndicator = 0;
          }
        //}}}
        //{{{  Load Materials
        if (firstToken (curline) == "mtllib") {
          // Generate mLoadedMaterial

          // Generate a path to the material file
          vector<string> temp;
          split (fileName, temp, "/");

          string pathtomat = "";

          if (temp.size() != 1) {
            for (int i = 0; i < temp.size() - 1; i++)
              pathtomat += temp[i] + "/";
            }


          pathtomat += tail (curline);

          if (kLoaderDebug)
            cout << endl << "- find materials in: " << pathtomat << endl;

          // Load Materials
          loadMaterials (pathtomat);
          }
        //}}}
        if (kLoaderDebug)
          cout << endl;
        }

      // last mesh
      if (!Indices.empty() && !Vertices.empty()) {
        tempMesh = sMesh(Vertices, Indices);
        tempMesh.MeshName = meshname;
        mLoadedMeshes.push_back (tempMesh);
        }

      file.close();

      //{{{  Set Materials for each Mesh
      for (int i = 0; i < MeshMatNames.size(); i++) {
        string matname = MeshMatNames[i];

        // Find corresponding material name in loaded materials
        // when found copy material variables into mesh material
        for (int j = 0; j < mLoadedMaterials.size(); j++) {
          if (mLoadedMaterials[j].name == matname) {
            mLoadedMeshes[i].MeshMaterial = mLoadedMaterials[j];
            break;
            }
          }
        }
      //}}}
      if (mLoadedMeshes.empty() && mLoadedVertices.empty() && mLoadedIndices.empty())
        return false;
      else
        return true;
      }
    //}}}

    // vars
    vector<sVertex> mLoadedVertices;
    vector<sMesh> mLoadedMeshes;
    vector<unsigned int> mLoadedIndices;
    vector<sMaterial> mLoadedMaterials;

  private:
    //{{{
    void genVerticesFromRawOBJ (vector<sVertex>& oVerts,
                                const vector<sVector3>& iPositions,
                                const vector<sVector2>& iTCoords,
                                const vector<sVector3>& iNormals,
                                string icurline) {
    // Generate vertices from a list of positions,
    //  tcoords, normals and a face line

      vector<string> sface, svert;
      sVertex vVert;
      split (tail(icurline), sface, " ");

      bool noNormal = false;

      // For every given vertex do this
      for (int i = 0; i < int(sface.size()); i++) {
        // See What type the vertex is.
        int vtype;
        split (sface[i], svert, "/");

        // Check for just position - v1
        if (svert.size() == 1) // Only position
          vtype = 1;

        // Check for position & texture - v1/vt1
        if (svert.size() == 2) // Position & Texture
          vtype = 2;

        // Check for Position, Texture and Normal - v1/vt1/vn1
        // or if Position and Normal - v1//vn1
        if (svert.size() == 3) {
          if (svert[1] != "") // Position, Texture, and Normal
            vtype = 4;
          else // Position & Normal
            vtype = 3;
          }

        // Calculate and store the vertex
        switch (vtype) {
          //{{{
          case 1: // P
          {
            vVert.Position = getElement(iPositions, svert[0]);
            vVert.TextureCoordinate = sVector2(0, 0);
            noNormal = true;
            oVerts.push_back(vVert);
            break;
          }
          //}}}
          //{{{
          case 2: // P/T
          {
            vVert.Position = getElement(iPositions, svert[0]);
            vVert.TextureCoordinate = getElement(iTCoords, svert[1]);
            noNormal = true;
            oVerts.push_back(vVert);
            break;
          }
          //}}}
          //{{{
          case 3: // P//N
          {
            vVert.Position = getElement(iPositions, svert[0]);
            vVert.TextureCoordinate = sVector2(0, 0);
            vVert.Normal = getElement(iNormals, svert[2]);
            oVerts.push_back(vVert);
            break;
          }
          //}}}
          //{{{
          case 4: // P/T/N
          {
            vVert.Position = getElement(iPositions, svert[0]);
            vVert.TextureCoordinate = getElement(iTCoords, svert[1]);
            vVert.Normal = getElement(iNormals, svert[2]);
            oVerts.push_back(vVert);
            break;
          }
          //}}}
          //{{{
          default:
            break;
          //}}}
          }
        }

      // take care of missing normals
      // these may not be truly acurate but it is the
      // best they get for not compiling a mesh with normals
      if (noNormal) {
        sVector3 A = oVerts[0].Position - oVerts[1].Position;
        sVector3 B = oVerts[2].Position - oVerts[1].Position;
        sVector3 normal = crossV3(A, B);
        for (int i = 0; i < int(oVerts.size()); i++)
          oVerts[i].Normal = normal;
        }
      }
    //}}}
    //{{{
    // Triangulate a list of vertices into a face by printing
    //  inducies corresponding with triangles within it
    void vertexTriangluation (vector<unsigned int>& oIndices,
                              const vector<sVertex>& iVerts) {

      // If there are 2 or less verts, no triangle can be created, so exit
      if (iVerts.size() < 3)
        return;

      // If it is a triangle no need to calculate it
      if (iVerts.size() == 3) {
        oIndices.push_back (0);
        oIndices.push_back (1);
        oIndices.push_back (2);
        return;
        }

      // Create a list of vertices
      vector<sVertex> tVerts = iVerts;

      while (true) {
        // For every vertex
        for (int i = 0; i < int(tVerts.size()); i++) {
          // pPrev = the previous vertex in the list
          sVertex pPrev;
          if (i == 0)
            pPrev = tVerts[tVerts.size() - 1];
          else
            pPrev = tVerts[i - 1];

          // pCur = the current vertex;
          sVertex pCur = tVerts[i];

          // pNext = the next vertex in the list
          sVertex pNext;
          if (i == tVerts.size() - 1)
            pNext = tVerts[0];
          else
            pNext = tVerts[i + 1];

          // Check to see if there are only 3 verts left
          // if so this is the last triangle
          if (tVerts.size() == 3) {
            // Create a triangle from pCur, pPrev, pNext
            for (int j = 0; j < int(tVerts.size()); j++) {
              if (iVerts[j].Position == pCur.Position)
                oIndices.push_back(j);
              if (iVerts[j].Position == pPrev.Position)
                oIndices.push_back(j);
              if (iVerts[j].Position == pNext.Position)
                oIndices.push_back(j);
              }

            tVerts.clear();
            break;
            }

          if (tVerts.size() == 4) {
            // Create a triangle from pCur, pPrev, pNext
            for (int j = 0; j < int(iVerts.size()); j++) {
              if (iVerts[j].Position == pCur.Position)
                oIndices.push_back(j);
              if (iVerts[j].Position == pPrev.Position)
                oIndices.push_back(j);
              if (iVerts[j].Position == pNext.Position)
                oIndices.push_back(j);
              }

            sVector3 tempVec;
            for (int j = 0; j < int(tVerts.size()); j++) {
              if (tVerts[j].Position != pCur.Position
                && tVerts[j].Position != pPrev.Position
                && tVerts[j].Position != pNext.Position) {
                tempVec = tVerts[j].Position;
                break;
                }
              }

            // Create a triangle from pCur, pPrev, pNext
            for (int j = 0; j < int(iVerts.size()); j++) {
              if (iVerts[j].Position == pPrev.Position)
                oIndices.push_back(j);
              if (iVerts[j].Position == pNext.Position)
                oIndices.push_back(j);
              if (iVerts[j].Position == tempVec)
                oIndices.push_back(j);
              }

            tVerts.clear();
            break;
            }

          // If Vertex is not an interior vertex
          float angle = angleBetweenV3 (pPrev.Position - pCur.Position, pNext.Position - pCur.Position) * (180.f / 3.14159265359f);
          if (angle <= 0 && angle >= 180)
            continue;

          // If any vertices are within this triangle
          bool inTri = false;
          for (int j = 0; j < int(iVerts.size()); j++) {
            if (inTriangle (iVerts[j].Position, pPrev.Position, pCur.Position, pNext.Position)
              && iVerts[j].Position != pPrev.Position
              && iVerts[j].Position != pCur.Position
              && iVerts[j].Position != pNext.Position) {
              inTri = true;
              break;
              }
            }
          if (inTri)
            continue;

          // Create a triangle from pCur, pPrev, pNext
          for (int j = 0; j < int(iVerts.size()); j++) {
            if (iVerts[j].Position == pCur.Position)
              oIndices.push_back(j);
            if (iVerts[j].Position == pPrev.Position)
              oIndices.push_back(j);
            if (iVerts[j].Position == pNext.Position)
              oIndices.push_back(j);
            }

          // Delete pCur from the list
          for (int j = 0; j < int(tVerts.size()); j++) {
            if (tVerts[j].Position == pCur.Position) {
              tVerts.erase (tVerts.begin() + j);
              break;
              }
            }

          // reset i to the start
          // -1 since loop will add 1 to it
          i = -1;
          }

        // if no triangles were created
        if (oIndices.size() == 0)
          break;

        // if no more vertices
        if (tVerts.size() == 0)
          break;
        }
      }
    //}}}
    //{{{
    // Load Materials from .mtl file
    bool loadMaterials (string path) {

      // If the file is not a material file return false
      if (path.substr(path.size() - 4, path.size()) != ".mtl")
        return false;

      ifstream file(path);

      // If the file is not found return false
      if (!file.is_open())
        return false;

      sMaterial tempMaterial;

      bool listening = false;

      // Go through each line looking for material variables
      string curline;
      while (getline(file, curline)) {
        // new material and material name
        if (firstToken(curline) == "newmtl") {
          if (!listening) {
            listening = true;
            if (curline.size() > 7)
              tempMaterial.name = tail (curline);
            else
              tempMaterial.name = "none";
            }
          else {
            // Generate the material
            // Push Back loaded Material
            mLoadedMaterials.push_back (tempMaterial);

            // Clear mLoaded Material
            tempMaterial = sMaterial();

            if (curline.size() > 7)
              tempMaterial.name = tail (curline);
            else
              tempMaterial.name = "none";
            }
          }

        //{{{  Ambient Color
        if (firstToken (curline) == "Ka") {
          vector<string> temp;
          split (tail (curline), temp, " ");

          if (temp.size() != 3)
            continue;

          tempMaterial.Ka.X = stof (temp[0]);
          tempMaterial.Ka.Y = stof (temp[1]);
          tempMaterial.Ka.Z = stof (temp[2]);
        }
        //}}}
        //{{{  Diffuse Color
        if (firstToken(curline) == "Kd") {
          vector<string> temp;
          split(tail(curline), temp, " ");

          if (temp.size() != 3)
            continue;

          tempMaterial.Kd.X = stof(temp[0]);
          tempMaterial.Kd.Y = stof(temp[1]);
          tempMaterial.Kd.Z = stof(temp[2]);
        }
        //}}}
        //{{{  Specular Color
        if (firstToken(curline) == "Ks") {
          vector<string> temp;
          split(tail(curline), temp, " ");

          if (temp.size() != 3)
            continue;

          tempMaterial.Ks.X = stof(temp[0]);
          tempMaterial.Ks.Y = stof(temp[1]);
          tempMaterial.Ks.Z = stof(temp[2]);
        }
        //}}}
        //{{{  Specular Exponent
        if (firstToken (curline) == "Ns")
          tempMaterial.Ns = stof (tail(curline));
        //}}}
        //{{{  Optical Density
        if (firstToken (curline) == "Ni")
          tempMaterial.Ni = stof (tail(curline));
        //}}}
        //{{{  Dissolve
        if (firstToken (curline) == "d")
          tempMaterial.d = stof (tail(curline));
        //}}}
        //{{{  Illumination
        if (firstToken (curline) == "illum")
          tempMaterial.illum = stoi (tail(curline));
        //}}}
        //{{{  Ambient Texture Map
        if (firstToken(curline) == "map_Ka")
          tempMaterial.map_Ka = tail(curline);
        //}}}
        //{{{  Diffuse Texture Map
        if (firstToken(curline) == "map_Kd")
          tempMaterial.map_Kd = tail(curline);
        //}}}
        //{{{  Specular Texture Map
        if (firstToken(curline) == "map_Ks")
          tempMaterial.map_Ks = tail(curline);
        //}}}
        //{{{  Specular Hightlight Map
        if (firstToken(curline) == "map_Ns")
          tempMaterial.map_Ns = tail(curline);
        //}}}
        //{{{  Alpha Texture Map
        if (firstToken(curline) == "map_d")
          tempMaterial.map_d = tail(curline);
        //}}}
        //{{{  Bump Map
        if (firstToken(curline) == "map_Bump" || firstToken(curline) == "map_bump" || firstToken(curline) == "bump")
          tempMaterial.map_bump = tail(curline);
        //}}}
        }

      // Deal with last material

      // Push Back loaded Material
      mLoadedMaterials.push_back (tempMaterial);

      // Test to see if anything was loaded
      // If not return false
      if (mLoadedMaterials.empty())
        return false;
      // If so return true
      else
        return true;
      }
    //}}}
    };
  //}}}
  }

// cModel
//{{{
void cModel::loadIndex (int modelIndex) {

  if (modelIndex == 0)
    loadRawFile ("../models/stanford_dragon_v40k_f80k.raw");
  else
    loadRawFile ("../models/stanford_dragon_v344k_f688k.raw");
  }
//}}}
//{{{
void cModel::loadObjFile (const string& fileName) {

  cObjLoader loader;
  if (!loader.loadFile (fileName)) {
    //{{{  error, return
    cLog::log (LOGERROR, fmt::format ("cModel::loadObjFile {} not found", fileName));
    return;
    }
    //}}}

  for (auto& mesh : loader.mLoadedMeshes) {
    cLog::log (LOGINFO, fmt::format ("cModel::loadObjFile mesh {}", mesh.MeshName));
    //{{{  print material info
    //cout << "Material: " << mesh.MeshMaterial.name << "\n";
    //cout << "Ambient Color: " << mesh.MeshMaterial.Ka.X << ", " << mesh.MeshMaterial.Ka.Y << ", " << mesh.MeshMaterial.Ka.Z << "\n";
    //cout << "Diffuse Color: " << mesh.MeshMaterial.Kd.X << ", " << mesh.MeshMaterial.Kd.Y << ", " << mesh.MeshMaterial.Kd.Z << "\n";
    //cout << "Specular Color: " << mesh.MeshMaterial.Ks.X << ", " << mesh.MeshMaterial.Ks.Y << ", " << mesh.MeshMaterial.Ks.Z << "\n";
    //cout << "Specular Exponent: " << mesh.MeshMaterial.Ns << "\n";
    //cout << "Optical Density: " << mesh.MeshMaterial.Ni << "\n";
    //cout << "Dissolve: " << mesh.MeshMaterial.d << "\n";
    //cout << "Illumination: " << mesh.MeshMaterial.illum << "\n";
    //cout << "Ambient Texture Map: " << mesh.MeshMaterial.map_Ka << "\n";
    //cout << "Diffuse Texture Map: " << mesh.MeshMaterial.map_Kd << "\n";
    //cout << "Specular Texture Map: " << mesh.MeshMaterial.map_Ks << "\n";
    //cout << "Alpha Texture Map: " << mesh.MeshMaterial.map_d << "\n";
    //cout << "Bump Map: " << mesh.MeshMaterial.map_bump << "\n";
    //cout << "\n";
    //}}}

    for (size_t i = 0; i < loader.mLoadedVertices.size(); ++i) {
      // load vertices from .obj
      mVertices.push_back (Eigen::Vector3f (loader.mLoadedVertices[i].Position.X,
                                            loader.mLoadedVertices[i].Position.Y,
                                            loader.mLoadedVertices[i].Position.Z));
      // load normals from .obj
      mNormals.push_back (Eigen::Vector3f (loader.mLoadedVertices[i].Normal.X,
                                           loader.mLoadedVertices[i].Normal.Y,
                                           loader.mLoadedVertices[i].Normal.Z));
      }

    // load faces from .obj indices
    for (int j = 0; j < mesh.Indices.size(); j += 3)
      mFaces.push_back ({ mesh.Indices[j], mesh.Indices[j+1], mesh.Indices[j+2] });
    }

  cLog::log (LOGINFO, fmt::format ("cModel::loadObjFile {} vertices:{} faces:{}",
                                   fileName, mVertices.size(), mFaces.size()));
  normaliseVertices();
  //setVertexNormals();
  }
//}}}
//{{{
void cModel::loadRawFile (const string& fileName) {

  ifstream inputFind (fileName);
    if (!inputFind.good()) {
      //{{{  error, return
       cLog::log (LOGERROR, fmt::format ("cModel::loadFile - cannot find {}", fileName));
      throw runtime_error ("cannot find");
      return;
      }
      //}}}

  inputFind.close();

  ifstream input (fileName, ios::in | ios::binary);
  if (input.fail()) {
    ostringstream error_message;
    cLog::log (LOGERROR, fmt::format ("cModel::loadFile - cannot open {}", fileName));
    throw runtime_error (error_message.str().c_str());
    }

  unsigned int nv;
  input.read (reinterpret_cast<char*>(&nv), sizeof(unsigned int));
  mVertices.resize (nv);
  for (size_t i = 0; i < nv; ++i)
    input.read (reinterpret_cast<char*>(mVertices[i].data()), 3 * sizeof(float));

  unsigned int nf;
  input.read (reinterpret_cast<char*>(&nf), sizeof(unsigned int));
  mFaces.resize (nf);
  for (size_t i = 0; i < nf; ++i)
    input.read (reinterpret_cast<char*>(mFaces[i].data()), 3 * sizeof(unsigned int));

  input.close();

  cLog::log (LOGINFO, fmt::format ("cModel::loadRawFile {} vertices:{} faces:{}",
                                   fileName, mVertices.size(), mFaces.size()));
  normaliseVertices();
  setVertexNormals();
  }
//}}}
//{{{
void cModel::normaliseVertices() {
// normalise vertices to +/- 0.5 range, centre on mid ranges

  // original range
  float minX = numeric_limits<float>::max();
  float maxX = numeric_limits<float>::min();
  float minY = numeric_limits<float>::max();
  float maxY = numeric_limits<float>::min();
  float minZ = numeric_limits<float>::max();
  float maxZ = numeric_limits<float>::min();
  for (auto& vertice : mVertices) {
    //{{{  min,max xyz
    minX = min (vertice.data()[0], minX);
    maxX = max (vertice.data()[0], maxX);
    minY = min (vertice.data()[1], minY);
    maxY = max (vertice.data()[1], maxY);
    minZ = min (vertice.data()[2], minZ);
    maxZ = max (vertice.data()[2], maxZ);
    }
    //}}}
  cLog::log (LOGINFO, fmt::format ("range {:6.4f}:{:6.4f} {:6.4f}:{:6.4f} {:6.4f}:{:6.4f}",
                                   minX, maxX, minY, maxY, minZ, maxZ));

  // centre range
  mCentre = Eigen::Vector3f ((maxX + minX) / 2.f, (maxY + minY) / 2.f, (maxZ + minZ) / 2.f);

  minX = numeric_limits<float>::max();
  maxX = numeric_limits<float>::min();
  minY = numeric_limits<float>::max();
  maxY = numeric_limits<float>::min();
  minZ = numeric_limits<float>::max();
  maxZ = numeric_limits<float>::min();
  for (auto& vertice : mVertices) {
    //{{{  centre xyz
    vertice.data()[0] -= mCentre.data()[0];
    minX = min (vertice.data()[0], minX);
    maxX = max (vertice.data()[0], maxX);

    vertice.data()[1] -= mCentre.data()[1];
    minY = min (vertice.data()[1], minY);
    maxY = max (vertice.data()[1], maxY);

    vertice.data()[2] -= mCentre.data()[2];
    minZ = min (vertice.data()[2], minZ);
    maxZ = max (vertice.data()[2], maxZ);
    }
    //}}}
  cLog::log (LOGINFO, fmt::format ("- centred {:6.4f}:{:6.4f} {:6.4f}:{:6.4f} {:6.4f}:{:6.4f}",
                                   minX, maxX, minY, maxY, minZ, maxZ));

  //  scale to +/- 0.5
  float rangeX = maxX - minX;
  float rangeY = maxY - minY;
  float rangeZ = maxZ - minZ;
  float maxRange = max (max (rangeX, rangeY), rangeZ);
  float mScale = 1.f / maxRange;

  // scaled range
  minX = numeric_limits<float>::max();
  maxX = numeric_limits<float>::min();
  minY = numeric_limits<float>::max();
  maxY = numeric_limits<float>::min();
  minZ = numeric_limits<float>::max();
  maxZ = numeric_limits<float>::min();
  for (auto& vertice : mVertices) {
    //{{{  scale xyz
    vertice.data()[0] *= mScale;
    minX = min (vertice.data()[0], minX);
    maxX = max (vertice.data()[0], maxX);

    vertice.data()[1] *= mScale;
    minY = min (vertice.data()[1], minY);
    maxY = max (vertice.data()[1], maxY);

    vertice.data()[2] *= mScale;
    minZ = min (vertice.data()[2], minZ);
    maxZ = max (vertice.data()[2], maxZ);
    }
    //}}}
  cLog::log (LOGINFO, fmt::format ("- scaled:{:4.2f} x:{:6.4f}:{:6.4f} y:{:6.4f}:{:6.4f} z:{:6.4f}:{:6.4f}",
                                   mScale, minX, maxX, minY, maxY, minZ, maxZ));
  }
//}}}

//{{{
void cModel::ripple() {

  mTime += 25.f / 1000.f;

  const float k = 50.0f;
  const float a = 0.03f;
  const float v = 10.0f;
  for (unsigned int i(0); i < mVertices.size(); ++i) {
    const float x = mRefVertices[i].x() + mRefVertices[i].y() + mRefVertices[i].z();
    const float u = 5.0f * (x - 0.75f * sin(2.5f * mTime));
    const float w = (a / 2.0f) * (1.0f + sin(k * x + v * mTime));
    mVertices[i] = mRefVertices[i] + (exp(-u * u) * w) * mRefNormals[i];
    }

  setVertexNormals();
  }
//}}}

// - private
//{{{
void cModel::setVertexNormals() {

  unsigned int nf(static_cast<unsigned int>(mFaces.size()));
  unsigned int nv(static_cast<unsigned int>(mVertices.size()));

  mNormals.resize (mVertices.size());
  fill (mNormals.begin(), mNormals.end(), Eigen::Vector3f::Zero());

  for (size_t i = 0; i < mFaces.size(); ++i) {
    array<unsigned int, 3> const& f_i = mFaces[i];

    Eigen::Vector3f const& p0(mVertices[f_i[0]]);
    Eigen::Vector3f const& p1(mVertices[f_i[1]]);
    Eigen::Vector3f const& p2(mVertices[f_i[2]]);

    Eigen::Vector3f n_i = (p0 - p1).cross(p0 - p2);

    mNormals[f_i[0]] += n_i;
    mNormals[f_i[1]] += n_i;
    mNormals[f_i[2]] += n_i;
    }

  for (size_t i = 0; i < mVertices.size(); ++i)
    if (!mNormals[i].isZero())
      mNormals[i].normalize();
  }
//}}}
