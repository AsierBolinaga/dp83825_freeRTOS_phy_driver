/*
 * mdio_phydp83825.c
 *
 *  Created on: Apr 4, 2023
 *      Author: abolinaga
 */
#include "fsl_phy.h"
#include "fsl_phydp83825.h"
#include "fsl_iomuxc.h"
#include "fsl_enet.h"

phy_dp83825_resource_t g_phy_resource;

static void MDIO_Init(void)
{
    (void)CLOCK_EnableClock(s_enetClock[ENET_GetInstance(ENET)]);
    ENET_SetSMI(ENET, CLOCK_GetFreq(kCLOCK_IpgClk), false);
}

static status_t MDIO_Write(uint8_t phyAddr, uint8_t regAddr, uint16_t data)
{
    return ENET_MDIOWrite(ENET, phyAddr, regAddr, data);
}

static status_t MDIO_Read(uint8_t phyAddr, uint8_t regAddr, uint16_t *pData)
{
    return ENET_MDIORead(ENET, phyAddr, regAddr, pData);
}

static status_t MDIO_ExtendedWrite(uint8_t phyAddr, uint8_t devAddr, uint16_t regAddr, uint16_t data)
{
    return ENET_MDIOC45Write(ENET, phyAddr, devAddr, regAddr, data);
}

static status_t MDIO_ExtendedRead(uint8_t phyAddr, uint8_t devAddr, uint16_t regAddr, uint16_t *pData)
{
    return ENET_MDIOC45Read(ENET, phyAddr, devAddr, regAddr, pData);
}

void mdio_phydp83825_init(void)
{
	MDIO_Init();
    g_phy_resource.read  = MDIO_Read;
    g_phy_resource.write = MDIO_Write;
    g_phy_resource.readExt = MDIO_ExtendedRead;
    g_phy_resource.writeExt = MDIO_ExtendedWrite;
}
