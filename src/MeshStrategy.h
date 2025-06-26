#pragma once

#include "gsUtils/gsMesh/gsMesh.h"
#include <gismo.h>

enum MeshType
{
    TRIANGLE_MESH = 0,
    SQUARE_MESH = 1,
};

enum OptionFlag
{
    INVERT_NORMAL = 1 << 0,
    WITH_COLOR = 1 << 1,
};

class BasisMeshStrategy
{
protected:
    using VertexHandle = gismo::gsMesh<>::VertexHandle;

    MeshType _meshType;
    OptionFlag _optionFlag;

public:
    BasisMeshStrategy(MeshType meshType = TRIANGLE_MESH, OptionFlag optionFlag = static_cast<OptionFlag>(0))
        : _meshType(meshType), _optionFlag(optionFlag) {}
    virtual ~BasisMeshStrategy() = default;
    virtual bool BuildMesh(gismo::gsMesh<> &mesh, const gsMatrix<> &support, const gsVector<int> &numSample) = 0;
    virtual bool BuildMesh(gismo::gsMesh<> &mesh, const gsMatrix<> &support, int numSample = 64)
    {
        gsVector<int> numSampleVec(3);
        numSampleVec.setConstant(numSample);
        return BuildMesh(mesh, support, numSampleVec);
    }
    virtual bool BuildMesh(gismo::gsMesh<> &mesh, int numSample = 64)
    {
        gsMatrix<> support(3, 2);
        support << 0, 1, 0, 1, 0, 1; // Default support for a unit cube
        return BuildMesh(mesh, support, numSample);
    }
};

class SurfaceMeshStrategy : public BasisMeshStrategy
{
public:
    SurfaceMeshStrategy(MeshType meshType = TRIANGLE_MESH, OptionFlag optionFlag = static_cast<OptionFlag>(0))
        : BasisMeshStrategy(meshType, optionFlag) {}
    virtual bool BuildMesh(gismo::gsMesh<> &mesh, const gsMatrix<> &support, const gsVector<int> &numSample) override;
};

class VolumeSurfaceMeshStrategy : public BasisMeshStrategy
{
public:
    VolumeSurfaceMeshStrategy(MeshType meshType = TRIANGLE_MESH, OptionFlag optionFlag = static_cast<OptionFlag>(0))
        : BasisMeshStrategy(meshType, optionFlag) {}
    virtual bool BuildMesh(gismo::gsMesh<> &mesh, const gsMatrix<> &support, const gsVector<int> &numSample) override;
};