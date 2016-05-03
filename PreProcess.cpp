#include "PreProcess.h"


PreProcess::PreProcess()
{
    //Constructor
    
}

PreProcess::~PreProcess()
{
    //Destructor
}

PreProcess::ImageType::Pointer PreProcess::nonLinearIntensityRemap(float lowerThreshold, float upperThreshold, float alpha, float beta)
{
    
    typedef itk::SigmoidImageFilter< ImageType, ImageType >  SigmoidFilterType;
    SigmoidFilterType::Pointer sigmoidFilter = SigmoidFilterType::New();
    sigmoidFilter->SetInput( OriginalImage );
    sigmoidFilter->SetOutputMinimum(   lowerThreshold  );
    sigmoidFilter->SetOutputMaximum(   upperThreshold  );
    sigmoidFilter->SetAlpha(  alpha  );
    sigmoidFilter->SetBeta(  beta );
    
    typedef itk::RescaleIntensityImageFilter< ImageType, ImageType > RescaleFilter;
    RescaleFilter::Pointer rescale = RescaleFilter::New();
    rescale->SetInput( sigmoidFilter->GetOutput() );
    rescale->SetOutputMinimum(0.0f);
    rescale->SetOutputMaximum(255.0f);
    rescale->Update();
    
    
    return rescale->GetOutput();
}

PreProcess::ImageType::Pointer PreProcess::SmoothImage(ImageType::Pointer remappedImage)
{
    typedef   itk::CurvatureAnisotropicDiffusionImageFilter< ImageType, ImageType >  SmoothingFilterType;
    SmoothingFilterType::Pointer smoothing = SmoothingFilterType::New();
    smoothing->SetInput( remappedImage );
    smoothing->SetTimeStep( 0.04 );
    smoothing->SetNumberOfIterations(  5 );
    smoothing->SetConductanceParameter( 9.0 );
    smoothing->Update();
    
    typedef itk::RescaleIntensityImageFilter< ImageType, ImageType > RescaleFilter;
    RescaleFilter::Pointer rescale = RescaleFilter::New();
    rescale->SetInput( smoothing->GetOutput() );
    rescale->SetOutputMinimum(0.0f);
    rescale->SetOutputMaximum(255.0f);
    rescale->Update();
    
    
    return rescale->GetOutput();
}


void PreProcess::RunPreProcess(std::string inputFilename, std::string outputFilename, float lowerThreshold, float upperThreshold, float alpha, float beta)
{
  
    //Read Input Image
    typedef itk::ImageFileReader< ImageType > ReaderType;
    ReaderType::Pointer reader  = ReaderType::New();
    reader->SetFileName(inputFilename);
    reader->Update();
    
    OriginalImage = reader->GetOutput();
    
    //Calculate remapped image to get the vessel intensiy range enhanced
    ImageType::Pointer remappedImage = nonLinearIntensityRemap(lowerThreshold, upperThreshold, alpha, beta);
    
    //smooth the image
    ImageType::Pointer smoothedImage = SmoothImage(remappedImage);
    
    typedef itk::ImageFileWriter<ImageType> WriterType;
    WriterType::Pointer writer = WriterType::New();
    writer->SetInput( smoothedImage );
    writer->SetFileName(outputFilename);
    writer->Update();
    
}
