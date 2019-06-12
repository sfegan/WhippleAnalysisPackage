#include <qfiledialog.h>

#include <sstream>
#include <string>

#include <UtilityFunctions.h>
#include <CameraConfiguration.h>

#include "qteventviewer_loadfile.h"

using namespace NS_Analysis;

LoadDialog::LoadDialog( QWidget* parent, const char* name, 
			bool modal, WFlags fl )
  : LoadDialogBase(parent,name,modal,fl)
{
  connect(GainsApplyCheck, SIGNAL(toggled(bool)), 
	  this, SLOT(gainsToggled(bool)));
  connect(CameraBrowse, SIGNAL(clicked()), this, SLOT(browseCameras()));
}

LoadDialog::~LoadDialog()
{
}


int LoadDialog::exec(const std::string& name, const string& date, 
		     const std::string& peds, const std::vector<string>& gains, 
		     const std::string& camera)
{
  PedsDate->setText(date.c_str());
  PedsFile->setText(peds.c_str());

  if(!GainsApplyCheck->isOn())GainsApplyCheck->toggle();
  GainsDate->setText(date.c_str());
  for(std::vector<std::string>::const_iterator x=gains.begin(); x!=gains.end(); x++)
    GainsFile->insertItem(x->c_str());
  if(gains.size()==0)GainsApplyCheck->toggle();
  
  CameraFile->setText(camera.c_str());
  
  return QDialog::exec();
}

std::string LoadDialog::pedsDate() const
{
  return std::string(PedsDate->text().ascii());
}

std::string LoadDialog::pedsFile() const
{
  return std::string(PedsFile->text().ascii());
}

bool   LoadDialog::gainsApply() const
{
  return GainsApplyCheck->isOn();
}

std::string LoadDialog::gainsDate() const
{
  return std::string(GainsDate->text().ascii());
}

std::string LoadDialog::gainsFile() const
{
  return std::string(GainsFile->currentText().ascii());
}

std::string LoadDialog::cameraFile() const
{
  return std::string(CameraFile->text().ascii());
}

void LoadDialog::browseCameras()
{
  QString filename = 
    QFileDialog::getOpenFileName(CameraFile->text(),
				 "HDF5 Files (*.h5);;All Files (*.*)",
				 this,"","Load Camera File");
  if(!filename.isEmpty())CameraFile->setText(filename);
}

void LoadDialog::gainsToggled(bool on)
{
  GainsDate->setEnabled(on);
  GainsDateLabel->setEnabled(on);
  GainsFile->setEnabled(on);
  GainsFileLabel->setEnabled(on);
}

#include "qteventviewer_loadfile_moc.cxx"
#include "qteventviewer_loadfile_base_moc.cxx"

