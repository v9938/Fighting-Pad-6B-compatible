/********************************************************************
 Software License Agreement:

 The software supplied herewith by Microchip Technology Incorporated
 (the "Company") for its PIC(R) Microcontroller is intended and
 supplied to you, the Company's customer, for use solely and
 exclusively on Microchip PIC Microcontroller products. The
 software is owned by the Company and/or its supplier, and is
 protected under applicable copyright laws. All rights are reserved.
 Any use in violation of the foregoing restrictions may subject the
 user to criminal sanctions under applicable laws, as well as to
 civil liability for the breach of the terms and conditions of this
 license.

 THIS SOFTWARE IS PROVIDED IN AN "AS IS" CONDITION. NO WARRANTIES,
 WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
 TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
 IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
 CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *******************************************************************/

#ifndef USBJOYSTICK_C
#define USBJOYSTICK_C

/** INCLUDES *******************************************************/
#include "usb.h"
#include "usb_device_hid.h"
#include "system.h"
#include "stdint.h"


/** TYPE DEFINITIONS ************************************************/
typedef union _INTPUT_CONTROLS_TYPEDEF
{
    uint8_t val[8];
} INPUT_CONTROLS;


/** VARIABLES ******************************************************/
/* Some processors have a limited range of RAM addresses where the USB module
 * is able to access.  The following section is for those devices.  This section
 * assigns the buffers that need to be used by the USB module into those
 * specific areas.
 */
#if defined(FIXED_ADDRESS_MEMORY)
    #if defined(COMPILER_MPLAB_C18)
        #pragma udata JOYSTICK_DATA=JOYSTICK_DATA_ADDRESS
            INPUT_CONTROLS joystick_input;
        #pragma udata
    #elif defined(__XC8)
        INPUT_CONTROLS joystick_input @ JOYSTICK_DATA_ADDRESS;
    #endif
#else
    INPUT_CONTROLS joystick_input;
#endif


USB_VOLATILE USB_HANDLE lastTransmission = 0;


/*********************************************************************
* Function: void APP_DeviceJoystickInitialize(void);
*
* Overview: Initializes the demo code
*
* PreCondition: None
*
* Input: None
*
* Output: None
*
********************************************************************/
void APP_DeviceJoystickInitialize(void)
{  
    //initialize the variable holding the handle for the last
    // transmission
    lastTransmission = 0;

    //enable the HID endpoint
    USBEnableEndpoint(JOYSTICK_EP,USB_IN_ENABLED|USB_HANDSHAKE_ENABLED|USB_DISALLOW_SETUP);
}//end UserInit

//Helper function to execute some blocking delay.
void nDelay(unsigned int DelayAmount)
{
    while(DelayAmount)
    {
        DelayAmount--;
    }
}

/*********************************************************************
* Function: void APP_DeviceJoystickTasks(void);
*
* Overview: Keeps the demo running.
*
* PreCondition: The demo should have been initialized and started via
*   the APP_DeviceJoystickInitialize() and APP_DeviceJoystickStart() demos
*   respectively.
*
* Input: None
*
* Output: None
*
********************************************************************/
void APP_DeviceJoystickTasks(void)
{  
    // User Application USB tasks
    if((USBDeviceState < CONFIGURED_STATE)||(USBSuspendControl==1)) return;

    //If the last transmision is complete
    if(!HIDTxHandleBusy(lastTransmission))    {
		
		//Default Value
        joystick_input.val[0] = 0x01;		// ID
        joystick_input.val[1] = 0x7f;		// [Not Use] 0x7f
        joystick_input.val[2] = 0x7f;		// [Not Use] 0x7f
        joystick_input.val[3] = 0x7f;		// LEFT   0x00, RIGHT  0xFF
        joystick_input.val[4] = 0x7f;		// UP     0x00, DOWN   0xFF
        joystick_input.val[5] = 0x0f;		// X A B Y 1b 1b 1b 1b
        joystick_input.val[6] = 0x00;		// 0b 0b START MODE 0b 0b C Z
        joystick_input.val[7] = 0x00;		// [Not Use] 0x00
        
        // Stick
        if (PORTCbits.RC0 == 0 ) joystick_input.val[4] = 0x00;		// UP     :RC0
        if (PORTCbits.RC1 == 0 ) joystick_input.val[4] = 0xff;		// DOWN   :RC1
        if (PORTCbits.RC2 == 0 ) joystick_input.val[3] = 0x00;		// LEFT   :RC2
        if (PORTCbits.RC3 == 0 ) joystick_input.val[3] = 0xff;		// RIGH   :RC3
        

		// Button (3B)
        if (PORTCbits.RC4 == 0 ) joystick_input.val[6] |= 0x20;		// START  :RC4
        if (PORTCbits.RC5 == 0 ) joystick_input.val[5] |= 0x40;		// A      :RC5
        if (PORTCbits.RC6 == 0 ) joystick_input.val[5] |= 0x20;		// B      :RC6
        if (PORTCbits.RC7 == 0 ) joystick_input.val[6] |= 0x02;		// C      :RC7

		// Button (6B)
        if (PORTBbits.RB7 == 0 ) joystick_input.val[5] |= 0x80;  	// X      :RB7
        if (PORTBbits.RB6 == 0 ) joystick_input.val[5] |= 0x10;  	// Y      :RB6
        if (PORTBbits.RB5 == 0 ) joystick_input.val[6] |= 0x01;  	// Z      :RB5
        if (PORTBbits.RB4 == 0 ) joystick_input.val[6] |= 0x10;  	// MODE   :RB4

        // Bug support: USB framework... 
        // Incorrect data may be retransmitted after send ACK.
        nDelay(40);
        

        //Send the packet over USB to the host.
        lastTransmission = HIDTxPacket(JOYSTICK_EP, (uint8_t*)&joystick_input, sizeof(joystick_input));

    }
    
}//end ProcessIO

#endif
