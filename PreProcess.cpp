//
//  PreProcess.cpp
//  
//
//  Created by Rahul Kumar on 07/07/2014.
//
//

#include "PreProcess.h"


int main( int argc, char * argv[] )
{
    if( argc < 3 )
    {
        std::cerr << "Usage: " << std::endl;
        std::cerr << argv[0] << " inputImageFile outputFilename [no_iterators_smoothing] [resample_spacing]" << std::endl;
        return EXIT_FAILURE;
    }

    
    clock_t start, end; 
	double elapsed;
	int min;
	double sec;
	start = clock();

    const unsigned int Dimension = 3;
	typedef float      PixelType;
    typedef itk::Image< PixelType, Dimension>            ImageType;
	typedef itk::ImageFileReader< ImageType  >      ImageReaderType;
    
	
	ImageReaderType::Pointer   reader = ImageReaderType::New();
	reader->SetFileName ( argv[1] );
    reader->Update();
    
    ImageType::Pointer Image = ImageType::New();
    Image->SetRegions( reader->GetOutput()->GetRequestedRegion() );
    Image->CopyInformation( reader->GetOutput() );
    Image->Allocate();
    Image->FillBuffer(0.0f);
    
    
    //Below-Zero elimination
    typedef itk::ImageRegionIteratorWithIndex<ImageType> IteratorType;
    IteratorType itin(reader->GetOutput(), reader->GetOutput()->GetRequestedRegion());
    IteratorType itout(Image, Image->GetRequestedRegion());
    itin.GoToBegin();
    itout.GoToBegin();
    while (!itin.IsAtEnd()) {
        if (itin.Get()<0.0f) {
            itout.Set(0.0f);
        }
        else
            itout.Set(itin.Get());
        ++itin;
        ++itout;
    }
    
    //Rescale to 0.0f-255.0f
    typedef itk::RescaleIntensityImageFilter< ImageType,ImageType>	RescaleFilterType;
	RescaleFilterType::Pointer rescale = RescaleFilterType::New();
	rescale->SetInput( Image );
	rescale->SetOutputMinimum(   0 );
	rescale->SetOutputMaximum( 255 );
	rescale->Update();
    
    
    //CurvatureFlow Smoothing
    typedef itk::CurvatureFlowImageFilter< ImageType, ImageType >CurvatureFlowImageFilterType;
    CurvatureFlowImageFilterType::Pointer smoothing = CurvatureFlowImageFilterType::New();
    int iterations = 5;
//    if (argc > 3)
//    {
//        iterations = atoi(argv[3]);
//    }
    smoothing->SetInput( rescale->GetOutput() );
    smoothing->SetNumberOfIterations( iterations );
    smoothing->SetTimeStep( 0.1025 );
    smoothing->Update();
    
    
    //Mean Image Filter
    typedef itk::MeanImageFilter< ImageType, ImageType >  MeanType;
    MeanType::Pointer mean = MeanType::New();
    ImageType::SizeType indexRadius;
    indexRadius[0] = 1; // radius along x
    indexRadius[1] = 1; // radius along y
    indexRadius[2] = 1; // radius along z
    mean->SetRadius( indexRadius );
    mean->SetInput( smoothing->GetOutput() );
    mean->Update();

    
    //Rescample Image to isotrpic 1mm spacing
    typedef itk::ResampleImageFilter< ImageType, ImageType >  ResampleType;
    ResampleType::Pointer resample = ResampleType::New();
    typedef itk::AffineTransform< double, Dimension >  TransformType;
    TransformType::Pointer transform = TransformType::New();
    typedef itk::LinearInterpolateImageFunction< ImageType, double >  InterpolatorType;
    InterpolatorType::Pointer interpolator = InterpolatorType::New();
    resample->SetInterpolator( interpolator );
    resample->SetDefaultPixelValue( 0 );
    double spacing[ Dimension ];
    spacing[0] = 1.0f; // pixel spacing in millimeters along X
    spacing[1] = 1.0f; // pixel spacing in millimeters along Y
    spacing[2] = 1.0f; // pixel spacing in millimeters along Z
//    if (argc > 4) {
//        spacing[0] = atof(argv[4]); // pixel spacing in millimeters along X
//        spacing[1] = atof(argv[4]); // pixel spacing in millimeters along Y
//        spacing[2] = atof(argv[4]); // pixel spacing in millimeters along Z
//    }
    resample->SetOutputSpacing( spacing );
    resample->SetOutputDirection( mean->GetOutput()->GetDirection() );
    double origin[ Dimension ];
    origin[0] = mean->GetOutput()->GetOrigin()[0];
    origin[1] = mean->GetOutput()->GetOrigin()[1];
    origin[2] = mean->GetOutput()->GetOrigin()[2];
    resample->SetOutputOrigin( origin );
    ImageType::SizeType   size;
    size[0] = int((double(mean->GetOutput()->GetLargestPossibleRegion().GetSize()[0])*double(mean->GetOutput()->GetSpacing()[0]))/double(spacing[0]));  // number of pixels along X
    size[1] = int((double(mean->GetOutput()->GetLargestPossibleRegion().GetSize()[1])*double(mean->GetOutput()->GetSpacing()[1]))/double(spacing[1]));  // number of pixels along Y
    size[2] = int((double(mean->GetOutput()->GetLargestPossibleRegion().GetSize()[2])*double(mean->GetOutput()->GetSpacing()[2]))/double(spacing[2]));  // number of pixels along Z
    resample->SetSize( size );
    resample->SetInput( mean->GetOutput() );
    transform->SetIdentity();
    resample->SetTransform( transform );
    resample->Update();
    
    //Rescale to 0.0f-255.0f
    RescaleFilterType::Pointer rescaler = RescaleFilterType::New();
    rescaler->SetOutputMinimum(   0 );
    rescaler->SetOutputMaximum( 255 );
    rescaler->SetInput( resample->GetOutput() );
    rescaler->Update();
    
    
    //Write Output_PreProcessed_Image
    typedef itk::ImageFileWriter< ImageType >  WriterType;
    WriterType::Pointer writer = WriterType::New();
    writer->SetFileName( argv[2] );
    writer->SetInput( rescaler->GetOutput() );
    writer->Update();
    
    end = clock();
	elapsed = ((double) (end - start)) / CLOCKS_PER_SEC;
	min=(int)(elapsed/60);
	sec=elapsed-double(min*60);
    std::cout<<"Time taken for Pre-Processing         : "<<min<<"min "<<sec<<"sec\n"<<std::endl;
    
    return EXIT_SUCCESS;
}