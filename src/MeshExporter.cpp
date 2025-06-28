#include "MeshExporter.h"
#include <fstream>

#ifdef ASSIMP_USE
void AssimpMeshExporter::ExportMeshtoScene(const gismo::gsMesh<> &mesh,
                         const std::map<gismo::gsMesh<>::FaceHandle, index_t> &faceIndexMap,
                         aiScene &scene)
{
    scene.mRootNode = new aiNode();
    // Create mesh
    scene.mMeshes = new aiMesh*[1];
    scene.mMeshes[0] = new aiMesh();
    aiMesh* meshPtr = scene.mMeshes[0];
    meshPtr->mNumVertices = mesh.numVertices();
    meshPtr->mVertices = new aiVector3D[mesh.numVertices()];
    for(int i = 0; i < mesh.numVertices(); ++i) {
        const auto& vertex = mesh.vertex(i);
        meshPtr->mVertices[i] = aiVector3D(vertex[0], vertex[1], vertex[2]);
    }
    meshPtr->mNumFaces = mesh.numFaces();
    meshPtr->mFaces = new aiFace[mesh.numFaces()];
    for(int i = 0; i < mesh.numFaces(); ++i) {
        const auto& face = mesh.face(i);
        meshPtr->mFaces[i].mNumIndices = face->vertices.size();
        meshPtr->mFaces[i].mIndices = new unsigned int[face->vertices.size()];
        for(size_t j = 0; j < face->vertices.size(); ++j) {
            meshPtr->mFaces[i].mIndices[j] = face->vertices[j]->getId();
        }
    }

    // Create material (even if not needed, an empty material is required for some formats)
    scene.mMaterials = new aiMaterial*[1];
    scene.mMaterials[0] = new aiMaterial();
    scene.mNumMaterials = 1;

    meshPtr->mMaterialIndex = 0;

    scene.mNumMeshes = 1;
    scene.mRootNode->mMeshes = new unsigned int[1];
    scene.mRootNode->mMeshes[0] = 0;
    scene.mRootNode->mNumMeshes = 1;
}

bool AssimpMeshExporter::ExportMesh(const gismo::gsMesh<> &mesh,
                                   const std::map<gismo::gsMesh<>::FaceHandle, index_t> &faceIndexMap,
                                   const std::string &format,
                                   const std::string &filename)
{
    aiScene scene;
    
    ExportMeshtoScene(mesh, faceIndexMap, scene);

    Assimp::Exporter exporter;
    aiReturn ret = exporter.Export(&scene, format.c_str(), filename.c_str());
    
    if (ret != AI_SUCCESS) {
        std::cerr << "Assimp export error: " << exporter.GetErrorString() << '\n';
        return false;
    }
    return true;
}
#endif

bool OffMeshExporter::ExportMesh(const gismo::gsMesh<> &mesh,
                                 const std::map<gismo::gsMesh<>::FaceHandle, index_t> &faceIndexMap,
                                 const std::string &format,
                                 const std::string &filename)
{
    std::fstream fileOut(filename, std::ios::out);
    if (!fileOut.is_open()) {
        std::cerr << "Failed to open file for writing: " << filename << "\n";
        return false;
    }   
    fileOut << "OFF\n";
    fileOut << mesh.numVertices() << " " << mesh.numFaces() << " 0\n";
    fileOut << std::fixed << std::setprecision(std::numeric_limits<long double>::digits10);
    for (const auto& vertex : mesh.vertices()) {
        fileOut << vertex->x() << " " << vertex->y() << " " << vertex->z() << '\n';
    }
    for (const auto& face : mesh.faces()) {
        fileOut << face->vertices.size();
        for (const auto& v : face->vertices) {
            fileOut << " " << v->getId();
        }
        if (_optionFlag & WITH_COLOR) {
            index_t colorIndex = faceIndexMap.at(face);
            fileOut << " " << _colors[colorIndex][0]
                    << " " << _colors[colorIndex][1]
                    << " " << _colors[colorIndex][2];
        }
        fileOut << '\n';
    }
    
    fileOut.close();
    return true;
}

bool ObjMeshExporter::ExportMeshOnly(const gismo::gsMesh<> &mesh,
                                     const std::string &filename)
{
    std::fstream fileOut(filename, std::ios::out);
    if (!fileOut.is_open()) {
        std::cerr << "Failed to open file for writing: " << filename << "\n";
        return false;
    }

    fileOut << std::fixed << std::setprecision(std::numeric_limits<long double>::digits10);
    for (const auto& vertex : mesh.vertices()) {
        fileOut << "v " << vertex->x() << " " << vertex->y() << " " << vertex->z() << '\n';
    }
    
    for (const auto& face : mesh.faces()) {
        fileOut << "f";
        for (const auto& v : face->vertices) {
            fileOut << " " << v->getId() + 1; // OBJ format is 1-indexed
        }
        fileOut << '\n';
    }
    fileOut.close();
    return true;
}

bool ObjMeshExporter::ExportMeshWithColor(const gismo::gsMesh<> &mesh,
                                          const std::map<gismo::gsMesh<>::FaceHandle, index_t> &faceIndexMap,
                                          const std::string &filename)
{
    std::fstream fileOut(filename, std::ios::out);
    if (!fileOut.is_open()) {
        std::cerr << "Failed to open file for writing: " << filename << "\n";
        return false;
    }
    std::fstream fileMatOut(filename + ".mtl", std::ios::out);
    if (!fileMatOut.is_open()) {
        std::cerr << "Failed to open material file for writing: " << filename + ".mtl\n";
        return false;
    }

    fileOut << "###\n";
    fileOut << "#\n";
    fileOut << "# OBJ File " << filename << '\n';
    fileOut << "# Exported by Spline_to_mesh\n";
    fileOut << "#\n";
    fileOut << "# Vertices: " << mesh.numVertices() << '\n';
    fileOut << "# Faces: " << mesh.numFaces() << '\n';
    fileOut << "#\n";
    fileOut << "###\n";
    fileOut << "mtllib ./" << filename + ".mtl" << "\n\n";

    fileOut << std::fixed << std::setprecision(std::numeric_limits<long double>::digits10);
    for (const auto& vertex : mesh.vertices()) {
        fileOut << "v " << vertex->x() << " " << vertex->y() << " " << vertex->z() << '\n';
    }
    fileOut << "\n\n";
    
    // Count number of color of faces
    index_t colorNum = std::max_element(faceIndexMap.begin(), faceIndexMap.end(),
        [](const auto& a, const auto& b) {
            return a.second < b.second; }
        )->second + 1;
    
    // Store faces with colors
    std::vector<std::vector<index_t>> facesWithColor(colorNum);
    for (const auto& face : mesh.faces()) {
        index_t colorIndex = faceIndexMap.at(face);
        facesWithColor[colorIndex].push_back(face->getId()); 
    }
    // Write faces with colors
    for (index_t i = 0; i < colorNum; ++i) {
        fileOut << "usemtl material_" << i << '\n';
        for (const auto& faceId : facesWithColor[i]) {
            fileOut << "f";
            const auto& face = mesh.face(faceId);
            for (const auto& v : face->vertices) {
                fileOut << " " << v->getId() + 1; // OBJ format is 1-indexed
            }
            fileOut << '\n';
        }
        fileOut << '\n';
    }
    // Write material definitions
    fileMatOut << "#\n";
    fileMatOut << "# Material definitions\n";
    fileMatOut << "# Exported by Spline_to_mesh\n";
    fileMatOut << "#\n\n";
    fileMatOut << std::fixed << std::setprecision(6);
    for(index_t i = 0; i < colorNum; ++i) {
        fileMatOut << "newmtl material_" << i << '\n';
        fileMatOut << "Ka " << 0.2 << " " << 0.2 << " " << 0.2 << '\n';
        fileMatOut << "Kd " << _colors[i][0] / 255.0 << " "
                   << _colors[i][1] / 255.0 << " "
                   << _colors[i][2] / 255.0 << '\n';
        fileMatOut << "Ks " << 1.0 << ' ' << 1.0 << ' ' << 1.0 << '\n';
        fileMatOut << "Tr " << 0.0 << '\n';
        fileMatOut << "illum 2\n";
        fileMatOut << "Ns " << 0.0 << "\n\n";
    }

    fileOut.close();
    fileMatOut.close();
    return true;
}

bool ObjMeshExporter::ExportMesh(const gismo::gsMesh<> &mesh,
                                 const std::map<gismo::gsMesh<>::FaceHandle, index_t> &faceIndexMap,
                                 const std::string &format,
                                 const std::string &filename)
{
    if(_optionFlag & WITH_COLOR) {
        return ExportMeshWithColor(mesh, faceIndexMap, filename);
    } else {
        return ExportMeshOnly(mesh, filename);
    }
}

bool PlyMeshExporter::ExportMesh(const gismo::gsMesh<> &mesh,
                                 const std::map<gismo::gsMesh<>::FaceHandle, index_t> &faceIndexMap,
                                 const std::string &format,
                                 const std::string &filename)
{
    std::fstream fileOut(filename, std::ios::out);
    if (!fileOut.is_open()) {
        std::cerr << "Failed to open file for writing: " << filename << "\n";
        return false;
    }
    
    fileOut << "ply\n";
    fileOut << "format ascii 1.0\n";
    fileOut << "element vertex " << mesh.numVertices() << "\n";
    fileOut << "property float x\n";
    fileOut << "property float y\n";
    fileOut << "property float z\n";
    fileOut << "element face " << mesh.numFaces() << "\n";
    fileOut << "property list uchar int vertex_indices\n";
    
    if (_optionFlag & WITH_COLOR) {
        fileOut << "property uchar red\n";
        fileOut << "property uchar green\n";
        fileOut << "property uchar blue\n";
    }
    
    fileOut << "end_header\n";

    fileOut << std::fixed << std::setprecision(std::numeric_limits<long double>::digits10);
    for (const auto& vertex : mesh.vertices()) {
        fileOut << vertex->x() << " " << vertex->y() << " " << vertex->z();
        fileOut << '\n';
    }

    for (const auto& face : mesh.faces()) {
        fileOut << face->vertices.size();
        for (const auto& v : face->vertices) {
            fileOut << " " << v->getId();
        }
        if (_optionFlag & WITH_COLOR) {
            index_t colorIndex = faceIndexMap.at(face);
            fileOut << " "
                    << static_cast<int>(_colors[colorIndex][0]) << " "
                    << static_cast<int>(_colors[colorIndex][1]) << " "
                    << static_cast<int>(_colors[colorIndex][2]);
        }
        fileOut << '\n';
    }
    fileOut.close();
    return true;
}