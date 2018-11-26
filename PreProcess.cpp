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

//PreProcess::ImageType::Pointer PreProcess::curvatureSmoothImage(ImageType::Pointer remappedImage, float conductance, int numberOfIterations)
//{
//    const ImageType::SpacingType& sp = remappedImage->GetSpacing();
//    float timeStep = sp[2]/(powf(2, 4));        //4 = Dimension + 1
//    std::cout<<"TimeStep = "<<timeStep<<std::endl;
//
//    int numberOfIterations;
//    if (sp[2] >= 1.0f) {
//        numberOfIterations = 5;
//    }
//    else {
//        numberOfIterations = 10;
//    }
//
//    typedef   itk::CurvatureAnisotropicDiffusionImageFilter< ImageType, ImageType >  SmoothingFilterType;
//    SmoothingFilterType::Pointer smoothing = SmoothingFilterType::New();
//    smoothing->SetInput( remappedImage );
//    smoothing->SetTimeStep( timeStep );
//    smoothing->SetNumberOfIterations(  5  );
//    smoothing->SetConductanceParameter( 9.0 );
//    smoothing->Update();
//
//    typedef itk::RescaleIntensityImageFilter< ImageType, ImageType > RescaleFilter;
//    RescaleFilter::Pointer rescale = RescaleFilter::New();
//    rescale->SetInput( smoothing->GetOutput() );
//    rescale->SetOutputMinimum(0.0f);
//    rescale->SetOutputMaximum(255.0f);
//    rescale->Update();
//
//
//    return rescale->GetOutput();
//}

PreProcess::ImageType::Pointer PreProcess::gradientSmoothImage(ImageType::Pointer remappedImage, float conductance, int numberOfIterations)
{
    const ImageType::SpacingType& sp = remappedImage->GetSpacing();
    float min_Spacing = sp[0];
    if (min_Spacing > sp[1]) {
        min_Spacing = sp[1];
    }
    if (min_Spacing > sp[2]){
        min_Spacing = sp[2];
    }
    float timeStep = min_Spacing/(powf(2, 4));        //4 = Dimension + 1
//    int numberOfIterations = 10; //30
//    float conductance = 8.0f;   //30

    
    //GPU Smoothing
    typedef itk::GPUGradientAnisotropicDiffusionImageFilter< ImageType, ImageType > GPUAnisoDiffFilterType;
    typename GPUAnisoDiffFilterType::Pointer smoothing = GPUAnisoDiffFilterType::New();
    smoothing->SetInput( remappedImage );
    smoothing->SetNumberOfIterations( numberOfIterations );
    smoothing->SetTimeStep( timeStep );
    smoothing->SetConductanceParameter( conductance );
    smoothing->UseImageSpacingOn();
    smoothing->Update();
    
//    typedef   itk::GradientAnisotropicDiffusionImageFilter< ImageType, ImageType >  SmoothingFilterType;
//    SmoothingFilterType::Pointer smoothing = SmoothingFilterType::New();
//    smoothing->SetInput( remappedImage );
//    smoothing->SetTimeStep( timeStep );
//    smoothing->SetNumberOfIterations(  numberOfIterations  );
//    smoothing->SetConductanceParameter( 9.0 );
//    smoothing->Update();
    
    typedef itk::RescaleIntensityImageFilter< ImageType, ImageType > RescaleFilter;
    RescaleFilter::Pointer rescale = RescaleFilter::New();
    rescale->SetInput( smoothing->GetOutput() );
    rescale->SetOutputMinimum(0.0f);
    rescale->SetOutputMaximum(255.0f);
    rescale->Update();
    
    
    return rescale->GetOutput();
}

PreProcess::ImageType::Pointer PreProcess::resampleImage(ImageType::Pointer smoothedImage)
{
    std::cout << "resampleImage" << std::endl;
    
    typedef itk::Image<ImageType::PixelType,3> CPUImageType;
    
    typedef itk::ResampleImageFilter< CPUImageType, ImageType >  FilterType;
    FilterType::Pointer filter = FilterType::New();
    
    typedef itk::AffineTransform< double, 3 >  TransformType;
    TransformType::Pointer transform = TransformType::New();
    //transform->SetIdentity();
    filter->SetTransform( transform );
    
    typedef itk::LinearInterpolateImageFunction< CPUImageType, double >  InterpolatorType;
    InterpolatorType::Pointer interpolator = InterpolatorType::New();
    filter->SetInterpolator( interpolator );
    
    filter->SetDefaultPixelValue( 0 );
    
    const ImageType::SpacingType& sp = smoothedImage->GetSpacing();
    float min_Spacing = sp[0];
    if (min_Spacing > sp[1]) {
        min_Spacing = sp[1];
    }
    if (min_Spacing > sp[2]){
        min_Spacing = sp[2];
    }
    ImageType::SpacingType newSp;
    newSp[0] = min_Spacing;
    newSp[1] = min_Spacing;
    newSp[2] = min_Spacing;
    std::cout<<"new spacing"<<newSp<<std::endl;
    
    filter->SetOutputSpacing( newSp );
    filter->SetOutputOrigin( smoothedImage->GetOrigin() );
    
    ImageType::DirectionType direction;
    direction.SetIdentity();
    filter->SetOutputDirection( direction );
    
    ImageType::SizeType   NewSize;
    NewSize[0] = int((double(smoothedImage->GetLargestPossibleRegion().GetSize()[0])*double(smoothedImage->GetSpacing()[0]))/double(newSp[0]));  // number of pixels along X
    NewSize[1] = int((double(smoothedImage->GetLargestPossibleRegion().GetSize()[1])*double(smoothedImage->GetSpacing()[1]))/double(newSp[1]));  // number of pixels along Y
    NewSize[2] = int((double(smoothedImage->GetLargestPossibleRegion().GetSize()[2])*double(smoothedImage->GetSpacing()[2]))/double(newSp[2]));  // number of pixels along Z
    std::cout<<"new size"<<NewSize<<std::endl;
    filter->SetSize( NewSize );
    
    filter->SetInput( smoothedImage );
    filter->Update();
    std::cout<<"Done resampleImage"<<std::endl;
    
    return filter->GetOutput();
}


void PreProcess::RunPreProcess(std::string inputFilename, std::string outputFilename, float lowerThreshold, float upperThreshold, float alpha, float beta, float conductance, int numberOfIterations)
{
    //Read Input Image
    typedef itk::ImageFileReader< ImageType > ReaderType;
    ReaderType::Pointer reader  = ReaderType::New();
    reader->SetFileName(inputFilename);
    reader->Update();
    
    OriginalImage = reader->GetOutput();
    
    
    itk::TimeProbe clock1;
    clock1.Start();
    
    //Calculate remapped image to get the vessel intensiy range enhanced
    std::cout<< std::endl <<"Intensity Remapping ..."<<std::endl;
    ImageType::Pointer remappedImage = nonLinearIntensityRemap(lowerThreshold, upperThreshold, alpha, beta);
    std::cout<< " Done (1/3)"<< std::endl;
    
    //smooth the image
    std::cout<< std::endl <<"Smoothing ..."<<std::endl;
    ImageType::Pointer smoothedImage = gradientSmoothImage(remappedImage, conductance, numberOfIterations);
    std::cout<< " Done (2/3)"<< std::endl;
    
    // resample the smoothed image to isotropic image voxels
    std::cout<< std::endl <<"Resampling ..."<<std::endl;
    ImageType::Pointer resampledImage = resampleImage(smoothedImage);
    std::cout<< " Done (3/3)"<< std::endl;
    
    clock1.Stop();
    std::cout<< std::endl<<"Total Time taken for Preprocessing Image = "<< clock1.GetMean() <<"sec\n"<< std::endl;
    
    
    typedef itk::ImageFileWriter<ImageType> WriterType;
    WriterType::Pointer writer = WriterType::New();
    writer->SetInput( resampledImage );
    writer->SetFileName(outputFilename);
    writer->Update();
    
}
