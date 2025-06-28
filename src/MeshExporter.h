#pragma once

#include <gismo.h>

#ifdef ASSIMP_USE
#include <assimp/Exporter.hpp>
#include <assimp/scene.h>
#endif

#include "MeshStrategy.h"

class BasisMeshExporter
{
protected:
    OptionFlag _optionFlag = static_cast<OptionFlag>(0);
    std::vector<std::array<index_t, 3>> _colors = {
        {255, 0, 0},
        {0, 255, 0},
        {0, 0, 255},
        {255, 255, 0},
        {255, 0, 255},
        {0, 255, 255}
    };
public:
    BasisMeshExporter(OptionFlag optionFlag = static_cast<OptionFlag>(0))
        : _optionFlag(optionFlag) {}
    virtual ~BasisMeshExporter() = default;
    virtual void SetColors(const std::vector<std::array<index_t, 3>> &colors) {
        _colors = colors;
    }
    virtual bool ExportMesh(const gismo::gsMesh<> &mesh,
                            const std::map<gismo::gsMesh<>::FaceHandle, index_t> &faceIndexMap,
                            const std::string &format,
                            const std::string &filename) = 0;
};

#ifdef ASSIMP_USE
class AssimpMeshExporter : public BasisMeshExporter
{
private:
    void ExportMeshtoScene(const gismo::gsMesh<> &mesh,
                           const std::map<gismo::gsMesh<>::FaceHandle, index_t> &faceIndexMap,
                           aiScene &scene);
public:
    AssimpMeshExporter(OptionFlag optionFlag = static_cast<OptionFlag>(0))
        : BasisMeshExporter(optionFlag) {}
    virtual bool ExportMesh(const gismo::gsMesh<> &mesh,
                            const std::map<gismo::gsMesh<>::FaceHandle, index_t> &faceIndexMap,
                            const std::string &format,
                            const std::string &filename) override;
};
#endif

class OffMeshExporter : public BasisMeshExporter
{
public:
    OffMeshExporter(OptionFlag optionFlag = static_cast<OptionFlag>(0))
        : BasisMeshExporter(optionFlag) {}
    virtual bool ExportMesh(const gismo::gsMesh<> &mesh,
                            const std::map<gismo::gsMesh<>::FaceHandle, index_t> &faceIndexMap,
                            const std::string &format,
                            const std::string &filename) override;
};

class ObjMeshExporter : public BasisMeshExporter
{
private:
    bool ExportMeshOnly(const gismo::gsMesh<> &mesh,
                        const std::string &filename);
    bool ExportMeshWithColor(const gismo::gsMesh<> &mesh,
                             const std::map<gismo::gsMesh<>::FaceHandle, index_t> &faceIndexMap,
                             const std::string &filename);
public:
    ObjMeshExporter(OptionFlag optionFlag = static_cast<OptionFlag>(0))
        : BasisMeshExporter(optionFlag) {}
    virtual bool ExportMesh(const gismo::gsMesh<> &mesh,
                            const std::map<gismo::gsMesh<>::FaceHandle, index_t> &faceIndexMap,
                            const std::string &format,
                            const std::string &filename) override;
};

class PlyMeshExporter : public BasisMeshExporter
{
public:
    PlyMeshExporter(OptionFlag optionFlag = static_cast<OptionFlag>(0))
        : BasisMeshExporter(optionFlag) {}
    virtual bool ExportMesh(const gismo::gsMesh<> &mesh,
                            const std::map<gismo::gsMesh<>::FaceHandle, index_t> &faceIndexMap,
                            const std::string &format,
                            const std::string &filename) override;
};