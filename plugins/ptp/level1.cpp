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
					if(ifc->Class() == 6)
					{
						#ifdef DEBUG
						lflevel1 = fopen(LOGFILE,"a");
						fprintf(lflevel1,"Device %s found and attached!\n",dev->ProductString());
						fclose(lflevel1);
						#endif
						appDev = dev;
						if(appDev->InitCheck() || appDev->SetConfiguration(appDev->ConfigurationAt(0)))
							logError(PTPCAM_DEV_NO_FIND);
						else
						{
							// initialize the ptp device
							if(PTP_init_ptp_usb(params,appDev) == B_OK)
							{ 
								ptp_opensession(params,1);
								// send a message to the system core
								BMessage *core_msg;
								core_msg = new BMessage(CAM_CONNECTED);
								core_msg->AddString("product",dev->ProductString());
								if(msgtarget != NULL)
								{
									#ifdef DEBUG
									lflevel1 = fopen(LOGFILE,"a");
									fprintf(lflevel1,"PTP: Send message to the system core\n");
									fclose(lflevel1);
									#endif
									msgtarget->PostMessage(core_msg);
								}
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
		//ptp_closesession(params);
		appDev = NULL;
		// send a message to the system core
		BMessage *core_msg;
		core_msg = new BMessage(CAM_DISCONNECTED);
		core_msg->AddString("product",dev->ProductString());
		if(msgtarget != NULL)
		{
			#ifdef DEBUG
			lflevel1 = fopen(LOGFILE,"a");
			fprintf(lflevel1,"PTP: Send message to the system core\n");
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
	ver.major = 2;
	ver.middle = 0;
	ver.minor = 0;
	ver.variety = 0;
	ver.internal = 0;
	sprintf(ver.short_info,"Jan-Rixt Van Hoye 2007");
	sprintf(ver.long_info,"BDCP PTP Cameras Plugin");
}

void getSupportedCameras(vector<string> & listofcams)
{
	listofcams.push_back("PTP(Picture Transfer Protocol) camera");
	listofcams.push_back("Nikon Coolpix 2000");
	listofcams.push_back("Nikon Coolpix SQ");
}

status_t openCamera(void)
{
	// Check USB Devices
	params = new PTPParams;
	#ifdef DEBUG
	lflevel1 = fopen(LOGFILE,"a");
	fprintf(lflevel1,"PTP - Start the roster\n");
	fclose(lflevel1);
	#endif
	roster = new Roster;
	roster->Start();
	#ifdef DEBUG
	lflevel1 = fopen(LOGFILE,"a");
	fprintf(lflevel1,"PTP - Roster started\n");
	fclose(lflevel1);
	#endif
	return(B_NO_ERROR);
}

status_t closeCamera(void)
{
	// Close the camera
	#ifdef DEBUG
	lflevel1 = fopen(LOGFILE,"a");
	fprintf(lflevel1,"PTP - Close camera\n");
	fclose(lflevel1);
	#endif
	//ptp_closesession(params);
	roster->Stop();
	return(B_NO_ERROR);
}

status_t getNumberofPics(int &number)
{
	int i=0;
	number = 0;
	#ifdef DEBUG
	lflevel1 = fopen(LOGFILE,"a");
	fprintf(lflevel1,"PTP - Get number of pictures\n");
	fclose(lflevel1);
	#endif
	//ptp_opensession(params,1);
	ptp_getobjecthandles(params, 0xffffffff, 0x000000, 0x000000,&params->handles);
	#ifdef DEBUG
	lflevel1 = fopen(LOGFILE,"a");
	fprintf(lflevel1,"PTP - Got objecthandles\n");
	fclose(lflevel1);
	#endif
	if((*params).handles.n == 0)
	{
		logError(PTPCAM_NO_HANDLES);
		return(B_ERROR);
	}
	else
	{
		#ifdef DEBUG
		lflevel1 = fopen(LOGFILE,"a");
		fprintf(lflevel1,"PTP - Part1\n");
		fclose(lflevel1);
		#endif
		handles = new int[(*params).handles.n];
		(*params).objectinfo =(PTPObjectInfo*)malloc(sizeof(PTPObjectInfo)* (*params).handles.n);
		#ifdef DEBUG
		lflevel1 = fopen(LOGFILE,"a");
		fprintf(lflevel1,"PTP - Part2\n");
		fclose(lflevel1);
		#endif
		for (uint32 j = 0; j < (*params).handles.n;j++)
		{
			ptp_getobjectinfo(params,(*params).handles.Handler[j],&params->objectinfo[j]);
			#ifdef DEBUG
			lflevel1 = fopen(LOGFILE,"a");
			fprintf(lflevel1,"PTP - Part2 - %d\n",j);
			fclose(lflevel1);
			#endif
			if((*params).objectinfo[j].ObjectFormat != PTP_OFC_Undefined && (*params).objectinfo[j].ObjectFormat != PTP_OFC_Association && (*params).objectinfo[j].ObjectFormat != PTP_OFC_DPOF)
			{
				handles[i] = j;//(*params).handles.Handler[j];
				i++;
			}
		}
		number = i;
		#ifdef DEBUG
		lflevel1 = fopen(LOGFILE,"a");
		fprintf(lflevel1,"PTP - Get part3\n");
		fclose(lflevel1);
		#endif
	}
	//ptp_closesession(params);
	#ifdef DEBUG
	lflevel1 = fopen(LOGFILE,"a");
	fprintf(lflevel1,"PTP - Number of pictures is: %d\n",number);
	fclose(lflevel1);
	#endif	
	return(B_NO_ERROR);
}

status_t setCurrentPicture(int picturenum)
{
	#ifdef DEBUG
	lflevel1 = fopen(LOGFILE,"a");
	fprintf(lflevel1,"PTP - Set current picture\n");
	fclose(lflevel1);
	#endif
	currentpicturenumber = handles[picturenum];
	#ifdef DEBUG
	lflevel1 = fopen(LOGFILE,"a");
	fprintf(lflevel1,"PTP - Current picnumber is: %d\n",picturenum);
	fprintf(lflevel1,"PTP - Current picnumber(handle) is: %d\n",currentpicturenumber);
	fclose(lflevel1);
	#endif
	return(B_NO_ERROR);
}

status_t downloadPicture(BPath savedir)
{
	char *image = NULL;
	char filename[255];
	long int size=0;
	int ret=0;
	
	#ifdef DEBUG
	lflevel1 = fopen(LOGFILE,"a");
	fprintf(lflevel1,"PTP - Download pictures\n");
	fclose(lflevel1);
	#endif
	//ptp_opensession(params,1);
	 // BEGIN Bugfix: 09/03/2007
    //(*params).objectinfo =(PTPObjectInfo*)malloc(sizeof(PTPObjectInfo));
	//ptp_getobjectinfo(params,currentitemhandle,&params->objectinfo[0]);
    #ifdef DEBUG
	lflevel1 = fopen(LOGFILE,"a");
	fprintf(lflevel1,"Type: %d\n",&params->objectinfo[0].ObjectFormat);
	fclose(lflevel1);
	#endif
	//size=(*params).objectinfo[0].ObjectCompressedSize;
	size=(*params).objectinfo[currentpicturenumber].ObjectCompressedSize;
	// END
	//ret = ptp_getobject(params,currentitemhandle,&image);
	ret = ptp_getobject(params,(*params).handles.Handler[currentpicturenumber],&image);
	if ( ret == PTP_RC_OK)
	{
		strcpy(filename,savedir.Path());
		strcat(filename,"/");
		strcat(filename,(*params).objectinfo[currentpicturenumber].Filename);
		//strcat(filename,(*params).objectinfo[0].Filename);
		if(saveCamPicture(image,size,filename))
		{	
			image = NULL;
			//ptp_closesession(params);
			return(B_NO_ERROR);
		}
			
	}
	//ptp_closesession(params);
	return(B_ERROR);
}

bool saveCamPicture (char *data, long int size, const char filename[255])
{
	int					systemresult;
	BFile				*fh;
	BNodeInfo			*ni;
	
	#ifdef DEBUG
	lflevel1 = fopen(LOGFILE,"a");
	fprintf(lflevel1,"PTP - Save picture @ %s with size %d\n", filename,size);
	fclose(lflevel1);
	#endif
	if((fh=new BFile(filename, B_WRITE_ONLY | B_CREATE_FILE )))
	{
		long int			fherr;

		if((fherr = fh->InitCheck()) != B_OK)
			return(B_ERROR);
		if((ni=new BNodeInfo(fh)))
		{
			ni->SetType("image/jpeg");
			delete ni;
		}
		if(( (fherr = fh->Write(data, size)) != size))
		{
			delete fh;
			return(B_ERROR);			
		}
		else
		{
			delete fh;
			//return(B_ERROR);
		}
	}
	systemresult=-1;
	
return (B_NO_ERROR);
}

