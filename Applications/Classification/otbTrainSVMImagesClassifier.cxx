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

#include "otbWrapperApplication.h"
#include "otbWrapperApplicationFactory.h"


#include <iostream>
#include "otbConfigurationFile.h"

//Image
#include "otbImage.h"
#include "otbVectorImage.h"
#include "otbVectorData.h"
#include "otbListSampleGenerator.h"

// ListSample
#include "itkListSample.h"
#include "itkVariableLengthVector.h"
#include "itkFixedArray.h"

// SVM estimator
#include "otbSVMSampleListModelEstimator.h"

// Statistic XML Reader
#include "otbStatisticsXMLFileReader.h"

// Validation
#include "otbSVMClassifier.h"
#include "otbConfusionMatrixCalculator.h"

#include "itkTimeProbe.h"
#include "otbStandardFilterWatcher.h"

// Normalize the samples
#include "otbShiftScaleSampleListFilter.h"

// List sample concatenation
#include "otbConcatenateSampleListFilter.h"

// Balancing ListSample
#include "otbListSampleToBalancedListSampleFilter.h"

// VectorData projection filter
#include "otbVectorDataProjectionFilter.h"

// Extract a ROI of the vectordata
#include "otbVectorDataIntoImageProjectionFilter.h"

namespace otb
{
namespace Wrapper
{

class TrainSVMImagesClassifier: public Application
{
public:
  /** Standard class typedefs. */
  typedef TrainSVMImagesClassifier Self;
  typedef Application Superclass;
  typedef itk::SmartPointer<Self> Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

  /** Standard macro */
  itkNewMacro(Self)
;

  itkTypeMacro(TrainSVMImagesClassifier, otb::Application)
;

  typedef otb::Image<FloatVectorImageType::InternalPixelType, 2> ImageReaderType;

  typedef FloatVectorImageType::PixelType PixelType;
  typedef FloatVectorImageType VectorImageType;
  typedef FloatImageType ImageType;

  // Training vectordata
  typedef itk::VariableLengthVector<ImageType::PixelType> MeasurementType;

  // SampleList manipulation
  typedef otb::ListSampleGenerator<VectorImageType, VectorDataType> ListSampleGeneratorType;

  typedef ListSampleGeneratorType::ListSampleType ListSampleType;
  typedef ListSampleGeneratorType::LabelType LabelType;
  typedef ListSampleGeneratorType::ListLabelType LabelListSampleType;
  typedef otb::Statistics::ConcatenateSampleListFilter<ListSampleType> ConcatenateListSampleFilterType;
  typedef otb::Statistics::ConcatenateSampleListFilter<LabelListSampleType> ConcatenateLabelListSampleFilterType;

  // Statistic XML file Reader
  typedef otb::StatisticsXMLFileReader<MeasurementType> StatisticsReader;

  // Enhance List Sample
  typedef otb::Statistics::ListSampleToBalancedListSampleFilter<ListSampleType, LabelListSampleType>
      BalancingListSampleFilterType;
  typedef otb::Statistics::ShiftScaleSampleListFilter<ListSampleType, ListSampleType> ShiftScaleFilterType;

  // SVM Estimator
  typedef otb::Functor::VariableLengthVectorToMeasurementVectorFunctor<MeasurementType> MeasurementVectorFunctorType;
  typedef otb::SVMSampleListModelEstimator<ListSampleType, LabelListSampleType, MeasurementVectorFunctorType>
      SVMEstimatorType;

  typedef otb::SVMClassifier<ListSampleType, LabelType::ValueType> ClassifierType;

  // Estimate performance on validation sample
  typedef otb::ConfusionMatrixCalculator<LabelListSampleType, LabelListSampleType> ConfusionMatrixCalculatorType;
  typedef ClassifierType::OutputType ClassifierOutputType;

  // VectorData projection filter
  typedef otb::VectorDataProjectionFilter<VectorDataType, VectorDataType> VectorDataProjectionFilterType;

  // Extract ROI
  typedef otb::VectorDataIntoImageProjectionFilter<VectorDataType, VectorImageType> VectorDataReprojectionType;

private:
  TrainSVMImagesClassifier()
  {
    SetName("TrainSVMImagesClassifier");
    SetDescription("Perform SVM training from multiple input images and multiple vector data.");

    // Documentation
    SetDocName("Train SVM images Application");
    SetDocLongDescription("This application performs SVM training from multiple input images and multiple vector data.");
    SetDocLimitations("None");
    SetDocAuthors("OTB-Team");
    SetDocSeeAlso(" ");
    SetDocCLExample("otbApplicationLauncherCommandLine TrainSVMImagesClassifier ${OTB-BIN}/bin"
      "--il ${OTB-DATA}/Classification/QB_1_ortho.tif "
      "--vd ${OTB-DATA}/Classification/ectorData_QB1.shp"
      "--imstat ${OTB-Data}/Baseline/OTB-Applications/Files/clImageStatisticsQB1.xml"
      "--b 2 --mv 100 --vtr 0.5 --opt true -out svmModelQB1_allOpt.svm");
    AddDocTag("Classification");
    AddDocTag("Training");
    AddDocTag("SVM");
  }

  virtual ~TrainSVMImagesClassifier()
  {
  }

  void DoCreateParameters()
  {

    AddParameter(ParameterType_InputImageList, "il", "Input Image List");
    SetParameterDescription("il", "a list of input images.");
    AddParameter(ParameterType_InputVectorDataList, "vd", "Vector Data List");
    SetParameterDescription("vd", "A list of vector data sample used to train the estimator.");
    AddParameter(ParameterType_Filename, "dem", "DEM repository");
    MandatoryOff("dem");
    SetParameterDescription("dem", "path to SRTM repository");
    AddParameter(ParameterType_Filename, "imstat", "XML image statistics file");
    MandatoryOff("imstat");
    SetParameterDescription("imstat", "filename of an XML file containing mean and standard deviation of input images.");
    AddParameter(ParameterType_Float, "m", "Margin for SVM learning");
    SetParameterFloat("m", 1.0);
    SetParameterDescription("m", "Margin for SVM learning.(1 by default).");
    AddParameter(ParameterType_Int, "b", "Balance and grow the training set");
    SetParameterDescription("b", "Balance and grow the training set.");
    MandatoryOff("b");
    AddParameter(ParameterType_Choice, "k", "SVM Kernel Type");
    AddChoice("k.linear", "Linear");
    AddChoice("k.rbf", "Neareast Neighbor");
    AddChoice("k.poly", "Polynomial");
    AddChoice("k.sigmoid", "Sigmoid");
    SetParameterString("k", "linear");
    SetParameterDescription("k", "SVM Kernel Type.");
    AddParameter(ParameterType_Int, "mt", "Maximum training sample size");
    //MandatoryOff("mt");
    SetDefaultParameterInt("mt", -1);
    SetParameterDescription("mt", "Maximum size of the training sample (default = -1).");
    AddParameter(ParameterType_Int, "mv", "Maximum validation sample size");
    // MandatoryOff("mv");
    SetDefaultParameterInt("mv", -1);
    SetParameterDescription("mv", "Maximum size of the validation sample (default = -1)");
    AddParameter(ParameterType_Float, "vtr", "training and validation sample ratio");
    SetParameterDescription("vtr",
                            "Ratio between training and validation sample (0.0 = all training, 1.0 = all validation) default = 0.5.");
    SetParameterFloat("vtr", 0.5);
    AddParameter(ParameterType_Empty, "opt", "parameters optimization");
    MandatoryOff("opt");
    SetParameterDescription("opt", "SVM parameters optimization");
    AddParameter(ParameterType_Filename, "vfn", "Name of the discrimination field");
    SetParameterDescription("vfn", "Name of the field using to discriminate class in the vector data files.");
    SetParameterString("vfn", "Class");
    AddParameter(ParameterType_Filename, "out", "Output SVM model");
    SetParameterDescription("out", "Output SVM model");
    SetParameterRole("out",Role_Output);

  }

  void DoUpdateParameters()
  {
    // Nothing to do here : all parameters are independent
  }

  void DoExecute()
  {
    GetLogger()->Debug("Entering DoExecute\n");
    //Create training and validation for list samples and label list samples
    ConcatenateLabelListSampleFilterType::Pointer
        concatenateTrainingLabels = ConcatenateLabelListSampleFilterType::New();
    ConcatenateListSampleFilterType::Pointer concatenateTrainingSamples = ConcatenateListSampleFilterType::New();
    ConcatenateLabelListSampleFilterType::Pointer
        concatenateValidationLabels = ConcatenateLabelListSampleFilterType::New();
    ConcatenateListSampleFilterType::Pointer concatenateValidationSamples = ConcatenateListSampleFilterType::New();

    MeasurementType meanMeasurementVector;
    MeasurementType stddevMeasurementVector;

    //--------------------------
    // Load measurements from images
    unsigned int nbBands = 0;
    //Iterate over all input images

    FloatVectorImageListType* imageList = GetParameterImageList("il");
    VectorDataListType* vectorDataList = GetParameterVectorDataList("vd");

    //Iterate over all input images
    for (unsigned int imgIndex = 0; imgIndex < imageList->Size(); ++imgIndex)
      {
      FloatVectorImageType::Pointer image = imageList->GetNthElement(imgIndex);
      image->UpdateOutputInformation();

      if (imgIndex == 0)
        {
        nbBands = image->GetNumberOfComponentsPerPixel();
        }

      // read the Vectordata
      VectorDataType::Pointer vectorData = vectorDataList->GetNthElement(imgIndex);
      vectorData->Update();

      VectorDataReprojectionType::Pointer vdreproj = VectorDataReprojectionType::New();
      vdreproj->SetInputImage(image);
      vdreproj->SetInput(vectorData);
      vdreproj->SetUseOutputSpacingAndOriginFromImage(false);

      // Configure DEM directory
      if (IsParameterEnabled("dem"))
        {
        vdreproj->SetDEMDirectory(GetParameterString("dem"));
        }
      else
        {
        if (otb::ConfigurationFile::GetInstance()->IsValid())
          {
          vdreproj->SetDEMDirectory(otb::ConfigurationFile::GetInstance()->GetDEMDirectory());
          }
        }

      vdreproj->Update();

      //Sample list generator
      ListSampleGeneratorType::Pointer sampleGenerator = ListSampleGeneratorType::New();

      //m_sampleGenerator = sampleGenerator;
      //Set inputs of the sample generator
      sampleGenerator->SetInput(image);
      sampleGenerator->SetInputVectorData(vdreproj->GetOutput());

      sampleGenerator->SetClassKey(GetParameterString("vfn"));
      sampleGenerator->SetMaxTrainingSize(GetParameterInt("mt"));
      sampleGenerator->SetMaxValidationSize(GetParameterInt("mv"));
      sampleGenerator->SetValidationTrainingProportion(GetParameterFloat("vtr"));

      sampleGenerator->Update();

      //Concatenate training and validation samples from the image
      concatenateTrainingLabels->AddInput(sampleGenerator->GetTrainingListLabel());
      concatenateTrainingSamples->AddInput(sampleGenerator->GetTrainingListSample());
      concatenateValidationLabels->AddInput(sampleGenerator->GetValidationListLabel());
      concatenateValidationSamples->AddInput(sampleGenerator->GetValidationListSample());
      }
    // Update
    concatenateTrainingSamples->Update();
    concatenateTrainingLabels->Update();
    concatenateValidationSamples->Update();
    concatenateValidationLabels->Update();

    if (IsParameterEnabled("imstat"))
      {
      StatisticsReader::Pointer statisticsReader = StatisticsReader::New();
      statisticsReader->SetFileName(GetParameterString("imstat"));
      meanMeasurementVector = statisticsReader->GetStatisticVectorByName("mean");
      stddevMeasurementVector = statisticsReader->GetStatisticVectorByName("stddev");
      }
    else
      {
      meanMeasurementVector.SetSize(nbBands);
      meanMeasurementVector.Fill(0.);
      stddevMeasurementVector.SetSize(nbBands);
      stddevMeasurementVector.Fill(1.);
      }

    // Shift scale the samples
    ShiftScaleFilterType::Pointer trainingShiftScaleFilter = ShiftScaleFilterType::New();
    trainingShiftScaleFilter->SetInput(concatenateTrainingSamples->GetOutput());
    trainingShiftScaleFilter->SetShifts(meanMeasurementVector);
    trainingShiftScaleFilter->SetScales(stddevMeasurementVector);
    trainingShiftScaleFilter->Update();

    ShiftScaleFilterType::Pointer validationShiftScaleFilter = ShiftScaleFilterType::New();
    validationShiftScaleFilter->SetInput(concatenateValidationSamples->GetOutput());
    validationShiftScaleFilter->SetShifts(meanMeasurementVector);
    validationShiftScaleFilter->SetScales(stddevMeasurementVector);
    validationShiftScaleFilter->Update();

    ListSampleType::Pointer listSample;
    LabelListSampleType::Pointer labelListSample;
    //--------------------------
    // Balancing training sample (if needed)
    if (IsParameterEnabled("b"))
      {
      // Balance the list sample.
      otbAppLogINFO("Number of training samples before balancing: " << concatenateTrainingSamples->GetOutputSampleList()->Size())
      BalancingListSampleFilterType::Pointer balancingFilter = BalancingListSampleFilterType::New();
      balancingFilter->SetInput(trainingShiftScaleFilter->GetOutput()/*GetOutputSampleList()*/);
      balancingFilter->SetInputLabel(concatenateTrainingLabels->GetOutput()/*GetOutputSampleList()*/);
      balancingFilter->SetBalancingFactor(GetParameterInt("b"));
      balancingFilter->Update();
      listSample = balancingFilter->GetOutputSampleList();
      labelListSample = balancingFilter->GetOutputLabelSampleList();
      otbAppLogINFO("Number of samples after balancing: " << balancingFilter->GetOutputSampleList()->Size());

      }
    else
      {
      listSample = trainingShiftScaleFilter->GetOutputSampleList();
      labelListSample = concatenateTrainingLabels->GetOutputSampleList();
      otbAppLogINFO("Number of training samples: " << concatenateTrainingSamples->GetOutputSampleList()->Size());
      }
    //--------------------------
    // Split the data set into training/validation set
    ListSampleType::Pointer trainingListSample = listSample;
    ListSampleType::Pointer validationListSample = validationShiftScaleFilter->GetOutputSampleList();
    LabelListSampleType::Pointer trainingLabeledListSample = labelListSample;
    LabelListSampleType::Pointer validationLabeledListSample = concatenateValidationLabels->GetOutputSampleList();
    otbAppLogINFO("Size of training set: " << trainingListSample->Size());
    otbAppLogINFO("Size of validation set: " << validationListSample->Size());
    otbAppLogINFO("Size of labeled training set: " << trainingLabeledListSample->Size());
    otbAppLogINFO("Size of labeled validation set: " << validationLabeledListSample->Size());

    //--------------------------
    // Estimate SVM model
    SVMEstimatorType::Pointer svmestimator = SVMEstimatorType::New();
    svmestimator->SetInputSampleList(trainingListSample);
    svmestimator->SetTrainingSampleList(trainingLabeledListSample);
    //SVM Option
    //TODO : Add other options ?
    if (IsParameterEnabled("opt"))
      {
      svmestimator->SetParametersOptimization(true);
      }


    svmestimator->SetC(GetParameterFloat("m"));


    switch (GetParameterInt("k"))
      {
      case 0: // LINEAR
        svmestimator->SetKernelType(LINEAR);
        break;
      case 1: // RBF
        svmestimator->SetKernelType(RBF);
        break;
      case 2: // POLY
        svmestimator->SetKernelType(POLY);
        break;
      case 3: // SIGMOID
        svmestimator->SetKernelType(SIGMOID);
        break;
      default: // DEFAULT = LINEAR
        svmestimator->SetKernelType(LINEAR);
        break;
      }

    svmestimator->Update();
    svmestimator->GetModel()->SaveModel(GetParameterString("out"));

    otbAppLogINFO( "Learning done -> Final SVM accuracy: " << svmestimator->GetFinalCrossValidationAccuracy() << std::endl);

    //--------------------------
    // Performances estimation
    ClassifierType::Pointer validationClassifier = ClassifierType::New();
    validationClassifier->SetSample(validationListSample);
    validationClassifier->SetNumberOfClasses(svmestimator->GetModel()->GetNumberOfClasses());
    validationClassifier->SetModel(svmestimator->GetModel());
    validationClassifier->Update();

    // Estimate performances
    ClassifierOutputType::ConstIterator it = validationClassifier->GetOutput()->Begin();
    ClassifierOutputType::ConstIterator itEnd = validationClassifier->GetOutput()->End();

    LabelListSampleType::Pointer classifierListLabel = LabelListSampleType::New();

    while (it != itEnd)
      {
      // Due to a bug in SVMClassifier, outlier in one-class SVM are labeled with unsigned int max
      classifierListLabel->PushBack(
                                    it.GetClassLabel() == itk::NumericTraits<unsigned int>::max() ? 2
                                                                                                  : it.GetClassLabel());
      ++it;
      }

    ConfusionMatrixCalculatorType::Pointer confMatCalc = ConfusionMatrixCalculatorType::New();

    confMatCalc->SetReferenceLabels(validationLabeledListSample);
    confMatCalc->SetProducedLabels(classifierListLabel);

    confMatCalc->Update();

    otbAppLogINFO("*** SVM training performances ***\n" << "Confusion matrix:\n" << confMatCalc->GetConfusionMatrix());

    for (unsigned int itClasses = 0; itClasses < svmestimator->GetModel()->GetNumberOfClasses(); itClasses++)
      {
      otbAppLogINFO("Precision of class [" << itClasses << "] vs all: " << confMatCalc->GetPrecisions()[itClasses]);
      otbAppLogINFO("Recall of class [" << itClasses << "] vs all: " << confMatCalc->GetRecalls()[itClasses]);
      otbAppLogINFO("F-score of class [" << itClasses << "] vs all: " << confMatCalc->GetFScores()[itClasses] << "\n");
      }
    otbAppLogINFO("Global performance, Kappa index: " << confMatCalc->GetKappaIndex());
    // TODO: implement hyperplan distance classifier and performance validation (cf. object detection) ?


  }

};

}
}

OTB_APPLICATION_EXPORT(otb::Wrapper::TrainSVMImagesClassifier)

