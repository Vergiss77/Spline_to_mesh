#include <gismo.h>
#include "SplineProcess.h"

int main(int argc, char *argv[])
{
    std::string outputfile("output.off"), inputfile("");
    index_t numSample = 64;
    bool withColor = false;
    bool invertNormal = false;
    bool squareMesh = false;
    bool showFormat = false;

    gsCmdLine cmd("Give me a file (eg: .xml) with Spline and I will try to convert it to mesh!");

    cmd.addPlainString("filename", "File containing spline to convert (.xml)", inputfile);
    cmd.addString("o", "oname", "Output file name", outputfile);
    cmd.addInt("n", "num", "Number of samples to use for building the model", numSample);
    cmd.addSwitch("color", "Use color for the model", withColor);
    cmd.addSwitch("invert", "Invert the color of the model", invertNormal);
    cmd.addSwitch("square", "Use square mesh instead of triangle mesh", squareMesh);
    cmd.addSwitch("showFormat", "Show supported export formats", showFormat);

    try { cmd.getValues(argc,argv); } catch (int rv) { return rv; }

    if (showFormat)
    {
        BasisSplineProcess basisSplineProcess;
        basisSplineProcess.ShowExportFormatsSupported();
    }

    if ( inputfile.empty() )
    {
        gsInfo<< cmd.getMessage();
        gsInfo<<"\nType "<< argv[0]<< " -h, to get the list of command line options.\n";
        return EXIT_FAILURE;
    }

    OptionFlag optionFlag = static_cast<OptionFlag>(0);
    optionFlag = withColor ? static_cast<OptionFlag>(optionFlag | WITH_COLOR) : optionFlag;
    optionFlag = invertNormal ? static_cast<OptionFlag>(optionFlag | INVERT_NORMAL) : optionFlag;
    MeshType meshType = squareMesh ? SQUARE_MESH : TRIANGLE_MESH;
    BasisSplineProcess basisSplineProcess(optionFlag, meshType);
    if(!basisSplineProcess.LoadSplinefromFile(inputfile)) {
        gsInfo << "Failed to load spline from file: " << inputfile << "\n";
        return EXIT_FAILURE;
    }
    std::unique_ptr<BasisSplineProcess> splineProcessPtr;
    if(basisSplineProcess.GetDimension() == 3) {
        splineProcessPtr = std::make_unique<VolumeSplineProcess>(basisSplineProcess);
    } else if(basisSplineProcess.GetDimension() == 2) {
        splineProcessPtr = std::make_unique<SurfaceSplineProcess>(basisSplineProcess);
    } else {
        gsInfo << "Unsupported dimension: " << basisSplineProcess.GetDimension() << "\n";
        return EXIT_FAILURE;
    }
    gsInfo << "Spline dimension: " << splineProcessPtr->GetDimension() << "\n";
    splineProcessPtr->InitializeMeshStrategy();
    if(!splineProcessPtr->BuildSurfacetoFile(outputfile, numSample)){
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
