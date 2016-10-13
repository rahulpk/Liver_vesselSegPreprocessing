#ifndef _PreProcess_h
#define _PreProcess_h

#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkSigmoidImageFilter.h>
#include <itkRescaleIntensityImageFilter.h>
#include <itkCurvatureAnisotropicDiffusionImageFilter.h>
#include <itkGradientAnisotropicDiffusionImageFilter.h>
#include <itkTimeProbe.h>
#include <itkResampleImageFilter.h>
#include <itkAffineTransform.h>
#include <itkLinearInterpolateImageFunction.h>

#include <itkOpenCLUtil.h>
#include <itkGPUImage.h>
#include <itkGPUKernelManager.h>
#include <itkGPUContextManager.h>
#include <itkGPUImageToImageFilter.h>
#include <itkGPUGradientAnisotropicDiffusionImageFilter.h>


class PreProcess
{
    
public:
    
    PreProcess();
    ~PreProcess();
    
    //Main Run function to run all the functions
    void RunPreProcess(std::string inputFilename, std::string outputFilename, float lowerThreshold, float upperThreshold, float alpha, float beta);
    
protected:
    
private:
    
    typedef itk::GPUImage<float, 3>  ImageType;
    
    //Original Input Image
    ImageType::Pointer OriginalImage;
    
    //Function for non-linearly remapping the image
    ImageType::Pointer nonLinearIntensityRemap(float lowerThreshold, float upperThreshold, float alpha, float beta);
    
    //Function to smooth the image
    ImageType::Pointer curvatureSmoothImage(ImageType::Pointer remappedImage);
    
    //Function to smooth the image
    ImageType::Pointer gradientSmoothImage(ImageType::Pointer remappedImage);
    
    ImageType::Pointer resampleImage(ImageType::Pointer smoothedImage);
    
    
};


#endif
