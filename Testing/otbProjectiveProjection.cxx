/*=========================================================================

  Program:   ORFEO Toolbox
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


  Copyright (c) Centre National d'Etudes Spatiales. All rights reserved.
  See OTBCopyright.txt for details.


  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "otbImage.h"
#include "otbVectorImage.h"
#include "otbImageFileReader.h"
#include "otbImageFileWriter.h"

#include "otbDotProductImageFilter.h"
#include "otbProjectiveProjectionImageFilter.h"
#include "otbMatrixMultiplyImageFilter.h"
#include "otbVectorImageToMatrixImageFilter.h"
#include "otbStreamingStatisticsImageFilter.h"
#include "otbStreamingStatisticsVectorImageFilter2.h"

const unsigned int Dimension = 2;
typedef double PixelType;
typedef double PrecisionType;

typedef otb::Image<PixelType, Dimension> ImageType;
typedef otb::VectorImage<PixelType, Dimension> VectorImageType;
typedef otb::ImageFileReader<VectorImageType> ReaderType;
typedef otb::ProjectiveProjectionImageFilter<VectorImageType,VectorImageType,PrecisionType> ProjectiveProjectionImageFilterType;
typedef otb::DotProductImageFilter<VectorImageType,ImageType> DotProductImageFilterType;
typedef otb::MatrixMultiplyImageFilter<VectorImageType,VectorImageType,PrecisionType> MatrixMultiplyImageFilterType;
typedef otb::VectorImageToMatrixImageFilter<VectorImageType> VectorImageToMatrixImageFilterType;
typedef otb::ImageFileWriter<VectorImageType> WriterType;
typedef otb::StreamingStatisticsVectorImageFilter2<VectorImageType> StreamingStatisticsVectorImageFilterType;
typedef otb::StreamingStatisticsImageFilter<ImageType> StreamingStatisticsImageFilterType;

typedef StreamingStatisticsVectorImageFilterType::MatrixType MatrixType;

int otbProjectiveProjectionTestHighSNR(int argc, char * argv[])
{
  const char * inputImage = argv[1];
  const unsigned int nbEndmembers = atoi(argv[2]);
  const char * outputImage = argv[3];


  ReaderType::Pointer readerImage = ReaderType::New();
  readerImage->SetFileName(inputImage);

  std::cout << "Computing image stats" << std::endl;
  StreamingStatisticsVectorImageFilterType::Pointer statsInput = \
      StreamingStatisticsVectorImageFilterType::New();

  statsInput->SetInput(readerImage->GetOutput());
  statsInput->Update();

  std::cout << "Computing SVD of correlation matrix" << std::endl;
  // Take the correlation matrix
  vnl_matrix<PrecisionType> R = statsInput->GetCorrelation().GetVnlMatrix();

  // Apply SVD
  vnl_svd<PrecisionType>    svd(R);
  vnl_matrix<PrecisionType> U = svd.U();
  std::cout << "U : "<< U.rows() << " " << U.cols() << std::endl;
  vnl_matrix<PrecisionType> Ud = U.get_n_columns(0, nbEndmembers).transpose();
  std::cout << "Ud : "<< Ud.rows() << " " << Ud.cols() << std::endl;

  std::cout << "Apply dimensionnality reduction" << std::endl;
  // Xd = Ud.'*M;
  MatrixMultiplyImageFilterType::Pointer mulUd = MatrixMultiplyImageFilterType::New();
  mulUd->SetInput(readerImage->GetOutput());
  mulUd->SetMatrix(Ud);
  mulUd->UpdateOutputInformation();

  VectorImageType::Pointer Xd = mulUd->GetOutput();
  std::cout << "Xd = " << Xd << std::endl;

/*
  std::cout << "Write Xd output" << std::endl;
  WriterType::Pointer writer = WriterType::New();
  writer->SetFileName(outputImage);
  wrtier->SetInput(Xd);
  writer->Update();
*/

  // Compute mean(Xd)
  std::cout << "Compute mean(Xd)" << std::endl;
  StreamingStatisticsVectorImageFilterType::Pointer statsXd = \
      StreamingStatisticsVectorImageFilterType::New();
  statsXd->SetInput(Xd);
  statsXd->Update();
  VectorImageType::PixelType Xdmean = statsXd->GetMean();
  std::cout << "mean(Xd) = " << Xdmean << std::endl;
  std::cout << "mean(Xd) size = " << Xdmean.Size() << std::endl;

  // Compute Xd ./ repmat( sum( Xd .* repmat(u,[1 N]) ) ,[d 1]);
  // -> divides each pixel component by the dot product <Xd(i,j), mean(Xd)>
  std::cout << "Compute projective projection" << std::endl;
  ProjectiveProjectionImageFilterType::Pointer proj = ProjectiveProjectionImageFilterType::New();
  proj->SetInput(Xd);
  proj->SetProjectionDirection(Xdmean);

  std::cout << "Write output" << std::endl;
  WriterType::Pointer writer = WriterType::New();
  writer->SetFileName(outputImage);
  writer->SetInput(proj->GetOutput());
  writer->Update();

  return EXIT_SUCCESS;
}
