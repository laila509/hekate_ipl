/*
 * Copyright (c) 2018 naehrwert
 * Copyright (c) 2018-2022 CTCaer
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <string.h>

#include <mem/heap.h>
#include <soc/timer.h>
#include <storage/emmc.h>
#include <storage/sdmmc.h>
#include <storage/mmc.h>
#include <storage/sd.h>
#include <storage/sd_def.h>
#include <memory_map.h>
#include <gfx_utils.h>

//#define DPRINTF(...) gfx_printf(__VA_ARGS__)
#define DPRINTF(...)

u32 sd_power_cycle_time_start;

static inline u32 unstuff_bits(u32 *resp, u32 start, u32 size)
{
	const u32 mask = (size < 32 ? 1 << size : 0) - 1;
	const u32 off = 3 - ((start) / 32);
	const u32 shft = (start) & 31;
	u32 res = resp[off] >> shft;
	if (size + shft > 32)
		res |= resp[off - 1] << ((32 - shft) % 32);
	return res & mask;
}

/*
 * Common functions for SD and MMC.
 */

static int _sdmmc_storage_check_card_status(u32 res)
{
	//Error mask:
	//TODO: R1_SWITCH_ERROR can be skipped for certain card types.
	if (res &
		(R1_OUT_OF_RANGE       | R1_ADDRESS_ERROR | R1_BLOCK_LEN_ERROR |
		 R1_ERASE_SEQ_ERROR    | R1_ERASE_PARAM   | R1_WP_VIOLATION    |
		 R1_LOCK_UNLOCK_FAILED | R1_COM_CRC_ERROR | R1_ILLEGAL_COMMAND |
		 R1_CARD_ECC_FAILED    | R1_CC_ERROR      | R1_ERROR           |
		 R1_CID_CSD_OVERWRITE  | R1_WP_ERASE_SKIP | R1_ERASE_RESET     |
		 R1_SWITCH_ERROR))
		return 0;

	// No errors.
	return 1;
}

static int _sdmmc_storage_execute_cmd_type1_ex(sdmmc_storage_t *storage, u32 *resp, u32 cmd, u32 arg, u32 check_busy, u32 expected_state, u32 mask)
{
	sdmmc_cmd_t cmdbuf;
	sdmmc_init_cmd(&cmdbuf, cmd, arg, SDMMC_RSP_TYPE_1, check_busy);
	if (!sdmmc_execute_cmd(storage->sdmmc, &cmdbuf, NULL, NULL))
		return 0;

	sdmmc_get_rsp(storage->sdmmc, resp, 4, SDMMC_RSP_TYPE_1);
	if (mask)
		*resp &= ~mask;

	if (_sdmmc_storage_check_card_status(*resp))
		if (expected_state == R1_SKIP_STATE_CHECK || R1_CURRENT_STATE(*resp) == expected_state)
			return 1;

	return 0;
}

static int _sdmmc_storage_execute_cmd_type1(sdmmc_storage_t *storage, u32 cmd, u32 arg, u32 check_busy, u32 expected_state)
{
	u32 tmp;
	return _sdmmc_storage_execute_cmd_type1_ex(storage, &tmp, cmd, arg, check_busy, expected_state, 0);
}

static int _sdmmc_storage_go_idle_state(sdmmc_storage_t *storage)
{
	sdmmc_cmd_t cmdbuf;
	sdmmc_init_cmd(&cmdbuf, MMC_GO_IDLE_STATE, 0, SDMMC_RSP_TYPE_0, 0);

	return sdmmc_execute_cmd(storage->sdmmc, &cmdbuf, NULL, NULL);
}

static int _sdmmc_storage_get_cid(sdmmc_storage_t *storage)
{
	sdmmc_cmd_t cmdbuf;
	sdmmc_init_cmd(&cmdbuf, MMC_ALL_SEND_CID, 0, SDMMC_RSP_TYPE_2, 0);
	if (!sdmmc_execute_cmd(storage->sdmmc, &cmdbuf, NULL, NULL))
		return 0;

	sdmmc_get_rsp(storage->sdmmc, (u32 *)storage->raw_cid, 16, SDMMC_RSP_TYPE_2);

	return 1;
}

static int _sdmmc_storage_select_card(sdmmc_storage_t *storage)
{
	return _sdmmc_storage_execute_cmd_type1(storage, MMC_SELECT_CARD, storage->rca << 16, 1, R1_SKIP_STATE_CHECK);
}

static int _sdmmc_storage_get_csd(sdmmc_storage_t *storage)
{
	sdmmc_cmd_t cmdbuf;
	sdmmc_init_cmd(&cmdbuf, MMC_SEND_CSD, storage->rca << 16, SDMMC_RSP_TYPE_2, 0);
	if (!sdmmc_execute_cmd(storage->sdmmc, &cmdbuf, NULL, NULL))
		return 0;

	sdmmc_get_rsp(storage->sdmmc, (u32 *)storage->raw_csd, 16, SDMMC_RSP_TYPE_2);

	return 1;
}

static int _sdmmc_storage_set_blocklen(sdmmc_storage_t *storage, u32 blocklen)
{
	return _sdmmc_storage_execute_cmd_type1(storage, MMC_SET_BLOCKLEN, blocklen, 0, R1_STATE_TRAN);
}

static int _sdmmc_storage_get_status(sdmmc_storage_t *storage, u32 *resp, u32 mask)
{
	return _sdmmc_storage_execute_cmd_type1_ex(storage, resp, MMC_SEND_STATUS, storage->rca << 16, 0, R1_STATE_TRAN, mask);
}

static int _sdmmc_storage_check_status(sdmmc_storage_t *storage)
{
	u32 tmp;
	return _sdmmc_storage_get_status(storage, &tmp, 0);
}

int sdmmc_storage_execute_vendor_cmd(sdmmc_storage_t *storage, u32 arg)
{
	sdmmc_cmd_t cmdbuf;
	sdmmc_init_cmd(&cmdbuf, MMC_VENDOR_62_CMD, arg, SDMMC_RSP_TYPE_1, 1);
	if (!sdmmc_execute_cmd(storage->sdmmc, &cmdbuf, 0, 0))
		return 0;

	u32 resp;
	sdmmc_get_rsp(storage->sdmmc, &resp, 4, SDMMC_RSP_TYPE_1);

	resp = -1;
	u32 timeout = get_tmr_ms() + 1500;
	while (true)
	{
		_sdmmc_storage_get_status(storage, &resp, 0);

		if (resp == (R1_READY_FOR_DATA | R1_STATE(R1_STATE_TRAN)))
			break;

		if (get_tmr_ms() > timeout)
			break;
		msleep(10);
	}

	return _sdmmc_storage_check_card_status(resp);
}

int sdmmc_storage_vendor_sandisk_report(sdmmc_storage_t *storage, void *buf)
{
	// Request health report.
	if (!sdmmc_storage_execute_vendor_cmd(storage, MMC_SANDISK_HEALTH_REPORT))
		return 2;

	u32 tmp = 0;
	sdmmc_cmd_t cmdbuf;
	sdmmc_req_t reqbuf;

	sdmmc_init_cmd(&cmdbuf, MMC_VENDOR_63_CMD, 0, SDMMC_RSP_TYPE_1, 0); // similar to CMD17 with arg 0x0.

	reqbuf.buf              = buf;
	reqbuf.num_sectors      = 1;
	reqbuf.blksize          = 512;
	reqbuf.is_write         = 0;
	reqbuf.is_multi_block   = 0;
	reqbuf.is_auto_stop_trn = 0;

	u32 blkcnt_out;
	if (!sdmmc_execute_cmd(storage->sdmmc, &cmdbuf, &reqbuf, &blkcnt_out))
	{
		sdmmc_stop_transmission(storage->sdmmc, &tmp);
		_sdmmc_storage_get_status(storage, &tmp, 0);

		return 0;
	}

	return 1;
}

static int _sdmmc_storage_readwrite_ex(sdmmc_storage_t *storage, u32 *blkcnt_out, u32 sector, u32 num_sectors, void *buf, u32 is_write)
{
	u32 tmp = 0;
	sdmmc_cmd_t cmdbuf;
	sdmmc_req_t reqbuf;

	// If SDSC convert block address to byte address.
	if (!storage->has_sector_access)
		sector <<= 9;

	sdmmc_init_cmd(&cmdbuf, is_write ? MMC_WRITE_MULTIPLE_BLOCK : MMC_READ_MULTIPLE_BLOCK, sector, SDMMC_RSP_TYPE_1, 0);

	reqbuf.buf              = buf;
	reqbuf.num_sectors      = num_sectors;
	reqbuf.blksize          = 512;
	reqbuf.is_write         = is_write;
	reqbuf.is_multi_block   = 1;
	reqbuf.is_auto_stop_trn = 1;

	if (!sdmmc_execute_cmd(storage->sdmmc, &cmdbuf, &reqbuf, blkcnt_out))
	{
		sdmmc_stop_transmission(storage->sdmmc, &tmp);
		_sdmmc_storage_get_status(storage, &tmp, 0);

		return 0;
	}

	return 1;
}

int sdmmc_storage_end(sdmmc_storage_t *storage)
{
	if (!_sdmmc_storage_go_idle_state(storage))
		return 0;

	sdmmc_end(storage->sdmmc);

	storage->initialized = 0;

	return 1;
}

static int _sdmmc_storage_readwrite(sdmmc_storage_t *storage, u32 sector, u32 num_sectors, void *buf, u32 is_write)
{
	u8 *bbuf = (u8 *)buf;
	u32 sct_off = sector;
	u32 sct_total = num_sectors;
	bool first_reinit = true;

	// Exit if not initialized.
	if (!storage->initialized)
		return 0;

	while (sct_total)
	{
		u32 blkcnt = 0;
		// Retry 5 times if failed.
		u32 retries = 5;
		do
		{
reinit_try:
			if (_sdmmc_storage_readwrite_ex(storage, &blkcnt, sct_off, MIN(sct_total, 0xFFFF), bbuf, is_write))
				goto out;
			else
				retries--;

			sd_error_count_increment(SD_ERROR_RW_RETRY);

			msleep(50);
		} while (retries);

		// Disk IO failure! Reinit SD/EMMC to a lower speed.
		if (storage->sdmmc->id == SDMMC_1 || storage->sdmmc->id == SDMMC_4)
		{
			int res;

			if (storage->sdmmc->id == SDMMC_1)
			{
				sd_error_count_increment(SD_ERROR_RW_FAIL);

				if (first_reinit)
					res = sd_initialize(true);
				else
				{
					res = sd_init_retry(true);
					if (!res)
						sd_error_count_increment(SD_ERROR_INIT_FAIL);
				}
			}
			else if (storage->sdmmc->id == SDMMC_4)
			{
				emmc_error_count_increment(EMMC_ERROR_RW_FAIL);

				if (first_reinit)
					res = emmc_initialize(true);
				else
				{
					res = emmc_init_retry(true);
					if (!res)
						emmc_error_count_increment(EMMC_ERROR_INIT_FAIL);
				}
			}

			// Reset values for a retry.
			blkcnt = 0;
			retries = 3;
			first_reinit = false;

			// If successful reinit, restart xfer.
			if (res)
			{
				bbuf = (u8 *)buf;
				sct_off = sector;
				sct_total = num_sectors;

				goto reinit_try;
			}
		}

		// Failed.
		return 0;

out:
		sct_off += blkcnt;
		sct_total -= blkcnt;
		bbuf += 512 * blkcnt;
	}

	return 1;
}

int sdmmc_storage_read(sdmmc_storage_t *storage, u32 sector, u32 num_sectors, void *buf)
{
	// Ensure that SDMMC has access to buffer and it's SDMMC DMA aligned.
	if (mc_client_has_access(buf) && !((u32)buf % 8))
		return _sdmmc_storage_readwrite(storage, sector, num_sectors, buf, 0);

	if (num_sectors > (SDMMC_UP_BUF_SZ / 512))
		return 0;

	u8 *tmp_buf = (u8 *)SDMMC_UPPER_BUFFER;
	if (_sdmmc_storage_readwrite(storage, sector, num_sectors, tmp_buf, 0))
	{
		memcpy(buf, tmp_buf, 512 * num_sectors);
		return 1;
	}
	return 0;
}

int sdmmc_storage_write(sdmmc_storage_t *storage, u32 sector, u32 num_sectors, void *buf)
{
	// Ensure that SDMMC has access to buffer and it's SDMMC DMA aligned.
	if (mc_client_has_access(buf) && !((u32)buf % 8))
		return _sdmmc_storage_readwrite(storage, sector, num_sectors, buf, 1);

	if (num_sectors > (SDMMC_UP_BUF_SZ / 512))
		return 0;

	u8 *tmp_buf = (u8 *)SDMMC_UPPER_BUFFER;
	memcpy(tmp_buf, buf, 512 * num_sectors);
	return _sdmmc_storage_readwrite(storage, sector, num_sectors, tmp_buf, 1);
}

/*
* MMC specific functions.
*/

static int _mmc_storage_get_op_cond_inner(sdmmc_storage_t *storage, u32 *pout, u32 power)
{
	sdmmc_cmd_t cmdbuf;

	u32 arg = 0;
	switch (power)
	{
	case SDMMC_POWER_1_8:
		arg = MMC_CARD_CCS | MMC_CARD_VDD_18;
		break;

	case SDMMC_POWER_3_3:
		arg = MMC_CARD_CCS | MMC_CARD_VDD_27_34;
		break;

	default:
		return 0;
	}

	sdmmc_init_cmd(&cmdbuf, MMC_SEND_OP_COND, arg, SDMMC_RSP_TYPE_3, 0);
	if (!sdmmc_execute_cmd(storage->sdmmc, &cmdbuf, NULL, NULL))
		return 0;

	return sdmmc_get_rsp(storage->sdmmc, pout, 4, SDMMC_RSP_TYPE_3);
}

static int _mmc_storage_get_op_cond(sdmmc_storage_t *storage, u32 power)
{
	u32 timeout = get_tmr_ms() + 1500;

	while (true)
	{
		u32 cond = 0;
		if (!_mmc_storage_get_op_cond_inner(storage, &cond, power))
			break;

		// Check if power up is done.
		if (cond & MMC_CARD_BUSY)
		{
			// Check if card is high capacity.
			if (cond & MMC_CARD_CCS)
				storage->has_sector_access = 1;

			return 1;
		}
		if (get_tmr_ms() > timeout)
			break;

		usleep(1000);
	}

	return 0;
}

static int _mmc_storage_set_relative_addr(sdmmc_storage_t *storage)
{
	return _sdmmc_storage_execute_cmd_type1(storage, MMC_SET_RELATIVE_ADDR, storage->rca << 16, 0, R1_SKIP_STATE_CHECK);
}

static void _mmc_storage_parse_cid(sdmmc_storage_t *storage)
{
	u32 *raw_cid = (u32 *)&(storage->raw_cid);

	switch (storage->csd.mmca_vsn)
	{
	case 0: /* MMC v1.0 - v1.2 */
	case 1: /* MMC v1.4 */
		storage->cid.prod_name[6] = unstuff_bits(raw_cid, 48, 8);
		storage->cid.manfid       = unstuff_bits(raw_cid, 104, 24);
		storage->cid.hwrev        = unstuff_bits(raw_cid, 44, 4);
		storage->cid.fwrev        = unstuff_bits(raw_cid, 40, 4);
		storage->cid.serial       = unstuff_bits(raw_cid, 16, 24);
		break;

	case 2: /* MMC v2.0 - v2.2 */
	case 3: /* MMC v3.1 - v3.3 */
	case 4: /* MMC v4 */
		storage->cid.manfid = unstuff_bits(raw_cid, 120, 8);
		storage->cid.oemid  = unstuff_bits(raw_cid, 104, 8);
		storage->cid.prv    = unstuff_bits(raw_cid, 48, 8);
		storage->cid.serial = unstuff_bits(raw_cid, 16, 32);
		break;

	default:
		break;
	}

	storage->cid.prod_name[0] = unstuff_bits(raw_cid, 96, 8);
	storage->cid.prod_name[1] = unstuff_bits(raw_cid, 88, 8);
	storage->cid.prod_name[2] = unstuff_bits(raw_cid, 80, 8);
	storage->cid.prod_name[3] = unstuff_bits(raw_cid, 72, 8);
	storage->cid.prod_name[4] = unstuff_bits(raw_cid, 64, 8);
	storage->cid.prod_name[5] = unstuff_bits(raw_cid, 56, 8);

	storage->cid.month = unstuff_bits(raw_cid, 12, 4);
	storage->cid.year  = unstuff_bits(raw_cid, 8, 4) + 1997;
	if (storage->ext_csd.rev >= 5)
	{
		if (storage->cid.year < 2010)
			storage->cid.year += 16;
	}
}

static void _mmc_storage_parse_csd(sdmmc_storage_t *storage)
{
	u32 *raw_csd = (u32 *)storage->raw_csd;

	storage->csd.mmca_vsn = unstuff_bits(raw_csd, 122, 4);
	storage->csd.structure = unstuff_bits(raw_csd, 126, 2);
	storage->csd.cmdclass = unstuff_bits(raw_csd, 84, 12);
	storage->csd.read_blkbits = unstuff_bits(raw_csd, 80, 4);
	storage->csd.capacity = (1 + unstuff_bits(raw_csd, 62, 12)) << (unstuff_bits(raw_csd, 47, 3) + 2);
	storage->sec_cnt = storage->csd.capacity;
}

static void _mmc_storage_parse_ext_csd(sdmmc_storage_t *storage, u8 *buf)
{
	storage->ext_csd.rev          = buf[EXT_CSD_REV];
	storage->ext_csd.ext_struct   = buf[EXT_CSD_STRUCTURE];
	storage->ext_csd.card_type    = buf[EXT_CSD_CARD_TYPE];
	storage->ext_csd.dev_version  = *(u16 *)&buf[EXT_CSD_DEVICE_VERSION];
	storage->ext_csd.boot_mult    = buf[EXT_CSD_BOOT_MULT];
	storage->ext_csd.rpmb_mult    = buf[EXT_CSD_RPMB_MULT];
	//storage->ext_csd.bkops        = buf[EXT_CSD_BKOPS_SUPPORT];
	//storage->ext_csd.bkops_en     = buf[EXT_CSD_BKOPS_EN];
	//storage->ext_csd.bkops_status = buf[EXT_CSD_BKOPS_STATUS];

	storage->ext_csd.pre_eol_info   = buf[EXT_CSD_PRE_EOL_INFO];
	storage->ext_csd.dev_life_est_a = buf[EXT_CSD_DEVICE_LIFE_TIME_EST_TYP_A];
	storage->ext_csd.dev_life_est_b = buf[EXT_CSD_DEVICE_LIFE_TIME_EST_TYP_B];

	storage->ext_csd.cache_size   =  buf[EXT_CSD_CACHE_SIZE]            |
									(buf[EXT_CSD_CACHE_SIZE + 1] << 8)  |
									(buf[EXT_CSD_CACHE_SIZE + 2] << 16) |
									(buf[EXT_CSD_CACHE_SIZE + 3] << 24);

	storage->ext_csd.max_enh_mult = (buf[EXT_CSD_MAX_ENH_SIZE_MULT]             |
									(buf[EXT_CSD_MAX_ENH_SIZE_MULT + 1] << 8)   |
									(buf[EXT_CSD_MAX_ENH_SIZE_MULT + 2] << 16)) *
									 buf[EXT_CSD_HC_WP_GRP_SIZE] * buf[EXT_CSD_HC_ERASE_GRP_SIZE];

	storage->sec_cnt = *(u32 *)&buf[EXT_CSD_SEC_CNT];
}

int mmc_storage_get_ext_csd(sdmmc_storage_t *storage, void *buf)
{
	sdmmc_cmd_t cmdbuf;
	sdmmc_init_cmd(&cmdbuf, MMC_SEND_EXT_CSD, 0, SDMMC_RSP_TYPE_1, 0);

	sdmmc_req_t reqbuf;
	reqbuf.buf = buf;
	reqbuf.blksize = 512;
	reqbuf.num_sectors = 1;
	reqbuf.is_write = 0;
	reqbuf.is_multi_block = 0;
	reqbuf.is_auto_stop_trn = 0;

	if (!sdmmc_execute_cmd(storage->sdmmc, &cmdbuf, &reqbuf, NULL))
		return 0;

	u32 tmp = 0;
	sdmmc_get_rsp(storage->sdmmc, &tmp, 4, SDMMC_RSP_TYPE_1);
	_mmc_storage_parse_ext_csd(storage, buf);

	return _sdmmc_storage_check_card_status(tmp);
}

static int _mmc_storage_switch(sdmmc_storage_t *storage, u32 arg)
{
	return _sdmmc_storage_execute_cmd_type1(storage, MMC_SWITCH, arg, 1, R1_SKIP_STATE_CHECK);
}

static int _mmc_storage_switch_buswidth(sdmmc_storage_t *storage, u32 bus_width)
{
	if (bus_width == SDMMC_BUS_WIDTH_1)
		return 1;

	u32 arg = 0;
	switch (bus_width)
	{
	case SDMMC_BUS_WIDTH_4:
		arg = SDMMC_SWITCH(MMC_SWITCH_MODE_WRITE_BYTE, EXT_CSD_BUS_WIDTH, EXT_CSD_BUS_WIDTH_4);
		break;

	case SDMMC_BUS_WIDTH_8:
		arg = SDMMC_SWITCH(MMC_SWITCH_MODE_WRITE_BYTE, EXT_CSD_BUS_WIDTH, EXT_CSD_BUS_WIDTH_8);
		break;
	}

	if (_mmc_storage_switch(storage, arg))
		if (_sdmmc_storage_check_status(storage))
		{
			sdmmc_set_bus_width(storage->sdmmc, bus_width);

			return 1;
		}

	return 0;
}

static int _mmc_storage_enable_HS(sdmmc_storage_t *storage, bool check_sts_before_clk_setup)
{
	if (!_mmc_storage_switch(storage, SDMMC_SWITCH(MMC_SWITCH_MODE_WRITE_BYTE, EXT_CSD_HS_TIMING, EXT_CSD_TIMING_HS)))
		return 0;

	if (check_sts_before_clk_setup && !_sdmmc_storage_check_status(storage))
		return 0;

	if (!sdmmc_setup_clock(storage->sdmmc, SDHCI_TIMING_MMC_HS52))
		return 0;

DPRINTF("[MMC] switched to HS52\n");
	storage->csd.busspeed = 52;

	if (check_sts_before_clk_setup || _sdmmc_storage_check_status(storage))
		return 1;

	return 0;
}

static int _mmc_storage_enable_HS200(sdmmc_storage_t *storage)
{
	if (!_mmc_storage_switch(storage, SDMMC_SWITCH(MMC_SWITCH_MODE_WRITE_BYTE, EXT_CSD_HS_TIMING, EXT_CSD_TIMING_HS200)))
		return 0;

	if (!sdmmc_setup_clock(storage->sdmmc, SDHCI_TIMING_MMC_HS200))
		return 0;

	if (!sdmmc_tuning_execute(storage->sdmmc, SDHCI_TIMING_MMC_HS200, MMC_SEND_TUNING_BLOCK_HS200))
		return 0;

DPRINTF("[MMC] switched to HS200\n");
	storage->csd.busspeed = 200;

	return _sdmmc_storage_check_status(storage);
}

static int _mmc_storage_enable_HS400(sdmmc_storage_t *storage)
{
	if (!_mmc_storage_enable_HS200(storage))
		return 0;

	sdmmc_save_tap_value(storage->sdmmc);

	if (!_mmc_storage_enable_HS(storage, false))
		return 0;

	if (!_mmc_storage_switch(storage, SDMMC_SWITCH(MMC_SWITCH_MODE_WRITE_BYTE, EXT_CSD_BUS_WIDTH, EXT_CSD_DDR_BUS_WIDTH_8)))
		return 0;

	if (!_mmc_storage_switch(storage, SDMMC_SWITCH(MMC_SWITCH_MODE_WRITE_BYTE, EXT_CSD_HS_TIMING, EXT_CSD_TIMING_HS400)))
		return 0;

	if (!sdmmc_setup_clock(storage->sdmmc, SDHCI_TIMING_MMC_HS400))
		return 0;

DPRINTF("[MMC] switched to HS400\n");
	storage->csd.busspeed = 400;

	return _sdmmc_storage_check_status(storage);
}

static int _mmc_storage_enable_highspeed(sdmmc_storage_t *storage, u32 card_type, u32 type)
{
	if (sdmmc_get_io_power(storage->sdmmc) != SDMMC_POWER_1_8)
		goto hs52_mode;

	// HS400 needs 8-bit bus width mode.
	if (sdmmc_get_bus_width(storage->sdmmc) == SDMMC_BUS_WIDTH_8 &&
		card_type & EXT_CSD_CARD_TYPE_HS400_1_8V && type == SDHCI_TIMING_MMC_HS400)
		return _mmc_storage_enable_HS400(storage);

	// Try HS200 if HS400 and 4-bit width bus or just HS200.
	if ((sdmmc_get_bus_width(storage->sdmmc) == SDMMC_BUS_WIDTH_8 ||
		 sdmmc_get_bus_width(storage->sdmmc) == SDMMC_BUS_WIDTH_4) &&
		card_type & EXT_CSD_CARD_TYPE_HS200_1_8V &&
		(type == SDHCI_TIMING_MMC_HS400 || type == SDHCI_TIMING_MMC_HS200))
		return _mmc_storage_enable_HS200(storage);

hs52_mode:
	if (card_type & EXT_CSD_CARD_TYPE_HS_52)
		return _mmc_storage_enable_HS(storage, true);

	return 1;
}

/*
static int _mmc_storage_enable_auto_bkops(sdmmc_storage_t *storage)
{
	if (!_mmc_storage_switch(storage, SDMMC_SWITCH(MMC_SWITCH_MODE_SET_BITS, EXT_CSD_BKOPS_EN, EXT_CSD_AUTO_BKOPS_MASK)))
		return 0;

	return _sdmmc_storage_check_status(storage);
}
*/

int sdmmc_storage_init_mmc(sdmmc_storage_t *storage, sdmmc_t *sdmmc, u32 bus_width, u32 type)
{
	memset(storage, 0, sizeof(sdmmc_storage_t));
	storage->sdmmc = sdmmc;
	storage->rca = 2; // Set default device address. This could be a config item.

	if (!sdmmc_init(sdmmc, SDMMC_4, SDMMC_POWER_1_8, SDMMC_BUS_WIDTH_1, SDHCI_TIMING_MMC_ID, SDMMC_POWER_SAVE_DISABLE))
		return 0;
DPRINTF("[MMC] after init\n");

	usleep(1000 + (74000 + sdmmc->divisor - 1) / sdmmc->divisor);

	if (!_sdmmc_storage_go_idle_state(storage))
		return 0;
DPRINTF("[MMC] went to idle state\n");

	if (!_mmc_storage_get_op_cond(storage, SDMMC_POWER_1_8))
		return 0;
DPRINTF("[MMC] got op cond\n");

	if (!_sdmmc_storage_get_cid(storage))
		return 0;
DPRINTF("[MMC] got cid\n");

	if (!_mmc_storage_set_relative_addr(storage))
		return 0;
DPRINTF("[MMC] set relative addr\n");

	if (!_sdmmc_storage_get_csd(storage))
		return 0;
DPRINTF("[MMC] got csd\n");
	_mmc_storage_parse_csd(storage);

	if (!sdmmc_setup_clock(storage->sdmmc, SDHCI_TIMING_MMC_LS26))
		return 0;
DPRINTF("[MMC] after setup clock\n");

	if (!_sdmmc_storage_select_card(storage))
		return 0;
DPRINTF("[MMC] card selected\n");

	if (!_sdmmc_storage_set_blocklen(storage, 512))
		return 0;
DPRINTF("[MMC] set blocklen to 512\n");

	// Check system specification version, only version 4.0 and later support below features.
	if (storage->csd.mmca_vsn < CSD_SPEC_VER_4)
		goto done;

	if (!_mmc_storage_switch_buswidth(storage, bus_width))
		return 0;
DPRINTF("[MMC] switched buswidth\n");

	if (!mmc_storage_get_ext_csd(storage, (u8 *)SDMMC_UPPER_BUFFER))
		return 0;
DPRINTF("[MMC] got ext_csd\n");

	_mmc_storage_parse_cid(storage); // This needs to be after csd and ext_csd.

/*
	if (storage->ext_csd.bkops & 0x1 && !(storage->ext_csd.bkops_en & EXT_CSD_AUTO_BKOPS_MASK))
	{
		_mmc_storage_enable_auto_bkops(storage);
DPRINTF("[MMC] BKOPS enabled\n");
	}
*/

	if (!_mmc_storage_enable_highspeed(storage, storage->ext_csd.card_type, type))
		return 0;
DPRINTF("[MMC] successfully switched to HS mode\n");

	sdmmc_card_clock_powersave(storage->sdmmc, SDMMC_POWER_SAVE_ENABLE);

done:
	storage->initialized = 1;

	return 1;
}

int sdmmc_storage_set_mmc_partition(sdmmc_storage_t *storage, u32 partition)
{
	if (!_mmc_storage_switch(storage, SDMMC_SWITCH(MMC_SWITCH_MODE_WRITE_BYTE, EXT_CSD_PART_CONFIG, partition)))
		return 0;

	if (!_sdmmc_storage_check_status(storage))
		return 0;

	storage->partition = partition;

	return 1;
}

/*
 * SD specific functions.
 */

static int _sd_storage_execute_app_cmd(sdmmc_storage_t *storage, u32 expected_state, u32 mask, sdmmc_cmd_t *cmdbuf, sdmmc_req_t *req, u32 *blkcnt_out)
{
	u32 tmp;
	if (!_sdmmc_storage_execute_cmd_type1_ex(storage, &tmp, MMC_APP_CMD, storage->rca << 16, 0, expected_state, mask))
		return 0;

	return sdmmc_execute_cmd(storage->sdmmc, cmdbuf, req, blkcnt_out);
}

static int _sd_storage_execute_app_cmd_type1(sdmmc_storage_t *storage, u32 *resp, u32 cmd, u32 arg, u32 check_busy, u32 expected_state)
{
	if (!_sdmmc_storage_execute_cmd_type1(storage, MMC_APP_CMD, storage->rca << 16, 0, R1_STATE_TRAN))
		return 0;

	return _sdmmc_storage_execute_cmd_type1_ex(storage, resp, cmd, arg, check_busy, expected_state, 0);
}

static int _sd_storage_send_if_cond(sdmmc_storage_t *storage, bool *is_sdsc)
{
	sdmmc_cmd_t cmdbuf;
	u16 vhd_pattern = SD_VHD_27_36 | 0xAA;
	sdmmc_init_cmd(&cmdbuf, SD_SEND_IF_COND, vhd_pattern, SDMMC_RSP_TYPE_5, 0);
	if (!sdmmc_execute_cmd(storage->sdmmc, &cmdbuf, NULL, NULL))
	{
		*is_sdsc = 1; // The SD Card is version 1.X
		return 1;
	}

	// For Card version >= 2.0, parse results.
	u32 resp = 0;
	sdmmc_get_rsp(storage->sdmmc, &resp, 4, SDMMC_RSP_TYPE_5);

	// Check if VHD was accepted and pattern was properly returned.
	if ((resp & 0xFFF) == vhd_pattern)
		return 1;

	return 0;
}

static int _sd_storage_get_op_cond_once(sdmmc_storage_t *storage, u32 *cond, bool is_sdsc, int bus_uhs_support)
{
	sdmmc_cmd_t cmdbuf;
	// Support for Current > 150mA
	u32 arg = !is_sdsc ? SD_OCR_XPC : 0;
	// Support for handling block-addressed SDHC cards
	arg	|= !is_sdsc ? SD_OCR_CCS : 0;
	// Support for 1.8V
	arg |= (bus_uhs_support && !is_sdsc) ? SD_OCR_S18R : 0;
	// This is needed for most cards. Do not set bit7 even if 1.8V is supported.
	arg |= SD_OCR_VDD_32_33;
	sdmmc_init_cmd(&cmdbuf, SD_APP_OP_COND, arg, SDMMC_RSP_TYPE_3, 0);
	if (!_sd_storage_execute_app_cmd(storage, R1_SKIP_STATE_CHECK, is_sdsc ? R1_ILLEGAL_COMMAND : 0, &cmdbuf, NULL, NULL))
		return 0;

	return sdmmc_get_rsp(storage->sdmmc, cond, 4, SDMMC_RSP_TYPE_3);
}

static int _sd_storage_get_op_cond(sdmmc_storage_t *storage, bool is_sdsc, int bus_uhs_support)
{
	u32 timeout = get_tmr_ms() + 1500;

	while (true)
	{
		u32 cond = 0;
		if (!_sd_storage_get_op_cond_once(storage, &cond, is_sdsc, bus_uhs_support))
			break;

		// Check if power up is done.
		if (cond & SD_OCR_BUSY)
		{
DPRINTF("[SD] op cond: %08X, lv: %d\n", cond, bus_uhs_support);

			// Check if card is high capacity.
			if (cond & SD_OCR_CCS)
				storage->has_sector_access = 1;

			// Check if card supports 1.8V signaling.
			if (cond & SD_ROCR_S18A && bus_uhs_support)
			{
				// Switch to 1.8V signaling.
				if (_sdmmc_storage_execute_cmd_type1(storage, SD_SWITCH_VOLTAGE, 0, 0, R1_STATE_READY))
				{
					if (!sdmmc_enable_low_voltage(storage->sdmmc))
						return 0;
					storage->is_low_voltage = 1;

DPRINTF("-> switched to low voltage\n");
				}
			}
			else
			{
DPRINTF("[SD] no low voltage support\n");
			}

			return 1;
		}
		if (get_tmr_ms() > timeout)
			break;
		msleep(10); // Needs to be at least 10ms for some SD Cards
	}

	return 0;
}

static int _sd_storage_get_rca(sdmmc_storage_t *storage)
{
	sdmmc_cmd_t cmdbuf;
	sdmmc_init_cmd(&cmdbuf, SD_SEND_RELATIVE_ADDR, 0, SDMMC_RSP_TYPE_4, 0);

	u32 timeout = get_tmr_ms() + 1500;

	while (true)
	{
		if (!sdmmc_execute_cmd(storage->sdmmc, &cmdbuf, NULL, NULL))
			break;

		u32 resp = 0;
		if (!sdmmc_get_rsp(storage->sdmmc, &resp, 4, SDMMC_RSP_TYPE_4))
			break;

		if (resp >> 16)
		{
			storage->rca = resp >> 16;
			return 1;
		}

		if (get_tmr_ms() > timeout)
			break;
		usleep(1000);
	}

	return 0;
}

static void _sd_storage_parse_scr(sdmmc_storage_t *storage)
{
	// unstuff_bits can parse only 4 u32
	u32 resp[4];

	resp[3] = *(u32 *)&storage->raw_scr[4];
	resp[2] = *(u32 *)&storage->raw_scr[0];

	storage->scr.sda_vsn = unstuff_bits(resp, 56, 4);
	storage->scr.bus_widths = unstuff_bits(resp, 48, 4);

	/* If v2.0 is supported, check if Physical Layer Spec v3.0 is supported */
	if (storage->scr.sda_vsn == SCR_SPEC_VER_2)
		storage->scr.sda_spec3 = unstuff_bits(resp, 47, 1);
	if (storage->scr.sda_spec3)
		storage->scr.cmds = unstuff_bits(resp, 32, 2);
}

int sd_storage_get_scr(sdmmc_storage_t *storage, u8 *buf)
{
	sdmmc_cmd_t cmdbuf;
	sdmmc_init_cmd(&cmdbuf, SD_APP_SEND_SCR, 0, SDMMC_RSP_TYPE_1, 0);

	sdmmc_req_t reqbuf;
	reqbuf.buf              = buf;
	reqbuf.blksize          = 8;
	reqbuf.num_sectors      = 1;
	reqbuf.is_write         = 0;
	reqbuf.is_multi_block   = 0;
	reqbuf.is_auto_stop_trn = 0;

	if (!_sd_storage_execute_app_cmd(storage, R1_STATE_TRAN, 0, &cmdbuf, &reqbuf, NULL))
		return 0;

	u32 tmp = 0;
	sdmmc_get_rsp(storage->sdmmc, &tmp, 4, SDMMC_RSP_TYPE_1);
	//Prepare buffer for unstuff_bits
	for (int i = 0; i < 8; i+=4)
	{
		storage->raw_scr[i + 3] = buf[i];
		storage->raw_scr[i + 2] = buf[i + 1];
		storage->raw_scr[i + 1] = buf[i + 2];
		storage->raw_scr[i]     = buf[i + 3];
	}
	_sd_storage_parse_scr(storage);

	return _sdmmc_storage_check_card_status(tmp);
}

static int _sd_storage_switch_get(sdmmc_storage_t *storage, void *buf)
{
	sdmmc_cmd_t cmdbuf;
	sdmmc_init_cmd(&cmdbuf, SD_SWITCH, 0xFFFFFF, SDMMC_RSP_TYPE_1, 0);

	sdmmc_req_t reqbuf;
	reqbuf.buf              = buf;
	reqbuf.blksize          = 64;
	reqbuf.num_sectors      = 1;
	reqbuf.is_write         = 0;
	reqbuf.is_multi_block   = 0;
	reqbuf.is_auto_stop_trn = 0;

	if (!sdmmc_execute_cmd(storage->sdmmc, &cmdbuf, &reqbuf, NULL))
		return 0;

	//gfx_hexdump(0, (u8 *)buf, 64);

	u32 tmp = 0;
	sdmmc_get_rsp(storage->sdmmc, &tmp, 4, SDMMC_RSP_TYPE_1);
	return _sdmmc_storage_check_card_status(tmp);
}

static int _sd_storage_switch(sdmmc_storage_t *storage, void *buf, int mode, int group, u32 arg)
{
	sdmmc_cmd_t cmdbuf;
	u32 switchcmd = mode << 31 | 0x00FFFFFF;
	switchcmd &= ~(0xF << (group * 4));
	switchcmd |= arg << (group * 4);
	sdmmc_init_cmd(&cmdbuf, SD_SWITCH, switchcmd, SDMMC_RSP_TYPE_1, 0);

	sdmmc_req_t reqbuf;
	reqbuf.buf              = buf;
	reqbuf.blksize          = 64;
	reqbuf.num_sectors      = 1;
	reqbuf.is_write         = 0;
	reqbuf.is_multi_block   = 0;
	reqbuf.is_auto_stop_trn = 0;

	if (!sdmmc_execute_cmd(storage->sdmmc, &cmdbuf, &reqbuf, NULL))
		return 0;

	u32 tmp = 0;
	sdmmc_get_rsp(storage->sdmmc, &tmp, 4, SDMMC_RSP_TYPE_1);
	return _sdmmc_storage_check_card_status(tmp);
}

static void _sd_storage_set_power_limit(sdmmc_storage_t *storage, u16 power_limit, u8 *buf)
{
	u32 pwr = SD_SET_POWER_LIMIT_0_72;

	if (power_limit & SD_MAX_POWER_2_88)
		pwr = SD_SET_POWER_LIMIT_2_88;
	else if (power_limit & SD_MAX_POWER_2_16)
		pwr = SD_SET_POWER_LIMIT_2_16;
	else if (power_limit & SD_MAX_POWER_1_44)
		pwr = SD_SET_POWER_LIMIT_1_44;

	_sd_storage_switch(storage, buf, SD_SWITCH_SET, 3, pwr);

	if (((buf[15] >> 4) & 0x0F) == pwr)
	{
		switch (pwr)
		{
		case SD_SET_POWER_LIMIT_2_88:
DPRINTF("[SD] power limit raised to 2880 mW\n");
			break;

		case SD_SET_POWER_LIMIT_2_16:
DPRINTF("[SD] power limit raised to 2160 mW\n");
			break;

		case SD_SET_POWER_LIMIT_1_44:
DPRINTF("[SD] power limit raised to 1440 mW\n");
			break;

		default:
		case SD_SET_POWER_LIMIT_0_72:
DPRINTF("[SD] power limit defaulted to 720 mW\n");
			break;
		}
	}
}

static int _sd_storage_enable_highspeed(sdmmc_storage_t *storage, u32 hs_type, u8 *buf)
{
	if (!_sd_storage_switch(storage, buf, SD_SWITCH_CHECK, 0, hs_type))
		return 0;
DPRINTF("[SD] supports (U)HS mode: %d\n", buf[16] & 0xF);

	u32 type_out = buf[16] & 0xF;
	if (type_out != hs_type)
		return 0;
DPRINTF("[SD] supports selected (U)HS mode\n");

	u16 total_pwr_consumption = ((u16)buf[0] << 8) | buf[1];
DPRINTF("[SD] total max power: %d mW\n", total_pwr_consumption * 3600 / 1000);

	if (total_pwr_consumption <= 800)
	{
		if (!_sd_storage_switch(storage, buf, SD_SWITCH_SET, 0, hs_type))
			return 0;

		if (type_out != (buf[16] & 0xF))
			return 0;

		return 1;
	}

DPRINTF("[SD] card max power over limit\n");
	return 0;
}

static int _sd_storage_enable_uhs_low_volt(sdmmc_storage_t *storage, u32 type, u8 *buf)
{
	if (sdmmc_get_bus_width(storage->sdmmc) != SDMMC_BUS_WIDTH_4)
		return 0;

	if (!_sd_storage_switch_get(storage, buf))
		return 0;

	u8  access_mode = buf[13];
	u16 power_limit = buf[7] | buf[6] << 8;
DPRINTF("[SD] access: %02X, power: %02X\n", access_mode, power_limit);

	// Try to raise the power limit to let the card perform better.
	_sd_storage_set_power_limit(storage, power_limit, buf);

	u32 hs_type = 0;
	switch (type)
	{
	case SDHCI_TIMING_UHS_SDR104:
	case SDHCI_TIMING_UHS_SDR82:
		// Fall through if not supported.
		if (access_mode & SD_MODE_UHS_SDR104)
		{
			hs_type = UHS_SDR104_BUS_SPEED;
DPRINTF("[SD] bus speed set to SDR104\n");
			switch (type)
			{
			case SDHCI_TIMING_UHS_SDR104:
				storage->csd.busspeed = 104;
				break;
			case SDHCI_TIMING_UHS_SDR82:
				storage->csd.busspeed = 82;
				break;
			}
			break;
		}
	case SDHCI_TIMING_UHS_SDR50:
		if (access_mode & SD_MODE_UHS_SDR50)
		{
			type = SDHCI_TIMING_UHS_SDR50;
			hs_type = UHS_SDR50_BUS_SPEED;
DPRINTF("[SD] bus speed set to SDR50\n");
			storage->csd.busspeed = 50;
			break;
		}
/*
	case SDHCI_TIMING_UHS_SDR25:
		if (access_mode & SD_MODE_UHS_SDR25)
		{
			type = SDHCI_TIMING_UHS_SDR25;
			hs_type = UHS_SDR25_BUS_SPEED;
DPRINTF("[SD] bus speed set to SDR25\n");
			storage->csd.busspeed = 25;
			break;
		}
*/
	case SDHCI_TIMING_UHS_SDR12:
		if (!(access_mode & SD_MODE_UHS_SDR12))
			return 0;
		type = SDHCI_TIMING_UHS_SDR12;
		hs_type = UHS_SDR12_BUS_SPEED;
DPRINTF("[SD] bus speed set to SDR12\n");
		storage->csd.busspeed = 12;
		break;

	default:
		return 0;
		break;
	}

	if (!_sd_storage_enable_highspeed(storage, hs_type, buf))
		return 0;
DPRINTF("[SD] card accepted UHS\n");
	if (!sdmmc_setup_clock(storage->sdmmc, type))
		return 0;
DPRINTF("[SD] after setup clock\n");
	if (!sdmmc_tuning_execute(storage->sdmmc, type, MMC_SEND_TUNING_BLOCK))
		return 0;
DPRINTF("[SD] after tuning\n");
	return _sdmmc_storage_check_status(storage);
}

static int _sd_storage_enable_hs_high_volt(sdmmc_storage_t *storage, u8 *buf)
{
	if (!_sd_storage_switch_get(storage, buf))
		return 0;

	u8  access_mode = buf[13];
	u16 power_limit = buf[7] | buf[6] << 8;
DPRINTF("[SD] access: %02X, power: %02X\n", access_mode, power_limit);

	// Try to raise the power limit to let the card perform better.
	_sd_storage_set_power_limit(storage, power_limit, buf);

	if (!(access_mode & SD_MODE_HIGH_SPEED))
		return 1;

	if (!_sd_storage_enable_highspeed(storage, HIGH_SPEED_BUS_SPEED, buf))
		return 0;

	if (!_sdmmc_storage_check_status(storage))
		return 0;

	return sdmmc_setup_clock(storage->sdmmc, SDHCI_TIMING_SD_HS25);
}

u32 sd_storage_get_ssr_au(sdmmc_storage_t *storage)
{
	u32 au_size = storage->ssr.uhs_au_size;

	if (!au_size)
		au_size = storage->ssr.au_size;

	if (au_size <= 10)
	{
		u32 shift = au_size;
		au_size = shift ? 8 : 0;
    	au_size <<= shift;
	}
	else
	{
		switch (au_size)
		{
		case 11:
			au_size = 12288;
			break;
		case 12:
			au_size = 16384;
			break;
		case 13:
			au_size = 24576;
			break;
		case 14:
			au_size = 32768;
			break;
		case 15:
			au_size = 65536;
			break;
		}
	}

	return au_size;
}

static void _sd_storage_parse_ssr(sdmmc_storage_t *storage)
{
	// unstuff_bits supports only 4 u32 so break into 2 x u32x4 groups.
	u32 raw_ssr1[4]; // 511:384.
	u32 raw_ssr2[4]; // 383:256.

	memcpy(raw_ssr1, &storage->raw_ssr[0],  16);
	memcpy(raw_ssr2, &storage->raw_ssr[16], 16);

	storage->ssr.bus_width = (unstuff_bits(raw_ssr1, 510 - 384, 2) & SD_BUS_WIDTH_4) ? 4 : 1;
	storage->ssr.protected_size = unstuff_bits(raw_ssr1, 448 - 384, 32);

	u32 speed_class = unstuff_bits(raw_ssr1, 440 - 384, 8);
	switch(speed_class)
	{
	case 0:
	case 1:
	case 2:
	case 3:
		storage->ssr.speed_class = speed_class << 1;
		break;

	case 4:
		storage->ssr.speed_class = 10;
		break;

	default:
		storage->ssr.speed_class = speed_class;
		break;
	}
	storage->ssr.uhs_grade   = unstuff_bits(raw_ssr1, 396 - 384, 4);
	storage->ssr.video_class = unstuff_bits(raw_ssr1, 384 - 384, 8);
	storage->ssr.app_class   = unstuff_bits(raw_ssr2, 336 - 256, 4);

	storage->ssr.au_size     = unstuff_bits(raw_ssr1, 428 - 384, 4);
	storage->ssr.uhs_au_size = unstuff_bits(raw_ssr1, 392 - 384, 4);
}

int sd_storage_get_ssr(sdmmc_storage_t *storage, u8 *buf)
{
	sdmmc_cmd_t cmdbuf;
	sdmmc_init_cmd(&cmdbuf, SD_APP_SD_STATUS, 0, SDMMC_RSP_TYPE_1, 0);

	sdmmc_req_t reqbuf;
	reqbuf.buf              = buf;
	reqbuf.blksize          = 64;
	reqbuf.num_sectors      = 1;
	reqbuf.is_write         = 0;
	reqbuf.is_multi_block   = 0;
	reqbuf.is_auto_stop_trn = 0;

	if (!(storage->csd.cmdclass & CCC_APP_SPEC))
	{
DPRINTF("[SD] ssr: Not supported\n");
		return 0;
	}

	if (!_sd_storage_execute_app_cmd(storage, R1_STATE_TRAN, 0, &cmdbuf, &reqbuf, NULL))
		return 0;

	u32 tmp = 0;
	sdmmc_get_rsp(storage->sdmmc, &tmp, 4, SDMMC_RSP_TYPE_1);

	// Convert buffer to LE.
	for (int i = 0; i < 64; i += 4)
	{
		storage->raw_ssr[i + 3] = buf[i];
		storage->raw_ssr[i + 2] = buf[i + 1];
		storage->raw_ssr[i + 1] = buf[i + 2];
		storage->raw_ssr[i]     = buf[i + 3];
	}

	_sd_storage_parse_ssr(storage);

	return _sdmmc_storage_check_card_status(tmp);
}

static void _sd_storage_parse_cid(sdmmc_storage_t *storage)
{
	u32 *raw_cid = (u32 *)&(storage->raw_cid);

	storage->cid.manfid       = unstuff_bits(raw_cid, 120, 8);
	storage->cid.oemid        = unstuff_bits(raw_cid, 104, 16);
	storage->cid.prod_name[0] = unstuff_bits(raw_cid, 96,  8);
	storage->cid.prod_name[1] = unstuff_bits(raw_cid, 88,  8);
	storage->cid.prod_name[2] = unstuff_bits(raw_cid, 80,  8);
	storage->cid.prod_name[3] = unstuff_bits(raw_cid, 72,  8);
	storage->cid.prod_name[4] = unstuff_bits(raw_cid, 64,  8);
	storage->cid.hwrev        = unstuff_bits(raw_cid, 60,  4);
	storage->cid.fwrev        = unstuff_bits(raw_cid, 56,  4);
	storage->cid.serial       = unstuff_bits(raw_cid, 24,  32);
	storage->cid.year         = unstuff_bits(raw_cid, 12,  8) + 2000;
	storage->cid.month        = unstuff_bits(raw_cid, 8,   4);
}

static void _sd_storage_parse_csd(sdmmc_storage_t *storage)
{
	u32 *raw_csd = (u32 *)&(storage->raw_csd);

	storage->csd.structure     = unstuff_bits(raw_csd, 126, 2);
	storage->csd.cmdclass      = unstuff_bits(raw_csd, 84, 12);
	storage->csd.read_blkbits  = unstuff_bits(raw_csd, 80, 4);
	storage->csd.write_protect = unstuff_bits(raw_csd, 12, 2);
	switch(storage->csd.structure)
	{
	case 0:
		storage->csd.capacity = (1 + unstuff_bits(raw_csd, 62, 12)) << (unstuff_bits(raw_csd, 47, 3) + 2);
		storage->csd.capacity <<= unstuff_bits(raw_csd, 80, 4) - 9; // Convert native block size to LBA 512B.
		break;

	case 1:
		storage->csd.c_size       = (1 + unstuff_bits(raw_csd, 48, 22));
		storage->csd.capacity     = storage->csd.c_size << 10;
		storage->csd.read_blkbits = 9;
		break;

	default:
DPRINTF("[SD] unknown CSD structure %d\n", storage->csd.structure);
		break;
	}

	storage->sec_cnt = storage->csd.capacity;
}

static bool _sdmmc_storage_get_bus_uhs_support(u32 bus_width, u32 type)
{
	switch (type)
	{
	case SDHCI_TIMING_UHS_SDR12:
	case SDHCI_TIMING_UHS_SDR25:
	case SDHCI_TIMING_UHS_SDR50:
	case SDHCI_TIMING_UHS_SDR104:
	case SDHCI_TIMING_UHS_SDR82:
	case SDHCI_TIMING_UHS_DDR50:
		if (bus_width == SDMMC_BUS_WIDTH_4)
			return true;
	default:
		return false;
	}
}

void sdmmc_storage_init_wait_sd()
{
	// T210/T210B01 WAR: Wait exactly 239ms for IO and Controller power to discharge.
	u32 sd_poweroff_time = (u32)get_tmr_ms() - sd_power_cycle_time_start;
	if (sd_poweroff_time < 239)
		msleep(239 - sd_poweroff_time);
}

int sdmmc_storage_init_sd(sdmmc_storage_t *storage, sdmmc_t *sdmmc, u32 bus_width, u32 type)
{
	u32  tmp = 0;
	int  is_sdsc = 0;
	u8  *buf = (u8 *)SDMMC_UPPER_BUFFER;
	bool bus_uhs_support = _sdmmc_storage_get_bus_uhs_support(bus_width, type);

DPRINTF("[SD] init: bus: %d, type: %d\n", bus_width, type);

	// Some cards (SanDisk U1), do not like a fast power cycle. Wait min 100ms.
	sdmmc_storage_init_wait_sd();

	memset(storage, 0, sizeof(sdmmc_storage_t));
	storage->sdmmc = sdmmc;

	if (!sdmmc_init(sdmmc, SDMMC_1, SDMMC_POWER_3_3, SDMMC_BUS_WIDTH_1, SDHCI_TIMING_SD_ID, SDMMC_POWER_SAVE_DISABLE))
		return 0;
DPRINTF("[SD] after init\n");

	usleep(1000 + (74000 + sdmmc->divisor - 1) / sdmmc->divisor);

	if (!_sdmmc_storage_go_idle_state(storage))
		return 0;
DPRINTF("[SD] went to idle state\n");

	if (!_sd_storage_send_if_cond(storage, &is_sdsc))
		return 0;
DPRINTF("[SD] after send if cond\n");

	if (!_sd_storage_get_op_cond(storage, is_sdsc, bus_uhs_support))
		return 0;
DPRINTF("[SD] got op cond\n");

	if (!_sdmmc_storage_get_cid(storage))
		return 0;
DPRINTF("[SD] got cid\n");
	_sd_storage_parse_cid(storage);

	if (!_sd_storage_get_rca(storage))
		return 0;
DPRINTF("[SD] got rca (= %04X)\n", storage->rca);

	if (!_sdmmc_storage_get_csd(storage))
		return 0;
DPRINTF("[SD] got csd\n");
	_sd_storage_parse_csd(storage);

	if (!storage->is_low_voltage)
	{
		if (!sdmmc_setup_clock(storage->sdmmc, SDHCI_TIMING_SD_DS12))
			return 0;
DPRINTF("[SD] after setup default clock\n");
	}

	if (!_sdmmc_storage_select_card(storage))
		return 0;
DPRINTF("[SD] card selected\n");

	if (!_sdmmc_storage_set_blocklen(storage, 512))
		return 0;
DPRINTF("[SD] set blocklen to 512\n");

	// Disconnect Card Detect resistor from DAT3.
	if (!_sd_storage_execute_app_cmd_type1(storage, &tmp, SD_APP_SET_CLR_CARD_DETECT, 0, 0, R1_STATE_TRAN))
		return 0;
DPRINTF("[SD] cleared card detect\n");

	if (!sd_storage_get_scr(storage, buf))
		return 0;
DPRINTF("[SD] got scr\n");

	// If card supports a wider bus and if it's not SD Version 1.0 switch bus width.
	if (bus_width == SDMMC_BUS_WIDTH_4 && (storage->scr.bus_widths & BIT(SD_BUS_WIDTH_4)) && storage->scr.sda_vsn)
	{
		if (!_sd_storage_execute_app_cmd_type1(storage, &tmp, SD_APP_SET_BUS_WIDTH, SD_BUS_WIDTH_4, 0, R1_STATE_TRAN))
			return 0;

		sdmmc_set_bus_width(storage->sdmmc, SDMMC_BUS_WIDTH_4);
DPRINTF("[SD] switched to wide bus width\n");
	}
	else
	{
		bus_width = SDMMC_BUS_WIDTH_1;
DPRINTF("[SD] SD does not support wide bus width\n");
	}

	if (storage->is_low_voltage)
	{
		if (!_sd_storage_enable_uhs_low_volt(storage, type, buf))
			return 0;
DPRINTF("[SD] enabled UHS\n");
	}
	else if (type != SDHCI_TIMING_SD_DS12 && storage->scr.sda_vsn) // Not default speed and not SD Version 1.0.
	{
		if (!_sd_storage_enable_hs_high_volt(storage, buf))
			return 0;

DPRINTF("[SD] enabled HS\n");
		switch (bus_width)
		{
		case SDMMC_BUS_WIDTH_4:
			storage->csd.busspeed = 25;
			break;

		case SDMMC_BUS_WIDTH_1:
			storage->csd.busspeed = 6;
			break;
		}
	}

	// Parse additional card info from sd status.
	if (sd_storage_get_ssr(storage, buf))
	{
DPRINTF("[SD] got sd status\n");
	}

	sdmmc_card_clock_powersave(sdmmc, SDMMC_POWER_SAVE_ENABLE);

	storage->initialized = 1;

	return 1;
}

/*
 * Gamecard specific functions.
 */

int _gc_storage_custom_cmd(sdmmc_storage_t *storage, void *buf)
{
	u32 resp;
	sdmmc_cmd_t cmdbuf;
	sdmmc_init_cmd(&cmdbuf, MMC_VENDOR_60_CMD, 0, SDMMC_RSP_TYPE_1, 1);

	sdmmc_req_t reqbuf;
	reqbuf.buf              = buf;
	reqbuf.blksize          = 64;
	reqbuf.num_sectors      = 1;
	reqbuf.is_write         = 1;
	reqbuf.is_multi_block   = 0;
	reqbuf.is_auto_stop_trn = 0;

	if (!sdmmc_execute_cmd(storage->sdmmc, &cmdbuf, &reqbuf, NULL))
	{
		sdmmc_stop_transmission(storage->sdmmc, &resp);
		return 0;
	}

	if (!sdmmc_get_rsp(storage->sdmmc, &resp, 4, SDMMC_RSP_TYPE_1))
		return 0;
	if (!_sdmmc_storage_check_card_status(resp))
		return 0;
	return _sdmmc_storage_check_status(storage);
}

int sdmmc_storage_init_gc(sdmmc_storage_t *storage, sdmmc_t *sdmmc)
{
	memset(storage, 0, sizeof(sdmmc_storage_t));
	storage->sdmmc = sdmmc;

	if (!sdmmc_init(sdmmc, SDMMC_2, SDMMC_POWER_1_8, SDMMC_BUS_WIDTH_8, SDHCI_TIMING_MMC_DDR100, SDMMC_POWER_SAVE_DISABLE))
		return 0;
DPRINTF("[gc] after init\n");

	usleep(1000 + (10000 + sdmmc->divisor - 1) / sdmmc->divisor);

	if (!sdmmc_tuning_execute(storage->sdmmc, SDHCI_TIMING_MMC_DDR100, MMC_SEND_TUNING_BLOCK_HS200))
		return 0;
DPRINTF("[gc] after tuning\n");

	sdmmc_card_clock_powersave(sdmmc, SDMMC_POWER_SAVE_ENABLE);

	storage->initialized = 1;

	return 1;
}
