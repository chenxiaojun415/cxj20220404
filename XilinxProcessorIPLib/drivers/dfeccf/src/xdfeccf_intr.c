/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdfeccf_intr.c
* @addtogroup dfeccf_v1_0
* @{
*
* This file contains functions related to Channel Filter interrupt handling.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---    -------- -----------------------------------------------
* 1.0   dc     12/10/20 Initial version
* </pre>
*
******************************************************************************/

#include "xdfeccf.h"
#include "xdfeccf_hw.h"

/**************************** Macros Definitions ****************************/

/************************** Function Prototypes *****************************/
extern u32 XDfeCcf_RdBitField(u32 FieldWidth, u32 FieldOffset, u32 Data);
extern u32 XDfeCcf_WrBitField(u32 FieldWidth, u32 FieldOffset, u32 Data,
			      u32 Val);

/****************************************************************************/
/**
*
* Get event status.
*
* @param    InstancePtr is a pointer to the channel filter instance.
* @param    Status is a pointer to a returned event status.
*
* @return   None
*
* @note     None
*
****************************************************************************/
void XDfeCcf_GetEventStatus(const XDfeCcf *InstancePtr, XDfeCcf_Status *Status)
{
	u32 Val;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFECCF_STATE_OPERATIONAL);
	Xil_AssertVoid(Status != NULL);

	Val = XDfeCcf_ReadReg(InstancePtr, XDFECCF_ISR);
	Status->CCUpdate =
		XDfeCcf_RdBitField(XDFECCF_CC_UPDATE_TRIGGERED_WIDTH,
				   XDFECCF_CC_UPDATE_TRIGGERED_OFFSET, Val);
	Status->CCSequenceError =
		XDfeCcf_RdBitField(XDFECCF_CC_SEQUENCE_ERROR_WIDTH,
				   XDFECCF_CC_SEQUENCE_ERROR_OFFSET, Val);

	Val = XDfeCcf_ReadReg(InstancePtr, XDFECCF_OVERFLOW);
	Status->OverflowBeforeGainReal =
		XDfeCcf_RdBitField(XDFECCF_OVERFLOW_BEFORE_GAIN_REAL_WIDTH,
				   XDFECCF_OVERFLOW_BEFORE_GAIN_REAL_OFFSET,
				   Val);
	Status->OverflowBeforeGainImag =
		XDfeCcf_RdBitField(XDFECCF_OVERFLOW_BEFORE_GAIN_IMAG_WIDTH,
				   XDFECCF_OVERFLOW_BEFORE_GAIN_IMAG_OFFSET,
				   Val);
	Status->OverflowAfterGainReal =
		XDfeCcf_RdBitField(XDFECCF_OVERFLOW_AFTER_GAIN_REAL_WIDTH,
				   XDFECCF_OVERFLOW_AFTER_GAIN_REAL_OFFSET,
				   Val);
	Status->OverflowAfterGainImag =
		XDfeCcf_RdBitField(XDFECCF_OVERFLOW_AFTER_GAIN_IMAG_WIDTH,
				   XDFECCF_OVERFLOW_AFTER_GAIN_IMAG_OFFSET,
				   Val);
	Status->OveflowAntenna = XDfeCcf_RdBitField(
		XDFECCF_ANTENNA_WIDTH, XDFECCF_ANTENNA_OFFSET, Val);
	Status->OverflowCCID = XDfeCcf_RdBitField(XDFECCF_CCID_WIDTH,
						  XDFECCF_CCID_OFFSET, Val);
}

/****************************************************************************/
/**
*
* Clear all event statuses.
*
* @param    InstancePtr is a pointer to the channel filter instance.
*
* @return   None
*
* @note     None
*
****************************************************************************/
void XDfeCcf_ClearEventStatus(const XDfeCcf *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFECCF_STATE_OPERATIONAL);

	XDfeCcf_WriteReg(InstancePtr, XDFECCF_COEFF_CFG,
			 XDFECCF_IRQ_FLAGS_MASK);
}

/****************************************************************************/
/**
*
* Set interrupt masks.
*
* @param    InstancePtr is a pointer to the channel filter instance.
* @param    Mask is a interrupt mask value.
*
* @return   None
*
* @note     None
*
****************************************************************************/
void XDfeCcf_SetInterruptMask(const XDfeCcf *InstancePtr,
			      const XDfeCcf_InterruptMask *Mask)
{
	u32 Val;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->StateId == XDFECCF_STATE_OPERATIONAL);
	Xil_AssertVoid(Mask != NULL);

	Val = XDfeCcf_WrBitField(XDFECCF_OVERFLOW_WIDTH,
				 XDFECCF_OVERFLOW_OFFSET, 0U, Mask->Overflow);
	Val = XDfeCcf_WrBitField(XDFECCF_CC_UPDATE_TRIGGERED_WIDTH,
				 XDFECCF_CC_UPDATE_TRIGGERED_OFFSET, Val,
				 Mask->CCUpdate);
	Val = XDfeCcf_WrBitField(XDFECCF_CC_SEQUENCE_ERROR_WIDTH,
				 XDFECCF_CC_SEQUENCE_ERROR_OFFSET, Val,
				 Mask->CCSequenceError);
	XDfeCcf_WriteReg(InstancePtr, XDFECCF_COEFF_CFG, Val);
}

/** @} */