#include "SplineProcess.h"

bool BasisSplineProcess::LoadSplinefromFile(const std::string &filename)
{
    gsInfo << "Loading Spline from file...\n";
    gismo::gsFileData<> fileData;
    if(!fileData.read(filename)) {
        return false;
    }
    if(fileData.has<gismo::gsGeometry<>>()){
        std::vector<gsGeometry<>::uPtr> geos = fileData.getAll< gsGeometry<> >();
        gsInfo<< "Got "<< geos.size() <<" patch"<<(geos.size() == 1 ? "." : "es.") <<"\n";
        if(geos.size() != 1){
            gsInfo<< "Warning: Multiple geometries found, only the first one will be used.\n";
            return false;
        } else {
            _spline_ptr = std::move(geos[0]);
        }
    } else {
        return false;
    }

    gsInfo << "Loading done.\n";
    return true;
}

void BasisSplineProcess::SaveSplinetoFile(const std::string &filename)
{
    gsInfo << "Saving Spline to file...\n";
    if(!_spline_ptr) {
        gsInfo << "No spline loaded to save.\n";
        return;
    }
    gismo::gsFileData<> fileData;
    fileData << *_spline_ptr;
    fileData.save(filename);
    gsInfo << "Saving done.\n";
}

void BasisSplineProcess::BuildSurfacetoMesh(gismo::gsMesh<> &mesh, index_t numSample)
{
    if(!_spline_ptr) {
        gsInfo << "No spline loaded to build mesh.\n";
        return;
    }
    if(!_meshStrategyPtr) {
        InitializeMeshStrategy();
    }
    if(_meshStrategyPtr->BuildMesh(mesh, _spline_ptr->support(), numSample)){
        _spline_ptr->evaluateMesh(mesh);
    } else {
        gsInfo << "Failed to build mesh.\n";
        return;
    }
};

void BasisSplineProcess::BuildSurfacetoFileOFF(const std::string &filename, index_t num)
{
    gsInfo << "Building model to file...\n";
    if(!_spline_ptr) {
        gsInfo << "No spline loaded to build model.\n";
        return;
    }
    gismo::gsMesh<> mesh;
    std::map<gismo::gsMesh<>::FaceHandle, index_t> faceIndexMap;
    BuildSurfacetoMesh(mesh, num);
    SetMeshColorIndexMap(mesh, _spline_ptr->support(), faceIndexMap);

    std::fstream fileOut(filename, std::ios::out);
    fileOut << "OFF\n";
    fileOut << mesh.numVertices() << " " << mesh.numFaces() << " 0\n";
    fileOut << std::setprecision(std::numeric_limits<long double>::digits10);
    for(auto &vertex : mesh.vertices()) {
        fileOut << vertex->x() << " " << vertex->y() << " " << vertex->z() << '\n';
    }
    for(auto &face : mesh.faces()) {
        fileOut << face->vertices.size();
        for(auto &v : face->vertices) {
            fileOut << " " << v->getId();
        }
        if(_optionFlag & WITH_COLOR) {
            index_t colorIndex = faceIndexMap[face];
            fileOut << " " << _colors[colorIndex][0] << " " << _colors[colorIndex][1] << " " << _colors[colorIndex][2];
        }
        fileOut << '\n';
    }
    fileOut.close();

    gsInfo << "Building model done.\n";
}

void VolumeSplineProcess::SetMeshColorIndexMap(const gismo::gsMesh<> &mesh, const gismo::gsMatrix<> &support,
                                               std::map<gismo::gsMesh<>::FaceHandle, index_t> &faceIndexMap)
{
    for(const auto &face : mesh.faces()) {
        auto v = face->vertices.front();
        if(v->x() == support(0, 0)){
            faceIndexMap[face] = 0; // Back face
        } else if(v->x() == support(0, 1)) {
            faceIndexMap[face] = 5; // Front face
        } else if(v->y() == support(1, 0)) {
            faceIndexMap[face] = 1; // Left face
        } else if(v->y() == support(1, 1)) {
            faceIndexMap[face] = 4; // Right face
        } else if(v->z() == support(2, 0)) {
            faceIndexMap[face] = 2; // Bottom face
        } else if(v->z() == support(2, 1)) {
            faceIndexMap[face] = 3; // Top face
        }
    }
}

void SurfaceSplineProcess::SetMeshColorIndexMap(const gismo::gsMesh<> &mesh, const gismo::gsMatrix<> &support,
                                               std::map<gismo::gsMesh<>::FaceHandle, index_t> &faceIndexMap)
{
    for(const auto &face : mesh.faces()) {
        faceIndexMap[face] = 0; // All faces are treated the same for surfaces
    }
}
