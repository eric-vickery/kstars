/*  Telescope wizard
    Copyright (C) 2003 Jasem Mutlaq (mutlaqja@ikarustech.com)

    This application is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
 */

#include <qpixmap.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qwidgetstack.h>
#include <qdatetime.h>
#include <qstring.h>
#include <qtimer.h>
#include <qtable.h>
#include <qtextedit.h>

#include <klistview.h>
#include <klineedit.h>
#include <kmessagebox.h>
#include <kprogress.h>

#include "kstars.h"
#include "telescopewizardprocess.h"
#include "timedialog.h"
#include "ksutils.h"

#include "indimenu.h"
#include "indidriver.h"
#include "indidevice.h"

#define TIMEOUT_THRESHHOLD	20

telescopeWizardProcess::telescopeWizardProcess( QWidget* parent, const char* name ) : telescopeWizard(parent, name)
{
   currentPort  = -1;
   timeOutCount = 0;
   indiDev = NULL;
   progressScan = NULL;
   linkRejected = false;

   QString locStr;
   QFile sideIMG;

   if (KSUtils::openDataFile(sideIMG, "wizardside.png"))
       wizardPix->setPixmap(QPixmap(sideIMG.name()));

   backB->hide();
   currentPage = INTRO_P;

   ksw = (KStars *) parent;

   ksw->establishINDI();

   indimenu   = ksw->getINDIMenu();
   indidriver = ksw->getINDIDriver();

   INDIMessageBar = ksw->options()->indiMessages;
   ksw->options()->indiMessages = false;

  QTime newTime( ksw->data()->LTime.time() );
  QDate newDate( ksw->data()->LTime.date() );

  timeOut->setText( QString().sprintf("%02d:%02d:%02d", newTime.hour(), newTime.minute(), newTime.second()));
  dateOut->setText( QString().sprintf("%d-%02d-%02d", newDate.year(), newDate.month(), newDate.day()));

  if (ksw->geo()->translatedProvince().isEmpty())
  	locationOut->setText( QString("%1, %2").arg(ksw->geo()->translatedName()).arg(ksw->geo()->translatedCountry()));
  else
  	locationOut->setText( QString("%1, %2, %3").arg(ksw->geo()->translatedName())
  						     .arg(ksw->geo()->translatedProvince())
						     .arg(ksw->geo()->translatedCountry()));


   for (unsigned int i=0; i < indidriver->devices.size(); i++)
   		telescopeCombo->insertItem(indidriver->devices[i]->label);

   portList << "/dev/ttyS0" <<  "/dev/ttyS1" << "/dev/ttyS2" << "/dev/ttyS3" << "/dev/ttyS4"
            << "/dev/ttyUSB0" << "/dev/ttyUSB1" << "/dev/ttyUSB2" << "/dev/ttyUSB3";// << "/dev/ttyUSB4";

   connect(helpB, SIGNAL(clicked()), parent, SLOT(appHelpActivated()));
   connect(nextB, SIGNAL(clicked()), this, SLOT(processNext()));
   connect(backB, SIGNAL(clicked()), this, SLOT(processBack()));
   connect(setTimeB, SIGNAL(clicked()), this, SLOT(newTime()));
   connect(setLocationB, SIGNAL(clicked()), this, SLOT(newLocation()));

   newDeviceTimer = new QTimer(this);
   QObject::connect( newDeviceTimer, SIGNAL(timeout()), this, SLOT(processPort()) );

}

telescopeWizardProcess::~telescopeWizardProcess()
{
  if (progressScan)
    if (progressScan->wasCancelled())
      indidriver->processDeviceStatus(1);

    ksw->options()->indiMessages = INDIMessageBar;

    Reset();
}

void telescopeWizardProcess::processNext(void)
{
  int linkResult=0;

 switch (currentPage)
 {
   case INTRO_P:
     currentPage++;
     backB->show();
     wizardContainer->raiseWidget(currentPage);
     break;
  case MODEL_P:
     currentPage++;
     wizardContainer->raiseWidget(currentPage);
     break;
  case TELESCOPE_P:
     currentPage++;
     wizardContainer->raiseWidget(currentPage);
     break;
  case LOCAL_P:
     currentPage++;
     wizardContainer->raiseWidget(currentPage);
     break;
  case PORT_P:
     linkResult = establishLink();
     if ( linkResult == 1)
     {
progressScan = new KProgressDialog(this, "autoscan", i18n("Autoscan"), i18n("Please wait while KStars scan communication ports for attached telescopes.\nThis process might take few minutes to complete."), true);
   progressScan->setAllowCancel(true);
   progressScan->setAutoClose(true);
   progressScan->setAutoReset(true);
   progressScan->progressBar()->setRange(0, portList.count());
   progressScan->progressBar()->setValue(0);
   progressScan->show();
    }
    else if (linkResult == 2)
      KMessageBox::queuedMessageBox(0, KMessageBox::Information, i18n("Please wait while KStars tries to connect to your telescope..."));
     break;
  default:
     break;
  }

}

void telescopeWizardProcess::processBack(void)
{
 // for now, just display the next page, and restart once we reached the end

 switch (currentPage)
 {
   case INTRO_P:
   // we shouldn't be here!
   break;
     break;
  case MODEL_P:
     currentPage--;
     backB->hide();
     wizardContainer->raiseWidget(currentPage);
     break;
  case TELESCOPE_P:
     currentPage--;
     wizardContainer->raiseWidget(currentPage);
     break;
  case LOCAL_P:
     currentPage--;
     wizardContainer->raiseWidget(currentPage);
     break;
  case PORT_P:
     currentPage--;
     wizardContainer->raiseWidget(currentPage);
     break;
  default:
     break;
  }

}

void telescopeWizardProcess::newTime()
{

  TimeDialog timedialog (ksw->data()->LTime, ksw);

	if ( timedialog.exec() == QDialog::Accepted )
	{
		QTime newTime( timedialog.selectedTime() );
		QDate newDate( timedialog.selectedDate() );

		ksw->data()->changeTime(newDate, newTime);

  		timeOut->setText( QString().sprintf("%02d:%02d:%02d", newTime.hour(), newTime.minute(), newTime.second()));
  		dateOut->setText( QString().sprintf("%d-%02d-%02d", newDate.year(), newDate.month(), newDate.day()));
	}

}

void telescopeWizardProcess::newLocation()
{

   ksw->slotGeoLocator();

   locationOut->setText( QString("%1, %2, %3").arg(ksw->geo()->translatedName())
  					     .arg(ksw->geo()->translatedProvince())
					     .arg(ksw->geo()->translatedCountry()));
   timeOut->setText( QString().sprintf("%02d:%02d:%02d", ksw->data()->LTime.time().hour(), ksw->data()->LTime.time().minute(), ksw->data()->LTime.time().second()));

  dateOut->setText( QString().sprintf("%d-%02d-%02d", ksw->data()->LTime.date().year(),
  ksw->data()->LTime.date().month() ,ksw->data()->LTime.date().day()));



}

int telescopeWizardProcess::establishLink()
{

	if (!indidriver || !indimenu)
	  return (0);

	QListViewItem *driverItem = NULL;

	if (!indidriver->isDeviceRunning(telescopeCombo->currentText()))
	{
        	driverItem = indidriver->localListView->findItem(telescopeCombo->currentText(), 0);

		indimenu->setCustomLabel(telescopeCombo->currentText());
		currentDevice = indimenu->currentLabel;
		indimenu->currentLabel = "";

		if (driverItem)
		{

	 		indidriver->localListView->setSelected(driverItem, true);
	 		indidriver->processDeviceStatus(0);
		}
	}
	else
		currentDevice = telescopeCombo->currentText();


	if (!indidriver->isDeviceRunning(telescopeCombo->currentText()))
	 return (3);

	newDeviceTimer->start(1500);

	if (portIn->text().isEmpty())
	 return (1);
        else
	 return (2);

}

void telescopeWizardProcess::processPort()
{
     INDI_P * pp;


     if (!indidriver || !indimenu)
       return;


     timeOutCount++;

     if (timeOutCount >= TIMEOUT_THRESHHOLD)
     {
       indidriver->processDeviceStatus(1);
       Reset();
       KMessageBox::error(0, i18n("Error: connection timeout. Unable to communicate with an INDI server"));
       close();
       return;
     }


    indiDev = indimenu->findDeviceByLabel(currentDevice);

    if (!indiDev)
      return;

     // port empty, start autoscan
     if (portIn->text().isEmpty())
     {
       newDeviceTimer->stop();
       linkRejected = false;
       connect(indiDev, SIGNAL(linkRejected()), this, SLOT(scanPorts()));
       connect(indiDev, SIGNAL(linkAccepted()), this, SLOT(linkSuccess()));
       scanPorts();
       return;
     }

     pp = indiDev->findProp(QString("Ports"));

     if (!pp)
       return;


     if (pp->perm == PP_RW)
     {
            	pp->table_w->setText(0, 1, portIn->text());
		pp->newText();
     }
	    else if (pp->perm == PP_WO)
     {
		pp->table_w->setText(0, 0, portIn->text());
		pp->newText();
     }

     pp = indiDev->findProp(QString("CONNECTION"));

     if (!pp)
      return;

     newDeviceTimer->stop();

     ksw->options()->indiMessages = INDIMessageBar;

     pp->newSwitch(0);

     timeOutCount = 0;

     indimenu->show();

     close();

}

void telescopeWizardProcess::scanPorts()
{

     if (!indiDev || !indidriver || !indimenu || linkRejected)
      return;

     currentPort++;

     progressScan->progressBar()->setValue(currentPort);

     if ( (unsigned) currentPort >= portList.count())
     {
      KMessageBox::sorry(0, i18n("Sorry. KStars failed to detect any attached telescopes, please check your settings and try again."));
      linkRejected = true;
      indidriver->processDeviceStatus(1);
      Reset();
      return;
     }

     INDI_P * pp;

     if (indiDev->msgST_w)
     	indiDev->msgST_w->clear();

     pp = indiDev->findProp(QString("Ports"));

     if (!pp)
       return;

    if (pp->perm == PP_RW)
    	{
		pp->table_w->setText(0, 1, portList[currentPort]);
		pp->newText();
	}
	    else if (pp->perm == PP_WO)
	{
		pp->table_w->setText(0, 0, portList[currentPort]);
		pp->newText();
     	}

     pp = indiDev->findProp(QString("CONNECTION"));

     if (!pp)
      return;

     pp->newSwitch(0);


}

void telescopeWizardProcess::linkSuccess()
{
  Reset();

  indimenu->show();

  close();

}

void telescopeWizardProcess::Reset()
{

  currentPort = -1;
  timeOutCount = 0;

  if (progressScan)
  	progressScan->close();

  indiDev = NULL;

}

#include "telescopewizardprocess.moc"
