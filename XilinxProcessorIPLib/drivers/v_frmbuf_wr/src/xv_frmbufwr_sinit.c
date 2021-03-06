// ==============================================================
// Copyright (c) 1986 - 2021 Xilinx Inc. All rights reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

#ifndef __linux__

#include "xstatus.h"
#include "xparameters.h"
#include "xv_frmbufwr.h"

extern XV_frmbufwr_Config XV_frmbufwr_ConfigTable[];

XV_frmbufwr_Config *XV_frmbufwr_LookupConfig(u16 DeviceId) {
    XV_frmbufwr_Config *ConfigPtr = NULL;

    int Index;

    for (Index = 0; Index < XPAR_XV_FRMBUFWR_NUM_INSTANCES; Index++) {
        if (XV_frmbufwr_ConfigTable[Index].DeviceId == DeviceId) {
            ConfigPtr = &XV_frmbufwr_ConfigTable[Index];
            break;
        }
    }

    return ConfigPtr;
}

int XV_frmbufwr_Initialize(XV_frmbufwr *InstancePtr, u16 DeviceId) {
    XV_frmbufwr_Config *ConfigPtr;

    Xil_AssertNonvoid(InstancePtr != NULL);

    ConfigPtr = XV_frmbufwr_LookupConfig(DeviceId);
    if (ConfigPtr == NULL) {
        InstancePtr->IsReady = 0;
        return (XST_DEVICE_NOT_FOUND);
    }

    return XV_frmbufwr_CfgInitialize(InstancePtr,
                                      ConfigPtr,
                                      ConfigPtr->BaseAddress);
}

#endif
