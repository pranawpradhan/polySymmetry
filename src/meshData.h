/*
    PLUGIN NAME: polySymmetry
*/

#ifndef MESH_DATA_CMD_H
#define MESH_DATA_CMD_H

#include <vector>
#include <unordered_map>

#include <maya/MDagPath.h>
#include <maya/MItMeshEdge.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MItMeshVertex.h>

using namespace std;

struct ComponentData
{
    vector<int> connectedVertices;
    vector<int> connectedEdges;
    vector<int> connectedFaces;
};

struct VertexData : ComponentData
{
    unordered_map<int,vector<int>> faceVertexSiblings;
};
    
struct EdgeData : ComponentData {};
struct FaceData : ComponentData {};

class MeshData
{
public:
    MeshData();
    virtual ~MeshData();

    virtual void        unpackMesh(MDagPath &meshDagPath);
    virtual void        clear();

private:
    
    virtual void        unpackEdges(MItMeshEdge &edges);
    virtual void        unpackFaces(MItMeshPolygon &faces);
    virtual void        unpackVertices(MItMeshVertex &vertices);
    virtual void        unpackVertexSiblings();
    
    virtual void        insertAll(MIntArray &src, vector<int> &dest);

public:
    int                     numberOfVertices;
    int                     numberOfEdges;
    int                     numberOfFaces;
    
    vector<VertexData>      vertexData;
    vector<EdgeData>        edgeData;
    vector<FaceData>        faceData;
};


#endif