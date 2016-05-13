#ifndef _PreProcess_h
#define _PreProcess_h

#include <Image.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkSigmoidImageFilter.h>
#include <itkRescaleIntensityImageFilter.h>
#include <itkCurvatureAnisotropicDiffusionImageFilter.h>
#include <itkTimeProbe.h>


class PreProcess
{
    
public:
    
    PreProcess();
    ~PreProcess();
    
    //Main Run function to run all the functions
    void RunPreProcess(std::string inputFilename, std::string outputFilename, float lowerThreshold, float upperThreshold, float alpha, float beta);
    
protected:
    
private:
    
    typedef itk::Image<float, 3>  ImageType;
    
    //Original Input Image
    ImageType::Pointer OriginalImage;
    
    //Function for non-linearly remapping the image
    ImageType::Pointer nonLinearIntensityRemap(float lowerThreshold, float upperThreshold, float alpha, float beta);
    
    //Function to smooth the image
    ImageType::Pointer SmoothImage(ImageType::Pointer remappedImage);
    
    
};


#endif
