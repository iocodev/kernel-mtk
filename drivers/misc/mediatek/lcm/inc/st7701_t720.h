/* SPDX-License-Identifier: GPL-2.0 */
/* Shared T720 panel behaviour recovered from the shipping kernel. */
#ifndef __ST7701_T720_H__
#define __ST7701_T720_H__

#include <linux/kernel.h>
#include <linux/string.h>

#include "lcm_drv.h"

#define ST7701_T720_DELAY	0xfe
#define ST7701_T720_END	0xfd

struct st7701_t720_cmd {
	unsigned int cmd;
	unsigned char count;
	unsigned char data[64];
};

static struct LCM_UTIL_FUNCS lcm_util;

static struct st7701_t720_cmd lcm_suspend_setting[] = {
	{0x28, 1, {0x00} },
	{ST7701_T720_DELAY, 120, {0} },
	{0x10, 1, {0x00} },
	{ST7701_T720_DELAY, 200, {0} },
	{0xff, 6, {0x77, 0x01, 0x00, 0x00, 0x11, 0x80} },
	{0xff, 6, {0x77, 0x01, 0x00, 0x00, 0x91, 0x00} },
	{ST7701_T720_END, 0, {0} },
};

static struct st7701_t720_cmd lcm_read_id_setting[] = {
	{0x01, 1, {0x00} },
	{ST7701_T720_DELAY, 200, {0} },
	{0xff, 5, {0x77, 0x01, 0x00, 0x00, 0x11} },
	{0xd1, 1, {0x11} },
	{ST7701_T720_END, 0, {0} },
};

static void lcm_set_util_funcs(const struct LCM_UTIL_FUNCS *util)
{
	memcpy(&lcm_util, util, sizeof(lcm_util));
}

static void lcm_push_table(struct st7701_t720_cmd *table,
			   unsigned int count)
{
	unsigned int i;

	for (i = 0; i < count; i++) {
		switch (table[i].cmd) {
		case ST7701_T720_DELAY:
			lcm_util.mdelay(table[i].count);
			break;
		case ST7701_T720_END:
			break;
		default:
			lcm_util.dsi_set_cmdq_V2(table[i].cmd, table[i].count,
					       table[i].data, 1);
			break;
		}
	}
}

static void lcm_get_params(struct LCM_PARAMS *params)
{
	memset(params, 0, sizeof(*params));
	params->type = LCM_TYPE_DSI;
	params->width = 480;
	params->height = 800;
	params->dbi.te_mode = LCM_DBI_TE_MODE_VSYNC_ONLY;
	params->dbi.te_edge_polarity = LCM_POLARITY_RISING;
	params->dsi.mode = SYNC_PULSE_VDO_MODE;
	params->dsi.LANE_NUM = LCM_TWO_LANE;
	params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
	params->dsi.data_format.trans_seq = LCM_DSI_TRANS_SEQ_MSB_FIRST;
	params->dsi.data_format.padding = LCM_DSI_PADDING_ON_LSB;
	params->dsi.data_format.format = LCM_DSI_FORMAT_RGB888;
	params->dsi.intermediat_buffer_num = 2;
	params->dsi.PS = LCM_PACKED_PS_24BIT_RGB888;
	params->dsi.packet_size = 256;
	params->dsi.vertical_sync_active = 4;
	params->dsi.vertical_backporch = 20;
	params->dsi.vertical_frontporch = 20;
	params->dsi.vertical_active_line = 800;
	params->dsi.horizontal_sync_active = 10;
	params->dsi.horizontal_backporch = 80;
	params->dsi.horizontal_frontporch = 80;
	params->dsi.horizontal_active_pixel = 480;
	params->dsi.PLL_CLOCK = 200;
}

static void lcm_reset(void)
{
	lcm_util.set_reset_pin(1);
	lcm_util.set_reset_pin(0);
	lcm_util.mdelay(5);
	lcm_util.set_reset_pin(1);
	lcm_util.mdelay(120);
}

static void lcm_suspend(void)
{
	lcm_push_table(lcm_suspend_setting, ARRAY_SIZE(lcm_suspend_setting));
}

/* The shipping kernel leaves panel power under the existing board control. */
static void lcm_init_power(void)
{
}

static void lcm_suspend_power(void)
{
}

static void lcm_resume_power(void)
{
	lcm_util.set_reset_pin(0);
}

static unsigned int lcm_compare_id(void)
{
	unsigned int data = 0x00023700;
	unsigned char id[2] = {0};

	lcm_util.set_reset_pin(1);
	lcm_util.mdelay(10);
	lcm_util.set_reset_pin(0);
	lcm_util.mdelay(20);
	lcm_util.set_reset_pin(1);
	lcm_util.mdelay(120);
	lcm_push_table(lcm_read_id_setting, ARRAY_SIZE(lcm_read_id_setting));
	lcm_util.dsi_set_cmdq(&data, 1, 1);
	lcm_util.dsi_dcs_read_lcm_reg_v2(0xa1, id, sizeof(id));
	pr_info("st7701_t720: panel ID = 0x%04x\n", (id[0] << 8) | id[1]);

	/* Preserve kernel behaviour: LK selects the variant by name. */
	return 0;
}

#endif /* __ST7701_T720_H__ */
