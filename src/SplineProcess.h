#pragma once

#include <gismo.h>

#define Eigen gsEigen

class BasisSplineProcess
{
protected:
    std::unique_ptr<gismo::gsGeometry<>> _spline_ptr;

public:
    BasisSplineProcess() = default;
    BasisSplineProcess(const std::string &filename) { LoadSplinefromFile(filename); }
    BasisSplineProcess(BasisSplineProcess &other) { _spline_ptr = std::move(other._spline_ptr); }

    bool LoadSplinefromFile(const std::string &filename);
    void SaveSplinetoFile(const std::string &filename);

    int GetDimension() const { return _spline_ptr->parDim(); }

    virtual void BuildSurfacetoMatrix(Eigen::MatrixXd &V, Eigen::MatrixXi &F,
                                      std::map<int, int> *faceIndexMap = nullptr,
                                      index_t numSample = 64, bool invertNormal = false) {};
    virtual void BuildSurfacetoFileOFF(const std::string &filename, index_t numSample = 64,
                                       bool withColor = false, bool invertNormal = false) {};
};

class VolumeSplineProcess : public BasisSplineProcess
{
private:
    const int DIM = 3;
public:
    VolumeSplineProcess() = default;
    VolumeSplineProcess(const std::string &filename) : BasisSplineProcess(filename) {}
    VolumeSplineProcess(BasisSplineProcess &other) : BasisSplineProcess(other) {}

    void BuildSurfacetoMatrix(Eigen::MatrixXd &V, Eigen::MatrixXi &F,
                              std::map<int, int> *faceIndexMap = nullptr,
                              index_t numSample = 64, bool invertNormal = false);
    void BuildSurfacetoFileOFF(const std::string &filename, index_t numSample = 64,
                               bool withColor = false, bool invertNormal = false);

};

class SurfaceSplineProcess : public BasisSplineProcess
{
private:
    const int DIM = 2;
public:
    SurfaceSplineProcess() = default;
    SurfaceSplineProcess(const std::string &filename) : BasisSplineProcess(filename) {}
    SurfaceSplineProcess(BasisSplineProcess &other) : BasisSplineProcess(other) {}

    void BuildSurfacetoMatrix(Eigen::MatrixXd &V, Eigen::MatrixXi &F,
                              std::map<int, int> *faceIndexMap = nullptr,
                              index_t numSample = 64, bool invertNormal = false);
    void BuildSurfacetoFileOFF(const std::string &filename, index_t numSample = 64,
                               bool withColor = false, bool invertNormal = false);
};
