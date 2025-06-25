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
    gismo::gsFileData<> fileData;
    fileData << *_spline_ptr;
    fileData.save(filename);
    gsInfo << "Saving done.\n";
}

void VolumeSplineProcess::BuildSurfacetoMatrix(Eigen::MatrixXd &V, Eigen::MatrixXi &F,
                                               std::map<int, int> *faceIndexMap,
                                               index_t numSample, bool invertNormal)
{
    int pointNum = numSample + 1;
    int pointSumNum = pointNum * pointNum * pointNum 
                    - (pointNum - 2) * (pointNum - 2) * (pointNum - 2);

    auto support = _spline_ptr->support();
    gismo::gsMatrix<> param(DIM, pointSumNum);
    std::map< std::array<int, 3>, int> indexMap;
    int index = 0;
    for(int i = 0; i < pointNum; i++) {
        for(int j = 0; j < pointNum; j++) {
            for(int k = 0; k < pointNum; k++) {
                if(i == 0 || i == pointNum - 1 ||
                   j == 0 || j == pointNum - 1 ||
                   k == 0 || k == pointNum - 1) {
                    param(0, index) = support(0, 0) + i / (double)numSample * (support(0, 1) - support(0, 0));
                    param(1, index) = support(1, 0) + j / (double)numSample * (support(1, 1) - support(1, 0));
                    param(2, index) = support(2, 0) + k / (double)numSample * (support(2, 1) - support(2, 0));
                    indexMap[{i, j, k}] = index;
                    index++;
                }
            }
        }
    }

    gismo::gsMatrix<> resultValue(3, pointSumNum);
    _spline_ptr->eval_into(param, resultValue);

    int faceNum = (numSample * numSample * 6) * 2;
    V.resize(pointSumNum, 3);
    for(int i = 0; i < pointSumNum; i++) {
        for(int j = 0; j < 3; j++) {
            V(i, j) = resultValue(j, i);
        }
    }
    F.resize(faceNum, 3);

    std::vector<std::array<int, 4>> faceVec;
    for(int t = 0; t < 6; t++) {
        std::cout << "t = " << t << '\n';
        std::array<int, 4> indexVec;
        for(int i = 0; i < numSample; i++) {
            for(int j = 0; j < numSample; j++) {
                indexVec.fill(-1);
                switch(t) {
                case 0:
                    indexVec[0] = indexMap[{0, j, i}];
                    indexVec[1] = indexMap[{0, j, i + 1}];
                    indexVec[2] = indexMap[{0, j + 1, i}];
                    indexVec[3] = indexMap[{0, j + 1, i + 1}];
                    break;
                case 1:
                    indexVec[0] = indexMap[{i, 0, j}];
                    indexVec[1] = indexMap[{i + 1, 0, j}];
                    indexVec[2] = indexMap[{i, 0, j + 1}];
                    indexVec[3] = indexMap[{i + 1, 0, j + 1}];
                    break;
                case 2:
                    indexVec[0] = indexMap[{j, i, 0}];
                    indexVec[1] = indexMap[{j, i + 1, 0}];
                    indexVec[2] = indexMap[{j + 1, i, 0}];
                    indexVec[3] = indexMap[{j + 1, i + 1, 0}];
                    break;
                case 3:
                    indexVec[0] = indexMap[{i, j, pointNum - 1}];
                    indexVec[1] = indexMap[{i + 1, j, pointNum - 1}];
                    indexVec[2] = indexMap[{i, j + 1, pointNum - 1}];
                    indexVec[3] = indexMap[{i + 1, j + 1, pointNum - 1}];
                    break;
                case 4:
                    indexVec[0] = indexMap[{j, pointNum - 1, i}];
                    indexVec[1] = indexMap[{j, pointNum - 1, i + 1}];
                    indexVec[2] = indexMap[{j + 1, pointNum - 1, i}];
                    indexVec[3] = indexMap[{j + 1, pointNum - 1, i + 1}];
                    break;
                case 5:
                    indexVec[0] = indexMap[{pointNum - 1, i, j}];
                    indexVec[1] = indexMap[{pointNum - 1, i + 1, j}];
                    indexVec[2] = indexMap[{pointNum - 1, i, j + 1}];
                    indexVec[3] = indexMap[{pointNum - 1, i + 1, j + 1}];
                    break;
                }

                if(invertNormal){
                    std::swap(indexVec[0], indexVec[3]);
                }
                // std::cout << "indexVec = " << indexVec[0] << " " << indexVec[1] << " " << indexVec[2] << " " << indexVec[3] << '\n';
                if(faceIndexMap) {
                    faceIndexMap->insert({faceVec.size(), t});
                }
                faceVec.push_back({indexVec[0], indexVec[3], indexVec[2]});
                if(faceIndexMap) {
                    faceIndexMap->insert({faceVec.size(), t});
                }
                faceVec.push_back({indexVec[0], indexVec[1], indexVec[3]});
            }
        }
    }
    if(F.rows() != faceVec.size()) {
        std::cout << "Error: faceNum != faceVec.size()\n";
        std::cout << "faceNum = " << faceNum << '\n';
        std::cout << "faceVec.size() = " << faceVec.size() << '\n';
    }
    for(int i = 0; i < faceVec.size(); i++) {
        for(int j = 0; j < 3; j++) {
            F(i, j) = faceVec[i][j];
        }
    }

}

void VolumeSplineProcess::BuildSurfacetoFileOFF(const std::string &filename, index_t num,
                                                bool withColor, bool invertNormal)
{
    gsInfo << "Building model to file...\n";

    std::vector<std::array<index_t, 3>> colors = {
        {255, 0, 0},
        {0, 255, 0},
        {0, 0, 255},
        {255, 255, 0},
        {255, 0, 255},
        {0, 255, 255}
    };
    Eigen::MatrixXd V;
    Eigen::MatrixXi F;
    std::map<int, int> faceIndexMap;
    BuildSurfacetoMatrix(V, F, &faceIndexMap, num, invertNormal);

    std::fstream fileOut(filename, std::ios::out);
    fileOut << "OFF\n";
    fileOut << V.rows() << " " << F.rows() << " 0\n";
    fileOut << std::setprecision(std::numeric_limits<long double>::digits10);
    for(index_t i = 0; i < V.rows(); i++) {
        fileOut << V(i, 0) << " " << V(i, 1) << " " << V(i, 2) << '\n';
    }
    for(index_t i = 0; i < F.rows(); i++) {
        fileOut << "3 " << F(i, 0) << " " << F(i, 1) << " " << F(i, 2);
        if(withColor) {
            auto color = colors[faceIndexMap[i] % colors.size()];
            fileOut << " " << color[0] << " " << color[1] << " " << color[2];
        }
        fileOut << '\n';
    }
    fileOut.close();

    gsInfo << "Building model done.\n";
}

void SurfaceSplineProcess::BuildSurfacetoMatrix(Eigen::MatrixXd &V, Eigen::MatrixXi &F,
                                                std::map<int, int> *faceIndexMap,
                                                index_t numSample, bool invertNormal)
{
    int pointNum = numSample + 1;
    int pointSumNum = pointNum * pointNum;

    auto support = _spline_ptr->support();
    gismo::gsMatrix<> param(DIM, pointSumNum);
    std::map< std::array<int, 2>, int> indexMap;
    int index = 0;
    for(int i = 0; i < pointNum; i++) {
        for(int j = 0; j < pointNum; j++) {
            param(0, index) = support(0, 0) + i / (double)numSample * (support(0, 1) - support(0, 0));
            param(1, index) = support(1, 0) + j / (double)numSample * (support(1, 1) - support(1, 0));
            indexMap[{i, j}] = index;
            index++;
        }
    }

    gismo::gsMatrix<> resultValue(3, pointSumNum);
    _spline_ptr->eval_into(param, resultValue);

    int faceNum = numSample * numSample * 2;
    V.resize(pointSumNum, 3);
    for(int i = 0; i < pointSumNum; i++) {
        for(int j = 0; j < 3; j++) {
            V(i, j) = resultValue(j, i);
        }
    }
    F.resize(faceNum, 3);

    std::vector<std::array<int, 4>> faceVec;
    std::array<int, 4> indexVec;
    for(int i = 0; i < numSample; i++) {
        for(int j = 0; j < numSample; j++) {
            indexVec[0] = indexMap[{i, j}];
            indexVec[1] = indexMap[{i + 1, j}];
            indexVec[2] = indexMap[{i, j + 1}];
            indexVec[3] = indexMap[{i + 1, j + 1}];
            // std::cout << "indexVec = " << indexVec[0] << " " << indexVec[1] << " " << indexVec[2] << " " << indexVec[3] << '\n';
            if(invertNormal){
                std::swap(indexVec[0], indexVec[3]);
            }
            faceVec.push_back({indexVec[0], indexVec[3], indexVec[2]});
            faceVec.push_back({indexVec[0], indexVec[1], indexVec[3]});
        }
    }
    
    if(F.rows() != faceVec.size()) {
        std::cout << "Error: faceNum != faceVec.size()\n";
        std::cout << "faceNum = " << faceNum << '\n';
        std::cout << "faceVec.size() = " << faceVec.size() << '\n';
    }
    for(int i = 0; i < faceVec.size(); i++) {
        for(int j = 0; j < 3; j++) {
            F(i, j) = faceVec[i][j];
        }
    }
}
void SurfaceSplineProcess::BuildSurfacetoFileOFF(const std::string &filename, index_t num,
                                                 bool withColor, bool invertNormal)
{
    gsInfo << "Building model to file...\n";

    std::array<index_t, 3> color = { 255, 0, 0 };
    Eigen::MatrixXd V;
    Eigen::MatrixXi F;
    BuildSurfacetoMatrix(V, F, nullptr, num, invertNormal);

    std::fstream fileOut(filename, std::ios::out);
    fileOut << "OFF\n";
    fileOut << V.rows() << " " << F.rows() << " 0\n";
    fileOut << std::setprecision(std::numeric_limits<long double>::digits10);
    for(index_t i = 0; i < V.rows(); i++) {
        fileOut << V(i, 0) << " " << V(i, 1) << " " << V(i, 2) << '\n';
    }
    for(index_t i = 0; i < F.rows(); i++) {
        fileOut << "3 " << F(i, 0) << " " << F(i, 1) << " " << F(i, 2);
        if(withColor) {
            fileOut << " " << color[0] << " " << color[1] << " " << color[2];
        }
        fileOut << '\n';
    }
    fileOut.close();

    gsInfo << "Building model done.\n";
}
