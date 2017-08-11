/*
 * Copyright (C) 2005-2017 Centre National d'Etudes Spatiales (CNES)
 *
 * This file is part of Orfeo Toolbox
 *
 *     https://www.orfeo-toolbox.org/
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "otbClassKMeansBase.h"

namespace otb
{
namespace Wrapper
{

class KMeansClassification: public ClassKMeansBase
{
public:
  /** Standard class typedefs. */
  typedef KMeansClassification Self;
  typedef ClassKMeansBase Superclass;
  typedef itk::SmartPointer<Self> Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

  /** Standard macro */
  itkNewMacro(Self);

  itkTypeMacro(Self, Superclass);

private:
  void DoInit() ITK_OVERRIDE
  {
    SetName("KMeansClassification");
    SetDescription("Unsupervised KMeans image classification");

    SetDocName("Unsupervised KMeans image classification");
    SetDocLongDescription("Performs unsupervised KMeans image classification."
      "KMeansClassification is a composite application, "
      "using an existing training and classification application."
      "The SharkKMeans model is used.\n"
      "The steps of this composite application :\n"
        "1) ImageEnveloppe : create a shapefile (1 polygon),\n"
        "2) PolygonClassStatistics : compute the statistics,\n"
        "3) SampleSelection : select the samples by constant strategy in the shapefile "
            "(1000000 samples max),\n"
        "4) SamplesExtraction : extract the samples descriptors (update of SampleSelection output file),\n"
        "5) ComputeImagesStatistics : compute images second order statistics,\n"
        "6) TrainVectorClassifier : train the SharkKMeans model,\n"
        "7) ImageClassifier : performs the classification of the input image "
            "according to a model file.\n\n"
        "It's possible to choice random/periodic modes of the SampleSelection application.\n"
        "If you want keep the temporary files (sample selected, model file, ...), "
        "initialize cleanup parameter.\n"
        "For more information on shark KMeans algorithm [1].");

    SetDocLimitations("None");
    SetDocAuthors("OTB-Team");
    SetDocSeeAlso("ImageEnveloppe PolygonClassStatistics SampleSelection SamplesExtraction "
      "PolygonClassStatistics TrainVectorClassifier ImageClassifier\n"
      "[1] http://image.diku.dk/shark/sphinx_pages/build/html/rest_sources/tutorials/algorithms/kmeans.html");

    AddDocTag(Tags::Learning);
    AddDocTag(Tags::Segmentation);

    // Perform initialization
    ClearApplications();

    // initialisation parameters and synchronizes parameters
    initKMParams();

    AddRANDParameter();

    // Doc example parameter settings
    SetDocExampleParameterValue("in", "QB_1_ortho.tif");
    SetDocExampleParameterValue("ts", "1000");
    SetDocExampleParameterValue("nc", "5");
    SetDocExampleParameterValue("maxit", "1000");
    SetDocExampleParameterValue("out", "ClassificationFilterOutput.tif uint8");

    SetOfficialDocLink();
  }

  void DoUpdateParameters() ITK_OVERRIDE
  {
  }

  void DoExecute() ITK_OVERRIDE
  {
    if (IsParameterEnabled("vm") && HasValue("vm")) ConnectKMClassificationMask();

    KMeansFileNamesHandler fileNames;

    const std::string fieldName = "field";

    fileNames.CreateTemporaryFileNames(GetParameterString( "out" ));

    // Create an image envelope
    ComputeImageEnvelope(fileNames.tmpVectorFile);
    // Add a new field at the ImageEnvelope output file
    ComputeAddField(fileNames.tmpVectorFile, fieldName);

    // Compute PolygonStatistics app
    UpdateKMPolygonClassStatisticsParameters(fileNames.tmpVectorFile);
    ComputePolygonStatistics(fileNames.polyStatOutput, fieldName);

    // Compute number of sample max for KMeans
    const int theoricNBSamplesForKMeans = GetParameterInt("ts");
    const int upperThresholdNBSamplesForKMeans = 1000 * 1000;
    const int actualNBSamplesForKMeans = std::min(theoricNBSamplesForKMeans,
                                                  upperThresholdNBSamplesForKMeans);
    otbAppLogINFO(<< actualNBSamplesForKMeans << " is the maximum sample size that will be used." \
                  << std::endl);

    // Compute SampleSelection and SampleExtraction app
    SelectAndExtractSamples(fileNames.polyStatOutput, fieldName,
                            fileNames.sampleOutput,
                            actualNBSamplesForKMeans);

    // Compute Images second order statistics
    ComputeImageStatistics(GetParameterString("in"), fileNames.imgStatOutput);

    // Compute a train model with TrainVectorClassifier app
    TrainKMModel(GetParameterImage("in"), fileNames.sampleOutput,
                 fileNames.modelFile);

    // Compute a classification of the input image according to a model file
    KMeansClassif();

    // Create the output text file containing centroids positions
    CreateOutMeansFile(GetParameterImage("in"), fileNames.modelFile, GetParameterInt("nc"));

    // Remove all tempory files
    if( IsParameterEnabled( "cleanup" ) )
      {
      otbAppLogINFO( <<"Final clean-up ..." );
      fileNames.clear();
      }
  }

private :
  void UpdateKMPolygonClassStatisticsParameters(const std::string &vectorFileName)
  {
    GetInternalApplication( "polystats" )->SetParameterString( "vec", vectorFileName, false );
    UpdateInternalParameters( "polystats" );
  }

};

}
}

OTB_APPLICATION_EXPORT(otb::Wrapper::KMeansClassification)

