
#include "PreProcess.h"


int main( int argc, char * argv[] )
{
    if( argc < 9 )
    {
        std::cerr << "Usage: " << std::endl;
        std::cerr << argv[0] << " inputImageFile outputFilename lowerThreshold upperThreshold alpha beta conductance numberOfIterations" << std::endl;
        return EXIT_FAILURE;
    }

    std::string inputFilename = argv[1];
    std::string outputFilename = argv[2];
    
    float lowerThreshold = atof(argv[3]);
    float upperThreshold = atof(argv[4]);
    
    float alpha = atof(argv[5]);
    float beta = atof(argv[6]);
    
    float   conductance = atof(argv[7]);
    int     numberOfIterations = atoi(argv[8]);
    
    PreProcess preProcess;
    preProcess.RunPreProcess(inputFilename, outputFilename, lowerThreshold, upperThreshold, alpha, beta, conductance, numberOfIterations);

    
    return EXIT_SUCCESS;
}
