/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdfemix.h
* @addtogroup xdfemix_v1_0
* @{
*
* The Channel Mixer IP provides a wrapper around the Channel Mixer and
* DUC_DDC block (Combined DUC/DDC and Mixer). Each IP instance can support up
* to 64 CC, arranged across a maximum of 8 Antennas using 4 blocks.
* The wrapper provides access to the underlying blocks via TDMA Axi-stream data
* interfaces. Output from the block is arranged as an AXI-Stream, running at
* fs, per antenna. An AXI memory mapped interface is provided, enabling
* configuration and control of the block from a microprocessor.
* The features that the Mixer IP and driver support are:
* - Supports ibw of 200MHz and 400MHz.
* - Supports a maximum sample rate of 491.52MHz.
* - Supports reallocation of DL TDM slots.
* - Supports Up conversion rate set on a per channel basis, programmed via
*   processor interface.
* - UL TDM output pattern programmed via register interface.
* - Supports down conversion rate set on a per channel basis, programmed via
*   processor interface.
* - Supports TDD power down via a processor interface and TUSER input.
* - Supports automatic flushing of the internal buffers.
* - TUSER/TLAST information accompanying the data is delay matched through
*   the IP.
*
* The number of antennas supported by the IP Core is given by the NUM_ANTENNAS
* parameter. The Number of CC's per antenna is given by the NUM_CC parameter.
* The combination of these two parameters will give the total number of
* channels the IP core will need to process and hence dictate the internal
* architecture. The IP core and driver provides the following features:
* - The Axi Memory map provides settings to enable/disable individual CCs.
* - CC settings apply to all antenna - the only setting that is specific to
*   an antenna is ANTENNA_GAIN.
* - Flushing of channels across all antenna for a particular CC is handled
* by the core, occurring prior to a CC being added.
* - Support for software reset. The IP software reset register allows
*   the assertion of the internal reset.
*
* An API which will read/write registers has been provided for debug purpose.
*
* There are plans to add more features.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---    -------- -----------------------------------------------
* 1.0   dc     07/22/20 Initial version
*       dc     02/02/21 Remove hard coded device node name
*       dc     02/15/21 align driver to curent specification
* </pre>
*
******************************************************************************/
#ifndef XDFEMIX_H_
#define XDFEMIX_H_

#ifdef __cplusplus
extern "C" {
#endif

/**************************** Includes ***************************************/
#ifdef __BAREMETAL__
#include "xil_types.h"
#include "xparameters.h"
#include "xstatus.h"
#else
#include <linux/types.h>
#include <assert.h>
#endif

#include "stdbool.h"
#include <metal/sys.h>

/**************************** Macros Definitions *****************************/
#ifndef __BAREMETAL__
#define XDFEMIX_MAX_NUM_INSTANCES 10U
#define Xil_AssertNonvoid(Expression) assert(Expression)
#define Xil_AssertVoid(Expression) assert(Expression)
#define Xil_AssertVoidAlways() assert(0)
#define XST_SUCCESS 0U
#define XST_FAILURE 1U
#else
#define XDFEMIX_MAX_NUM_INSTANCES XPAR_XDFEMIX_NUM_INSTANCES
#endif

#define XDFEMIX_CC_NUM 16U /**< Maximum CC number */
#define XDFEMIX_ANT_NUM_MAX 8U /**< Maximum anntena number */
#define XDFEMIX_SEQ_LENGTH_MAX 16U /**< Maximum sequence length */

#define XDFEMIX_RATE_MAX 5U /**< Maximum rate Id */
#define XDFEMIX_NCO_MAX 4U /**< Maximum NCO number */

/**************************** Type Definitions *******************************/
/*********** start - common code to all Logiccores ************/
#ifndef __BAREMETAL__
typedef __u32 u32;
typedef __u16 u16;
typedef __u8 u8;
typedef __s32 s32;
typedef __s16 s16;
typedef __u64 u64;
typedef __s64 s64;
typedef __s8 s8;
#endif

typedef enum {
	XDFEMIX_STATE_NOT_READY = 0, /**< Not ready state*/
	XDFEMIX_STATE_READY, /**< Ready state*/
	XDFEMIX_STATE_RESET, /**< Reset state*/
	XDFEMIX_STATE_CONFIGURED, /**< Configured state*/
	XDFEMIX_STATE_INITIALISED, /**< Initialised state*/
	XDFEMIX_STATE_OPERATIONAL /**< Operational state*/
} XDfeMix_StateId;

/**
 * Logicore version.
 */
typedef struct {
	u32 Major; /**< Major version number */
	u32 Minor; /**< Minor version number */
	u32 Revision; /**< Revision number */
	u32 Patch; /**< Patch number */
} XDfeMix_Version;

/**
 * Trigger configuration.
 */
typedef struct {
	u32 Enable; /**< [0,1], 0 = Disabled: Trigger disabled;
		1 = Enabled: Trigger enabled */
	u32 Source; /**< [0,1,2],
		0 = IMMEDIATE: write to the trigger configuration register
			immediately
		1 = TUSER: write on Edge detected on specified TUSER bit
		2 = TLAST: write on Edge detected on TLAST */
	u32 TUSERBit; /**< [0-7], Species which TUSER bit is used by
		the trigger */
	u32 Edge; /**< [0,1,2], 0 = Rising; 1 = Falling; 2 = Both */
	u32 OneShot; /**< [0,1],
		0 = Continuous: Once enabled trigger repeats continuously
		1 = OneShot: Once enabled trigger occurs once and then
			disables */
} XDfeMix_Trigger;

/**
 * All IP triggers.
 */
typedef struct {
	XDfeMix_Trigger Activate; /**< Toggle between "Initialized",
		ultra-low power state, and "Operational". One-shot trigger,
		disabled following a single event */
	XDfeMix_Trigger LowPower; /**< Toggle between "Low-power"
		and "Operational" state */
	XDfeMix_Trigger CCUpdate; /**< Transition to next CC
		configuration. Will initiate flush based on CC configuration */
} XDfeMix_TriggerCfg;

/**
 * Defines a CCID sequence.
 */
typedef struct {
	u32 Length; /**< [1-16] Sequence length */
	u32 CCID[XDFEMIX_SEQ_LENGTH_MAX]; /**< [0-15].Array of CCID's
		arranged in the order the CCIDs are required to be processed
		in the channel filter */
} XDfeMix_CCSequence;

/*********** end - common code to all Logiccores ************/
/**
 * Mixer model parameters structure. Data defined in Device tree/xparameters.h
 */
typedef struct {
	u32 BypassDDC; /**< [0,1] */
	u32 BypassMixer; /**< [0,1] */
	u32 EnableMixIf; /**< [0,1] */
	u32 Mode; /**< [0,1] 0=downlink, 1=uplink */
	u32 NumAntenna; /**< [1-8] */
	u32 NumCCPerAntenna; /**< [1-8] */
	u32 NumSlotChannels; /**< [1-4] */
} XDfeMix_ModelParameters;

/**
 * Configuration.
 */
typedef struct {
	XDfeMix_Version Version; /**< Logicore version */
	XDfeMix_ModelParameters ModelParams; /**< Logicore
		parameterization */
} XDfeMix_Cfg;

/**
 * Phase Offset.
 */
typedef struct {
	u32 PhaseOffset; /**< [0-2^17-1] Phase offset */
} XDfeMix_PhaseOffset;

/**
 * Defines frequency for single CC's NCO.
 */
typedef struct {
	u32 FrequencyControlWord; /**< [0-2^32-1] Phase increment */
	u32 SingleModCount; /**< [0-2^32-1] Single modulus cycle count (S) */
	u32 DualModCount; /**< [0-2^32-1] Dual modulus cycle count (T-S) */
	XDfeMix_PhaseOffset PhaseOffset;
} XDfeMix_Frequency;

/**
 * Defined phase for single CC's NCO.
 */
typedef struct {
	u32 PhaseAcc; /**< [0-2^32-1] Phase accumulator value */
	u32 DualModCount; /**< [0-2^32-1] Dual modulus count value */
	u32 DualModSel; /**< [0,1] Dual modulus select value */
} XDfeMix_Phase;

/**
 * Defines settings for single CC's NCO.
 */
typedef struct {
	XDfeMix_Frequency FrequencyCfg;
	XDfeMix_Phase PhaseCfg;
	u32 NCOGain; /**< [0,1,2,3] Scaling of NCO output (0=0dB, 1=-3dB,
		2=-6dB, 3=-9dB) */
} XDfeMix_NCO;

/**
 * Defines settings for single CC's DUC/DDC.
 */
typedef struct {
	u32 NCO; /**< [0-4] DUC/DDC NCO assignment */
	u32 Rate; /**< [0-5].Interpolation/decimation rate:
			0: Disabled
			1: 1x interpolation/decimation
			2: 2x interpolation/decimation
			3: 4x interpolation/decimation
			4: 8x interpolation/decimation
			5: 16x interpolation/decimation */
} XDfeMix_DUCDDCCfg;

/**
 * Configuration for a single CC (implementation note: notice that there are
 * 2 parts, one part (DUCDDCCfg) mapping to the CCCfg state, and another that
 * is written directly to NCO registers (xDFECCMixerNCOT). Note that CC Filter
 * does not have the second part. However from an API perspective this is
 * hidden.
 */
typedef struct {
	XDfeMix_DUCDDCCfg DUCDDCCfg; /**< Defines settings for single CC's
		DUC/DDC */
	XDfeMix_NCO NCO; /**< Defines settings for single CC's NCO */
} XDfeMix_CarrierCfg;

/**
 * Full CC configuration.
 */
typedef struct {
	XDfeMix_CCSequence Sequence; /**< CCID sequence */
	XDfeMix_DUCDDCCfg DUCDDCCfg[16]; /**< DUC/DDC configurations for all
		CCs */
	u32 AntennaCfg[8]; /**< [0: 0dB,1:-6dB] Antenna gain adjustment for all
		antenna */
} XDfeMix_CCCfg;

/**
 * DUC/DDC status.
 */
typedef struct {
	u32 RealOverflowStage; /**< [0-3] First stage in which overflow
		in real data has occurred. */
	u32 ImagOverflowStage; /**< [0-3] First stage in which overflow
		in imaginary data has occurred. */
	u32 FirstAntennaOverflowing; /**< [0-7] Lowest antenna in which
		overflow has occurred. */
	u32 FirstCCIDOverflowing; /**< [0-7] Lowest CCID in which overflow has
		occurred. */
} XDfeMix_DUCDDCStatus;

/**
 * Mixer status.
 */
typedef struct {
	u32 AdderStage; /**< [0,1] First adder stage in which overflow in real
		or imaginary data has occurred. */
	u32 AdderAntenna; /**< [0-7] Lowest antenna in which overflow has
		occurred. */
	u32 MixCCID; /**< [0-7] Lowest CCID on which overflow has occurred
		in mixer. */
	u32 MixAntenna; /**< [0-7] Lowest antenna in which overflow has
		occurred. */
} XDfeMix_MixerStatus;

/**
 * Interrupt status and mask.
 */
typedef struct {
	u32 DUCDDCOverflow; /**< [0,1] Mask overflow in DUC/DDC */
	u32 MixerOverflow; /**< [0,1] Mask overflow in mixer */
	u32 DLCCUpdate; /**< [0,1] Mask downlink update interrupt */
	u32 DLCCSequenceError; /**< [0,1] Mask downlink sequence error */
	u32 ULCCUpdate; /**< [0,1] Mask uplink update interrupt */
	u32 ULCCSequenceError; /**< [0,1] Mask uplink sequence error */
} XDfeMix_InterruptMask;

/**
 * Mixer Config Structure.
 */
typedef struct {
	u32 DeviceId;
	metal_phys_addr_t BaseAddr;
	u32 BypassDDC; /**< [0,1] */
	u32 BypassMixer; /**< [0,1] */
	u32 EnableMixIf; /**< [0,1] */
	u32 Mode; /**< [0,1] 0=downlink, 1=uplink */
	u32 NumAntenna; /**< [1-8] */
	u32 NumCCPerAntenna; /**< [1-8] */
	u32 NumSlotChannels; /**< [1-4] */
} XDfeMix_Config;

/**
 * Mixer Structure.
 */
typedef struct {
	XDfeMix_Config Config; /**< Config Structure */
	XDfeMix_StateId StateId; /**< StateId */
	struct metal_io_region *Io; /**< Libmetal IO structure */
	struct metal_device *Device; /**< Libmetal device structure */
} XDfeMix;

/**************************** API declarations *******************************/
/* System initialization API */
XDfeMix *XDfeMix_InstanceInit(u16 DeviceId, const char *DeviceNodeName);
void XDfeMix_InstanceClose(XDfeMix *InstancePtr);

/* Register access API */
void XDfeMix_WriteReg(const XDfeMix *InstancePtr, u32 AddrOffset, u32 Data);
u32 XDfeMix_ReadReg(const XDfeMix *InstancePtr, u32 AddrOffset);

/* DFE Mixer component initialization API */
void XDfeMix_Reset(XDfeMix *InstancePtr);
void XDfeMix_Configure(XDfeMix *InstancePtr, XDfeMix_Cfg *Cfg);
void XDfeMix_Initialize(XDfeMix *InstancePtr);
void XDfeMix_Activate(XDfeMix *InstancePtr, bool EnableLowPower);
void XDfeMix_Deactivate(XDfeMix *InstancePtr);

/* User APIs */
u32 XDfeMix_AddCC(const XDfeMix *InstancePtr, u32 CCID,
		  XDfeMix_CarrierCfg *CarrierCfg);
void XDfeMix_RemoveCC(const XDfeMix *InstancePtr, u32 CCID);
void XDfeMix_MoveCC(const XDfeMix *InstancePtr, u32 CCID, u32 Rate, u32 FromNCO,
		    u32 ToNCO);
void XDfeMix_UpdateCC(const XDfeMix *InstancePtr);
void XDfeMix_SetAntennaGain(const XDfeMix *InstancePtr, u32 AntennaId,
			    u32 AntennaGain);
void XDfeMix_GetTriggersCfg(const XDfeMix *InstancePtr,
			 XDfeMix_TriggerCfg *TriggerCfg);
void XDfeMix_SetTriggersCfg(const XDfeMix *InstancePtr,
			 XDfeMix_TriggerCfg *TriggerCfg);
void XDfeMix_GetDUCDDCStatus(const XDfeMix *InstancePtr, u32 CCID,
			     XDfeMix_DUCDDCStatus *DUCDDCStatus);
void XDfeMix_GetMixerStatus(const XDfeMix *InstancePtr, u32 CCID,
			    XDfeMix_MixerStatus *MixerStatus);
void XDfeMix_GetInterruptMask(const XDfeMix *InstancePtr,
			      XDfeMix_InterruptMask *Flags);
void XDfeMix_SetInterruptMask(const XDfeMix *InstancePtr,
			      const XDfeMix_InterruptMask *Flags);
void XDfeMix_InterruptEnable(const XDfeMix *InstancePtr,
			     const XDfeMix_InterruptMask *Flags);
void XDfeMix_InterruptDisable(const XDfeMix *InstancePtr,
			      const XDfeMix_InterruptMask *Flags);
void XDfeMix_GetInterruptStatus(const XDfeMix *InstancePtr,
				XDfeMix_InterruptMask *Flags);
void XDfeMix_ClearInterruptStatus(const XDfeMix *InstancePtr,
				  const XDfeMix_InterruptMask *Flags);
void XDfeMix_GetVersions(XDfeMix_Version *Version);

#ifdef __cplusplus
}
#endif

#endif
/** @} */
