/**
*
*  Copyright (C) 2018, Marvell International Ltd. and its affiliates.
*
*  This program and the accompanying materials are licensed and made available
*  under the terms and conditions of the BSD License which accompanies this
*  distribution. The full text of the license may be found at
*  http://opensource.org/licenses/bsd-license.php
*
*  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
*  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
*
*  Glossary - abbreviations used in Marvell SampleAtReset library implementation:
*  AP - Application Processor hardware block (Armada 7k8k incorporates AP806)
*  CP - South Bridge hardware blocks (Armada 7k8k incorporates CP110)
**/

#include <Uefi.h>

#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <Protocol/BoardDesc.h>

#include "Armada7k8kSoCDescLib.h"

EFI_STATUS
EFIAPI
ArmadaSoCDescComPhyGet (
  IN OUT MV_SOC_COMPHY_DESC  **ComPhyDesc,
  IN OUT UINTN                *DescCount
  )
{
  MV_SOC_COMPHY_DESC *Desc;
  UINTN CpCount, CpIndex;

  CpCount = FixedPcdGet8 (PcdMaxCpCount);

  Desc = AllocateZeroPool (CpCount * sizeof (MV_SOC_COMPHY_DESC));
  if (Desc == NULL) {
    DEBUG ((DEBUG_ERROR, "%a: Cannot allocate memory\n", __FUNCTION__));
    return EFI_OUT_OF_RESOURCES;
  }

  for (CpIndex = 0; CpIndex < CpCount; CpIndex++) {
    Desc[CpIndex].ComPhyBaseAddress = MV_SOC_COMPHY_BASE (CpIndex);
    Desc[CpIndex].ComPhyHpipe3BaseAddress = MV_SOC_HPIPE3_BASE (CpIndex);
    Desc[CpIndex].ComPhyLaneCount = MV_SOC_COMPHY_LANE_COUNT;
    Desc[CpIndex].ComPhyMuxBitCount = MV_SOC_COMPHY_MUX_BITS;
    Desc[CpIndex].ComPhyChipType = MvComPhyTypeCp110;
    Desc[CpIndex].ComPhyId = CpIndex;
  }

  *ComPhyDesc = Desc;
  *DescCount = CpCount;

  return EFI_SUCCESS;
}

EFI_PHYSICAL_ADDRESS
EFIAPI
ArmadaSoCDescCpBaseGet (
  IN UINTN  CpIndex
  )
{
  ASSERT (CpIndex < FixedPcdGet8 (PcdMaxCpCount));

  return MV_SOC_CP_BASE (CpIndex);
}

EFI_STATUS
EFIAPI
ArmadaSoCDescI2cGet (
  IN OUT MV_SOC_I2C_DESC  **I2cDesc,
  IN OUT UINTN             *DescCount
  )
{
  MV_SOC_I2C_DESC *Desc;
  UINTN CpCount, CpIndex, Index;

  CpCount = FixedPcdGet8 (PcdMaxCpCount);

  *DescCount = CpCount * MV_SOC_I2C_PER_CP_COUNT;
  Desc = AllocateZeroPool (*DescCount * sizeof (MV_SOC_I2C_DESC));
  if (Desc == NULL) {
    DEBUG ((DEBUG_ERROR, "%a: Cannot allocate memory\n", __FUNCTION__));
    return EFI_OUT_OF_RESOURCES;
  }

  *I2cDesc = Desc;

  for (CpIndex = 0; CpIndex < CpCount; CpIndex++) {
    for (Index = 0; Index < MV_SOC_I2C_PER_CP_COUNT; Index++) {
      Desc->I2cBaseAddress = MV_SOC_CP_BASE (CpIndex) + MV_SOC_I2C_BASE (Index);
      Desc++;
    }
  }

  return EFI_SUCCESS;
}

//
// Allocate the MSI address per interrupt Group,
// unsupported Groups get NULL address.
//
STATIC
MV_SOC_ICU_DESC mA7k8kIcuDescTemplate = {
  ICU_GIC_MAPPING_OFFSET,
  {
    /* Non secure interrupts */
    { IcuGroupNsr,  ICU_NSR_SET_SPI_BASE,  ICU_NSR_CLEAR_SPI_BASE },
    /* Secure interrupts */
    { IcuGroupSr,   ICU_GROUP_UNSUPPORTED, ICU_GROUP_UNSUPPORTED },
    /* LPI interrupts */
    { IcuGroupLpi,  ICU_GROUP_UNSUPPORTED, ICU_GROUP_UNSUPPORTED },
    /* Virtual LPI interrupts */
    { IcuGroupVlpi, ICU_GROUP_UNSUPPORTED, ICU_GROUP_UNSUPPORTED },
    /* System error interrupts */
    { IcuGroupSei,  ICU_SEI_SET_SPI_BASE,  ICU_SEI_CLEAR_SPI_BASE },
    /* RAM error interrupts */
    { IcuGroupRei,  ICU_REI_SET_SPI_BASE,  ICU_REI_CLEAR_SPI_BASE },
  }
};

EFI_STATUS
EFIAPI
ArmadaSoCDescIcuGet (
  IN OUT MV_SOC_ICU_DESC  **IcuDesc
  )
{
  *IcuDesc = AllocateCopyPool (sizeof (mA7k8kIcuDescTemplate),
               &mA7k8kIcuDescTemplate);
  if (*IcuDesc == NULL) {
    DEBUG ((DEBUG_ERROR, "%a: Cannot allocate memory\n", __FUNCTION__));
    return EFI_OUT_OF_RESOURCES;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
ArmadaSoCDescMdioGet (
  IN OUT MV_SOC_MDIO_DESC  **MdioDesc,
  IN OUT UINTN              *DescCount
  )
{
  MV_SOC_MDIO_DESC *Desc;
  UINTN CpCount, CpIndex;

  CpCount = FixedPcdGet8 (PcdMaxCpCount);

  Desc = AllocateZeroPool (CpCount * sizeof (MV_SOC_MDIO_DESC));
  if (Desc == NULL) {
    DEBUG ((DEBUG_ERROR, "%a: Cannot allocate memory\n", __FUNCTION__));
    return EFI_OUT_OF_RESOURCES;
  }

  for (CpIndex = 0; CpIndex < CpCount; CpIndex++) {
    Desc[CpIndex].MdioId = MV_SOC_MDIO_ID (CpIndex);
    Desc[CpIndex].MdioBaseAddress = MV_SOC_MDIO_BASE (CpIndex);
  }

  *MdioDesc = Desc;
  *DescCount = CpCount;

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
ArmadaSoCDescAhciGet (
  IN OUT MV_SOC_AHCI_DESC  **AhciDesc,
  IN OUT UINTN              *DescCount
  )
{
  MV_SOC_AHCI_DESC *Desc;
  UINTN CpCount, CpIndex;

  CpCount = FixedPcdGet8 (PcdMaxCpCount);

  Desc = AllocateZeroPool (CpCount * sizeof (MV_SOC_AHCI_DESC));
  if (Desc == NULL) {
    DEBUG ((DEBUG_ERROR, "%a: Cannot allocate memory\n", __FUNCTION__));
    return EFI_OUT_OF_RESOURCES;
  }

  for (CpIndex = 0; CpIndex < CpCount; CpIndex++) {
    Desc[CpIndex].AhciId = MV_SOC_AHCI_ID (CpIndex);
    Desc[CpIndex].AhciBaseAddress = MV_SOC_AHCI_BASE (CpIndex);
    Desc[CpIndex].AhciMemSize = SIZE_8KB;
    Desc[CpIndex].AhciDmaType = NonDiscoverableDeviceDmaTypeCoherent;
  }

  *AhciDesc = Desc;
  *DescCount = CpCount;

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
ArmadaSoCDescPp2Get (
  IN OUT MV_SOC_PP2_DESC  **Pp2Desc,
  IN OUT UINTN             *DescCount
  )
{
  MV_SOC_PP2_DESC *Desc;
  UINTN CpCount, CpIndex;

  CpCount = FixedPcdGet8 (PcdMaxCpCount);

  Desc = AllocateZeroPool (CpCount * sizeof (MV_SOC_PP2_DESC));
  if (Desc == NULL) {
    DEBUG ((DEBUG_ERROR, "%a: Cannot allocate memory\n", __FUNCTION__));
    return EFI_OUT_OF_RESOURCES;
  }

  for (CpIndex = 0; CpIndex < CpCount; CpIndex++) {
    Desc[CpIndex].Pp2BaseAddress = MV_SOC_PP2_BASE (CpIndex);
    Desc[CpIndex].Pp2ClockFrequency = MV_SOC_PP2_CLK_FREQ;
  }

  *Pp2Desc = Desc;
  *DescCount = CpCount;

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
ArmadaSoCDescSdMmcGet (
  IN OUT MV_SOC_SDMMC_DESC  **SdMmcDesc,
  IN OUT UINTN               *DescCount
  )
{
  MV_SOC_SDMMC_DESC *Desc;
  UINTN Index;

  Desc = AllocateZeroPool (MV_SOC_MAX_SDMMC_COUNT * sizeof (MV_SOC_SDMMC_DESC));
  if (Desc == NULL) {
    DEBUG ((DEBUG_ERROR, "%a: Cannot allocate memory\n", __FUNCTION__));
    return EFI_OUT_OF_RESOURCES;
  }

  for (Index = 0; Index < MV_SOC_MAX_SDMMC_COUNT; Index++) {
    Desc[Index].SdMmcBaseAddress = MV_SOC_SDMMC_BASE (Index);
    Desc[Index].SdMmcMemSize = SIZE_1KB;
    Desc[Index].SdMmcDmaType = NonDiscoverableDeviceDmaTypeCoherent;
  }

  *SdMmcDesc = Desc;
  *DescCount = MV_SOC_MAX_SDMMC_COUNT;

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
ArmadaSoCDescUtmiGet (
  IN OUT MV_SOC_UTMI_DESC  **UtmiDesc,
  IN OUT UINTN              *DescCount
  )
{
  MV_SOC_UTMI_DESC *Desc;
  UINTN CpCount, CpIndex, Index, UtmiIndex;

  CpCount = FixedPcdGet8 (PcdMaxCpCount);

  *DescCount = CpCount * MV_SOC_UTMI_PER_CP_COUNT;
  Desc = AllocateZeroPool (*DescCount * sizeof (MV_SOC_UTMI_DESC));
  if (Desc == NULL) {
    DEBUG ((DEBUG_ERROR, "%a: Cannot allocate memory\n", __FUNCTION__));
    return EFI_OUT_OF_RESOURCES;
  }

  *UtmiDesc = Desc;

  UtmiIndex = 0;
  for (CpIndex = 0; CpIndex < CpCount; CpIndex++) {
    for (Index = 0; Index < MV_SOC_UTMI_PER_CP_COUNT; Index++) {
      Desc->UtmiPhyId = MV_SOC_UTMI_ID (UtmiIndex);
      Desc->UtmiBaseAddress = MV_SOC_CP_BASE (CpIndex) + MV_SOC_UTMI_BASE (Index);
      Desc->UtmiConfigAddress = MV_SOC_CP_BASE (CpIndex) + MV_SOC_UTMI_CFG_BASE;
      Desc->UsbConfigAddress = MV_SOC_CP_BASE (CpIndex) + MV_SOC_UTMI_USB_CFG_BASE;
      Desc++;
      UtmiIndex++;
    }
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
ArmadaSoCDescXhciGet (
  IN OUT MV_SOC_XHCI_DESC  **XhciDesc,
  IN OUT UINTN              *DescCount
  )
{
  MV_SOC_XHCI_DESC *Desc;
  UINTN CpCount, CpIndex, Index;

  CpCount = FixedPcdGet8 (PcdMaxCpCount);

  *DescCount = CpCount * MV_SOC_XHCI_PER_CP_COUNT;
  Desc = AllocateZeroPool (*DescCount * sizeof (MV_SOC_XHCI_DESC));
  if (Desc == NULL) {
    DEBUG ((DEBUG_ERROR, "%a: Cannot allocate memory\n", __FUNCTION__));
    return EFI_OUT_OF_RESOURCES;
  }

  *XhciDesc = Desc;

  for (CpIndex = 0; CpIndex < CpCount; CpIndex++) {
    for (Index = 0; Index < MV_SOC_XHCI_PER_CP_COUNT; Index++) {
      Desc->XhciBaseAddress = MV_SOC_CP_BASE (CpIndex) + MV_SOC_XHCI_BASE (Index);
      Desc->XhciMemSize = SIZE_16KB;
      Desc->XhciDmaType = NonDiscoverableDeviceDmaTypeCoherent;
      Desc++;
    }
  }

  return EFI_SUCCESS;
}
