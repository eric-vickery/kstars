/***************************************************************************
                          obslistwizard.cpp  -  Display overhead view of the solar system
                             -------------------
    begin                : Thu 23 Jun 2005
    copyright            : (C) 2005 by Jason Harris
    email                : jharris@30doradus.org
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

//#include <qcheckbox.h>
//#include <qlabel.h>
//#include <qlayout.h>
//#include <q3widgetstack.h>

#include <QVBoxLayout>
#include <QFrame>
#include <knuminput.h>
#include <k3listview.h>
#include <kpushbutton.h>

#include "kstars.h"
#include "kstarsdata.h"
#include "locationdialog.h"
#include "skyobject.h"
#include "deepskyobject.h"
#include "starobject.h"
#include "widgets/dmsbox.h"
#include "widgets/magnitudespinbox.h"
//#include "libkdeedu/extdate/extdatetimeedit.h"

#include "obslistwizard.h"


ObsListWizardUI::ObsListWizardUI( QWidget *p ) : QFrame ( p ) {
  setupUi( this );
}

ObsListWizard::ObsListWizard( KStars *ksparent )
  : KDialog( ksparent ),  ksw( ksparent )
{
	olw = new ObsListWizardUI( this );
	setMainWidget( olw );
        setCaption( i18n("Observing List Wizard") );
        setButtons( KDialog::User1|KDialog::User2|KDialog::Ok|KDialog::Cancel );
        setButtonGuiItem( KDialog::User1, KGuiItem( QString("< ") + i18n("&Back"), QString(), i18n("Go to previous Wizard page") ) );
        setButtonGuiItem( KDialog::User2, KGuiItem( i18n("&Next") + QString(">"), QString(), i18n("Go to next Wizard page") ) );

	connect( olw->AllButton, SIGNAL( clicked() ), this, SLOT( slotAllButton() ) );
	connect( olw->NoneButton, SIGNAL( clicked() ), this, SLOT( slotNoneButton() ) );
	connect( olw->DeepSkyButton, SIGNAL( clicked() ), this, SLOT( slotDeepSkyButton() ) );
	connect( olw->SolarSystemButton, SIGNAL( clicked() ), this, SLOT( slotSolarSystemButton() ) );
	connect( olw->LocationButton, SIGNAL( clicked() ), this, SLOT( slotChangeLocation() ) );

	connect( this, SIGNAL( user1Clicked() ), this, SLOT( slotPrevPage() ) );
	connect( this, SIGNAL( user2Clicked() ), this, SLOT( slotNextPage() ) );

	//Update the count of objects when certain UI elements are modified
	connect( olw->TypeList, SIGNAL( itemSelectionChanged() ), this, SLOT( slotUpdateObjectCount() ) );
	connect( olw->ConstellationList, SIGNAL( itemSelectionChanged() ), this, SLOT( slotUpdateObjectCount() ) );
	connect( olw->RAMin, SIGNAL( lostFocus() ), this, SLOT( slotUpdateObjectCount() ) );
	connect( olw->RAMax, SIGNAL( lostFocus() ), this, SLOT( slotUpdateObjectCount() ) );
	connect( olw->DecMin, SIGNAL( lostFocus() ), this, SLOT( slotUpdateObjectCount() ) );
	connect( olw->DecMax, SIGNAL( lostFocus() ), this, SLOT( slotUpdateObjectCount() ) );
	connect( olw->RA, SIGNAL( lostFocus() ), this, SLOT( slotUpdateObjectCount() ) );
	connect( olw->Dec, SIGNAL( lostFocus() ), this, SLOT( slotUpdateObjectCount() ) );
	connect( olw->Radius, SIGNAL( lostFocus() ), this, SLOT( slotUpdateObjectCount() ) );
	connect( olw->Date, SIGNAL( valueChanged( ExtDate ) ), this, SLOT( slotUpdateObjectCount() ) );
	connect( olw->Mag, SIGNAL( valueChanged( double ) ), this, SLOT( slotUpdateObjectCount() ) );
	connect( olw->IncludeNoMag, SIGNAL( clicked() ), this, SLOT( slotUpdateObjectCount() ) );

	connect( this, SIGNAL( okClicked() ), this, SLOT( slotApplyFilters() ) );

	initialize();
}

ObsListWizard::~ObsListWizard()
{
}

void ObsListWizard::initialize()
{
	olw->olwStack->setCurrentIndex( 0 );

	//Populate the list of constellations
	foreach ( SkyObject *p, ksw->data()->skyComposite()->constellationNames() )
		olw->ConstellationList->addItem( p->name() );

	//unSelect all object types
	olw->TypeList->clearSelection();

	olw->Mag->setMinimum( -5.0 );
	olw->Mag->setMaximum( 20.0 );
	olw->Mag->setValue( 6.0 );

	olw->RA->setDegType( false );
	olw->RAMin->setDegType( false );
	olw->RAMax->setDegType( false );

	//Initialize object counts
	ObjectCount = 0; //number of objects in observing list
	StarCount = ksw->data()->skyComposite()->stars().size(); //total number of stars
	PlanetCount = 10; //Sun, Moon, 8 planets
	AsteroidCount = ksw->data()->skyComposite()->asteroids().size(); //total number of asteroids
	CometCount = ksw->data()->skyComposite()->comets().size(); //total number of comets
	//DeepSkyObjects
	OpenClusterCount = 0;
	GlobClusterCount = 0;
	GasNebCount = 0;
	PlanNebCount = 0;
	GalaxyCount = 0;

	foreach ( DeepSkyObject *o, ksw->data()->skyComposite()->deepSkyObjects() ) {
		if ( o->type() == SkyObject::GALAXY ) ++GalaxyCount; //most deepsky obj are galaxies, so check them first
		else if ( o->type() == SkyObject::STAR || o->type() == SkyObject::CATALOG_STAR ) ++StarCount;
		else if ( o->type() == SkyObject::OPEN_CLUSTER ) ++OpenClusterCount;
		else if ( o->type() == SkyObject::GLOBULAR_CLUSTER ) ++GlobClusterCount;
		else if ( o->type() == SkyObject::GASEOUS_NEBULA || o->type() == SkyObject::SUPERNOVA_REMNANT ) ++GasNebCount;
		else if ( o->type() == SkyObject::PLANETARY_NEBULA ) ++PlanNebCount;
	}
}

bool ObsListWizard::isItemSelected( const QString &name, QListWidget *listWidget, bool *ok ) {
	bool result = false;
	QList<QListWidgetItem*> items = listWidget->findItems( name, Qt::MatchContains );
	if ( items.size() ) result = listWidget->isItemSelected( items[0] );
	if ( ok ) *ok = (items.size()) ? true : false;
	return result;
}

void ObsListWizard::setItemSelected( const QString &name, QListWidget *listWidget, bool value, bool *ok ) {
	QList<QListWidgetItem*> items = listWidget->findItems( name, Qt::MatchContains );
	if ( items.size() ) listWidget->setItemSelected( items[0], value );
	if ( ok ) *ok = (items.size()) ? true : false;
}

//Advance to the next page in the stack.  However, on page 2 the user
//selects what regional filter they want to use, and this determnes
//what the page following page 2 should be:
// + Constellation(s): the next page index is 3
// + Rectangular region: the next page index is 4
// + Circular region: the next page index is 5
// + No region selected (a.k.a. "all over the sky"): the next page index is 6
//
//Also, if the current page index is 3 or 4, then the next page should be 6.
//
//NOTE: the page indexes are hard-coded here, which isn't ideal.  However,
//There's no easy way to access the pointers of widgets in the stack
//if you didn't save them at the start.
void ObsListWizard::slotNextPage() {
	int NextPage = olw->olwStack->currentIndex() + 1;

	if ( olw->olwStack->currentIndex() == 2 ) {
		//On the Region select page.  Determine what
		//the next page index should be.
		//No need to handle "by constellation, it's already currentIndex + 1.
		if ( isItemSelected( i18n("in a rectangular region"), olw->RegionList ) )
			NextPage = 4;
		if ( isItemSelected( i18n("in a circular region"), olw->RegionList ) )
			NextPage = 5;
		if ( isItemSelected( i18n("all over the sky"), olw->RegionList ) )
			NextPage = 6;
	}

	if ( olw->olwStack->currentIndex() == 3 || olw->olwStack->currentIndex() == 4 )
		NextPage = 6;

	olw->olwStack->setCurrentIndex( NextPage );
	if ( olw->olwStack->currentIndex() == olw->olwStack->count() - 1 )
		enableButton( KDialog::User1, false );

	enableButton( KDialog::User2, true );
}

//Advance to the previous page in the stack.  However, because the
//path through the wizard branches depending on the user's choice of
//Region filter, the previous page is not always currentPage-1.
//Specifically, if the current page index is 4, 5, or 6, then the Previous
//page index should be 2 rather than currentIndex-1.
void ObsListWizard::slotPrevPage() {
	int PrevPage = olw->olwStack->currentIndex() - 1;

	if ( olw->olwStack->currentIndex() == 4
			|| olw->olwStack->currentIndex() == 5
			|| olw->olwStack->currentIndex() == 6 )
		PrevPage = 2;

	olw->olwStack->setCurrentIndex( PrevPage );
	if ( olw->olwStack->currentIndex() == 0 )
		enableButton( KDialog::User2, false );

	enableButton( KDialog::User1, true );
}

void ObsListWizard::slotAllButton() {
	for ( int i=0; i<olw->TypeList->count(); ++i )
		olw->TypeList->setItemSelected( olw->TypeList->item(i), true );
}

void ObsListWizard::slotNoneButton() { olw->TypeList->clearSelection(); }

void ObsListWizard::slotDeepSkyButton()
{
	olw->TypeList->clearSelection();
	setItemSelected( i18n( "Open clusters"     ), olw->TypeList, true );
	setItemSelected( i18n( "Globular clusters" ), olw->TypeList, true );
	setItemSelected( i18n( "Gaseous nebulae"   ), olw->TypeList, true );
	setItemSelected( i18n( "Planetary nebulae" ), olw->TypeList, true );
	setItemSelected( i18n( "Galaxies"          ), olw->TypeList, true );
}

void ObsListWizard::slotSolarSystemButton()
{
	olw->TypeList->clearSelection();
	setItemSelected( i18n( "Sun, moon, planets" ), olw->TypeList, true );
	setItemSelected( i18n( "Comets"             ), olw->TypeList, true );
	setItemSelected( i18n( "Asteroids"          ), olw->TypeList, true );
}

void ObsListWizard::slotChangeLocation()
{
	LocationDialog ld( ksw );

	if ( ld.exec() == QDialog::Accepted ) {
		//set geographic location
	}
}

void ObsListWizard::slotUpdateObjectCount()
{
	ObjectCount = 0;
	if ( isItemSelected( i18n( "Stars" ), olw->TypeList ) )
		ObjectCount += StarCount;
	if ( isItemSelected( i18n( "Sun, moon, planets" ), olw->TypeList ) )
		ObjectCount += PlanetCount;
	if ( isItemSelected( i18n( "Comets" ), olw->TypeList ) )
		ObjectCount += CometCount;
	if ( isItemSelected( i18n( "Asteroids" ), olw->TypeList ) )
		ObjectCount += AsteroidCount;
	if ( isItemSelected( i18n( "Galaxies" ), olw->TypeList ) )
		ObjectCount += GalaxyCount;
	if ( isItemSelected( i18n( "Open clusters" ), olw->TypeList ) )
		ObjectCount += OpenClusterCount;
	if ( isItemSelected( i18n( "Globular clusters" ), olw->TypeList ) )
		ObjectCount += GlobClusterCount;
	if ( isItemSelected( i18n( "Gaseous nebulae" ), olw->TypeList ) )
		ObjectCount += GasNebCount;
	if ( isItemSelected( i18n( "Planetary nebulae" ), olw->TypeList ) )
		ObjectCount += PlanNebCount;

	applyFilters( false ); //false = only adjust counts, do not build list
}

void ObsListWizard::applyFilters( bool doBuildList )
{
	if ( doBuildList )
		obsList().clear();

	//We don't need to call applyRegionFilter() if no region filter is selected, *and*
	//we are just counting items (i.e., doBuildList is false)
	bool needRegion = true;
	if ( !doBuildList && isItemSelected( i18n( "all over the sky" ), olw->RegionList ) ) needRegion = false;

	double maglimit = 100.;
	if ( olw->SelectByMagnitude->isChecked() ) maglimit = olw->Mag->value();

	//Stars
	if ( isItemSelected( i18n( "Stars" ), olw->TypeList ) ) {
		QList<SkyObject*>& starList = ksw->data()->skyComposite()->stars();
		int starIndex( starList.size() );
		if ( maglimit < 100. ) {
			//Stars are sorted by mag, so use binary search algo to find index of faintest mag
			int low(0), high(starList.size()-1), middle(high);
			while ( low < high ) {
				middle = (low + high)/2;
				if ( maglimit == starList.at(middle)->mag() ) break;
				if ( maglimit <  starList.at(middle)->mag() ) high = middle - 1;
				if ( maglimit >  starList.at(middle)->mag() ) low = middle + 1;
			}
			//now, the star at "middle" has the right mag, but we want the *last* star that has this mag.
			for ( starIndex=middle+1; starIndex < starList.size(); ++starIndex ) {
				if ( starList.at(starIndex)->mag() > maglimit ) break;
			}
		}

		if ( doBuildList ) {
			for ( int i=0; i < starIndex; ++i ) {
				SkyObject *o = (SkyObject*)(starList[i]);
				if ( needRegion ) applyRegionFilter( o, doBuildList, false ); //false = don't adjust ObjectCount
			}
		} else {
			ObjectCount -= (starList.size() - starIndex); //reduce StarCount by appropriate amount
			for ( int i=0; i < starIndex; ++i ) {
				SkyObject *o = starList[i];
				if ( needRegion ) applyRegionFilter( o, doBuildList );
			}
		}
	}

	//Sun, Moon, Planets
	if ( isItemSelected( i18n( "Sun, moon, planets" ), olw->TypeList ) ) {
		if ( needRegion ) {
			applyRegionFilter( (SkyObject*)ksw->data()->skyComposite()->findByName("Sun"), doBuildList );
			applyRegionFilter( (SkyObject*)ksw->data()->skyComposite()->findByName("Moon"), doBuildList );
			applyRegionFilter( (SkyObject*)ksw->data()->skyComposite()->findByName("Mercury"), doBuildList );
			applyRegionFilter( (SkyObject*)ksw->data()->skyComposite()->findByName("Venus"), doBuildList );
			applyRegionFilter( (SkyObject*)ksw->data()->skyComposite()->findByName("Mars"), doBuildList );
			applyRegionFilter( (SkyObject*)ksw->data()->skyComposite()->findByName("Jupiter"), doBuildList );
			applyRegionFilter( (SkyObject*)ksw->data()->skyComposite()->findByName("Saturn"), doBuildList );
			applyRegionFilter( (SkyObject*)ksw->data()->skyComposite()->findByName("Uranus"), doBuildList );
			applyRegionFilter( (SkyObject*)ksw->data()->skyComposite()->findByName("Neptune"), doBuildList );
			applyRegionFilter( (SkyObject*)ksw->data()->skyComposite()->findByName("Pluto"), doBuildList );
		}
	}

	//Deep sky objects
	bool dso = ( isItemSelected( i18n( "Open clusters" ), olw->TypeList )
			|| isItemSelected( i18n( "Globular clusters" ), olw->TypeList )
			|| isItemSelected( i18n( "Gaseous nebulae" ), olw->TypeList )
			|| isItemSelected( i18n( "Planetary nebulae" ), olw->TypeList )
			|| isItemSelected( i18n( "Galaxies" ), olw->TypeList ) );

	if ( dso ) {
		//Don't need to do anything if we are just counting objects and not
		//filtering by region or magnitude
		if ( needRegion || olw->SelectByMagnitude->isChecked() ) {
			foreach ( DeepSkyObject *o, ksw->data()->skyComposite()->deepSkyObjects() ) {
				//Skip unselected object types
				bool typeSelected = false;
				if ( (o->type() == SkyObject::STAR || o->type() == SkyObject::CATALOG_STAR) &&
						isItemSelected( i18n( "Stars" ), olw->TypeList ) )
					typeSelected = true;
				if ( o->type() == SkyObject::OPEN_CLUSTER &&
						isItemSelected( i18n( "Open clusters" ), olw->TypeList ) )
					typeSelected = true;
				if ( o->type() == SkyObject::GLOBULAR_CLUSTER &&
						isItemSelected( i18n( "Globular clusters" ), olw->TypeList ) )
					typeSelected = true;
				if ( (o->type() == SkyObject::GASEOUS_NEBULA || o->type() == SkyObject::SUPERNOVA_REMNANT) &&
						isItemSelected( i18n( "Gaseous nebulae" ), olw->TypeList ) )
					typeSelected = true;
				if ( o->type() == SkyObject::PLANETARY_NEBULA &&
						isItemSelected( i18n( "Planetary nebulae" ), olw->TypeList ) )
					typeSelected = true;
				if ( o->type() == SkyObject::GALAXY &&
						isItemSelected( i18n( "Galaxies" ), olw->TypeList ) )
					typeSelected = true;
				if ( ! typeSelected ) continue;

				if ( olw->SelectByMagnitude->isChecked() ) {
					if ( o->mag() > 90. ) {
						if ( olw->IncludeNoMag->isChecked() )
							if ( needRegion ) applyRegionFilter( o, doBuildList );
						else if ( ! doBuildList )
							--ObjectCount;
					} else {
						if ( o->mag() <= maglimit )
							if ( needRegion ) applyRegionFilter( o, doBuildList );
						else if ( ! doBuildList )
							--ObjectCount;
					}
				} else {
					if ( needRegion ) applyRegionFilter( o, doBuildList );
				}
			}
		}
	}

	//Comets
	if ( needRegion && isItemSelected( i18n( "Comets" ), olw->TypeList ) ) {
		foreach ( SkyObject *o, ksw->data()->skyComposite()->comets() ) {
			//comets don't have magnitudes at this point, so skip mag check
			applyRegionFilter( o, doBuildList );
		}
	}

	//Asteroids
	if ( (needRegion || olw->SelectByMagnitude->isChecked() ) && isItemSelected( i18n( "Asteroids" ), olw->TypeList ) ) {
		foreach ( SkyObject *o, ksw->data()->skyComposite()->asteroids() ) {
			if ( olw->SelectByMagnitude->isChecked() ) {
				if ( o->mag() > 90. ) {
					if ( olw->IncludeNoMag->isChecked() )
						if ( needRegion ) applyRegionFilter( o, doBuildList );
					else if ( ! doBuildList )
						--ObjectCount;
				} else {
					if ( o->mag() <= maglimit )
						if ( needRegion ) applyRegionFilter( o, doBuildList );
					else if ( ! doBuildList )
						--ObjectCount;
				}
			} else {
				if ( needRegion ) applyRegionFilter( o, doBuildList );
			}
		}
	}

	//Update the object count label
	if ( doBuildList ) ObjectCount = obsList().size();
	olw->CountLabel->setText( i18np("Your observing list currently has 1 object", "Your observing list currently has %1 objects", ObjectCount ) );
}

void ObsListWizard::applyRegionFilter( SkyObject *o, bool doBuildList,
		bool doAdjustCount ) {

	//select by constellation
	if ( isItemSelected( i18n("by constellation"), olw->RegionList ) ) {
		QString c( ksw->data()->skyComposite()->constellation( o ) );
		if ( isItemSelected( c, olw->ConstellationList ) ) {
			if ( doBuildList ) obsList().append ( o );
		} else if ( doAdjustCount ) --ObjectCount;
	}

	//select by rectangular region
	else if ( isItemSelected( i18n("in a rectangular region"), olw->RegionList ) ) {
		bool rectOk = false;
		double ra1, ra2, dc1, dc2;

		ra1 = olw->RAMin->createDms( false, &rectOk ).Hours();
		if ( rectOk ) ra2 = olw->RAMax->createDms( false, &rectOk ).Hours();
		if ( rectOk ) dc1 = olw->DecMin->createDms( true, &rectOk ).Degrees();
		if ( rectOk ) dc2 = olw->DecMax->createDms( true, &rectOk ).Degrees();
		if ( ra2 == 0.0 ) ra2 = 24.0;

		if ( !rectOk ) {
			kWarning() << i18n( "Illegal rectangle specified, no region selection possible." ) << endl;
			return;
		}

		//Make sure dc1 < dc2.
		if ( dc1 > dc2 ) {
			double temp = dc2;
			dc2 = dc1;
			dc1 = temp;
		}

		//If ra1 > ra2, we may need to swap the two values, or subtract 24h from ra1.
		if ( ra1 > ra2 ) {
			if ( ra1 - ra2 > 12.0 ) { //the user probably wants a region that straddles 0h
				ra1 -= 24.0;
			} else { //the user probably wants ra2 to be the lower limit
				double temp = ra2;
				ra2 = ra1;
				ra1 = temp;
			}
		}

		double ra = o->ra()->Hours();
		double dec = o->dec()->Degrees();
		bool addObject = false;
		if ( dec >= dc1 && dec <= dc2 ) {
			if ( ra1 < 0.0 ) {
				if (ra >= ra1 + 24.0 || ra <= ra2 ) { addObject = true; }
			} else {
				if ( ra >= ra1 && ra <= ra2 ) { addObject = true; }
			}
		}

		if ( addObject && doBuildList ) obsList().append( o );
		if ( ! addObject && doAdjustCount ) --ObjectCount;
	}

	//select by circular region
	//make sure circ region data are valid
	else if ( isItemSelected( i18n("in a circular region"), olw->RegionList ) ) {
		bool circOk = false;
		double rCirc;
		SkyPoint pCirc;

		double ra = olw->RA->createDms( false, &circOk ).Hours();
		double dc(0.0);
		if ( circOk ) dc = olw->Dec->createDms( true, &circOk ).Degrees();
		if ( circOk ) {
			pCirc.set( ra, dc );
			rCirc = olw->Radius->createDms( true, &circOk ).Degrees();
		}

		if ( !circOk ) {
			kWarning() << i18n( "Illegal circle specified, no region selection possible." ) << endl;
			return;
		}

		if ( o->angularDistanceTo( &pCirc ).Degrees() < rCirc ) {
			if ( doBuildList ) obsList().append( o );
		} else if ( doAdjustCount ) --ObjectCount;
	}

	//No region filter, just add the object
	else if ( doBuildList ) {
		obsList().append( o );
	}
}

#include "obslistwizard.moc"
