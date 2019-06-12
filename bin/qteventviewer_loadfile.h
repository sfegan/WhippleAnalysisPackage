#ifndef LOADDIALOG_H
#define LOADDIALOG_H

#include<qcheckbox.h>
#include<qpushbutton.h>
#include<qlineedit.h>
#include<qcombobox.h>
#include<qlabel.h>

#include<string>

#include<RedFile.h>
#include<RedHeader.h>

#include "qteventviewer_loadfile_base.h"

class LoadDialog : public LoadDialogBase
{ 
  Q_OBJECT

public:
  LoadDialog( QWidget* parent = 0, const char* name = 0, 
	      bool modal = TRUE, WFlags fl = 0 );
  ~LoadDialog();
  
  std::string pedsDate() const;
  std::string pedsFile() const;
  bool   gainsApply() const;
  std::string gainsDate() const;
  std::string gainsFile() const;
  std::string cameraFile() const;

  int exec(const std::string& name, const std::string& date, const std::string& peds, 
	   const std::vector<std::string>& gains, const std::string& camera);
  
private slots:
  void browseCameras();
  void gainsToggled(bool on);
};

#endif // LOADDIALOG_H
