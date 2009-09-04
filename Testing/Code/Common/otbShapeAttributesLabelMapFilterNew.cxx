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

//#include "itkLabelMap.h"
#include "otbAttributesMapLabelObject.h"
//#include "itkLabelMap.h"
#include "otbShapeAttributesLabelMapFilter.h"
// #include "itkLabelMap.h"
int otbShapeAttributesLabelMapFilterNew(int argc, char * argv[])
{
  const unsigned int Dimension = 2;
  typedef unsigned short                         LabelType;
  
  typedef otb::AttributesMapLabelObject<LabelType,Dimension,double>      LabelObjectType;
  typedef itk::LabelMap<LabelObjectType>                                 LabelMapType;
  typedef otb::ShapeAttributesLabelMapFilter<LabelMapType>               ShapeLabelMapFilterType;
  
  // Instantiation
  ShapeLabelMapFilterType::Pointer ShapeLabelMapFilter = ShapeLabelMapFilterType::New();

  return EXIT_SUCCESS;
}
