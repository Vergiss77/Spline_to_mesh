#include "MeshStrategy.h"

bool SurfaceMeshStrategy::BuildMesh(gismo::gsMesh<> &mesh, const gsMatrix<> &support, const gsVector<int> &numSample)
{
    if(numSample.size() < 2 || support.rows() < 2) {
        gsInfo << "Invalid support or numSample size for surface mesh generation.\n";
        return false;
    }
    std::map<std::array<int, 2>, VertexHandle> vertexMap;
    // Create vertices.
    for(int i = 0; i < numSample[0] + 1; i++) {
        for(int j = 0; j  < numSample[1] + 1; j++) {
            gismo::gsVector<> point(3);
            point[0] = support(0, 0) + i / (double)numSample[0] * (support(0, 1) - support(0, 0));
            point[1] = support(1, 0) + j / (double)numSample[1] * (support(1, 1) - support(1, 0));
            point[2] = 0.0; // Assuming a surface in the XY plane
            auto v = mesh.addVertex(point);
            vertexMap[{i, j}] = v;
        }
    }
    // Create faces.
    for(int i = 0; i < numSample[0]; i++) {
        for(int j = 0; j < numSample[1]; j++) {
            auto v1 = vertexMap[{i, j}];
            auto v2 = vertexMap[{i + 1, j}];
            auto v3 = vertexMap[{i, j + 1}];
            auto v4 = vertexMap[{i + 1, j + 1}];
            if(_optionFlag & INVERT_NORMAL) {
                std::swap(v2, v3);
            }
            switch(_meshType) {
            case TRIANGLE_MESH:
                mesh.addFace(v1, v2, v3);
                mesh.addFace(v2, v4, v3);
                break;
            case SQUARE_MESH:
                mesh.addFace(v1, v2, v4, v3);
                break;
            default:
                gsInfo << "Unsupported mesh type: " << _meshType << ". Defaulting to TRIANGLE_MESH.\n";
                mesh.addFace(v1, v2, v3);
                mesh.addFace(v2, v4, v3);
                break;
            }
        }
    }
    return true;
}

bool VolumeSurfaceMeshStrategy::BuildMesh(gismo::gsMesh<> &mesh, const gsMatrix<> &support, const gsVector<int> &numSample)
{
    if(numSample.size() < 3 || support.rows() < 3) {
        gsInfo << "Invalid support or numSample size for volume mesh generation.\n";
        return false;
    }
    std::map<std::array<int, 3>, VertexHandle> vertexMap;
    // Create vertices.
    for(int i = 0; i < numSample[0] + 1; i++) {
        for(int j = 0; j < numSample[1] + 1; j++) {
            for(int k = 0; k < numSample[2] + 1; k++) {
                if(i == 0 || i == numSample[0] ||
                   j == 0 || j == numSample[1] ||
                   k == 0 || k == numSample[2]) {
                    // Only create vertices on the boundary
                    gismo::gsVector<> point(3);
                    point[0] = support(0, 0) + i / (double)numSample[0] * (support(0, 1) - support(0, 0));
                    point[1] = support(1, 0) + j / (double)numSample[1] * (support(1, 1) - support(1, 0));
                    point[2] = support(2, 0) + k / (double)numSample[2] * (support(2, 1) - support(2, 0));
                    auto v = mesh.addVertex(point);
                    vertexMap[{i, j, k}] = v;
                }
            }
        }
    }
    // Create faces.
    for(int t = 0; t < 6; t++){
        for(int i = 0; i < numSample[0]; i++) {
            for(int j = 0; j < numSample[1]; j++) {
                VertexHandle v1, v2, v3, v4;
                switch(t) {
                case 0:
                    // Back face
                    v1 = vertexMap[{0, j, i}];
                    v2 = vertexMap[{0, j, i + 1}];
                    v3 = vertexMap[{0, j + 1, i}];
                    v4 = vertexMap[{0, j + 1, i + 1}];
                    break;
                case 1:
                    // Left face
                    v1 = vertexMap[{i, 0, j}];
                    v2 = vertexMap[{i + 1, 0, j}];
                    v3 = vertexMap[{i, 0, j + 1}];
                    v4 = vertexMap[{i + 1, 0, j + 1}];
                    break;
                case 2:
                    // Bottom face
                    v1 = vertexMap[{j, i, 0}];
                    v2 = vertexMap[{j, i + 1, 0}];
                    v3 = vertexMap[{j + 1, i, 0}];
                    v4 = vertexMap[{j + 1, i + 1, 0}];
                    break;
                case 3:
                    // Top face
                    v1 = vertexMap[{i, j, numSample[2]}];
                    v2 = vertexMap[{i + 1, j, numSample[2]}];
                    v3 = vertexMap[{i, j + 1, numSample[2]}];
                    v4 = vertexMap[{i + 1, j + 1, numSample[2]}];
                    break;
                case 4:
                    // Right face
                    v1 = vertexMap[{j, numSample[1], i}];
                    v2 = vertexMap[{j, numSample[1], i + 1}];
                    v3 = vertexMap[{j + 1, numSample[1], i}];
                    v4 = vertexMap[{j + 1, numSample[1], i + 1}];
                    break;
                case 5:
                    // Front face
                    v1 = vertexMap[{numSample[0], i, j}];
                    v2 = vertexMap[{numSample[0], i + 1, j}];
                    v3 = vertexMap[{numSample[0], i, j + 1}];
                    v4 = vertexMap[{numSample[0], i + 1, j + 1}];
                    break;
                }

                if(_optionFlag & INVERT_NORMAL) {
                    std::swap(v2, v3);
                }
                switch(_meshType) {
                case TRIANGLE_MESH:
                    mesh.addFace(v1, v2, v3);
                    mesh.addFace(v2, v4, v3);
                    break;
                case SQUARE_MESH:
                    mesh.addFace(v1, v2, v4, v3);
                    break;
                default:
                    gsInfo << "Unsupported mesh type: " << _meshType << ". Defaulting to TRIANGLE_MESH.\n";
                    mesh.addFace(v1, v2, v3);
                    mesh.addFace(v2, v4, v3);
                    break;
                }
            }
        }
    }
    return true; // Placeholder return value
}