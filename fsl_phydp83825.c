/*
 * SPDX-License-Identifier: GPL-2.0
 * Driver for the Texas Instruments DP83822, DP83825 and DP83826 PHYs.
 *
 * Copyright (C) 2017 Texas Instruments Inc.
 */

#include "fsl_phydp83825.h"

#include "bits.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define DP83822_PHY_ID	    0x2000a240
#define DP83825S_PHY_ID		0x2000a140
#define DP83825I_PHY_ID		0x2000a150
#define DP83825CM_PHY_ID	0x2000a160
#define DP83825CS_PHY_ID	0x2000a170
#define DP83826C_PHY_ID		0x2000a130
#define DP83826NC_PHY_ID	0x2000a110

#define DP83822_DEVADDR		0x1f

#define MII_DP83822_CTRL_2	0x0a
#define MII_DP83822_PHYSTS	0x10
#define MII_DP83822_PHYSCR	0x11
#define MII_DP83822_MISR1	0x12
#define MII_DP83822_MISR2	0x13
#define MII_DP83822_FCSCR	0x14
#define MII_DP83822_BISCR   0x16
#define MII_DP83822_RCSR	0x17
#define MII_DP83822_PHYCR   0x19 /* Auto_MDI/X_Enable etc */
#define MII_DP83822_RESET_CTRL	0x1f
#define MII_DP83822_GENCFG	0x465
#define MII_DP83822_SOR1	0x467

/* DP83826 specific registers */
#define MII_DP83826_VOD_CFG1	0x30b
#define MII_DP83826_VOD_CFG2	0x30c

/* GENCFG */
#define DP83822_SIG_DET_LOW	BIT(0)

/* Control Register 2 bits */
#define DP83822_FX_ENABLE	BIT(14)

#define DP83822_HW_RESET	BIT(15)
#define DP83822_SW_RESET	BIT(14)

/* PHY STS bits */
#define DP83822_PHYSTS_DUPLEX		BIT(2)
#define DP83822_PHYSTS_10			BIT(1)
#define DP83822_PHYSTS_LINK			BIT(0)

/* PHYSCR Register Fields */
#define DP83822_PHYSCR_INT_OE		BIT(0) /* Interrupt Output Enable */
#define DP83822_PHYSCR_INTEN		BIT(1) /* Interrupt Enable */

/* MISR1 bits */
#define DP83822_RX_ERR_HF_INT_EN		BIT(0)
#define DP83822_FALSE_CARRIER_HF_INT_EN	BIT(1)
#define DP83822_ANEG_COMPLETE_INT_EN	BIT(2)
#define DP83822_DUP_MODE_CHANGE_INT_EN	BIT(3)
#define DP83822_SPEED_CHANGED_INT_EN	BIT(4)
#define DP83822_LINK_STAT_INT_EN		BIT(5)
#define DP83822_ENERGY_DET_INT_EN		BIT(6)
#define DP83822_LINK_QUAL_INT_EN		BIT(7)

/* MISR2 bits */
#define DP83822_JABBER_DET_INT_EN	BIT(0)
#define DP83822_WOL_PKT_INT_EN		BIT(1)
#define DP83822_SLEEP_MODE_INT_EN	BIT(2)
#define DP83822_MDI_XOVER_INT_EN	BIT(3)
#define DP83822_LB_FIFO_INT_EN		BIT(4)
#define DP83822_PAGE_RX_INT_EN		BIT(5)
#define DP83822_ANEG_ERR_INT_EN		BIT(6)
#define DP83822_EEE_ERROR_CHANGE_INT_EN	BIT(7)

/* INT_STAT1 bits */
#define DP83822_WOL_INT_EN	BIT(4)
#define DP83822_WOL_INT_STAT	BIT(12)

#define MII_DP83822_RXSOP1	0x04a5
#define	MII_DP83822_RXSOP2	0x04a6
#define	MII_DP83822_RXSOP3	0x04a7

/* WoL Registers */
#define	MII_DP83822_WOL_CFG		0x04a0
#define	MII_DP83822_WOL_STAT	0x04a1
#define	MII_DP83822_WOL_DA1		0x04a2
#define	MII_DP83822_WOL_DA2		0x04a3
#define	MII_DP83822_WOL_DA3		0x04a4

/* WoL bits */
#define DP83822_WOL_MAGIC_EN	BIT(0)
#define DP83822_WOL_SECURE_ON	BIT(5)
#define DP83822_WOL_EN		BIT(7)
#define DP83822_WOL_INDICATION_SEL BIT(8)
#define DP83822_WOL_CLR_INDICATION BIT(11)

/* PHYCR bits */
#define DP83822_MDIX_AUTO_EN     BIT(15)
#define DP83822_MDIX_FORCE_CROSS BIT(14)

/* RCSR bits */
/* 2 bit tolerance < 2400 byte packets (50ppm) */
#define DP83822_ELASTICBUF_2B   0x1
/* 6 bit tolerance < 7200 byte packets */
#define DP83822_ELASTICBUF_6B   0x2
/* 10 bit tolerance < 12000 byte packets */
#define DP83822_ELASTICBUF_10B  0x3
/* 14 bit tolerance < 16800 byte packets */
#define DP83822_ELASTICBUF_14B  0x0
#define DP83822_RMII_MODE_EN	BIT(5)
#define DP83822_RMII_MODE_SEL	BIT(7)
#define DP83822_RGMII_MODE_EN	BIT(9)
#define DP83822_RX_CLK_SHIFT	BIT(12)
#define DP83822_TX_CLK_SHIFT	BIT(11)

/* SOR1 mode */
#define DP83822_STRAP_MODE1	0
#define DP83822_STRAP_MODE2	BIT(0)
#define DP83822_STRAP_MODE3	BIT(1)
#define DP83822_STRAP_MODE4	GENMASK(1, 0)

#define DP83822_COL_STRAP_MASK	GENMASK(11, 10)
#define DP83822_COL_SHIFT	10
#define DP83822_RX_ER_STR_MASK	GENMASK(9, 8)
#define DP83822_RX_ER_SHIFT	8

/* DP83826: VOD_CFG1 & VOD_CFG2 */
#define DP83826_VOD_CFG1_MINUS_MDIX_MASK	GENMASK(13, 12)
#define DP83826_VOD_CFG1_MINUS_MDI_MASK		GENMASK(11, 6)
#define DP83826_VOD_CFG2_MINUS_MDIX_MASK	GENMASK(15, 12)
#define DP83826_VOD_CFG2_PLUS_MDIX_MASK		GENMASK(11, 6)
#define DP83826_VOD_CFG2_PLUS_MDI_MASK		GENMASK(5, 0)
#define DP83826_CFG_DAC_MINUS_MDIX_5_TO_4	GENMASK(5, 4)
#define DP83826_CFG_DAC_MINUS_MDIX_3_TO_0	GENMASK(3, 0)
#define DP83826_CFG_DAC_PERCENT_PER_STEP	625
#define DP83826_CFG_DAC_PERCENT_DEFAULT		10000
#define DP83826_CFG_DAC_MINUS_DEFAULT		0x30
#define DP83826_CFG_DAC_PLUS_DEFAULT		0x10

/* BISCR bits */
#define DP83822_BISCR_LOOPBACKMODE_MASK    GENMASK(4,0)
#define DP83822_LOOPBACKMODE_PCSIN         BIT(0)
#define DP83822_LOOPBACKMODE_PCSOUT        BIT(1)
#define DP83822_LOOPBACKMODE_DIGITAL       BIT(2)
#define DP83822_LOOPBACKMODE_ANALOG        BIT(3)
#define DP83822_LOOPBACKMODE_REVERSE       BIT(4)

#define MII_DP83822_FIBER_ADVERTISE    (ADVERTISED_TP | ADVERTISED_MII | \
					ADVERTISED_FIBRE | \
					ADVERTISED_Pause | ADVERTISED_Asym_Pause)

/*! @brief Defines the timeout macro. */
#define PHY_READID_TIMEOUT_COUNT (1000U)

/*! @brief Defines the PHY resource interface. */
#define PHY_DP83825_WRITE(handle, regAddr, data) \
    (((phy_dp83825_resource_t *)(handle)->resource)->write((handle)->phyAddr, regAddr, data))
#define PHY_DP83825_READ(handle, regAddr, pData) \
    (((phy_dp83825_resource_t *)(handle)->resource)->read((handle)->phyAddr, regAddr, pData))
#define PHY_DP83825_EXTWRITE(handle, devAddr, regAddr, data) \
    (((phy_dp83825_resource_t *)(handle)->resource)->writeExt((handle)->phyAddr, devAddr, regAddr, data))
#define PHY_DP83825_EXTREAD(handle, devAddr, regAddr, pData) \
    (((phy_dp83825_resource_t *)(handle)->resource)->readExt((handle)->phyAddr, devAddr, regAddr, pData))

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
const phy_operations_t phydp83825_ops = {.phyInit             = PHY_DP83825_Init,
                                         .phyWrite            = PHY_DP83825_Write,
                                         .phyRead             = PHY_DP83825_Read,
                                         .getAutoNegoStatus   = PHY_DP83825_GetAutoNegotiationStatus,
                                         .getLinkStatus       = PHY_DP83825_GetLinkStatus,
                                         .getLinkSpeedDuplex  = PHY_DP83825_GetLinkSpeedDuplex,
                                         .setLinkSpeedDuplex  = PHY_DP83825_SetLinkSpeedDuplex,
                                         .enableLoopback      = PHY_DP83825_EnableLoopback,
                                         .enableLinkInterrupt = PHY_DP83825_EnableLinkInterrupt,
                                         .clearInterrupt      = PHY_DP83825_ClearInterrupt};

/*******************************************************************************
 * Code
 ******************************************************************************/

status_t PHY_DP83825_Init(phy_handle_t *handle, const phy_config_t *config)
{
    int32_t counter  = PHY_READID_TIMEOUT_COUNT;
    status_t result   = kStatus_Success;
    uint16_t regValue = 0;

    /* Assign PHY address and operation resource. */
    handle->phyAddr  = config->phyAddr;
    handle->resource = config->resource;

    /* Check PHY ID. */
    do
    {
        uint32_t phyID;
        result = PHY_DP83825_READ(handle, PHY_ID1_REG, &regValue);
        if (result != kStatus_Success)
        {
            return result;
        }
        phyID = regValue << 16;
        result = PHY_DP83825_READ(handle, PHY_ID2_REG, &regValue);
        if (result != kStatus_Success)
        {
            return result;
        }
        phyID = (phyID | (regValue));
        switch (phyID) {
            case DP83822_PHY_ID:
            case DP83825S_PHY_ID:
            case DP83825I_PHY_ID:
            case DP83825CM_PHY_ID:
            case DP83825CS_PHY_ID:
            case DP83826C_PHY_ID:
            case DP83826NC_PHY_ID:
                counter = -1;
                break;
            default:
                counter--;
                break;
        }
    } while (counter > 0);

    if (counter == 0U)
    {
        return kStatus_Fail;
    }

    /* Reset PHY. */
    result = PHY_DP83825_WRITE(handle, PHY_BASICCONTROL_REG, PHY_BCTL_RESET_MASK);
    if (result == kStatus_Success)
    {
        /* RMII configuration */
        result = PHY_DP83825_WRITE(handle, MII_DP83822_RCSR, 
            (DP83822_RMII_MODE_SEL|DP83822_ELASTICBUF_14B));
        if (result != kStatus_Success)
        {
            return result;
        }

        /* Disable Wake on Lan. */
        result = PHY_DP83825_EnableWakeOnLan(handle, config->intrType, false);
        if (result != kStatus_Success)
        {
            return result;
        }

        /* Set PHY link status management interrupt. */
        result = PHY_DP83825_EnableLinkInterrupt(handle, config->intrType, config->enableLinkIntr);
        if (result != kStatus_Success)
        {
            return result;
        }

        /* Initialize AutoMDIX */
        result = PHY_DP83825_EnableAutoMDIX(handle, config->intrType, true);
        if (result != kStatus_Success)
        {
            return result;
        }

        if (config->autoNeg)
        {
            /* Set the auto-negotiation then start it. */
            result = PHY_DP83825_WRITE(
                handle, PHY_AUTONEG_ADVERTISE_REG,
                (PHY_100BASETX_FULLDUPLEX_MASK | PHY_100BASETX_HALFDUPLEX_MASK | PHY_10BASETX_FULLDUPLEX_MASK |
                 PHY_10BASETX_HALFDUPLEX_MASK | PHY_IEEE802_3_SELECTOR_MASK));
            if (result == kStatus_Success)
            {
                result = PHY_DP83825_WRITE(handle, PHY_BASICCONTROL_REG,
                                           (PHY_BCTL_AUTONEG_MASK | PHY_BCTL_RESTART_AUTONEG_MASK));
            }
        }
        else
        {
            /* This PHY only supports 10/100M speed. */
            assert(config->speed <= kPHY_Speed100M);

            /* Disable isolate mode */
            result = PHY_DP83825_READ(handle, PHY_BASICCONTROL_REG, &regValue);
            if (result != kStatus_Success)
            {
                return result;
            }
            regValue &= ~PHY_BCTL_ISOLATE_MASK;
            result = PHY_DP83825_WRITE(handle, PHY_BASICCONTROL_REG, regValue);
            if (result != kStatus_Success)
            {
                return result;
            }

            /* Disable the auto-negotiation and set user-defined speed/duplex configuration. */
            result = PHY_DP83825_SetLinkSpeedDuplex(handle, config->speed, config->duplex);
        }
    }
    return result;
}

status_t PHY_DP83825_Write(phy_handle_t *handle, uint8_t phyReg, uint16_t data)
{
    return PHY_DP83825_WRITE(handle, phyReg, data);
}

status_t PHY_DP83825_Read(phy_handle_t *handle, uint8_t phyReg, uint16_t *pData)
{
    return PHY_DP83825_READ(handle, phyReg, pData);
}

status_t PHY_DP83825_GetAutoNegotiationStatus(phy_handle_t *handle, bool *status)
{
    assert(status);

    status_t result;
    uint16_t regValue;

    *status = false;

    /* Check auto negotiation complete. */
    result = PHY_DP83825_READ(handle, PHY_BASICSTATUS_REG, &regValue);
    if (result == kStatus_Success)
    {
        if ((regValue & PHY_BSTATUS_AUTONEGCOMP_MASK) != 0U)
        {
            *status = true;
        }
    }
    return result;
}

status_t PHY_DP83825_GetLinkStatus(phy_handle_t *handle, bool *status)
{
    assert(status);

    status_t result;
    uint16_t regValue;

    /* Read the basic status register. */
    result = PHY_DP83825_READ(handle, PHY_BASICSTATUS_REG, &regValue);
    if (result == kStatus_Success)
    {
        if ((PHY_BSTATUS_LINKSTATUS_MASK & regValue) != 0U)
        {
            /* Link up. */
            *status = true;
        }
        else
        {
            /* Link down. */
            *status = false;
        }
    }
    return result;
}

status_t PHY_DP83825_GetLinkSpeedDuplex(phy_handle_t *handle, phy_speed_t *speed, phy_duplex_t *duplex)
{
    assert(!((speed == NULL) && (duplex == NULL)));

    status_t result;
    uint16_t regValue;

    /* Read the control register. */
    result = PHY_DP83825_READ(handle, MII_DP83822_PHYSTS, &regValue);
    if (result == kStatus_Success)
    {
        if (speed != NULL)
        {
            if (regValue & DP83822_PHYSTS_10)
            {
                *speed = kPHY_Speed10M;
            }
            else
            {
                *speed = kPHY_Speed100M;
            }
        }

        if (duplex != NULL)
        {
            if (regValue & DP83822_PHYSTS_DUPLEX)
            {
                *duplex = kPHY_FullDuplex;
            }
            else
            {
                *duplex = kPHY_HalfDuplex;
            }
        }
    }
    return result;
}

status_t PHY_DP83825_SetLinkSpeedDuplex(phy_handle_t *handle, phy_speed_t speed, phy_duplex_t duplex)
{
    /* This PHY only supports 10/100M speed. */
    assert(speed <= kPHY_Speed100M);

    status_t result;
    uint16_t regValue;

    result = PHY_DP83825_READ(handle, PHY_BASICCONTROL_REG, &regValue);
    if (result == kStatus_Success)
    {
        /* Disable the auto-negotiation and set according to user-defined configuration. */
        regValue &= ~PHY_BCTL_AUTONEG_MASK;
        if (speed == kPHY_Speed100M)
        {
            regValue |= PHY_BCTL_SPEED0_MASK;
        }
        else
        {
            regValue &= ~PHY_BCTL_SPEED0_MASK;
        }
        if (duplex == kPHY_FullDuplex)
        {
            regValue |= PHY_BCTL_DUPLEX_MASK;
        }
        else
        {
            regValue &= ~PHY_BCTL_DUPLEX_MASK;
        }
        result = PHY_DP83825_WRITE(handle, PHY_BASICCONTROL_REG, regValue);
    }
    return result;
}

status_t PHY_DP83825_EnableLoopback(phy_handle_t *handle, phy_loop_t mode, phy_speed_t speed, bool enable)
{
    /* This PHY only supports local/remote loopback and 10/100M speed. */
    assert(mode <= kPHY_RemoteLoop);
    assert(speed <= kPHY_Speed100M);

    status_t result;
    uint16_t regValue;

    /* Set the loop mode. */
    if (enable)
    {
        if (mode == kPHY_LocalLoop)
        {
            if (speed == kPHY_Speed100M)
            {
                regValue = PHY_BCTL_SPEED0_MASK | PHY_BCTL_DUPLEX_MASK | PHY_BCTL_LOOP_MASK;
            }
            else
            {
                regValue = PHY_BCTL_DUPLEX_MASK | PHY_BCTL_LOOP_MASK;
            }
            return PHY_DP83825_WRITE(handle, PHY_BASICCONTROL_REG, regValue);
        }
        else
        {
            /* Remote loopback only supports 100M full-duplex. */
            assert(speed == kPHY_Speed100M);

            regValue = PHY_BCTL_SPEED0_MASK | PHY_BCTL_DUPLEX_MASK | PHY_BCTL_LOOP_MASK;
            result   = PHY_DP83825_WRITE(handle, PHY_BASICCONTROL_REG, regValue);
            if (result != kStatus_Success)
            {
                return result;
            }
            /* Set the remote loopback bit. */
            result = PHY_DP83825_READ(handle, MII_DP83822_BISCR, &regValue);
            if (result == kStatus_Success)
            {
                regValue &= ~DP83822_BISCR_LOOPBACKMODE_MASK;
                regValue |= DP83822_LOOPBACKMODE_REVERSE;
                return PHY_DP83825_WRITE(handle, MII_DP83822_BISCR, regValue);
            }
        }
    }
    else
    {
        /* Disable the loop mode. */
        if (mode == kPHY_LocalLoop)
        {
            /* First read the current status in control register. */
            result = PHY_DP83825_READ(handle, PHY_BASICCONTROL_REG, &regValue);
            if (result == kStatus_Success)
            {
                regValue &= ~PHY_BCTL_LOOP_MASK;
                return PHY_DP83825_WRITE(handle, PHY_BASICCONTROL_REG, (regValue | PHY_BCTL_RESTART_AUTONEG_MASK));
            }
        }
        else
        {
            /* Clear the remote loopback bit. */
            result = PHY_DP83825_READ(handle, MII_DP83822_BISCR, &regValue);
            if (result == kStatus_Success)
            {
                regValue &= ~DP83822_BISCR_LOOPBACKMODE_MASK;
                return PHY_DP83825_WRITE(handle, MII_DP83822_BISCR, regValue);
            }
        }
    }
    return result;
}

status_t PHY_DP83825_EnableAutoMDIX(phy_handle_t *handle, phy_interrupt_type_t type, bool enable)
{
    status_t result;
    uint16_t regValue;

    result = PHY_DP83825_READ(handle, MII_DP83822_PHYCR, &regValue);
    if (result == kStatus_Success)
    {
        /* Enable/Disable Auto MDI/X. */
        if (enable)
        {
            regValue |= DP83822_MDIX_AUTO_EN;
        }
        else
        {
            // TODO: Implement
            return kStatus_InvalidArgument;
        }
        result = PHY_DP83825_WRITE(handle, MII_DP83822_MISR1, regValue);
    }
    if (result != kStatus_Success)
    {
        return result;
    } 
}

status_t PHY_DP83825_EnableWakeOnLan(phy_handle_t *handle, phy_interrupt_type_t type, bool enable)
{
    status_t result;
    uint16_t regValue;

    result = PHY_DP83825_EXTREAD(handle, DP83822_DEVADDR, MII_DP83822_WOL_CFG, &regValue);
    if (result == kStatus_Success)
    {
        /* Enable/Disable Wake on lan. */
        if (enable)
        {
            // TODO: Implement
            return kStatus_InvalidArgument;
        }
        else
        {
            regValue &= ~(DP83822_WOL_EN|DP83822_WOL_MAGIC_EN|DP83822_WOL_SECURE_ON);
        }
        result = PHY_DP83825_EXTWRITE(handle, DP83822_DEVADDR, MII_DP83822_MISR1, regValue);
    }
    if (result != kStatus_Success)
    {
        return result;
    }
}

status_t PHY_DP83825_EnableLinkInterrupt(phy_handle_t *handle, phy_interrupt_type_t type, bool enable)
{
    assert((type == kPHY_IntrActiveLow) || (type == kPHY_IntrActiveHigh));

    status_t result;
    uint16_t regValue;

    result = PHY_DP83825_READ(handle, MII_DP83822_MISR1, &regValue);
    if (result == kStatus_Success)
    {
        /* Enable/Disable link up+down interrupt. */
        if (enable)
        {
            regValue |= DP83822_LINK_STAT_INT_EN;
        }
        else
        {
            regValue &= ~DP83822_LINK_STAT_INT_EN;
        }
        result = PHY_DP83825_WRITE(handle, MII_DP83822_MISR1, regValue);
    }
    if (result != kStatus_Success)
    {
        return result;
    }
    result = PHY_DP83825_READ(handle, MII_DP83822_PHYSCR, &regValue);
    if (result == kStatus_Success)
    {
        /* Enable/Disable link up+down interrupt. */
        if (enable)
        {
            regValue |= DP83822_PHYSCR_INTEN;
            regValue |= DP83822_PHYSCR_INT_OE;
        }
        else
        {
            regValue &= ~DP83822_PHYSCR_INTEN;
            regValue &= ~DP83822_PHYSCR_INT_OE;
        }
        result = PHY_DP83825_WRITE(handle, MII_DP83822_MISR1, regValue);
    }
    if (result != kStatus_Success)
    {
        return result;
    }

    return result;
}

status_t PHY_DP83825_ClearInterrupt(phy_handle_t *handle)
{
    uint16_t regValue;
    status_t result;

    result = PHY_DP83825_READ(handle, MII_DP83822_MISR1, &regValue);
    if (result != kStatus_Success)
    {
        return result;
    }
    result = PHY_DP83825_READ(handle, MII_DP83822_MISR2, &regValue);

    return result;
}
