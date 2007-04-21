/*****************************************************************
 * Copyright (c) 2004-2007,	Jan-Rixt Van Hoye					 *
 * All rights reserved.											 *
 * Distributed under the terms of the MIT License.               *
 *****************************************************************/
 
//		System Includes
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream.h>
#include <NodeInfo.h>

//		User Includes
#include "level1.h"
#include "level2.h"
#include "debug.h"
//
FILE *lflevel1;
//		USB Roster class
class Roster : public BUSBRoster
{
public:
virtual status_t DeviceAdded(BUSBDevice *dev)
{
	// Initialize the USB device
	int i,j;
	const BUSBConfiguration *conf;
	const BUSBInterface *ifc;
	
	for(i=0;i<(int)dev->CountConfigurations();i++)
	{
		conf = dev->ConfigurationAt(i);
		if(conf)
		{
			for(j=0;j<(int)conf->CountInterfaces();j++)
			{
				ifc = conf->InterfaceAt(j);	
				if(ifc)
					if(ifc->Class() == 8)
					{
						#ifdef DEBUG
						lflevel1 = fopen(LOGFILE,"a");
						fprintf(lflevel1,"Device %s found and attached!\n",dev->ProductString());
						fclose(lflevel1);
						#endif
						appDev = dev;
						if(appDev->InitCheck() || appDev->SetConfiguration(appDev->ConfigurationAt(0)))
							logError(MSCAM_DEV_NO_FIND);
						else
						{
							// create the mass storage interface
							mscam = new MSDInterface(appDev);
							//PTP_init_ptp_usb(params,appDev);
							// send a message to the system core
							BMessage *core_msg;
							core_msg = new BMessage(CAM_CONNECTED);
							core_msg->AddString("product",dev->ProductString());
							if(msgtarget != NULL)
							{
								#ifdef DEBUG
								lflevel1 = fopen(LOGFILE,"a");
								fprintf(lflevel1,"MS: Send message to the system core\n");
								fclose(lflevel1);
								#endif
								msgtarget->PostMessage(core_msg);
							}
						}
					}	
			}
		}
	}
	
	return B_OK;
}
virtual void DeviceRemoved(BUSBDevice *dev)
	{
		fprintf(stderr,"removed %s @ '%s'\n",dev->IsHub() ? "hub" : "device", dev->Location());
		appDev = NULL;
		// send a message to the system core
		BMessage *core_msg;
		core_msg = new BMessage(CAM_DISCONNECTED);
		core_msg->AddString("product",dev->ProductString());
		if(msgtarget != NULL)
		{
			#ifdef DEBUG
			lflevel1 = fopen(LOGFILE,"a");
			fprintf(lflevel1,"MS: Send message to the system core\n");
			fclose(lflevel1);
			#endif
			msgtarget->PostMessage(core_msg);
		}
;	}
};

int get_BDCP_API_Revision(void)
{
	return(2);
}

void getPluginVersion(version_info &ver)
{
	ver.major = 1;
	ver.middle = 0;
	ver.minor = 0;
	ver.variety = 0;
	ver.internal = 0;
	sprintf(ver.short_info,"Jan-Rixt Van Hoye 2007");
	sprintf(ver.long_info,"BDCP Mass Storage Cameras Plugin");
}

void getSupportedCameras(vector<string> & listofcams)
{
	listofcams.push_back("Mass Storage camera");
}

status_t openCamera(void)
{
	// Check USB Devices
	//params = new PTPParams;
	#ifdef DEBUG
	lflevel1 = fopen(LOGFILE,"a");
	fprintf(lflevel1,"MS - Start the roster\n");
	fclose(lflevel1);
	#endif
	roster = new Roster;
	roster->Start();
	#ifdef DEBUG
	lflevel1 = fopen(LOGFILE,"a");
	fprintf(lflevel1,"MS - Roster started\n");
	fclose(lflevel1);
	#endif
	return(B_NO_ERROR);
}

status_t closeCamera(void)
{
	// Close the camera
	#ifdef DEBUG
	lflevel1 = fopen(LOGFILE,"a");
	fprintf(lflevel1,"MS - Close camera\n");
	fclose(lflevel1);
	#endif
	delete(mscam);
	roster->Stop();
	return(B_NO_ERROR);
}

status_t getNumberofPics(int &number)
{
	int i=0;
	number = 0;
	#ifdef DEBUG
	lflevel1 = fopen(LOGFILE,"a");
	fprintf(lflevel1,"MS - Get number of pictures\n");
	fclose(lflevel1);
	#endif
	number = mscam->getNumberOfItems();
	#ifdef DEBUG
	lflevel1 = fopen(LOGFILE,"a");
	fprintf(lflevel1,"MS - Number of pictures is: %d\n",number);
	fclose(lflevel1);
	#endif	
	return(B_NO_ERROR);
}

status_t setCurrentPicture(int picturenum)
{
	#ifdef DEBUG
	lflevel1 = fopen(LOGFILE,"a");
	fprintf(lflevel1,"MS - Set current picture\n");
	fclose(lflevel1);
	#endif
	currentitemhandle = picturenum;
	#ifdef DEBUG
	lflevel1 = fopen(LOGFILE,"a");
	fprintf(lflevel1,"MS - Current picnumber is: %d\n",picturenum);
	//fprintf(lflevel1,"MS - Current picnumber(handle) is: %d\n",currentitemhandle);
	fclose(lflevel1);
	#endif
	return(B_NO_ERROR);
}

status_t downloadPicture(BPath savedir)
{
	#ifdef DEBUG
	lflevel1 = fopen(LOGFILE,"a");
	fprintf(lflevel1,"MS - Download pictures\n");
	fclose(lflevel1);
	#endif
	mscam->downloadItem(currentitemhandle,savedir);
	return(B_NO_ERROR);
}

