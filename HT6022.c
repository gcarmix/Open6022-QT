/**
  ******************************************************************************
  * @file    HT6022.c
  * @author  Rodrigo Pedroso Mendes
  * @email   rpm_2003_rpm@yahoo.com.br
  * @version V1.0
  * @date    04-20-2013
  * @brief   This file provides functions to manage the following 
  *          functionalities of the HT6022 PC oscilloscope :          
  *           - Initialization and Configuration
  *           - Read
  *           - Set sample rate
  *           - Set input range
  *           - Set and get calibration levels
  * 
  *  @verbatim
  *
  *          ===================================================================
  *                                 How to use this driver
  *          ===================================================================  
  *  @endverbatim        
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>
#include "HT6022.h"
#include "HT6022fw.h"
#include <stdio.h>

#ifdef __cplusplus
 extern "C" {
#endif

/** @addtogroup HT6022_Driver
  * @{
  */ 

/* Private define ------------------------------------------------------------*/
#define HT6022_VENDOR_ID          0X4B5
#define HT6022_MODEL              0X602A
#define HT6022_IR1_REQUEST_TYPE 0X40
#define HT6022_IR1_REQUEST      0XE0
#define HT6022_IR1_VALUE        0X00
#define HT6022_IR1_INDEX        0X00
#define HT6022_IR1_SIZE         0X01
#define HT6022_IR2_REQUEST_TYPE 0X40
#define HT6022_IR2_REQUEST      0XE1
#define HT6022_IR2_VALUE        0X00
#define HT6022_IR2_INDEX        0X00
#define HT6022_IR2_SIZE         0X01
#define HT6022_SR_REQUEST_TYPE   0X40
#define HT6022_SR_REQUEST        0XE2
#define HT6022_SR_VALUE          0X00
#define HT6022_SR_INDEX          0X00
#define HT6022_SR_SIZE           0X01
#define HT6022_SETCALLEVEL_REQUEST_TYPE   0XC0
#define HT6022_SETCALLEVEL_REQUEST        0XA2
#define HT6022_SETCALLEVEL_VALUE          0X08
#define HT6022_SETCALLEVEL_INDEX          0X00
#define HT6022_GETCALLEVEL_REQUEST_TYPE   0X40
#define HT6022_GETCALLEVEL_REQUEST        0XA2
#define HT6022_GETCALLEVEL_VALUE          0X08
#define HT6022_GETCALLEVEL_INDEX          0X00
#define HT6022_READ_CONTROL_REQUEST_TYPE  0X40
#define HT6022_READ_CONTROL_REQUEST       0XE3
#define HT6022_READ_CONTROL_VALUE 	  0X00
#define HT6022_READ_CONTROL_INDEX         0X00
#define HT6022_READ_CONTROL_SIZE          0X01
#define HT6022_READ_CONTROL_DATA          0X01
#define HT6022_READ_BULK_PIPE             0X86


/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
unsigned char HT6022_AddressList [256] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	
	


/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

/* Public functions ---------------------------------------------------------*/

/** @defgroup HT6022_Driver_Group1  Initialization and Configuration
 *  @brief    Initialization and Configuration
 *
@verbatim   

 ===============================================================================
                          Initialization and Configuration
 =============================================================================== 
 

@endverbatim
  * @{
  */
/**
  * @brief  Initialize HT6022 library.
  *         This function must be called before calling any other HT6022_Driver function.
  * @param  None
  * @retval Error Code. See HT6022_ErrorTypeDef
  */
HT6022_ErrorTypeDef HT6022_Init (void)
{
	if (libusb_init(NULL) == 0)
		return HT6022_SUCCESS;
	else
		return HT6022_ERROR_OTHER; 
}

/**
  * @brief  Deinitialize HT6022_Driver.
  *	    Should be called after closing all open devices (HT6022_DeviceDeinit()) and before your application terminates. 
  * @param  None
  * @retval None
  */
void HT6022_Exit (void)
{
	libusb_exit(NULL);
}

/**
  * @brief  This function uploads the HT6022 firmware.
  * @param  None
  * @retval Error Code. See HT6022_ErrorTypeDef
  */
HT6022_ErrorTypeDef HT6022_FirmwareUpload (void)
{
	libusb_device_handle  *Dev_handle;
	unsigned char* Firmware;
	unsigned int Size;
	unsigned int Value;
	int n;
    Dev_handle = libusb_open_device_with_vid_pid(NULL, HT6022_FIRMWARE_VENDOR_ID + 1, HT6022_MODEL);
    if (Dev_handle != 0)
        return HT6022_LOADED;

	Dev_handle = libusb_open_device_with_vid_pid(NULL, HT6022_FIRMWARE_VENDOR_ID, HT6022_MODEL);
	if (Dev_handle == 0)
 		return HT6022_ERROR_NO_DEVICE;
	
	if(libusb_kernel_driver_active(Dev_handle, 0) == 1)
		if(libusb_detach_kernel_driver(Dev_handle, 0) != 0)
		{
			libusb_close(Dev_handle);
			return HT6022_ERROR_OTHER;
		}

	if(libusb_claim_interface(Dev_handle, 0) < 0)
	{
		libusb_close(Dev_handle);
		return HT6022_ERROR_OTHER;
	}

	n = HT6022_FIRMWARE_SIZE;
	Firmware = HT6022_Firmware;
	while (n)
	{	
		Size  = *Firmware + ((*(Firmware + 1))<<0x08);
		Firmware = Firmware + 2;
		Value = *Firmware + ((*(Firmware + 1))<<0x08);
		Firmware = Firmware + 2;
		if (libusb_control_transfer (Dev_handle,     
                                     	     HT6022_FIRMWARE_REQUEST_TYPE,               
                                     	     HT6022_FIRMWARE_REQUEST, 
					     Value,           
                                     	     HT6022_FIRMWARE_INDEX, 
                                     	     Firmware, Size, 0) != Size)
		{
			libusb_release_interface(Dev_handle, 0);
			libusb_close(Dev_handle);
			return HT6022_ERROR_OTHER;
		}
		Firmware = Firmware + Size;
		n--;
	}
	libusb_release_interface(Dev_handle, 0);
	libusb_close(Dev_handle);
	return HT6022_SUCCESS;
}

/**
  * @brief  ---
  * @param  ---
  * @retval Error Code. See HT6022_ErrorTypeDef
  */
HT6022_ErrorTypeDef HT6022_DeviceOpen (HT6022_DeviceTypeDef *Device)
{
	struct libusb_device_descriptor desc;
	libusb_device         **DeviceList;
    libusb_device_handle  *DeviceHandle;
	int DeviceCount;
	int DeviceIterator;
	int r; 
	unsigned char Address;
	
	if (Device == NULL)
		return HT6022_ERROR_INVALID_PARAM;

	Device->Address = 0;
	Device->DeviceHandle = NULL;
	
	/* Get device list */
	DeviceCount = libusb_get_device_list(NULL, &DeviceList);
	if (DeviceCount <= 0) 
		return HT6022_ERROR_OTHER;

	for(DeviceIterator = 0; DeviceIterator < DeviceCount; DeviceIterator++) 
	{
		Address = libusb_get_device_address (DeviceList[DeviceIterator]);
		if (HT6022_AddressList[Address] == 0)
			/* Get device descriptor*/
			if (libusb_get_device_descriptor(DeviceList[DeviceIterator], &desc) == 0)
            {
                printf(">%X:%X\r\n",desc.idVendor, desc.idProduct);
				/* Check VID and PID*/    
				if ((desc.idVendor  == HT6022_VENDOR_ID)&&(desc.idProduct == HT6022_MODEL))
                    break;
            }
	}

	if (DeviceIterator == DeviceCount)
	{
		libusb_free_device_list(DeviceList, 1);
		return HT6022_ERROR_NO_DEVICE;
	}

	r = libusb_open(DeviceList[DeviceIterator], &DeviceHandle);
	libusb_free_device_list(DeviceList, 1);
	if (r != 0)
	{
		if (r != HT6022_ERROR_NO_MEM && r != HT6022_ERROR_ACCESS)
			r = HT6022_ERROR_OTHER;
		return r;
	}
		
	if(libusb_kernel_driver_active(DeviceHandle, 0) == 1)
		if(libusb_detach_kernel_driver(DeviceHandle, 0) != 0)
		{
			libusb_close(DeviceHandle);
			return HT6022_ERROR_OTHER;
		}
						
	if(libusb_claim_interface(DeviceHandle, 0) != 0)
	{
		libusb_close(DeviceHandle);
		return HT6022_ERROR_OTHER;
	}
	
	HT6022_AddressList [Address] = 0x01;
	Device->Address      = Address;
	Device->DeviceHandle = DeviceHandle;

	return HT6022_SUCCESS;
}

/**
  * @brief  Close a device handle.
  *         Should be called on all open handles before your application exits.
  *         This is a non-blocking function.
  * @param  Device: a device handle 
  * @retval None
  */
void HT6022_DeviceClose  (HT6022_DeviceTypeDef *Device)
{
	if (Device != NULL)
	{
		libusb_release_interface(Device->DeviceHandle, 0);
		libusb_close(Device->DeviceHandle);
		HT6022_AddressList[Device->Address] = 0x00;
		Device->DeviceHandle = NULL;
		Device->Address = 0;
	}
}
/**
  * @}
  */ 

/** @defgroup HT6022_Driver_Group2  Data read functions
 *  @brief    Data read functions
 *
@verbatim   
 ===============================================================================
                                Data read functions
 ===============================================================================  
@endverbatim
  * @{
  */
/**
  * @brief  ---
  * @param  ---
  * @retval Error Code. See HT6022_ErrorTypeDef
  */
HT6022_ErrorTypeDef HT6022_ReadData (HT6022_DeviceTypeDef *Device,
				     unsigned char* CH1, unsigned char* CH2,
				     HT6022_DataSizeTypeDef DataSize, 
                                     unsigned int  TimeOut)
{
	   unsigned char *data;
	   unsigned char *data_temp;
	   int nread;
   	   int r;
	   
	   data = (unsigned char*) malloc (sizeof(unsigned char)*DataSize*2);
	   if (data == 0)
	   		return HT6022_ERROR_NO_MEM;

  	   if ((!IS_HT6022_DATASIZE (DataSize)) || (Device == NULL) ||  (CH1 == NULL) ||  (CH2 == NULL))
			return HT6022_ERROR_INVALID_PARAM;
	   
	   *data = HT6022_READ_CONTROL_DATA;
	   r = libusb_control_transfer (Device->DeviceHandle, 
				        HT6022_READ_CONTROL_REQUEST_TYPE, 
					HT6022_READ_CONTROL_REQUEST, 
					HT6022_READ_CONTROL_VALUE, 
					HT6022_READ_CONTROL_INDEX, 
					data,
					HT6022_READ_CONTROL_SIZE, 0);

	  if (r != HT6022_READ_CONTROL_SIZE)
	  {
		if (r != HT6022_ERROR_NO_DEVICE)
			r = HT6022_ERROR_OTHER;
		return r;
	  }
	
	  r = libusb_bulk_transfer (Device->DeviceHandle,
			            HT6022_READ_BULK_PIPE, 
				    data, 
				    DataSize*2, 
				    &nread, 
				    TimeOut); 

	  if (r != HT6022_SUCCESS || nread != DataSize*2)
	  {
		if (r != HT6022_ERROR_NO_DEVICE && r != HT6022_ERROR_TIMEOUT)
			r = HT6022_ERROR_OTHER;
		return r;
	  } 
	  
	  data_temp = data;
	  while (nread)
	  {
		*CH1++ = *data_temp++;
		*CH2++ = *data_temp++;
		nread -= 2;
	  }
	  
	  free (data);

	  return HT6022_SUCCESS;

}
/**
  * @}
  */

/** @defgroup HT6022_Driver_Group3  Read and Write calibration levels functions
 *  @brief   Read and Write calibration levels functions
 *
@verbatim   
 ===============================================================================
                      Read and Write calibration value functions
 ===============================================================================  
@endverbatim
  * @{
  */
/**
  * @brief  ---
  * @param  ---
  * @retval Error Code. See HT6022_ErrorTypeDef
  */
HT6022_ErrorTypeDef HT6022_SetCalValues (HT6022_DeviceTypeDef *Device,
					 unsigned char* CalValues,
                                     	 HT6022_CVSizeTypeDef CVSize)
{
   	   int r;
	   if ((!IS_HT6022_CVSIZE (CVSize)) || (Device == NULL) ||  (CalValues == NULL))
		   return HT6022_ERROR_INVALID_PARAM;	
	   r = libusb_control_transfer (Device->DeviceHandle, 
				    HT6022_SETCALLEVEL_REQUEST_TYPE, 
					HT6022_SETCALLEVEL_REQUEST, 
					HT6022_SETCALLEVEL_VALUE, 
					HT6022_SETCALLEVEL_INDEX, 
					CalValues,
					CVSize, 0);
	  if (r != CVSize)
	  {
		if (r != HT6022_ERROR_NO_DEVICE)
			r =  HT6022_ERROR_OTHER;
		return r;
	  }
	  return HT6022_SUCCESS; 
}

/**
  * @brief  ---
  * @param  ---
  * @retval Error Code. See HT6022_ErrorTypeDef
  */
HT6022_ErrorTypeDef HT6022_GetCalValues (HT6022_DeviceTypeDef *Device,
					 unsigned char* CalValues,   
					 HT6022_CVSizeTypeDef CVSize)
{
   	   int r;
	   if ((!IS_HT6022_CVSIZE (CVSize)) || (Device == NULL) ||  (CalValues == NULL)) 
	       return HT6022_ERROR_INVALID_PARAM;	
	   r = libusb_control_transfer (Device->DeviceHandle, 
				        HT6022_GETCALLEVEL_REQUEST_TYPE, 
					HT6022_GETCALLEVEL_REQUEST, 
					HT6022_GETCALLEVEL_VALUE, 
					HT6022_GETCALLEVEL_INDEX, 
					CalValues,
					CVSize, 0);
	  if (r != CVSize)
	  {
		if (r != HT6022_ERROR_NO_DEVICE)
			r = HT6022_ERROR_OTHER;
		return r;
	  }

	  return HT6022_SUCCESS; 
}
/**
  * @}
  */

/** @defgroup HT6022_Driver_Group4  Sample rate and input voltage range functions
 *  @brief   Sample rate and input voltage range configuration
 *
@verbatim   
 ===============================================================================
                 Sample rate and input voltage range configuration
 ===============================================================================  
@endverbatim
  * @{
  */
/**
  * @brief  ---
  * @param  ---
  * @retval Error Code. See HT6022_ErrorTypeDef
  */
HT6022_ErrorTypeDef	HT6022_SetSR (HT6022_DeviceTypeDef *Device,
				      HT6022_SRTypeDef SR)
{
   	   int r;
	   unsigned char SampleRate = SR;
	   if ((!IS_HT6022_SR (SR)) || (Device == NULL))
	       return HT6022_ERROR_INVALID_PARAM;

	   r = libusb_control_transfer (Device->DeviceHandle, 
				        HT6022_SR_REQUEST_TYPE, 
					HT6022_SR_REQUEST, 
					HT6022_SR_VALUE, 
					HT6022_SR_INDEX, 
					&SampleRate,
					HT6022_SR_SIZE, 0);
	  if (r != HT6022_SR_SIZE)
	  {
		if (r != HT6022_ERROR_NO_DEVICE)
			r = HT6022_ERROR_OTHER;
		return r;
	  }

	  return HT6022_SUCCESS;
}

/**
  * @brief  ---
  * @param  ---
  * @retval Error Code. See HT6022_ErrorTypeDef
  */
HT6022_ErrorTypeDef HT6022_SetCH1IR (HT6022_DeviceTypeDef *Device,
				          			 HT6022_IRTypeDef IR)
{
   	   int r;
	   unsigned char InputRange = IR;
	   if ((!IS_HT6022_IR (IR)) || (Device == NULL))
		   return HT6022_ERROR_INVALID_PARAM;	
	   r = libusb_control_transfer (Device->DeviceHandle, 
				        HT6022_IR1_REQUEST_TYPE, 
					HT6022_IR1_REQUEST, 
					HT6022_IR1_VALUE, 
					HT6022_IR1_INDEX, 
					&InputRange, 
					HT6022_IR1_SIZE, 0);
	  if (r != HT6022_IR1_SIZE)
	  {
		if (r != HT6022_ERROR_NO_DEVICE)
			r = HT6022_ERROR_OTHER;
		return r;
	  }

	  return HT6022_SUCCESS;
}

/**
  * @brief  None
  * @param  None
  * @retval Error Code. See HT6022_ErrorTypeDef
  */
HT6022_ErrorTypeDef HT6022_SetCH2IR (HT6022_DeviceTypeDef *Device,
				          			 HT6022_IRTypeDef IR)
{
   	   int r;
	   unsigned char InputRange = IR;
	   if ((!IS_HT6022_IR (IR)) || (Device == NULL))
			return HT6022_ERROR_INVALID_PARAM;	
	   r = libusb_control_transfer (Device->DeviceHandle, 
				        HT6022_IR2_REQUEST_TYPE, 
					HT6022_IR2_REQUEST, 
					HT6022_IR2_VALUE, 
					HT6022_IR2_INDEX, 
					&InputRange,
					HT6022_IR2_SIZE, 0);
	  if (r != HT6022_IR2_SIZE)
	  {
		if (r != HT6022_ERROR_NO_DEVICE)
			r = HT6022_ERROR_OTHER;
		return r;
	  }

	  return HT6022_SUCCESS; 
}

/**
  * @}
  */

/**
  * @}
  */

 
#ifdef __cplusplus
 	}
#endif

/****************************END OF FILE*********************************/
