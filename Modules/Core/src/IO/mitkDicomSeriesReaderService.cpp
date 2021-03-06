/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "mitkDicomSeriesReaderService.h"

#include <mitkCustomMimeType.h>
#include <mitkIOMimeTypes.h>
#include <mitkDicomSeriesReader.h>
#include <mitkLocaleSwitch.h>
#include <mitkProgressBar.h>
#include <mitkImage.h>

#include <iostream>

namespace mitk {

DicomSeriesReaderService::DicomSeriesReaderService()
  : AbstractFileReader(CustomMimeType(IOMimeTypes::DICOM_MIMETYPE()), "MITK DICOM Reader")
{
  this->RegisterService();
}

std::vector<itk::SmartPointer<BaseData> > DicomSeriesReaderService::Read()
{
  std::vector<BaseData::Pointer> result;

  mitk::LocaleSwitch localeSwitch("C");

  std::string fileName = this->GetLocalFileName();

  DicomSeriesReader::FileNamesGrouping imageBlocks = DicomSeriesReader::GetSeries(itksys::SystemTools::GetFilenamePath(fileName));
  const unsigned int size = imageBlocks.size();

  ProgressBar::GetInstance()->AddStepsToDo(size);
  ProgressBar::GetInstance()->Progress();

  unsigned int outputIndex = 0u;
  const DicomSeriesReader::FileNamesGrouping::const_iterator n_end = imageBlocks.end();

  for (DicomSeriesReader::FileNamesGrouping::const_iterator n_it = imageBlocks.begin(); n_it != n_end; ++n_it)
  {
    const std::string &uid = n_it->first;

    const DicomSeriesReader::ImageBlockDescriptor& imageBlockDescriptor( *n_it->second );

    MITK_INFO << "--------------------------------------------------------------------------------";
    MITK_INFO << "DicomSeriesReader: Loading DICOM series " << outputIndex << ": Series UID " << imageBlockDescriptor.GetSeriesInstanceUID() << std::endl;
    MITK_INFO << "  " << imageBlockDescriptor.size() << " '" << imageBlockDescriptor.GetModality() << "' files (" << imageBlockDescriptor.GetSOPClassUIDAsString() << ") loaded into 1 mitk::Image";
    MITK_INFO << "  multi-frame: " << (imageBlockDescriptor.IsMultiFrameImage()?"Yes":"No");
    MITK_INFO << "  reader support: " << DicomSeriesReader::ReaderImplementationLevelToString(imageBlockDescriptor.GetReaderImplementationLevel());
    MITK_INFO << "  pixel spacing type: " << DicomSeriesReader::PixelSpacingInterpretationToString( imageBlockDescriptor.GetPixelSpacingType() );
    MITK_INFO << "  gantry tilt corrected: " << (imageBlockDescriptor.HasGantryTiltCorrected()?"Yes":"No");
    MITK_INFO << "  3D+t: " << (imageBlockDescriptor.HasMultipleTimePoints()?"Yes":"No");
    MITK_INFO << "--------------------------------------------------------------------------------";

    if (auto data = n_it->second->GetImage())
    {
      PropertyList::Pointer dataProps = data->GetPropertyList();

      std::string nodeName(uid);
      std::string studyDescription;
      if ( dataProps->GetStringProperty( "dicom.study.StudyDescription", studyDescription ) )
      {
        nodeName = studyDescription;
        std::string seriesDescription;
        if ( dataProps->GetStringProperty( "dicom.series.SeriesDescription", seriesDescription ) )
        {
          nodeName += "/" + seriesDescription;
        }
      }

      StringProperty::Pointer nameProp = StringProperty::New(nodeName);
      data->SetProperty("name", nameProp);

      result.push_back(data.GetPointer());
      ++outputIndex;
    }
    else
    {
      MITK_ERROR << "DicomSeriesReader: Skipping series " << outputIndex << " due to some unspecified error..." << std::endl;
    }

    ProgressBar::GetInstance()->Progress();
  }

  return result;
}

DicomSeriesReaderService* DicomSeriesReaderService::Clone() const
{
  return new DicomSeriesReaderService(*this);
}

}
