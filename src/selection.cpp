#include "selection.h"
#include "meshData.h"
#include "util.h"

#include <sstream>
#include <stdio.h>
#include <vector>
#include <utility>

#include <maya/MDagPath.h>
#include <maya/MGlobal.h>
#include <maya/MObjectArray.h>
#include <maya/MIntArray.h>
#include <maya/MItGeometry.h>
#include <maya/MItMeshEdge.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MItMeshVertex.h>
#include <maya/MItSelectionList.h>
#include <maya/MSelectionList.h>
#include <maya/MObject.h>

using namespace std;

void getSelectedComponents(
    MDagPath &selectedMesh,
    MSelectionList &activeSelection, 
    MSelectionList &selection,
    MFn::Type componentType)
{
    MDagPath mesh;
    MObject component;

    MItSelectionList iterComponents(activeSelection, componentType);

    while (!iterComponents.isDone())
    {
        iterComponents.getDagPath(mesh, component);

        if (mesh.node().hasFn(MFn::kMesh)) { mesh.pop(); }

        if (mesh == selectedMesh)
        {
            selection.add(mesh, component, true);
        }

        iterComponents.next();
    }
}


void getSelectedComponentIndices(
    MSelectionList &activeSelection, 
    vector<int> &indices,
    MFn::Type componentType)
{
    MDagPath mesh;
    MObject component;

    MItSelectionList iterComponents(activeSelection, componentType);

    while (!iterComponents.isDone())
    {
        iterComponents.getDagPath(mesh, component);

        if (componentType == MFn::kMeshEdgeComponent)
        {
            MItMeshEdge iterGeo(mesh, component);

            while (!iterGeo.isDone())
            {
                indices.push_back(iterGeo.index());
                iterGeo.next();
            }
        } else if (componentType == MFn::kMeshVertComponent) {            
            MItMeshVertex iterGeo(mesh, component);

            while (!iterGeo.isDone())
            {
                indices.push_back(iterGeo.index());
                iterGeo.next();
            }
        } else if (componentType == MFn::kMeshPolygonComponent) {
            MItMeshPolygon iterGeo(mesh, component);

            while (!iterGeo.isDone())
            {
                indices.push_back(iterGeo.index());
                iterGeo.next();
            }
        }

        iterComponents.next();
    }
}


bool getSymmetricalComponentSelection(MeshData &meshData, MSelectionList &selection, ComponentSelection &componentSelection, bool leftSideVertexSelected)
{
    bool result = true;

    stringstream ss;

    MDagPath mesh;
    MObject component;

    vector<int> edgeIndices = vector<int>();
    vector<int> faceIndices = vector<int>();
    vector<int> vertexIndices = vector<int>();

    getSelectedComponentIndices(selection, edgeIndices, MFn::kMeshEdgeComponent);
    getSelectedComponentIndices(selection, faceIndices, MFn::kMeshPolygonComponent);
    getSelectedComponentIndices(selection, vertexIndices, MFn::kMeshVertComponent);

    int lastIndexPtr = -1;

    int numberOfVerticesSelected = (int) vertexIndices.size();

    if (
        edgeIndices.size() == 2 &&
        faceIndices.size() == 2 &&
        (leftSideVertexSelected ? numberOfVerticesSelected == 3 : numberOfVerticesSelected == 2)
    ) {
        vector<int> edgesOnFace0 = intersection(meshData.faceData[faceIndices[0]].connectedEdges, edgeIndices);
        vector<int> edgesOnFace1 = intersection(meshData.faceData[faceIndices[1]].connectedEdges, edgeIndices);

        vector<int> verticesOnFace0 = intersection(meshData.faceData[faceIndices[0]].connectedVertices, vertexIndices);
        vector<int> verticesOnFace1 = intersection(meshData.faceData[faceIndices[1]].connectedVertices, vertexIndices);

        vector<int> verticesOnEdge0 = intersection(meshData.edgeData[edgeIndices[0]].connectedVertices, vertexIndices);
        vector<int> verticesOnEdge1 = intersection(meshData.edgeData[edgeIndices[1]].connectedVertices, vertexIndices);

        bool edgesAreOnFaces = (edgesOnFace1.size() == 1 && edgesOnFace1.size() == 1 && edgesOnFace0[0] != edgesOnFace1[0]);
        bool verticesAreOnFaces = (verticesOnFace0.size() == 1 && verticesOnFace0.size() == 1 && verticesOnFace0[0] != verticesOnFace1[0]);
        bool verticesAreOnEdges = (verticesOnEdge0.size() == 1 && verticesOnEdge1.size() == 1 && verticesOnEdge0[0] != verticesOnEdge1[0]);

        int leftSideVertex = -1;

        for (int &v : vertexIndices)
        {
            if (contains(verticesOnFace0, v)) { continue; }
            if (contains(verticesOnFace1, v)) { continue; }
            if (contains(verticesOnEdge0, v)) { continue; }
            if (contains(verticesOnEdge1, v)) { continue; }

            leftSideVertex = v;
        }

        bool leftSideVertexFound = vertexIndices.size() == 3 ? leftSideVertex != -1 : true;

        result = (edgesAreOnFaces && verticesAreOnFaces && verticesAreOnEdges && leftSideVertexFound);

        if (result)
        {
            componentSelection.edgeIndices.first = edgeIndices[0];
            componentSelection.edgeIndices.second = edgeIndices[1];

            componentSelection.faceIndices.first = faceIndices[0];
            componentSelection.faceIndices.second = faceIndices[1];

            componentSelection.vertexIndices.first = verticesOnEdge0[0];
            componentSelection.vertexIndices.second = verticesOnEdge1[0];

            if (leftSideVertexFound) {
                componentSelection.leftVertexIndex = leftSideVertex;
            }
        }
    } else {
        result = false;
    }

    return result;
}