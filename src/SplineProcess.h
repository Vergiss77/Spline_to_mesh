#pragma once

#include <gismo.h>
#include "MeshStrategy.h"

#define Eigen gsEigen

class BasisSplineProcess
{
protected:
    std::unique_ptr<gismo::gsGeometry<>> _spline_ptr;
    std::unique_ptr<BasisMeshStrategy> _meshStrategyPtr;

    OptionFlag _optionFlag = static_cast<OptionFlag>(0);
    MeshType _meshType = TRIANGLE_MESH;

    std::vector<std::array<index_t, 3>> _colors = {
        {255, 0, 0},
        {0, 255, 0},
        {0, 0, 255},
        {255, 255, 0},
        {255, 0, 255},
        {0, 255, 255}
    };
public:
    BasisSplineProcess(OptionFlag optionFlag = static_cast<OptionFlag>(0), MeshType meshType = TRIANGLE_MESH)
        : _optionFlag(optionFlag), _meshType(meshType) {}
    BasisSplineProcess(const std::string &filename, OptionFlag optionFlag = static_cast<OptionFlag>(0), MeshType meshType = TRIANGLE_MESH)
        : _optionFlag(optionFlag), _meshType(meshType) {
        LoadSplinefromFile(filename);
    }
    BasisSplineProcess(BasisSplineProcess &other) {
        _spline_ptr = std::move(other._spline_ptr);
        _meshStrategyPtr = std::move(other._meshStrategyPtr);
        _optionFlag = other._optionFlag;
        _meshType = other._meshType;
    }
    virtual ~BasisSplineProcess() = default;

    bool LoadSplinefromFile(const std::string &filename);
    void SaveSplinetoFile(const std::string &filename);

    int GetDimension() const { return _spline_ptr->parDim(); }

    virtual void InitializeMeshStrategy() {}
    virtual void BuildSurfacetoMesh(gismo::gsMesh<> &mesh, std::map<gismo::gsMesh<>::FaceHandle, index_t> &faceIndexMap, index_t numSample = 64);
    virtual void SetMeshColorIndexMap(const gismo::gsMesh<> &mesh, const gismo::gsMatrix<> &support,
                                      std::map<gismo::gsMesh<>::FaceHandle, index_t> &faceIndexMap){};
    virtual void BuildSurfacetoFileOFF(const std::string &filename, index_t numSample = 64);
};

class VolumeSplineProcess : public BasisSplineProcess
{
public:
    VolumeSplineProcess() = default;
    VolumeSplineProcess(const std::string &filename) : BasisSplineProcess(filename) {}
    VolumeSplineProcess(BasisSplineProcess &other) : BasisSplineProcess(other) {}

    virtual void InitializeMeshStrategy() override {
        _meshStrategyPtr = std::make_unique<VolumeSurfaceMeshStrategy>(_meshType, _optionFlag);
    }
    virtual void SetMeshColorIndexMap(const gismo::gsMesh<> &mesh, const gismo::gsMatrix<> &support,
                                      std::map<gismo::gsMesh<>::FaceHandle, index_t> &faceIndexMap) override;
};

class SurfaceSplineProcess : public BasisSplineProcess
{
public:
    SurfaceSplineProcess() = default;
    SurfaceSplineProcess(const std::string &filename) : BasisSplineProcess(filename) {}
    SurfaceSplineProcess(BasisSplineProcess &other) : BasisSplineProcess(other) {}

    virtual void InitializeMeshStrategy() override {
        _meshStrategyPtr = std::make_unique<SurfaceMeshStrategy>(_meshType, _optionFlag);
    }
    virtual void SetMeshColorIndexMap(const gismo::gsMesh<> &mesh, const gismo::gsMatrix<> &support,
                                      std::map<gismo::gsMesh<>::FaceHandle, index_t> &faceIndexMap) override;
};
