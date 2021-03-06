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

#include "QmitkSurfaceToImageWidget.h"

#include <mitkException.h>
#include <mitkExceptionMacro.h>
#include <mitkProgressBar.h>
#include <mitkProperties.h>
#include <mitkSurfaceToImageFilter.h>
#include <mitkSurface.h>
#include <mitkImage.h>
#include <mitkLabelSetImage.h>

#include <qmessagebox.h>

static const QString HelpText = QWidget::tr("Select an image and a surface above");
static const QString TR_IMAGE_DIFFERENT = QWidget::tr("Image and surface are of different size");
static const QString TR_SURFACE_TO_IMAGE = QMessageBox::tr("Surface To Image");
static const QString TR_SELECTION_NOT_CONTAIN = QMessageBox::tr("Selection does not contain an image and/or a surface");
static const QString TR_CONVERT_SURFACE = QMessageBox::tr("Convert Surface to binary image failed");

QmitkSurfaceToImageWidget::QmitkSurfaceToImageWidget(mitk::SliceNavigationController* timeNavigationController, QWidget* parent)
  : QmitkSegmentationUtilityWidget(timeNavigationController, parent)
{
  m_Controls.setupUi(this);

  m_Controls.dataSelectionWidget->AddDataStorageComboBox(QmitkDataSelectionWidget::ImageAndSegmentationPredicate);
  m_Controls.dataSelectionWidget->AddDataStorageComboBox(QmitkDataSelectionWidget::SurfacePredicate);
  m_Controls.dataSelectionWidget->SetHelpText(HelpText);

  this->EnableButtons(false);

  connect (m_Controls.btnSurface2Image, SIGNAL(pressed()), this, SLOT(OnSurface2ImagePressed()));
  connect(m_Controls.dataSelectionWidget, SIGNAL(SelectionChanged(unsigned int, const mitk::DataNode*)),
    this, SLOT(OnSelectionChanged(unsigned int, const mitk::DataNode*)));

  if( m_Controls.dataSelectionWidget->GetSelection(0).IsNotNull() &&
    m_Controls.dataSelectionWidget->GetSelection(1).IsNotNull() )
  {
    this->OnSelectionChanged( 0, m_Controls.dataSelectionWidget->GetSelection(0));
  }
}

QmitkSurfaceToImageWidget::~QmitkSurfaceToImageWidget()
{
}

void QmitkSurfaceToImageWidget::EnableButtons(bool enable)
{
  m_Controls.btnSurface2Image->setEnabled(enable);
}

void QmitkSurfaceToImageWidget::OnSelectionChanged(unsigned int, const mitk::DataNode*)
{
  QmitkDataSelectionWidget* dataSelectionWidget = m_Controls.dataSelectionWidget;
  mitk::DataNode::Pointer imageNode = dataSelectionWidget->GetSelection(0);
  mitk::DataNode::Pointer surfaceNode = dataSelectionWidget->GetSelection(1);

  if (imageNode.IsNull() || surfaceNode.IsNull() )
  {
    dataSelectionWidget->SetHelpText(HelpText);
    this->EnableButtons(false);
  }
  else
  {
    mitk::Image::Pointer image = dynamic_cast<mitk::Image*>( dataSelectionWidget->GetSelection(0)->GetData() );
    mitk::Surface::Pointer surface = dynamic_cast<mitk::Surface*>( dataSelectionWidget->GetSelection(1)->GetData() );
    if( image->GetTimeSteps() != surface->GetTimeSteps() )
    {
      dataSelectionWidget->SetHelpText(TR_IMAGE_DIFFERENT);
      this->EnableButtons(false);
    }
    else
    {
      dataSelectionWidget->SetHelpText("");
      this->EnableButtons();
    }
  }
}

void QmitkSurfaceToImageWidget::OnSurface2ImagePressed()
{
  this->EnableButtons(false);
  mitk::ProgressBar::GetInstance()->AddStepsToDo(2);

  QmitkDataSelectionWidget* dataSelectionWidget = m_Controls.dataSelectionWidget;
  mitk::Image::Pointer image = dynamic_cast<mitk::Image*>( dataSelectionWidget->GetSelection(0)->GetData() );
  mitk::Surface::Pointer surface = dynamic_cast<mitk::Surface*>( dataSelectionWidget->GetSelection(1)->GetData() );

  if( image.IsNull() || surface.IsNull())
  {
    MITK_ERROR << "Selection does not contain an image and/or a surface";
    QMessageBox::information( this, TR_SURFACE_TO_IMAGE, TR_SELECTION_NOT_CONTAIN, QMessageBox::Ok );
    this->EnableButtons();
    return;
  }

  mitk::Image::Pointer resultImage(nullptr);
  resultImage = this->ConvertSurfaceToImage( image, surface );

  mitk::ProgressBar::GetInstance()->Progress();

  if( resultImage.IsNull() )
  {
    MITK_ERROR << "Convert Surface to binary image failed";
    QMessageBox::information( this, TR_SURFACE_TO_IMAGE, TR_CONVERT_SURFACE, QMessageBox::Ok );
    this->EnableButtons();
    mitk::ProgressBar::GetInstance()->Reset();
    return;
  }

  //create name for result node
  std::string nameOfResultImage = dataSelectionWidget->GetSelection(0)->GetName();
  nameOfResultImage.append("_");
  nameOfResultImage.append(dataSelectionWidget->GetSelection(1)->GetName());

  //create data node and add to data storage
  mitk::DataNode::Pointer resultNode = mitk::DataNode::New();
  resultNode->SetData( resultImage );
  resultNode->SetProperty("name", mitk::StringProperty::New(nameOfResultImage) );
//  resultNode->SetProperty("binary", mitk::BoolProperty::New(true) );

  dataSelectionWidget->GetDataStorage()->Add(resultNode, dataSelectionWidget->GetSelection(0));

  this->EnableButtons();

  mitk::ProgressBar::GetInstance()->Progress();
}

mitk::LabelSetImage::Pointer QmitkSurfaceToImageWidget::ConvertSurfaceToImage( mitk::Image::Pointer image, mitk::Surface::Pointer surface )
{
  mitk::ProgressBar::GetInstance()->AddStepsToDo(4);
  mitk::ProgressBar::GetInstance()->Progress();

  mitk::SurfaceToImageFilter::Pointer surfaceToImageFilter = mitk::SurfaceToImageFilter::New();
  surfaceToImageFilter->MakeOutputBinaryOn();
  surfaceToImageFilter->SetInput(surface);
  surfaceToImageFilter->SetImage(image);
  try
  {
    surfaceToImageFilter->Update();
  }
  catch(itk::ExceptionObject& excpt)
  {
    MITK_ERROR << excpt.GetDescription();
    return nullptr;
  }

  mitk::ProgressBar::GetInstance()->Progress(2);

  mitk::Image::Pointer resultImage = surfaceToImageFilter->GetOutput();
  mitk::LabelSetImage::Pointer multilabelImage = mitk::LabelSetImage::New();
  multilabelImage->InitializeByLabeledImage(resultImage);

  mitk::ProgressBar::GetInstance()->Progress();

  return multilabelImage;
}

