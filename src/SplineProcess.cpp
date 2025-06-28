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

void BasisSplineProcess::BuildSurfacetoMesh(gismo::gsMesh<> &mesh, std::map<gismo::gsMesh<>::FaceHandle, index_t> &faceIndexMap, index_t numSample)
{
    if(!_spline_ptr) {
        gsInfo << "No spline loaded to build mesh.\n";
        return;
    }
    if(!_meshStrategyPtr) {
        InitializeMeshStrategy();
    }
    if(_meshStrategyPtr->BuildMesh(mesh, _spline_ptr->support(), numSample)){
        SetMeshColorIndexMap(mesh, _spline_ptr->support(), faceIndexMap);
        _spline_ptr->evaluateMesh(mesh);
    } else {
        gsInfo << "Failed to build mesh.\n";
        return;
    }
};

bool BasisSplineProcess::SaveMeshtoFile(const gismo::gsMesh<> &mesh,
                                        const std::map<gismo::gsMesh<>::FaceHandle, index_t> &faceIndexMap,
                                        const std::string &filename)
{
    std::string format = filename.substr(filename.find_last_of('.') + 1);
    if(format == "off") {
        _meshExporterPtr = std::make_unique<OffMeshExporter>(_optionFlag);
    } else if (format == "obj") {
        _meshExporterPtr = std::make_unique<ObjMeshExporter>(_optionFlag);
    } else if (format == "ply") {
        _meshExporterPtr = std::make_unique<PlyMeshExporter>(_optionFlag);
    } else {
        #ifdef ASSIMP_USE
        _meshExporterPtr = std::make_unique<AssimpMeshExporter>(_optionFlag);
        #else
        gsInfo << "Unsupported mesh format: " << format << ". Please use .off, .obj, or .ply.\n";
        return false;
        #endif
    }
    if(_meshExporterPtr->ExportMesh(mesh, faceIndexMap, format, filename)){
        gsInfo << "Mesh saved to file: " << filename << "\n";
        return true;
    } else {
        gsInfo << "Failed to save mesh to file " << filename << " with format " << format << "\n";
        return false;
    }
}

bool BasisSplineProcess::BuildSurfacetoFile(const std::string &filename, index_t num)
{
    gsInfo << "Building model to file...\n";
    if(!_spline_ptr) {
        gsInfo << "No spline loaded to build model.\n";
        return false;
    }
    gismo::gsMesh<> mesh;
    std::map<gismo::gsMesh<>::FaceHandle, index_t> faceIndexMap;
    BuildSurfacetoMesh(mesh, faceIndexMap, num);
    if(!SaveMeshtoFile(mesh, faceIndexMap, filename)){
        gsInfo << "Failed to build model to file: " << filename << "\n";
        return false;
    }

    gsInfo << "Building model done.\n";
    return true;
}

void BasisSplineProcess::ShowExportFormatsSupported() const
{
    gsInfo << "Supported export formats:\n";
    gsInfo << "1. OFF (.off)\n";
    gsInfo << "2. OBJ (.obj)\n";
    gsInfo << "3. PLY (.ply)\n";
    
    #ifdef ASSIMP_USE
    Assimp::Exporter exporter;
    size_t n = exporter.GetExportFormatCount();
    gsInfo << "4. Assimp supported formats (" << n << " formats):\n";
    for(size_t i = 0; i < n; ++i) {
        const aiExportFormatDesc* desc = exporter.GetExportFormatDescription(i);
        gsInfo << "   " << desc->id << " : " << desc->description << " (." << desc->fileExtension << ")" << std::endl;
    }
    #else
    gsInfo << "4. Assimp is not enabled. Please compile with ASSIMP_USE defined to use Assimp exporter.\n";
    #endif
    gsInfo << '\n';

}

void VolumeSplineProcess::SetMeshColorIndexMap(const gismo::gsMesh<> &mesh, const gismo::gsMatrix<> &support,
                                               std::map<gismo::gsMesh<>::FaceHandle, index_t> &faceIndexMap)
{
    for(const auto &face : mesh.faces()) {
        gismo::gsVector<double> point = gismo::gsVector<double>::Zero(3);
        // Calculate the center of the face by averaging the vertices
        for(const auto &v : face->vertices) {
            point.x() += v->x();
            point.y() += v->y();
            point.z() += v->z();
        }
        point /= face->vertices.size();
        if(point.x() == support(0, 0)){
            faceIndexMap[face] = 0; // Back face
        } else if(point.x() == support(0, 1)) {
            faceIndexMap[face] = 5; // Front face
        } else if(point.y() == support(1, 0)) {
            faceIndexMap[face] = 1; // Left face
        } else if(point.y() == support(1, 1)) {
            faceIndexMap[face] = 4; // Right face
        } else if(point.z() == support(2, 0)) {
            faceIndexMap[face] = 2; // Bottom face
        } else if(point.z() == support(2, 1)) {
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
