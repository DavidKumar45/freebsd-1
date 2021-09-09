/*-
 * SPDX-License-Identifier: BSD-2-Clause-FreeBSD
 *
 * Copyright (c) 2018-2021 Emmanuel Vadot <manu@freebsd.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD$
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/bus.h>
#include <sys/rman.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <machine/bus.h>

#include <dev/fdt/simplebus.h>

#include <dev/ofw/ofw_bus.h>
#include <dev/ofw/ofw_bus_subr.h>

#include <dev/extres/clk/clk_div.h>
#include <dev/extres/clk/clk_fixed.h>
#include <dev/extres/clk/clk_mux.h>

#include <arm64/rockchip/clk/rk_cru.h>

/* Registers */
#define	RK3328_GRF_SOC_CON4	0x410
#define	RK3328_GRF_MAC_CON1	0x904
#define	RK3328_GRF_MAC_CON2	0x908

/* GATES */

#define	SCLK_I2S0		41
#define	SCLK_I2S1		42
#define	SCLK_I2S2		43
#define	SCLK_I2S1_OUT		44
#define	SCLK_I2S2_OUT		45
#define	SCLK_MAC2PHY_RXTX	83
#define	SCLK_MAC2PHY_SRC	84
#define	SCLK_MAC2PHY_REF	85
#define	SCLK_MAC2PHY_OUT	86
#define	SCLK_MAC2IO_RX		87
#define	SCLK_MAC2IO_TX		88
#define	SCLK_MAC2IO_REFOUT	89
#define	SCLK_MAC2IO_REF		90
#define	SCLK_MAC2IO_OUT		91
#define	SCLK_USB3OTG_REF	96
#define	SCLK_MAC2IO_SRC		99
#define	SCLK_MAC2IO		100
#define	SCLK_MAC2PHY		101
#define	SCLK_MAC2IO_EXT		102
#define	ACLK_USB3OTG		132
#define	ACLK_GMAC		146
#define	ACLK_MAC2PHY		149
#define	ACLK_MAC2IO		150
#define	ACLK_PERI		153
#define	PCLK_GPIO0		200
#define	PCLK_GPIO1		201
#define	PCLK_GPIO2		202
#define	PCLK_GPIO3		203
#define	PCLK_I2C0		205
#define	PCLK_I2C1		206
#define	PCLK_I2C2		207
#define	PCLK_I2C3		208
#define	PCLK_TSADC		213
#define	PCLK_GMAC		220
#define	PCLK_MAC2PHY		222
#define	PCLK_MAC2IO		223
#define	PCLK_USB3PHY_OTG	224
#define	PCLK_USB3PHY_PIPE	225
#define	PCLK_USB3_GRF		226
#define	PCLK_ACODECPHY		235
#define	HCLK_I2S0_8CH		311
#define	HCLK_I2S1_8CH		312
#define	HCLK_I2S2_2CH		313
#define	HCLK_SDMMC		317
#define	HCLK_SDIO		318
#define	HCLK_EMMC		319
#define	HCLK_SDMMC_EXT		320

static struct rk_cru_gate rk3328_gates[] = {
	/* CRU_CLKGATE_CON0 */
	CRU_GATE(0, "core_apll_clk_en", "apll", 0x200, 0)
	CRU_GATE(0, "core_dpll_clk_en", "dpll", 0x200, 1)
	CRU_GATE(0, "core_gpll_clk_en", "gpll", 0x200, 2)
	/* Bit 3 bus_src_clk_en */
	/* Bit 4 clk_ddrphy_src_en */
	/* Bit 5 clk_ddrpd_src_en */
	/* Bit 6 clk_ddrmon_en */
	/* Bit 7-8 unused */
	/* Bit 9 testclk_en */
	/* Bit 10 clk_wifi */
	/* Bit 11 clk_rtc32k_src_en */
	CRU_GATE(0, "core_npll_clk_en", "npll", 0x200, 12)
	/* Bit 13-15 unused */

	/* CRU_CLKGATE_CON1 */
	/* Bit 0 unused */
	CRU_GATE(0, "clk_i2s0_src_en", "clk_i2s0_mux", 0x204, 1)
	CRU_GATE(0, "clk_i2s0_frac_src_en", "clk_i2s0_mux", 0x204, 2)
	CRU_GATE(SCLK_I2S0, "clk_i2s0_en", "clk_i2s0_mux", 0x204, 3)
	CRU_GATE(0, "clk_i2s1_src_en", "clk_i2s1_mux", 0x204, 4)
	CRU_GATE(0, "clk_i2s1_frac_src_en", "clk_i2s1_mux", 0x204, 5)
	CRU_GATE(SCLK_I2S1, "clk_i2s1_en", "clk_i2s1_mux", 0x204, 6)
	CRU_GATE(0, "clk_i2s1_out_en", "clk_i2s1_mux", 0x204, 7)
	CRU_GATE(0, "clk_i2s2_src_en", "clk_i2s2_mux", 0x204, 8)
	CRU_GATE(0, "clk_i2s2_frac_src_en", "clk_i2s2_mux", 0x204, 9)
	CRU_GATE(SCLK_I2S2, "clk_i2s2_en", "clk_i2s2_mux", 0x204, 10)
	CRU_GATE(0, "clk_i2s2_out_en", "clk_i2s2_mux", 0x204, 11)
	/* Bit 12 clk_spdif_src_en */
	/* Bit 13 clk_spdif_frac_src_en */
	/* Bit 14 clk_uart0_src_en */
	/* Bit 15 clk_uart0_frac_src_en */

	/* CRU_CLKGATE_CON2 */
	/* Bit 0 clk_uart1_src_en */
	/* Bit 1 clk_uart1_frac_src_en */
	/* Bit 2 clk_uart2_src_en */
	/* Bit 3 clk_uart2_frac_src_en */
	/* Bit 4 clk_crypto_src_en */
	/* Bit 5 clk_tsp_src_en */
	/* Bit 6 clk_tsadc_src_en */
	/* Bit 7 clk_spi0_src_en */
	/* Bit 8 clk_pwm0_src_en */
	/* Bit 9 clk_i2c0_src_en */
	/* Bit 10 clk_i2c1_src_en */
	/* Bit 11 clk_i2c2_src_en */
	/* Bit 12 clk_i2c3_src_en */
	/* Bit 13 clk_efuse_src_en */
	/* Bit 14 clk_saradc_src_en */
	/* Bit 15 clk_pdm_src_en */

	/* CRU_CLKGATE_CON3 */
	/* Bit 0 clk_gmac2phy_src_en */
	/* Bit 1 clk_gmac2io_src_en */
	/* Bit 2 gmac_cpll_src_en */
	/* Bit 3 gmac_gpll_src_en */
	/* Bit 4 gmac_vpll_src_en */
	/* Bit 5 clk_gmac2io_out_en */
	/* Bit 6-7 unused */
	/* Bit 8 clk_otp_src_en */
	/* Bit 9-15 unused */

	/* CRU_CLKGATE_CON4 */
	CRU_GATE(0, "periph_gclk_src_en", "gpll", 0x210, 0)
	CRU_GATE(0, "periph_cclk_src_en", "cpll", 0x210, 1)
	/* Bit 2 periph_vclk_src_en */
	/* Bit 3 clk_mmc0_src_en */
	/* Bit 4 clk_sdio_src_en */
	/* Bit 5 clk_emmc_src_en */
	/* Bit 6 clk_otgphy0_src_en */
	CRU_GATE(SCLK_USB3OTG_REF, "clk_usb3_otg0_ref", "xin24m", 0x210, 7)
	/* Bit 8 clk_usb3_otg0_suspend_en */
	/* Bit 9 clk_usb3phy_ref_25m_en */
	/* Bit 10 clk_sdmmcext_src_en */
	/* Bit 11-15 unused */

	/* CRU_CLKGATE_CON5 */
	/* Bit 0 aclk_rga_src_en */
	/* Bit 1 clk_rga_src_en */
	/* Bit 2 aclk_vio_src_en */
	/* Bit 3 clk_cif_out_src_en */
	/* Bit 4 clk_hdmi_sfr_en */
	/* Bit 5 aclk_vop_src_en */
	/* Bit 6 dclk_vop_src_en */
	/* Bit 7-15 unused */

	/* CRU_CLKGATE_CON6 */
	/* Bit 0 aclk_rkvdec_src_en */
	/* Bit 1 clk_cabac_src_en */
	/* Bit 2 clk_vdec_core_src_en */
	/* Bit 3 aclk_rkvenc_src_en */
	/* Bit 4 clk_venc_core_src_en */
	/* Bit 5 aclk_vpu_src_en */
	/* Bit 6 aclk_gpu_src_en */
	/* Bit 7 clk_venc_dsp_src_en */
	/* Bit 8-15 unused */

	/* CRU_CLKGATE_CON7 */
	/* Bit 0 aclk_core_en */
	/* Bit 1 clk_core_periph_en */
	/* Bit 2 clk_jtag_en */
	/* Bit 3 unused */
	/* Bit 4 pclk_ddr_en */
	/* Bit 5-15 unused */

	/* CRU_CLKGATE_CON8 */
	/* Bit 0 aclk_bus_en */
	/* Bit 1 hclk_bus_en */
	/* Bit 2 pclk_bus_src_en */
	CRU_GATE(0, "pclk_bus_en", "pclk_bus_pre", 0x220, 3)
	CRU_GATE(0, "pclk_phy_en", "pclk_bus_pre", 0x220, 4)
	/* Bit 5 clk_timer0_en */
	/* Bit 6 clk_timer1_en */
	/* Bit 7 clk_timer2_en */
	/* Bit 8 clk_timer3_en */
	/* Bit 9 clk_timer4_en */
	/* Bit 10 clk_timer5_en */
	/* Bit 11-15 unused */

	/* CRU_CLKGATE_CON9 */
	/* Bit 0 pclk_gmac_en */
	CRU_GATE(SCLK_MAC2PHY_RXTX, "clk_gmac2phy_rx_en", "clk_mac2phy", 0x224, 1)
	/* Bit 2 clk_macphy_en */
	CRU_GATE(SCLK_MAC2PHY_REF, "clk_gmac2phy_ref_en", "clk_mac2phy", 0x224, 3)
	CRU_GATE(SCLK_MAC2IO_RX, "clk_gmac2io_rx_en", "clk_mac2io", 0x224, 4)
	CRU_GATE(SCLK_MAC2IO_TX, "clk_gmac2io_tx_en", "clk_mac2io", 0x224, 5)
	CRU_GATE(SCLK_MAC2IO_REFOUT, "clk_gmac2io_refout_en", "clk_mac2io", 0x224, 6)
	CRU_GATE(SCLK_MAC2IO_REF, "clk_gmac2io_ref_en", "clk_mac2io", 0x224, 7)
	/* Bit 8-15 unused */

	/* CRU_CLKGATE_CON10 */
	CRU_GATE(ACLK_PERI, "aclk_periph_en", "aclk_peri_pre", 0x228, 0)
	/* Bit 1 hclk_periph_en */
	/* Bit 2 pclk_periph_en */
	/* Bit 3-15 unused */

	/* CRU_CLKGATE_CON11 */
	/* Bit 0 hclk_rkvdec_en */
	/* Bit 1-3 unused */
	/* Bit 4 hclk_rkvenc_en */
	/* Bit 5-7 unused */
	/* Bit 8 hclk_vpu_en */
	/* Bit 9-15 unused */

	/* CRU_CLKGATE_CON12 */
	/* unused */

	/* CRU_CLKGATE_CON13 */
	/* Bit 0 aclk_core_niu_en */
	/* Bit 1 aclk_gic400_en */
	/* Bit 2-15 unused */

	/* CRU_CLKGATE_CON14 */
	/* Bit 0 aclk_gpu_en */
	/* Bit 1 aclk_gpu_niu_en */
	/* Bit 2-15 unused */

	/* CRU_CLKGATE_CON15*/
	/* Bit 0 aclk_intmem_en */
	/* Bit 1 aclk_dmac_bus_en */
	/* Bit 2 hclk_rom_en */
	CRU_GATE(HCLK_I2S0_8CH, "hclk_i2s0_8ch_en", "hclk_bus_pre", 0x23C, 3)
	CRU_GATE(HCLK_I2S1_8CH, "hclk_i2s1_8ch_en", "hclk_bus_pre", 0x23C, 4)
	CRU_GATE(HCLK_I2S2_2CH, "hclk_i2s2_2ch_en", "hclk_bus_pre", 0x23C, 5)
	/* Bit 6 hclk_spdif_8ch_en */
	/* Bit 7 mclk_crypto_en */
	/* Bit 8 sclk_crypto_en */
	/* Bit 9 pclk_efuse_1024_en */
	CRU_GATE(PCLK_I2C0, "pclk_i2c0_en", "pclk_bus_en", 0x23C, 10)
	/* Bit 11 aclk_dcf_en */
	/* Bit 12 aclk_bus_niu_en */
	/* Bit 13 hclk_bus_niu_en */
	/* Bit 14 pclk_bus_niu_en */
	/* Bit 15 pclk_phy_niu_en */

	/* CRU_CLKGATE_CON16 */
	CRU_GATE(PCLK_I2C1, "pclk_i2c1_en", "pclk_bus_en", 0x240, 0)
	CRU_GATE(PCLK_I2C2, "pclk_i2c2_en", "pclk_bus_en", 0x240, 1)
	CRU_GATE(PCLK_I2C3, "pclk_i2c3_en", "pclk_bus_en", 0x240, 2)
	/* Bit 3 pclk_timer0_en */
	/* Bit 4 pclk_stimer_en */
	/* Bit 5 pclk_spi0_en */
	/* Bit 6 pclk_rk_pwm_en */
	CRU_GATE(PCLK_GPIO0, "pclk_gpio0_en", "pclk_bus_en", 0x240, 7)
	CRU_GATE(PCLK_GPIO1, "pclk_gpio1_en", "pclk_bus_en", 0x240, 8)
	CRU_GATE(PCLK_GPIO2, "pclk_gpio2_en", "pclk_bus_en", 0x240, 9)
	CRU_GATE(PCLK_GPIO3, "pclk_gpio3_en", "pclk_bus_en", 0x240, 10)
	/* Bit 11 pclk_uart0_en */
	/* Bit 12 pclk_uart1_en */
	/* Bit 13 pclk_uart2_en */
	CRU_GATE(PCLK_TSADC, "pclk_tsadc_en", "pclk_bus_en", 0x240, 14)
	/* Bit 15 pclk_dcf_en */

	/* CRU_CLKGATE_CON17 */
	/* Bit 0 pclk_grf_en */
	/* Bit 1 unused */
	CRU_GATE(PCLK_USB3_GRF, "pclk_usb3grf_en", "pclk_phy_en", 0x244, 2)
	/* Bit 3 pclk_ddrphy_en */
	/* Bit 4 pclk_cru_en */
	CRU_GATE(PCLK_ACODECPHY, "pclk_acodecphy_en", "pclk_phy_en", 0x244, 5)
	/* Bit 6 pclk_sgrf_en */
	/* Bit 7 pclk_hdmiphy_en */
	/* Bit 8 pclk_vdacphy_en */
	/* Bit 9 unused */
	/* Bit 10 pclk_scr_en */
	/* Bit 11 hclk_tsp_en */
	/* Bit 12 aclk_tsp_en */
	/* Bit 13 clk_hsadc_0_tsp_en */
	/* Bit 14 pclk_usb_grf_en */
	/* Bit 15 pclk_saradc_en */

	/* CRU_CLKGATE_CON18 */
	/* Bit 0 unused */
	/* Bit 1 pclk_ddr_upctl_en */
	/* Bit 2 pclk_ddr_msch_en */
	/* Bit 3 pclk_ddr_mon_en */
	/* Bit 4 aclk_ddr_upctl_en */
	/* Bit 5 clk_ddr_upctl_en */
	/* Bit 6 clk_ddr_msch_en */
	/* Bit 7 pclk_ddrstdby_en */
	/* Bit 8-15 unused */

	/* CRU_CLKGATE_CON19 */
	CRU_GATE(HCLK_SDMMC, "hclk_sdmmc_en", "hclk_peri", 0x24C, 0)
	CRU_GATE(HCLK_SDIO, "hclk_sdio_en", "hclk_peri", 0x24C, 1)
	CRU_GATE(HCLK_EMMC, "hclk_emmc_en", "hclk_peri", 0x24C, 2)
	/* Bit 3-5 unused */
	/* Bit 6 hclk_host0_en */
	/* Bit 7 hclk_host0_arb_en */
	/* Bit 8 hclk_otg_en */
	/* Bit 9 hclk_otg_pmu_en */
	/* Bit 10 unused */
	/* Bit 11 aclk_peri_niu_en */
	CRU_GATE(0, "hclk_peri_niu_en", "hclk_peri", 0x24C, 12)
	CRU_GATE(0, "pclk_peri_niu_en", "hclk_peri", 0x24C, 13)
	CRU_GATE(ACLK_USB3OTG, "aclk_usb3otg_en", "aclk_periph_en", 0x24C, 14)
	CRU_GATE(HCLK_SDMMC_EXT, "hclk_sdmmc_ext_en", "hclk_peri", 0x24C, 15)

	/* CRU_CLKGATE_CON20 */
	/* unused */

	/* CRU_CLKGATE_CON21 */
	/* Bit 0-1 unused */
	/* Bit 2 aclk_vop_en */
	/* Bit 3 hclk_vop_en */
	/* Bit 4 aclk_vop_niu_en */
	/* Bit 5 hclk_vop_niu_en */
	/* Bit 6 aclk_iep_en */
	/* Bit 7 hclk_iep_en */
	/* Bit 8 aclk_cif_en */
	/* Bit 9 hclk_cif_en */
	/* Bit 10 aclk_rga_en */
	/* Bit 11 hclk_rga_en */
	/* Bit 12 hclk_ahb1tom_en */
	/* Bit 13 pclk_h2p_en */
	/* Bit 14 hclk_h2p_en */
	/* Bit 15 aclk_hdcp_en */

	/* CRU_CLKGATE_CON22 */
	/* Bit 0 hclk_hdcp_en */
	/* Bit 1 hclk_vio_niu_en */
	/* Bit 2 aclk_vio_niu_en */
	/* Bit 3 aclk_rga_niu_en */
	/* Bit 4 pclk_hdmi_ctrl_en */
	/* Bit 5 pclk_hdcp_ctrl_en */
	/* Bit 6-15 unused */

	/* CRU_CLKGATE_CON23 */
	/* Bit 0 aclk_vpu_en */
	/* Bit 1 hclk_vpu_en */
	/* Bit 2 aclk_vpu_niu_en */
	/* Bit 3 hclk_vpu_niu_en */
	/* Bit 4-15 unused */

	/* CRU_CLKGATE_CON24 */
	/* Bit 0 aclk_rkvdec_en */
	/* Bit 1 hclk_rkvdec_en */
	/* Bit 2 aclk_rkvdec_niu_en */
	/* Bit 3 hclk_rkvdec_niu_en */
	/* Bit 4-15 unused */

	/* CRU_CLKGATE_CON25 */
	/* Bit 0 aclk_rkvenc_niu_en */
	/* Bit 1 hclk_rkvenc_niu_en */
	/* Bit 2 aclk_h265_en */
	/* Bit 3 pclk_h265_en */
	/* Bit 4 aclk_h264_en */
	/* Bit 5 hclk_h264_en */
	/* Bit 6 aclk_axi2sram_en */
	/* Bit 7-15 unused */

	/* CRU_CLKGATE_CON26 */
	CRU_GATE(ACLK_MAC2PHY, "aclk_gmac2phy_en", "aclk_gmac", 0x268, 0)
	CRU_GATE(PCLK_MAC2PHY, "pclk_gmac2phy_en", "pclk_gmac", 0x268, 1)
	CRU_GATE(ACLK_MAC2IO, "aclk_gmac2io_en", "aclk_gmac", 0x268, 2)
	CRU_GATE(PCLK_MAC2IO, "pclk_gmac2io_en", "pclk_gmac", 0x268, 3)
	/* Bit 4 aclk_gmac_niu_en */
	/* Bit 5 pclk_gmac_niu_en */
	/* Bit 6-15 unused */

	/* CRU_CLKGATE_CON27 */
	/* Bit 0 clk_ddrphy_en */
	/* Bit 1 clk4x_ddrphy_en */

	/* CRU_CLKGATE_CON28 */
	/* Bit 0 hclk_pdm_en */
	CRU_GATE(PCLK_USB3PHY_OTG, "pclk_usb3phy_otg_en", "pclk_phy_en", 0x270, 1)
	CRU_GATE(PCLK_USB3PHY_PIPE, "pclk_usb3phy_pipe_en", "pclk_phy_en", 0x270, 2)
	/* Bit 3 pclk_pmu_en */
	/* Bit 4 pclk_otp_en */
	/* Bit 5-15 unused */
};

/*
 * PLLs
 */

#define PLL_APLL		1
#define PLL_DPLL		2
#define PLL_CPLL		3
#define PLL_GPLL		4
#define PLL_NPLL		5

#define PLL_RATE(_hz, _ref, _fb, _post1, _post2, _dspd, _frac)		\
{									\
	.freq = _hz,							\
	.refdiv = _ref,							\
	.fbdiv = _fb,							\
	.postdiv1 = _post1,						\
	.postdiv2 = _post2,						\
	.dsmpd = _dspd,							\
	.frac = _frac,							\
}

static struct rk_clk_pll_rate rk3328_pll_rates[] = {
	/* _mhz, _refdiv, _fbdiv, _postdiv1, _postdiv2, _dsmpd, _frac */
	PLL_RATE(1608000000, 1, 67, 1, 1, 1, 0),
	PLL_RATE(1584000000, 1, 66, 1, 1, 1, 0),
	PLL_RATE(1560000000, 1, 65, 1, 1, 1, 0),
	PLL_RATE(1536000000, 1, 64, 1, 1, 1, 0),
	PLL_RATE(1512000000, 1, 63, 1, 1, 1, 0),
	PLL_RATE(1488000000, 1, 62, 1, 1, 1, 0),
	PLL_RATE(1464000000, 1, 61, 1, 1, 1, 0),
	PLL_RATE(1440000000, 1, 60, 1, 1, 1, 0),
	PLL_RATE(1416000000, 1, 59, 1, 1, 1, 0),
	PLL_RATE(1392000000, 1, 58, 1, 1, 1, 0),
	PLL_RATE(1368000000, 1, 57, 1, 1, 1, 0),
	PLL_RATE(1344000000, 1, 56, 1, 1, 1, 0),
	PLL_RATE(1320000000, 1, 55, 1, 1, 1, 0),
	PLL_RATE(1296000000, 1, 54, 1, 1, 1, 0),
	PLL_RATE(1272000000, 1, 53, 1, 1, 1, 0),
	PLL_RATE(1248000000, 1, 52, 1, 1, 1, 0),
	PLL_RATE(1200000000, 1, 50, 1, 1, 1, 0),
	PLL_RATE(1188000000, 2, 99, 1, 1, 1, 0),
	PLL_RATE(1104000000, 1, 46, 1, 1, 1, 0),
	PLL_RATE(1100000000, 12, 550, 1, 1, 1, 0),
	PLL_RATE(1008000000, 1, 84, 2, 1, 1, 0),
	PLL_RATE(1000000000, 6, 500, 2, 1, 1, 0),
	PLL_RATE(984000000, 1, 82, 2, 1, 1, 0),
	PLL_RATE(960000000, 1, 80, 2, 1, 1, 0),
	PLL_RATE(936000000, 1, 78, 2, 1, 1, 0),
	PLL_RATE(912000000, 1, 76, 2, 1, 1, 0),
	PLL_RATE(900000000, 4, 300, 2, 1, 1, 0),
	PLL_RATE(888000000, 1, 74, 2, 1, 1, 0),
	PLL_RATE(864000000, 1, 72, 2, 1, 1, 0),
	PLL_RATE(840000000, 1, 70, 2, 1, 1, 0),
	PLL_RATE(816000000, 1, 68, 2, 1, 1, 0),
	PLL_RATE(800000000, 6, 400, 2, 1, 1, 0),
	PLL_RATE(700000000, 6, 350, 2, 1, 1, 0),
	PLL_RATE(696000000, 1, 58, 2, 1, 1, 0),
	PLL_RATE(600000000, 1, 75, 3, 1, 1, 0),
	PLL_RATE(594000000, 2, 99, 2, 1, 1, 0),
	PLL_RATE(504000000, 1, 63, 3, 1, 1, 0),
	PLL_RATE(500000000, 6, 250, 2, 1, 1, 0),
	PLL_RATE(408000000, 1, 68, 2, 2, 1, 0),
	PLL_RATE(312000000, 1, 52, 2, 2, 1, 0),
	PLL_RATE(216000000, 1, 72, 4, 2, 1, 0),
	PLL_RATE(96000000, 1, 64, 4, 4, 1, 0),
	{},
};

static struct rk_clk_pll_rate rk3328_pll_frac_rates[] = {
	PLL_RATE(1016064000, 3, 127, 1, 1, 0, 134217),
	PLL_RATE(983040000, 24, 983, 1, 1, 0, 671088),
	PLL_RATE(491520000, 24, 983, 2, 1, 0, 671088),
	PLL_RATE(61440000, 6, 215, 7, 2, 0, 671088),
	PLL_RATE(56448000, 12, 451, 4, 4, 0, 9797894),
	PLL_RATE(40960000, 12, 409, 4, 5, 0, 10066329),
	{},
};

static const char *pll_parents[] = {"xin24m"};
static struct rk_clk_pll_def apll = {
	.clkdef = {
		.id = PLL_APLL,
		.name = "apll",
		.parent_names = pll_parents,
		.parent_cnt = nitems(pll_parents),
	},
	.base_offset = 0x00,
	.gate_offset = 0x200,
	.gate_shift = 0,
	.mode_reg = 0x80,
	.mode_shift = 1,
	.flags = RK_CLK_PLL_HAVE_GATE,
	.frac_rates = rk3328_pll_frac_rates,
};

static struct rk_clk_pll_def dpll = {
	.clkdef = {
		.id = PLL_DPLL,
		.name = "dpll",
		.parent_names = pll_parents,
		.parent_cnt = nitems(pll_parents),
	},
	.base_offset = 0x20,
	.gate_offset = 0x200,
	.gate_shift = 1,
	.mode_reg = 0x80,
	.mode_shift = 4,
	.flags = RK_CLK_PLL_HAVE_GATE,
};

static struct rk_clk_pll_def cpll = {
	.clkdef = {
		.id = PLL_CPLL,
		.name = "cpll",
		.parent_names = pll_parents,
		.parent_cnt = nitems(pll_parents),
	},
	.base_offset = 0x40,
	.mode_reg = 0x80,
	.mode_shift = 8,
	.rates = rk3328_pll_rates,
};

static struct rk_clk_pll_def gpll = {
	.clkdef = {
		.id = PLL_GPLL,
		.name = "gpll",
		.parent_names = pll_parents,
		.parent_cnt = nitems(pll_parents),
	},
	.base_offset = 0x60,
	.gate_offset = 0x200,
	.gate_shift = 2,
	.mode_reg = 0x80,
	.mode_shift = 12,
	.flags = RK_CLK_PLL_HAVE_GATE,
	.frac_rates = rk3328_pll_frac_rates,
};

static struct rk_clk_pll_def npll = {
	.clkdef = {
		.id = PLL_NPLL,
		.name = "npll",
		.parent_names = pll_parents,
		.parent_cnt = nitems(pll_parents),
	},
	.base_offset = 0xa0,
	.gate_offset = 0x200,
	.gate_shift = 12,
	.mode_reg = 0x80,
	.mode_shift = 1,
	.flags = RK_CLK_PLL_HAVE_GATE,
	.rates = rk3328_pll_rates,
};

/* CRU_CLKSEL_CON0 */
#define ACLK_BUS_PRE		136

/* Needs hdmiphy as parent too*/
static const char *aclk_bus_pre_parents[] = {"cpll", "gpll"};
static struct rk_clk_composite_def aclk_bus_pre = {
	.clkdef = {
		.id = ACLK_BUS_PRE,
		.name = "aclk_bus_pre",
		.parent_names = aclk_bus_pre_parents,
		.parent_cnt = nitems(aclk_bus_pre_parents),
	},
	.muxdiv_offset = 0x100,
	.mux_shift = 13,
	.mux_width = 2,

	.div_shift = 8,
	.div_width = 5,

	.gate_offset = 0x220,
	.gate_shift = 0,

	.flags = RK_CLK_COMPOSITE_HAVE_MUX | RK_CLK_COMPOSITE_HAVE_GATE,
};

static struct rk_clk_armclk_rates rk3328_armclk_rates[] = {
	{
		.freq = 1296000000,
		.div = 1,
	},
	{
		.freq = 1200000000,
		.div = 1,
	},
	{
		.freq = 1104000000,
		.div = 1,
	},
	{
		.freq = 1008000000,
		.div = 1,
	},
	{
		.freq = 912000000,
		.div = 1,
	},
	{
		.freq = 816000000,
		.div = 1,
	},
	{
		.freq = 696000000,
		.div = 1,
	},
	{
		.freq = 600000000,
		.div = 1,
	},
	{
		.freq = 408000000,
		.div = 1,
	},
	{
		.freq = 312000000,
		.div = 1,
	},
	{
		.freq = 216000000,
		.div = 1,
	},
	{
		.freq = 96000000,
		.div = 1,
	},
};

#define	ARMCLK	6
static const char *armclk_parents[] = {"apll", "gpll", "dpll", "npll" };
static struct rk_clk_armclk_def armclk = {
	.clkdef = {
		.id = ARMCLK,
		.name = "armclk",
		.parent_names = armclk_parents,
		.parent_cnt = nitems(armclk_parents),
	},
	.muxdiv_offset = 0x100,
	.mux_shift = 6,
	.mux_width = 2,

	.div_shift = 0,
	.div_width = 5,

	.flags = RK_CLK_COMPOSITE_HAVE_MUX,
	.main_parent = 3, /* npll */
	.alt_parent = 0, /* apll */

	.rates = rk3328_armclk_rates,
	.nrates = nitems(rk3328_armclk_rates),
};

/* CRU_CLKSEL_CON1 */

#define PCLK_BUS_PRE		216
#define HCLK_BUS_PRE		328

static const char *hclk_bus_pre_parents[] = {"aclk_bus_pre"};
static struct rk_clk_composite_def hclk_bus_pre = {
	.clkdef = {
		.id = HCLK_BUS_PRE,
		.name = "hclk_bus_pre",
		.parent_names = hclk_bus_pre_parents,
		.parent_cnt = nitems(hclk_bus_pre_parents),
	},
	.muxdiv_offset = 0x104,

	.div_shift = 8,
	.div_width = 2,

	.gate_offset = 0x220,
	.gate_shift = 1,

	.flags = RK_CLK_COMPOSITE_HAVE_GATE,
};

static const char *pclk_bus_pre_parents[] = {"aclk_bus_pre"};
static struct rk_clk_composite_def pclk_bus_pre = {
	.clkdef = {
		.id = PCLK_BUS_PRE,
		.name = "pclk_bus_pre",
		.parent_names = pclk_bus_pre_parents,
		.parent_cnt = nitems(pclk_bus_pre_parents),
	},
	.muxdiv_offset = 0x104,

	.div_shift = 12,
	.div_width = 3,

	.gate_offset = 0x220,
	.gate_shift = 2,

	.flags = RK_CLK_COMPOSITE_HAVE_GATE,
};

/* CRU_CLKSEL_CON22 */

#define SCLK_TSADC		36

static const char *clk_tsadc_parents[] = {"xin24m"};
static struct rk_clk_composite_def clk_tsadc = {
	.clkdef = {
		.id = SCLK_TSADC,
		.name = "clk_tsadc",
		.parent_names = clk_tsadc_parents,
		.parent_cnt = nitems(clk_tsadc_parents),
	},
	.div_shift = 0,
	.div_width = 9,
};

/* CRU_CLKSEL_CON28 */

#define ACLK_PERI_PRE		137

static const char *aclk_peri_pre_parents[] = {"cpll", "gpll"/* , "hdmiphy" */};
static struct rk_clk_composite_def aclk_peri_pre = {
	.clkdef = {
		.id = ACLK_PERI_PRE,
		.name = "aclk_peri_pre",
		.parent_names = aclk_peri_pre_parents,
		.parent_cnt = nitems(aclk_peri_pre_parents),
	},
	.muxdiv_offset = 0x170,

	.mux_shift = 6,
	.mux_width = 2,

	.div_shift = 0,
	.div_width = 5,

	.flags = RK_CLK_COMPOSITE_HAVE_MUX,
};

/* CRU_CLKSEL_CON29 */

#define PCLK_PERI		230
#define HCLK_PERI		308

static const char *phclk_peri_parents[] = {"aclk_peri_pre"};
static struct rk_clk_composite_def pclk_peri = {
	.clkdef = {
		.id = PCLK_PERI,
		.name = "pclk_peri",
		.parent_names = phclk_peri_parents,
		.parent_cnt = nitems(phclk_peri_parents),
	},

	.div_shift = 0,
	.div_width = 2,

	/* CRU_CLKGATE_CON10 */
	.gate_offset = 0x228,
	.gate_shift = 2,

	.flags = RK_CLK_COMPOSITE_HAVE_GATE,
};

static struct rk_clk_composite_def hclk_peri = {
	.clkdef = {
		.id = HCLK_PERI,
		.name = "hclk_peri",
		.parent_names = phclk_peri_parents,
		.parent_cnt = nitems(phclk_peri_parents),
	},

	.div_shift = 4,
	.div_width = 3,

	/* CRU_CLKGATE_CON10 */
	.gate_offset = 0x228,
	.gate_shift = 1,

	.flags = RK_CLK_COMPOSITE_HAVE_GATE,
};

/* CRU_CLKSEL_CON30 */

#define SCLK_SDMMC		33

static const char *mmc_parents[] = {"cpll", "gpll", "xin24m"/* , "usb480m" */};
static struct rk_clk_composite_def sdmmc = {
	.clkdef = {
		.id = SCLK_SDMMC,
		.name = "clk_sdmmc",
		.parent_names = mmc_parents,
		.parent_cnt = nitems(mmc_parents),
	},
	.muxdiv_offset = 0x178,

	.mux_shift = 8,
	.mux_width = 2,

	.div_shift = 0,
	.div_width = 8,

	/* CRU_CLKGATE_CON4 */
	.gate_offset = 0x210,
	.gate_shift = 3,

	.flags = RK_CLK_COMPOSITE_HAVE_MUX | RK_CLK_COMPOSITE_HAVE_GATE,
};

/* CRU_CLKSEL_CON31 */
#define SCLK_SDIO		34

static struct rk_clk_composite_def sdio = {
	.clkdef = {
		.id = SCLK_SDIO,
		.name = "clk_sdio",
		.parent_names = mmc_parents,
		.parent_cnt = nitems(mmc_parents),
	},
	.muxdiv_offset = 0x17C,

	.mux_shift = 8,
	.mux_width = 2,

	.div_shift = 0,
	.div_width = 8,

	/* CRU_CLKGATE_CON4 */
	.gate_offset = 0x210,
	.gate_shift = 4,

	.flags = RK_CLK_COMPOSITE_HAVE_MUX | RK_CLK_COMPOSITE_HAVE_GATE,
};

/* CRU_CLKSEL_CON32 */
#define SCLK_EMMC		35

static struct rk_clk_composite_def emmc = {
	.clkdef = {
		.id = SCLK_EMMC,
		.name = "clk_emmc",
		.parent_names = mmc_parents,
		.parent_cnt = nitems(mmc_parents),
	},
	.muxdiv_offset = 0x180,

	.mux_shift = 8,
	.mux_width = 2,

	.div_shift = 0,
	.div_width = 8,

	/* CRU_CLKGATE_CON4 */
	.gate_offset = 0x210,
	.gate_shift = 5,

	.flags = RK_CLK_COMPOSITE_HAVE_MUX | RK_CLK_COMPOSITE_HAVE_GATE,
};

/* CRU_CLKSEL_CON34 */
#define	SCLK_I2C0	55
#define	SCLK_I2C1	56

static const char *i2c_parents[] = {"cpll", "gpll"};

static struct rk_clk_composite_def i2c0 = {
	.clkdef = {
		.id = SCLK_I2C0,
		.name = "clk_i2c0",
		.parent_names = i2c_parents,
		.parent_cnt = nitems(i2c_parents),
	},
	.muxdiv_offset = 0x188,

	.mux_shift = 7,
	.mux_width = 1,

	.div_shift = 0,
	.div_width = 6,

	/* CRU_CLKGATE_CON2 */
	.gate_offset = 0x208,
	.gate_shift = 9,

	.flags = RK_CLK_COMPOSITE_HAVE_MUX | RK_CLK_COMPOSITE_HAVE_GATE,
};

static struct rk_clk_composite_def i2c1 = {
	.clkdef = {
		.id = SCLK_I2C1,
		.name = "clk_i2c1",
		.parent_names = i2c_parents,
		.parent_cnt = nitems(i2c_parents),
	},
	.muxdiv_offset = 0x188,

	.mux_shift = 15,
	.mux_width = 1,

	.div_shift = 8,
	.div_width = 6,

	/* CRU_CLKGATE_CON2 */
	.gate_offset = 0x208,
	.gate_shift = 10,

	.flags = RK_CLK_COMPOSITE_HAVE_MUX | RK_CLK_COMPOSITE_HAVE_GATE,
};

/* CRU_CLKSEL_CON35 */
#define	SCLK_I2C2	57
#define	SCLK_I2C3	58

static struct rk_clk_composite_def i2c2 = {
	.clkdef = {
		.id = SCLK_I2C2,
		.name = "clk_i2c2",
		.parent_names = i2c_parents,
		.parent_cnt = nitems(i2c_parents),
	},
	.muxdiv_offset = 0x18C,

	.mux_shift = 7,
	.mux_width = 1,

	.div_shift = 0,
	.div_width = 6,

	/* CRU_CLKGATE_CON2 */
	.gate_offset = 0x208,
	.gate_shift = 11,

	.flags = RK_CLK_COMPOSITE_HAVE_MUX | RK_CLK_COMPOSITE_HAVE_GATE,
};

static struct rk_clk_composite_def i2c3 = {
	.clkdef = {
		.id = SCLK_I2C3,
		.name = "clk_i2c3",
		.parent_names = i2c_parents,
		.parent_cnt = nitems(i2c_parents),
	},
	.muxdiv_offset = 0x18C,

	.mux_shift = 15,
	.mux_width = 1,

	.div_shift = 8,
	.div_width = 6,

	/* CRU_CLKGATE_CON2 */
	.gate_offset = 0x208,
	.gate_shift = 12,

	.flags = RK_CLK_COMPOSITE_HAVE_MUX | RK_CLK_COMPOSITE_HAVE_GATE,
};

#define	SCLK_USB3_REF		72
#define	SCLK_USB3_SUSPEND	73
#define	SCLK_USB3PHY_REF	94
#define	SCLK_REF_USB3OTG	95
#define	SCLK_USB3OTG_SUSPEND	97
#define	SCLK_REF_USB3OTG_SRC	98

static const char *ref_usb3otg_parents[] = { "xin24m", "clk_usb3_otg0_ref" };

static struct rk_clk_composite_def ref_usb3otg = {
	.clkdef = {
		.id = SCLK_REF_USB3OTG,
		.name = "clk_ref_usb3otg",
		.parent_names = ref_usb3otg_parents,
		.parent_cnt = nitems(ref_usb3otg_parents),
	},
	.muxdiv_offset = 0x1B4,

	.mux_shift = 8,
	.mux_width = 1,

	.flags = RK_CLK_COMPOSITE_HAVE_MUX,
};

static const char *usb3otg_suspend_parents[] = { "xin24m"/*, "clk_rtc32k" */};

static struct rk_clk_composite_def usb3otg_suspend = {
	.clkdef = {
		.id = SCLK_USB3OTG_SUSPEND,
		.name = "clk_usb3otg_suspend",
		.parent_names = usb3otg_suspend_parents,
		.parent_cnt = nitems(usb3otg_suspend_parents),
	},
	.muxdiv_offset = 0x184,

	.mux_shift = 15,
	.mux_width = 1,

	.div_shift = 0,
	.div_width = 10,

	/* CRU_CLKGATE_CON4 */
	.gate_offset = 0x210,
	.gate_shift = 8,

	.flags = RK_CLK_COMPOSITE_HAVE_GATE,
};

static const char *ref_usb3otg_src_parents[] = { "cpll", "gpll" };

static struct rk_clk_composite_def ref_usb3otg_src = {
	.clkdef = {
		.id = SCLK_REF_USB3OTG_SRC,
		.name = "clk_ref_usb3otg_src",
		.parent_names = ref_usb3otg_src_parents,
		.parent_cnt = nitems(ref_usb3otg_src_parents),
	},
	.muxdiv_offset = 0x1B4,

	.mux_shift = 7,
	.mux_width = 1,

	.div_shift = 0,
	.div_width = 7,

	/* CRU_CLKGATE_CON4 */
	.gate_offset = 0x210,
	.gate_shift = 9,

	.flags = RK_CLK_COMPOSITE_HAVE_GATE,
};

/* I2S0 */
static const char *i2s0_div_parents[] = { "cpll", "gpll" };
static struct rk_clk_composite_def i2s0_div = {
	.clkdef = {
		.id = 0,
		.name = "clk_i2s0_div",
		.parent_names = i2s0_div_parents,
		.parent_cnt = nitems(i2s0_div_parents),
	},
	/* CRU_CLKSEL_CON6 */
	.muxdiv_offset = 0x118,

	.mux_shift = 15,
	.mux_width = 1,

	.div_shift = 0,
	.div_width = 7,

	/* CRU_CLKGATE_CON1 */
	.gate_offset = 0x204,
	.gate_shift = 1,

	.flags = RK_CLK_COMPOSITE_HAVE_GATE,
};

static const char *i2s0_frac_parents[] = { "clk_i2s0_div" };
static struct rk_clk_fract_def i2s0_frac = {
	.clkdef = {
		.id = 0,
		.name = "clk_i2s0_frac",
		.parent_names = i2s0_frac_parents,
		.parent_cnt = nitems(i2s0_frac_parents),
	},
	/* CRU_CLKSEL_CON7 */
	.offset = 0x11c,

	/* CRU_CLKGATE_CON1 */
	.gate_offset = 0x204,
	.gate_shift = 2,

	.flags = RK_CLK_FRACT_HAVE_GATE,
};

static const char *i2s0_mux_parents[] = { "clk_i2s0_div", "clk_i2s0_frac", "xin12m", "xin12m" };
static struct rk_clk_mux_def i2s0_mux = {
	.clkdef = {
		.id = 0,
		.name = "clk_i2s0_mux",
		.parent_names = i2s0_mux_parents,
		.parent_cnt = nitems(i2s0_mux_parents),
	},
	.offset = 0x118,

	.shift = 8,
	.width = 2,

	.mux_flags = RK_CLK_MUX_REPARENT,
};

/* I2S1 */
static const char *i2s1_div_parents[] = { "cpll", "gpll" };
static struct rk_clk_composite_def i2s1_div = {
	.clkdef = {
		.id = 0,
		.name = "clk_i2s1_div",
		.parent_names = i2s1_div_parents,
		.parent_cnt = nitems(i2s1_div_parents),
	},
	/* CRU_CLKSEL_CON8 */
	.muxdiv_offset = 0x120,

	.mux_shift = 15,
	.mux_width = 1,

	.div_shift = 0,
	.div_width = 7,

	/* CRU_CLKGATE_CON1 */
	.gate_offset = 0x204,
	.gate_shift = 4,

	.flags = RK_CLK_COMPOSITE_HAVE_GATE,
};

static const char *i2s1_frac_parents[] = { "clk_i2s1_div" };
static struct rk_clk_fract_def i2s1_frac = {
	.clkdef = {
		.id = 0,
		.name = "clk_i2s1_frac",
		.parent_names = i2s1_frac_parents,
		.parent_cnt = nitems(i2s1_frac_parents),
	},
	/* CRU_CLKSEL_CON9 */
	.offset = 0x124,

	/* CRU_CLKGATE_CON1 */
	.gate_offset = 0x204,
	.gate_shift = 5,

	.flags = RK_CLK_FRACT_HAVE_GATE,
};

static const char *i2s1_mux_parents[] = { "clk_i2s1_div", "clk_i2s1_frac", "clkin_i2s1", "xin12m" };
static struct rk_clk_mux_def i2s1_mux = {
	.clkdef = {
		.id = 0,
		.name = "clk_i2s1_mux",
		.parent_names = i2s1_mux_parents,
		.parent_cnt = nitems(i2s1_mux_parents),
	},
	.offset = 0x120,

	.shift = 8,
	.width = 2,
	.mux_flags = RK_CLK_MUX_REPARENT,
};

static struct clk_fixed_def clkin_i2s1 = {
	.clkdef = {
		.id = 0,
		.name = "clkin_i2s1",
		.parent_names = NULL,
		.parent_cnt = 0
	},

	.freq = 0,
};

/* I2S2 */
static const char *i2s2_div_parents[] = { "cpll", "gpll" };
static struct rk_clk_composite_def i2s2_div = {
	.clkdef = {
		.id = 0,
		.name = "clk_i2s2_div",
		.parent_names = i2s2_div_parents,
		.parent_cnt = nitems(i2s2_div_parents),
	},
	/* CRU_CLKSEL_CON10 */
	.muxdiv_offset = 0x128,

	.mux_shift = 15,
	.mux_width = 1,

	.div_shift = 0,
	.div_width = 7,

	/* CRU_CLKGATE_CON1 */
	.gate_offset = 0x204,
	.gate_shift = 8,

	.flags = RK_CLK_COMPOSITE_HAVE_GATE,
};

static const char *i2s2_frac_parents[] = { "clk_i2s2_div" };
static struct rk_clk_fract_def i2s2_frac = {
	.clkdef = {
		.id = 0,
		.name = "clk_i2s2_frac",
		.parent_names = i2s2_frac_parents,
		.parent_cnt = nitems(i2s2_frac_parents),
	},
	/* CRU_CLKSEL_CON11 */
	.offset = 0x12c,

	/* CRU_CLKGATE_CON1 */
	.gate_offset = 0x204,
	.gate_shift = 9,

	.flags = RK_CLK_FRACT_HAVE_GATE,
};

static const char *i2s2_mux_parents[] = { "clk_i2s2_div", "clk_i2s2_frac", "clkin_i2s2", "xin12m" };
static struct rk_clk_mux_def i2s2_mux = {
	.clkdef = {
		.id = 0,
		.name = "clk_i2s2_mux",
		.parent_names = i2s2_mux_parents,
		.parent_cnt = nitems(i2s2_mux_parents),
	},
	.offset = 0x128,

	.shift = 8,
	.width = 2,

	.mux_flags = RK_CLK_MUX_REPARENT,
};

static struct clk_fixed_def clkin_i2s2 = {
	.clkdef = {
		.id = 0,
		.name = "clkin_i2s2",
		.parent_names = NULL,
		.parent_cnt = 0
	},

	.freq = 0,
};

static struct clk_fixed_def xin12m = {
	.clkdef = {
		.id = 0,
		.name = "xin12m",
		.parent_names = NULL,
		.parent_cnt = 0
	},

	.freq = 12000000,
};

static const char *mac2io_src_parents[] = { "cpll", "gpll" };

static struct rk_clk_composite_def mac2io_src = {
	.clkdef = {
		.id = SCLK_MAC2IO_SRC,
		.name = "clk_mac2io_src",
		.parent_names = mac2io_src_parents,
		.parent_cnt = nitems(mac2io_src_parents),
	},
	/* CRU_CLKSEL_CON27 */
	.muxdiv_offset = 0x16c,

	.mux_shift = 7,
	.mux_width = 1,

	.div_shift = 0,
	.div_width = 5,

	/* CRU_CLKGATE_CON3 */
	.gate_offset = 0x20c,
	.gate_shift = 1,

	.flags = RK_CLK_COMPOSITE_HAVE_GATE | RK_CLK_COMPOSITE_HAVE_MUX,
};

static const char *mac2io_out_parents[] = { "cpll", "gpll" };

static struct rk_clk_composite_def mac2io_out = {
	.clkdef = {
		.id = SCLK_MAC2IO_OUT,
		.name = "clk_mac2io_out",
		.parent_names = mac2io_out_parents,
		.parent_cnt = nitems(mac2io_out_parents),
	},
	/* CRU_CLKSEL_CON27 */
	.muxdiv_offset = 0x16c,

	.mux_shift = 15,
	.mux_width = 1,

	.div_shift = 8,
	.div_width = 5,

	/* CRU_CLKGATE_CON3 */
	.gate_offset = 0x20c,
	.gate_shift = 5,

	.flags = RK_CLK_COMPOSITE_HAVE_GATE | RK_CLK_COMPOSITE_HAVE_MUX,
};

static const char *mac2io_parents[] = { "clk_mac2io_src", "gmac_clkin" };

static struct rk_clk_composite_def mac2io = {
	.clkdef = {
		.id = SCLK_MAC2IO,
		.name = "clk_mac2io",
		.parent_names = mac2io_parents,
		.parent_cnt = nitems(mac2io_parents),
	},
	.muxdiv_offset = RK3328_GRF_MAC_CON1,

	.mux_shift = 10,
	.mux_width = 1,

	.flags = RK_CLK_COMPOSITE_HAVE_MUX | RK_CLK_COMPOSITE_GRF
};

static const char *mac2io_ext_parents[] = { "clk_mac2io", "gmac_clkin" };

static struct rk_clk_composite_def mac2io_ext = {
	.clkdef = {
		.id = SCLK_MAC2IO_EXT,
		.name = "clk_mac2io_ext",
		.parent_names = mac2io_ext_parents,
		.parent_cnt = nitems(mac2io_ext_parents),
	},
	.muxdiv_offset = RK3328_GRF_SOC_CON4,

	.mux_shift = 14,
	.mux_width = 1,

	.flags = RK_CLK_COMPOSITE_HAVE_MUX | RK_CLK_COMPOSITE_GRF
};

static const char *mac2phy_src_parents[] = { "cpll", "gpll" };

static struct rk_clk_composite_def mac2phy_src = {
	.clkdef = {
		.id = SCLK_MAC2PHY_SRC,
		.name = "clk_mac2phy_src",
		.parent_names = mac2phy_src_parents,
		.parent_cnt = nitems(mac2phy_src_parents),
	},
	/* CRU_CLKSEL_CON26 */
	.muxdiv_offset = 0x168,

	.mux_shift = 7,
	.mux_width = 1,

	.div_shift = 0,
	.div_width = 5,

	/* CRU_CLKGATE_CON3 */
	.gate_offset = 0x20c,
	.gate_shift = 0,

	.flags = RK_CLK_COMPOSITE_HAVE_GATE | RK_CLK_COMPOSITE_HAVE_MUX,
};

static const char *mac2phy_parents[] = { "clk_mac2phy_src", "phy_50m_out" };

static struct rk_clk_composite_def mac2phy = {
	.clkdef = {
		.id = SCLK_MAC2PHY,
		.name = "clk_mac2phy",
		.parent_names = mac2phy_parents,
		.parent_cnt = nitems(mac2phy_parents),
	},
	.muxdiv_offset = RK3328_GRF_MAC_CON2,

	.mux_shift = 10,
	.mux_width = 1,

	.flags = RK_CLK_COMPOSITE_HAVE_MUX | RK_CLK_COMPOSITE_GRF
};

static const char *mac2phy_out_parents[] = { "clk_mac2phy" };

static struct rk_clk_composite_def mac2phy_out = {
	.clkdef = {
		.id = SCLK_MAC2PHY_OUT,
		.name = "clk_mac2phy_out",
		.parent_names = mac2phy_out_parents,
		.parent_cnt = nitems(mac2phy_out_parents),
	},
	/* CRU_CLKSEL_CON26 */
	.muxdiv_offset = 0x168,

	.div_shift = 8,
	.div_width = 2,

	/* CRU_CLKGATE_CON9 */
	.gate_offset = 0x224,
	.gate_shift = 2,

	.flags = RK_CLK_COMPOSITE_HAVE_GATE
};

static struct clk_fixed_def phy_50m_out = {
	.clkdef.name = "phy_50m_out",
	.freq = 50000000,
};

static struct clk_link_def gmac_clkin = {
	.clkdef.name = "gmac_clkin",
};

static const char *aclk_gmac_parents[] = { "cpll", "gpll" };

static struct rk_clk_composite_def aclk_gmac = {
	.clkdef = {
		.id = ACLK_GMAC,
		.name = "aclk_gmac",
		.parent_names = aclk_gmac_parents,
		.parent_cnt = nitems(aclk_gmac_parents),
	},
	/* CRU_CLKSEL_CON35 */
	.muxdiv_offset = 0x18c,

	.mux_shift = 6,
	.mux_width = 2,

	.div_shift = 0,
	.div_width = 5,

	/* CRU_CLKGATE_CON3 */
	.gate_offset = 0x20c,
	.gate_shift = 2,

	.flags = RK_CLK_COMPOSITE_HAVE_GATE | RK_CLK_COMPOSITE_HAVE_MUX,
};

static const char *pclk_gmac_parents[] = { "aclk_gmac" };

static struct rk_clk_composite_def pclk_gmac = {
	.clkdef = {
		.id = PCLK_GMAC,
		.name = "pclk_gmac",
		.parent_names = pclk_gmac_parents,
		.parent_cnt = nitems(pclk_gmac_parents),
	},
	/* CRU_CLKSEL_CON25 */
	.muxdiv_offset = 0x164,

	.div_shift = 8,
	.div_width = 3,

	/* CRU_CLKGATE_CON9 */
	.gate_offset = 0x224,
	.gate_shift = 0,

	.flags = RK_CLK_COMPOSITE_HAVE_GATE
};

static struct rk_clk rk3328_clks[] = {
	{
		.type = RK3328_CLK_PLL,
		.clk.pll = &apll
	},
	{
		.type = RK3328_CLK_PLL,
		.clk.pll = &dpll
	},
	{
		.type = RK3328_CLK_PLL,
		.clk.pll = &cpll
	},
	{
		.type = RK3328_CLK_PLL,
		.clk.pll = &gpll
	},
	{
		.type = RK3328_CLK_PLL,
		.clk.pll = &npll
	},

	{
		.type = RK_CLK_COMPOSITE,
		.clk.composite = &aclk_bus_pre
	},
	{
		.type = RK_CLK_COMPOSITE,
		.clk.composite = &hclk_bus_pre
	},
	{
		.type = RK_CLK_COMPOSITE,
		.clk.composite = &pclk_bus_pre
	},

	{
		.type = RK_CLK_ARMCLK,
		.clk.armclk = &armclk,
	},

	{
		.type = RK_CLK_COMPOSITE,
		.clk.composite = &clk_tsadc,
	},
	{
		.type = RK_CLK_COMPOSITE,
		.clk.composite = &aclk_peri_pre,
	},
	{
		.type = RK_CLK_COMPOSITE,
		.clk.composite = &pclk_peri,
	},
	{
		.type = RK_CLK_COMPOSITE,
		.clk.composite = &hclk_peri,
	},
	{
		.type = RK_CLK_COMPOSITE,
		.clk.composite = &sdmmc
	},
	{
		.type = RK_CLK_COMPOSITE,
		.clk.composite = &sdio
	},
	{
		.type = RK_CLK_COMPOSITE,
		.clk.composite = &emmc
	},

	{
		.type = RK_CLK_COMPOSITE,
		.clk.composite = &i2c0
	},
	{
		.type = RK_CLK_COMPOSITE,
		.clk.composite = &i2c1
	},
	{
		.type = RK_CLK_COMPOSITE,
		.clk.composite = &i2c2
	},
	{
		.type = RK_CLK_COMPOSITE,
		.clk.composite = &i2c3
	},

	{
		.type = RK_CLK_COMPOSITE,
		.clk.composite = &ref_usb3otg
	},
	{
		.type = RK_CLK_COMPOSITE,
		.clk.composite = &ref_usb3otg_src
	},
	{
		.type = RK_CLK_COMPOSITE,
		.clk.composite = &usb3otg_suspend
	},
	{
		.type = RK_CLK_COMPOSITE,
		.clk.composite = &i2s0_div
	},
	{
		.type = RK_CLK_FRACT,
		.clk.fract = &i2s0_frac
	},
	{
		.type = RK_CLK_MUX,
		.clk.mux = &i2s0_mux
	},
	{
		.type = RK_CLK_COMPOSITE,
		.clk.composite = &i2s1_div
	},
	{
		.type = RK_CLK_FRACT,
		.clk.fract = &i2s1_frac
	},
	{
		.type = RK_CLK_MUX,
		.clk.mux = &i2s1_mux
	},
	{
		.type = RK_CLK_FIXED,
		.clk.fixed = &clkin_i2s1
	},
	{
		.type = RK_CLK_COMPOSITE,
		.clk.composite = &i2s2_div
	},
	{
		.type = RK_CLK_FRACT,
		.clk.fract = &i2s2_frac
	},
	{
		.type = RK_CLK_MUX,
		.clk.mux = &i2s2_mux
	},
	{
		.type = RK_CLK_FIXED,
		.clk.fixed = &clkin_i2s2
	},
	{
		.type = RK_CLK_FIXED,
		.clk.fixed = &xin12m
	},
	{
		.type = RK_CLK_COMPOSITE,
		.clk.composite = &mac2io_src
	},
	{
		.type = RK_CLK_COMPOSITE,
		.clk.composite = &mac2io
	},
	{
		.type = RK_CLK_COMPOSITE,
		.clk.composite = &mac2io_out
	},
	{
		.type = RK_CLK_COMPOSITE,
		.clk.composite = &mac2io_ext
	},
	{
		.type = RK_CLK_COMPOSITE,
		.clk.composite = &mac2phy_src
	},
	{
		.type = RK_CLK_COMPOSITE,
		.clk.composite = &mac2phy
	},
	{
		.type = RK_CLK_COMPOSITE,
		.clk.composite = &mac2phy_out
	},
	{
		.type = RK_CLK_FIXED,
		.clk.fixed = &phy_50m_out
	},
	{
		.type = RK_CLK_LINK,
		.clk.link = &gmac_clkin
	},
	{
		.type = RK_CLK_COMPOSITE,
		.clk.composite = &aclk_gmac
	},
	{
		.type = RK_CLK_COMPOSITE,
		.clk.composite = &pclk_gmac
	},
};

static int
rk3328_cru_probe(device_t dev)
{

	if (!ofw_bus_status_okay(dev))
		return (ENXIO);

	if (ofw_bus_is_compatible(dev, "rockchip,rk3328-cru")) {
		device_set_desc(dev, "Rockchip RK3328 Clock and Reset Unit");
		return (BUS_PROBE_DEFAULT);
	}

	return (ENXIO);
}

static int
rk3328_cru_attach(device_t dev)
{
	struct rk_cru_softc *sc;

	sc = device_get_softc(dev);
	sc->dev = dev;

	sc->gates = rk3328_gates;
	sc->ngates = nitems(rk3328_gates);

	sc->clks = rk3328_clks;
	sc->nclks = nitems(rk3328_clks);

	sc->reset_offset = 0x300;
	sc->reset_num = 184;

	return (rk_cru_attach(dev));
}

static device_method_t rk3328_cru_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe,		rk3328_cru_probe),
	DEVMETHOD(device_attach,	rk3328_cru_attach),

	DEVMETHOD_END
};

static devclass_t rk3328_cru_devclass;

DEFINE_CLASS_1(rk3328_cru, rk3328_cru_driver, rk3328_cru_methods,
  sizeof(struct rk_cru_softc), rk_cru_driver);

EARLY_DRIVER_MODULE(rk3328_cru, simplebus, rk3328_cru_driver,
    rk3328_cru_devclass, 0, 0, BUS_PASS_BUS + BUS_PASS_ORDER_MIDDLE);
