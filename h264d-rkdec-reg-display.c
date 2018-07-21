#include <stdio.h>
#include <errno.h>
#include <stdint.h>

#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <stddef.h>

/* Number registers for the decoder */
#define DEC_VDPU1_REGISTERS         (101)

#if defined(_WIN32) && !defined(__MINGW32CE__)

typedef unsigned char           RK_U8;
typedef unsigned short          RK_U16;
typedef unsigned int            RK_U32;
typedef unsigned __int64        RK_U64;

typedef signed char             RK_S8;
typedef signed short            RK_S16;
typedef signed int              RK_S32;
typedef signed __int64          RK_S64;

#else

typedef unsigned char           RK_U8;
typedef unsigned short          RK_U16;
typedef unsigned int            RK_U32;
typedef unsigned long long int  RK_U64;


typedef signed char             RK_S8;
typedef signed short            RK_S16;
typedef signed int              RK_S32;
typedef signed long long int    RK_S64;

#endif

typedef struct {
    RK_U32 SwReg00; // Jamais réglé

    struct {
        RK_U32 sw_dec_en : 1; // Starts the decoder ? sw_dec_en = 1 [set_asic_regs]
        RK_U32 reserve0 : 3; //
        RK_U32 sw_dec_irq_dis_cfg : 1; // 0 - ??? |set_device_regs]
        RK_U32 reserve1 : 3;
        RK_U32 sw_dec_irq_config : 1;
        RK_U32 reserve2 : 3;
        RK_U32 sw_dec_rdy_int : 1; // 1 if Ready
        RK_U32 sw_dec_bus_int : 1; // ???
        RK_U32 sw_dec_buffer_int : 1; // ???
        RK_U32 sw_dec_aso_int : 1; // ???
        RK_U32 sw_dec_error_int : 1; // ???
        RK_U32 sw_dec_slice_int : 1; // ???
        RK_U32 sw_dec_timeout : 1; // ???
        RK_U32 reserve3 : 5;
        RK_U32 sw_dec_pic_inf : 1; // ???
        RK_U32 reserve4 : 7;
    } SwReg01;

    struct {
        RK_U32 sw_dec_max_burst : 5; // 16 - Valid values: 0, 4, 8, 16 [set_device_regs]
        RK_U32 sw_dec_scmd_dis : 1; // 0 - Disable [set_device_regs]
        RK_U32 sw_dec_adv_pre_dis : 1; // 0 - Disable [set_device_regs]
        RK_U32 sw_tiled_mode_lsb : 1; // ???
        RK_U32 sw_dec_out_endian : 1; // 1 - Little Endian [set_device_regs]
        RK_U32 sw_dec_in_endian : 1;  // 0 - Big Endian [set_device_regs]
        /* Clock_gating
         * 0:clock always on
         * 1: clock gating module control the key(turn off when decoder free)
         */
        RK_U32 sw_dec_clk_gate_e : 1; // 1 [set_device_regs]
        RK_U32 sw_dec_latency : 6; // 0 - Compensation for bus latency. Valid values : [0-63] [set_device_regs]
        RK_U32 sw_tiled_mode_msb : 1; // 0 - 0: raster scan  1: tiled [set_device_regs]
        RK_U32 sw_dec_data_disc_e : 1; // 0 - ??? [set_device_regs]
        RK_U32 sw_dec_outswap32_e : 1; // 1 - ??? [set_device_regs]
        RK_U32 sw_dec_inswap32_e : 1; // 1 - Little Endian (???) [set_device_regs]
        RK_U32 sw_dec_strendian_e : 1; // 1 - Little Endian [set_device_regs]
        RK_U32 sw_dec_strswap32_e : 1; // 1 - Little Endian [set_device_regs]
        RK_U32 sw_dec_timeout_e : 1; // 1 - ??? [set_device_regs]
        RK_U32 sw_dec_axi_rd_id : 8; // 255 - ??? [set_device_regs]
    } SwReg02;

    struct {
        RK_U32 sw_dec_axi_wr_id : 8; // 255 - ??? [set_device_regs]
        RK_U32 sw_dec_ahb_hlock_e : 1; // ???
        RK_U32 sw_picord_count_e : 1; // ??? [set_vlc_regs]
        RK_U32 sw_seq_mbaff_e : 1; // pp->MbaffFrameFlag - ??? [set_vlc_regs]
        RK_U32 sw_reftopfirst_e : 1; // ???
        RK_U32 sw_write_mvs_e : 1; // (p_long->nal_ref_idc != 0) ? 1 : 0; /* defalut set 1 */ - ??? [set_vlc_regs]
        RK_U32 sw_pic_fixed_quant : 1; // pp->curr_layer_id; //!< VDPU_MVC_E - ??? [set_asic_regs]
        RK_U32 sw_filtering_dis : 1; // 0 [set_asic_regs]
        RK_U32 sw_dec_out_dis : 1; // 0 - ??? [set_vlc_regs] [set_asic_regs]
        RK_U32 sw_ref_topfield_e : 1; // ???
        RK_U32 sw_sorenson_e : 1; // ???
        RK_U32 sw_fwd_interlace_e : 1; // ???
        RK_U32 sw_pic_topfield_e : 1; // (!pp->CurrPic.AssociatedFlag) ? 1 : 0; /* bottomFieldFlag */ - ??? [set_asic_regs]
        RK_U32 sw_pic_inter_e : 1; // ???
        RK_U32 sw_pic_b_e : 1; 
        RK_U32 sw_pic_fieldmode_e : 1;
        RK_U32 sw_pic_interlace_e : 1;
        RK_U32 sw_pjpeg_e : 1;
        RK_U32 sw_divx3_e : 1;
        RK_U32 sw_skip_mode : 1;
        RK_U32 sw_rlc_mode_e : 1; 
        RK_U32 sw_dec_mode : 4; // 0 [set_device_regs]
    } SwReg03;

    struct {
        RK_U32 sw_ref_frames : 5;
        RK_U32 reserve0 : 6;
        RK_U32 sw_pic_mb_height_p : 8;
        RK_U32 reserve1 : 4;
        RK_U32 sw_pic_mb_width : 9;
    } SwReg04;

    struct {
        RK_U32 sw_fieldpic_flag_e : 1;
        RK_U32 reserve0 : 13;
        RK_U32 sw_ch_qp_offset2 : 5;    /* Cr */
        RK_U32 sw_ch_qp_offset : 5;     /* Cb */
        RK_U32 sw_type1_quant_e : 1;
        RK_U32 sw_sync_marker_e : 1;
        RK_U32 sw_strm_start_bit : 6;
    } SwReg05;

    struct {
        RK_U32 sw_stream_len : 24;
        RK_U32 sw_ch_8pix_ileav_e : 1;
        RK_U32 sw_init_qp : 6;
        RK_U32 sw_start_code_e : 1;
    } SwReg06;

    struct {
        RK_U32 sw_framenum : 16;
        RK_U32 sw_framenum_len : 5;
        RK_U32 reserve0 : 5;
        RK_U32 sw_weight_bipr_idc : 2;
        RK_U32 sw_weight_pred_e : 1;
        RK_U32 sw_dir_8x8_infer_e : 1;
        RK_U32 sw_blackwhite_e : 1;
        RK_U32 sw_cabac_e : 1;
    } SwReg07;

    struct {
        RK_U32 sw_idr_pic_id : 16;
        RK_U32 sw_idr_pic_e : 1;
        RK_U32 sw_refpic_mk_len : 11;
        RK_U32 sw_8x8trans_flag_e : 1;
        RK_U32 sw_rdpic_cnt_pres : 1;
        RK_U32 sw_filt_ctrl_pres : 1;
        RK_U32 sw_const_intra_e : 1;
    } SwReg08;

    struct {
        RK_U32 sw_poc_length : 8;
        RK_U32 reserve0 : 6;
        RK_U32 sw_refidx0_active : 5;
        RK_U32 sw_refidx1_active : 5;
        RK_U32 sw_pps_id : 8;
    } SwReg09;

    struct {
        RK_U32 sw_pinit_rlist_f4 : 5;
        RK_U32 sw_pinit_rlist_f5 : 5;
        RK_U32 sw_pinit_rlist_f6 : 5;
        RK_U32 sw_pinit_rlist_f7 : 5;
        RK_U32 sw_pinit_rlist_f8 : 5;
        RK_U32 sw_pinit_rlist_f9 : 5;
        RK_U32 reserve0 : 2;
    } SwReg10;

    struct {
        RK_U32 sw_pinit_rlist_f10 : 5;
        RK_U32 sw_pinit_rlist_f11 : 5;
        RK_U32 sw_pinit_rlist_f12 : 5;
        RK_U32 sw_pinit_rlist_f13 : 5;
        RK_U32 sw_pinit_rlist_f14 : 5;
        RK_U32 sw_pinit_rlist_f15 : 5;
        RK_U32 sw_i4x4_or_dc_base : 2;
    } SwReg11;

    struct {
        RK_U32 rlc_vlc_st_adr : 32;
    } SwReg12;

    struct {
        RK_U32 dec_out_st_adr : 32;
    } SwReg13;

    /* MPP passes fd of reference frame to kernel
    * with the whole register rather than higher 30-bit.
    * At the same time, the lower 2-bit will be assigned
    * by kernel.
    * */
    struct {
        //RK_U32 sw_refer0_topc_e    : 1;
        //RK_U32 sw_refer0_field_e   : 1;
        RK_U32 sw_refer0_base : 32;
    } SwReg14;

    struct {
        //RK_U32 sw_refer1_topc_e    : 1;
        //RK_U32 sw_refer1_field_e   : 1;
        RK_U32 sw_refer1_base : 32;
    } SwReg15;

    struct {
        //RK_U32 sw_refer2_topc_e    : 1;
        //RK_U32 sw_refer2_field_e   : 1;
        RK_U32 sw_refer2_base : 32;
    } SwReg16;

    struct {
        //RK_U32 sw_refer3_topc_e    : 1;
        //RK_U32 sw_refer3_field_e   : 1;
        RK_U32 sw_refer3_base : 32;
    } SwReg17;

    struct {
        //RK_U32 sw_refer4_topc_e    : 1;
        //RK_U32 sw_refer4_field_e   : 1;
        RK_U32 sw_refer4_base : 32;
    } SwReg18;

    struct {
        //RK_U32 sw_refer5_topc_e    : 1;
        //RK_U32 sw_refer5_field_e   : 1;
        RK_U32 sw_refer5_base : 32;
    } SwReg19;

    struct {
        //RK_U32 sw_refer6_topc_e    : 1;
        //RK_U32 sw_refer6_field_e   : 1;
        RK_U32 sw_refer6_base : 32;
    } SwReg20;

    struct {
        //RK_U32 sw_refer7_topc_e    : 1;
        //RK_U32 sw_refer7_field_e   : 1;
        RK_U32 sw_refer7_base : 32;
    } SwReg21;

    struct {
        //RK_U32 sw_refer8_topc_e    : 1;
        //RK_U32 sw_refer8_field_e   : 1;
        RK_U32 sw_refer8_base : 32;
    } SwReg22;

    struct {
        //RK_U32 sw_refer9_topc_e    : 1;
        //RK_U32 sw_refer9_field_e   : 1;
        RK_U32 sw_refer9_base : 32;
    } SwReg23;

    struct {
        //RK_U32 sw_refer10_topc_e    : 1;
        //RK_U32 sw_refer10_field_e   : 1;
        RK_U32 sw_refer10_base : 32;
    } SwReg24;

    struct {
        //RK_U32 sw_refer11_topc_e    : 1;
        //RK_U32 sw_refer11_field_e   : 1;
        RK_U32 sw_refer11_base : 32;
    } SwReg25;

    struct {
        //RK_U32 sw_refer12_topc_e    : 1;
        //RK_U32 sw_refer12_field_e   : 1;
        RK_U32 sw_refer12_base : 32;
    } SwReg26;

    struct {
        //RK_U32 sw_refer13_topc_e    : 1;
        //RK_U32 sw_refer13_field_e   : 1;
        RK_U32 sw_refer13_base : 32;
    } SwReg27;

    struct {
        //RK_U32 sw_refer14_topc_e    : 1;
        //RK_U32 sw_refer14_field_e   : 1;
        RK_U32 sw_refer14_base : 32;
    } SwReg28;

    struct {
        //RK_U32 sw_refer15_topc_e    : 1;
        //RK_U32 sw_refer15_field_e   : 1;
        RK_U32 sw_refer15_base : 32;
    } SwReg29;

    struct {
        RK_U32 sw_refer0_nbr : 16;
        RK_U32 sw_refer1_nbr : 16;
    } SwReg30;

    struct {
        RK_U32 sw_refer2_nbr : 16;
        RK_U32 sw_refer3_nbr : 16;
    } SwReg31;

    struct {
        RK_U32 sw_refer4_nbr : 16;
        RK_U32 sw_refer5_nbr : 16;
    } SwReg32;

    struct {
        RK_U32 sw_refer6_nbr : 16;
        RK_U32 sw_refer7_nbr : 16;
    } SwReg33;

    struct {
        RK_U32 sw_refer8_nbr : 16;
        RK_U32 sw_refer9_nbr : 16;
    } SwReg34;

    struct {
        RK_U32 sw_refer10_nbr : 16;
        RK_U32 sw_refer11_nbr : 16;
    } SwReg35;

    struct {
        RK_U32 sw_refer12_nbr : 16;
        RK_U32 sw_refer13_nbr : 16;
    } SwReg36;

    struct {
        RK_U32 sw_refer14_nbr : 16;
        RK_U32 sw_refer15_nbr : 16;
    } SwReg37;

    struct {
        RK_U32 refpic_term_flag : 32;
    } SwReg38;

    struct {
        RK_U32 refpic_valid_flag : 32;
    } SwReg39;

    struct {
        RK_U32 qtable_st_adr : 32;
    } SwReg40;

    struct {
        RK_U32 dmmv_st_adr : 32;
    } SwReg41;

    struct {
        RK_U32 sw_binit_rlist_f0 : 5;
        RK_U32 sw_binit_rlist_b0 : 5;
        RK_U32 sw_binit_rlist_f1 : 5;
        RK_U32 sw_binit_rlist_b1 : 5;
        RK_U32 sw_binit_rlist_f2 : 5;
        RK_U32 sw_binit_rlist_b2 : 5;
        RK_U32 reserve0 : 2;
    } SwReg42;

    struct {
        RK_U32 sw_binit_rlist_f3 : 5;
        RK_U32 sw_binit_rlist_b3 : 5;
        RK_U32 sw_binit_rlist_f4 : 5;
        RK_U32 sw_binit_rlist_b4 : 5;
        RK_U32 sw_binit_rlist_f5 : 5;
        RK_U32 sw_binit_rlist_b5 : 5;
        RK_U32 reserve0 : 2;
    } SwReg43;

    struct {
        RK_U32 sw_binit_rlist_f6 : 5;
        RK_U32 sw_binit_rlist_b6 : 5;
        RK_U32 sw_binit_rlist_f7 : 5;
        RK_U32 sw_binit_rlist_b7 : 5;
        RK_U32 sw_binit_rlist_f8 : 5;
        RK_U32 sw_binit_rlist_b8 : 5;
        RK_U32 reserve0 : 2;
    } SwReg44;

    struct {
        RK_U32 sw_binit_rlist_f9 : 5;
        RK_U32 sw_binit_rlist_b9 : 5;
        RK_U32 sw_binit_rlist_f10 : 5;
        RK_U32 sw_binit_rlist_b10 : 5;
        RK_U32 sw_binit_rlist_f11 : 5;
        RK_U32 sw_binit_rlist_b11 : 5;
        RK_U32 reserve0 : 2;
    } SwReg45;

    struct {
        RK_U32 sw_binit_rlist_f12 : 5;
        RK_U32 sw_binit_rlist_b12 : 5;
        RK_U32 sw_binit_rlist_f13 : 5;
        RK_U32 sw_binit_rlist_b13 : 5;
        RK_U32 sw_binit_rlist_f14 : 5;
        RK_U32 sw_binit_rlist_b14 : 5;
        RK_U32 reserve0 : 2;
    } SwReg46;

    struct {
        RK_U32 sw_binit_rlist_f15 : 5;
        RK_U32 sw_binit_rlist_b15 : 5;
        RK_U32 sw_pinit_rlist_f0 : 5;
        RK_U32 sw_pinit_rlist_f1 : 5;
        RK_U32 sw_pinit_rlist_f2 : 5;
        RK_U32 sw_pinit_rlist_f3 : 5;
        RK_U32 reserve0 : 2;
    } SwReg47;

    struct {
        RK_U32 reserve0 : 15;
        RK_U32 sw_startmb_y : 8;
        RK_U32 sw_startmb_x : 9;
    } SwReg48;

    struct {
        RK_U32 reserve0 : 2;
        RK_U32 sw_pred_bc_tap_0_2 : 10; // 20: Prediction filter [set_deviceç_regs]
        RK_U32 sw_pred_bc_tap_0_1 : 10; // -5: Prediction filter [set_device_regs]
        RK_U32 sw_pred_bc_tap_0_0 : 10; // 1: Prediction filter [set_device_regs]
    } SwReg49;

    RK_U32 SwReg50_54[5];

    struct {
        RK_U32 sw_apf_threshold : 14; // 8 - ??? [set_device_regs]
        RK_U32 sw_refbu2_picid : 5; // 
        RK_U32 sw_refbu2_thr : 12;
        RK_U32 sw_refbu2_buf_e : 1;
    } SwReg55;

    RK_U32 SwReg56;

    struct {
        RK_U32 sw_stream_len_hi : 1;
        RK_U32 sw_inter_dblspeed : 1;
        RK_U32 sw_intra_dblspeed : 1;
        RK_U32 sw_intra_dbl3t : 1;
        RK_U32 sw_paral_bus : 1;
        RK_U32 sw_axiwr_sel : 1;
        RK_U32 sw_pref_sigchan : 1;
        RK_U32 sw_cache_en : 1;
        RK_U32 reserve0 : 24;
    } SwReg57;

    RK_U32 SwReg58_100[43];
} H264dVdpu1Regs_t;

static void print_usage(
	char const * __restrict const char_name)
{
	printf("Usage : %s reg_dump_file\n",
		char_name);
}

static void print_regs(
	H264dVdpu1Regs_t const * __restrict const regs)
{
	printf("SwReg00                       : 32 (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg00), regs->SwReg00                    );
	printf("SwReg01.sw_dec_en             : 1  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg01), regs->SwReg01.sw_dec_en          );
	printf("SwReg01.reserve0              : 3  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg01), regs->SwReg01.reserve0           );
	printf("SwReg01.sw_dec_irq_dis_cfg    : 1  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg01), regs->SwReg01.sw_dec_irq_dis_cfg );
	printf("SwReg01.reserve1              : 3  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg01), regs->SwReg01.reserve1           );
	printf("SwReg01.sw_dec_irq_config     : 1  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg01), regs->SwReg01.sw_dec_irq_config  );
	printf("SwReg01.reserve2              : 3  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg01), regs->SwReg01.reserve2           );
	printf("SwReg01.sw_dec_rdy_int        : 1  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg01), regs->SwReg01.sw_dec_rdy_int     );
	printf("SwReg01.sw_dec_bus_int        : 1  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg01), regs->SwReg01.sw_dec_bus_int     );
	printf("SwReg01.sw_dec_buffer_int     : 1  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg01), regs->SwReg01.sw_dec_buffer_int  );
	printf("SwReg01.sw_dec_aso_int        : 1  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg01), regs->SwReg01.sw_dec_aso_int     );
	printf("SwReg01.sw_dec_error_int      : 1  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg01), regs->SwReg01.sw_dec_error_int   );
	printf("SwReg01.sw_dec_slice_int      : 1  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg01), regs->SwReg01.sw_dec_slice_int   );
	printf("SwReg01.sw_dec_timeout        : 1  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg01), regs->SwReg01.sw_dec_timeout     );
	printf("SwReg01.reserve3              : 5  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg01), regs->SwReg01.reserve3           );
	printf("SwReg01.sw_dec_pic_inf        : 1  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg01), regs->SwReg01.sw_dec_pic_inf     );
	printf("SwReg01.reserve4              : 7  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg01), regs->SwReg01.reserve4           );
	printf("SwReg02.sw_dec_max_burst      : 5  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg02), regs->SwReg02.sw_dec_max_burst   );
	printf("SwReg02.sw_dec_scmd_dis       : 1  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg02), regs->SwReg02.sw_dec_scmd_dis    );
	printf("SwReg02.sw_dec_adv_pre_dis    : 1  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg02), regs->SwReg02.sw_dec_adv_pre_dis );
	printf("SwReg02.sw_tiled_mode_lsb     : 1  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg02), regs->SwReg02.sw_tiled_mode_lsb  );
	printf("SwReg02.sw_dec_out_endian     : 1  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg02), regs->SwReg02.sw_dec_out_endian  );
	printf("SwReg02.sw_dec_in_endian      : 1  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg02), regs->SwReg02.sw_dec_in_endian   );
	printf("SwReg02.sw_dec_clk_gate_e     : 1  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg02), regs->SwReg02.sw_dec_clk_gate_e  );
	printf("SwReg02.sw_dec_latency        : 6  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg02), regs->SwReg02.sw_dec_latency     );
	printf("SwReg02.sw_tiled_mode_msb     : 1  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg02), regs->SwReg02.sw_tiled_mode_msb  );
	printf("SwReg02.sw_dec_data_disc_e    : 1  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg02), regs->SwReg02.sw_dec_data_disc_e );
	printf("SwReg02.sw_dec_outswap32_e    : 1  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg02), regs->SwReg02.sw_dec_outswap32_e );
	printf("SwReg02.sw_dec_inswap32_e     : 1  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg02), regs->SwReg02.sw_dec_inswap32_e  );
	printf("SwReg02.sw_dec_strendian_e    : 1  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg02), regs->SwReg02.sw_dec_strendian_e );
	printf("SwReg02.sw_dec_strswap32_e    : 1  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg02), regs->SwReg02.sw_dec_strswap32_e );
	printf("SwReg02.sw_dec_timeout_e      : 1  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg02), regs->SwReg02.sw_dec_timeout_e   );
	printf("SwReg02.sw_dec_axi_rd_id      : 8  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg02), regs->SwReg02.sw_dec_axi_rd_id   );
	printf("SwReg03.sw_dec_axi_wr_id      : 8  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg03), regs->SwReg03.sw_dec_axi_wr_id   );
	printf("SwReg03.sw_dec_ahb_hlock_e    : 1  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg03), regs->SwReg03.sw_dec_ahb_hlock_e );
	printf("SwReg03.sw_picord_count_e     : 1  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg03), regs->SwReg03.sw_picord_count_e  );
	printf("SwReg03.sw_seq_mbaff_e        : 1  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg03), regs->SwReg03.sw_seq_mbaff_e     );
	printf("SwReg03.sw_reftopfirst_e      : 1  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg03), regs->SwReg03.sw_reftopfirst_e   );
	printf("SwReg03.sw_write_mvs_e        : 1  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg03), regs->SwReg03.sw_write_mvs_e     );
	printf("SwReg03.sw_pic_fixed_quant    : 1  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg03), regs->SwReg03.sw_pic_fixed_quant );
	printf("SwReg03.sw_filtering_dis      : 1  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg03), regs->SwReg03.sw_filtering_dis   );
	printf("SwReg03.sw_dec_out_dis        : 1  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg03), regs->SwReg03.sw_dec_out_dis     );
	printf("SwReg03.sw_ref_topfield_e     : 1  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg03), regs->SwReg03.sw_ref_topfield_e  );
	printf("SwReg03.sw_sorenson_e         : 1  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg03), regs->SwReg03.sw_sorenson_e      );
	printf("SwReg03.sw_fwd_interlace_e    : 1  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg03), regs->SwReg03.sw_fwd_interlace_e );
	printf("SwReg03.sw_pic_topfield_e     : 1  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg03), regs->SwReg03.sw_pic_topfield_e  );
	printf("SwReg03.sw_pic_inter_e        : 1  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg03), regs->SwReg03.sw_pic_inter_e     );
	printf("SwReg03.sw_pic_b_e            : 1  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg03), regs->SwReg03.sw_pic_b_e         );
	printf("SwReg03.sw_pic_fieldmode_e    : 1  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg03), regs->SwReg03.sw_pic_fieldmode_e );
	printf("SwReg03.sw_pic_interlace_e    : 1  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg03), regs->SwReg03.sw_pic_interlace_e );
	printf("SwReg03.sw_pjpeg_e            : 1  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg03), regs->SwReg03.sw_pjpeg_e         );
	printf("SwReg03.sw_divx3_e            : 1  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg03), regs->SwReg03.sw_divx3_e         );
	printf("SwReg03.sw_skip_mode          : 1  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg03), regs->SwReg03.sw_skip_mode       );
	printf("SwReg03.sw_rlc_mode_e         : 1  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg03), regs->SwReg03.sw_rlc_mode_e      );
	printf("SwReg03.sw_dec_mode           : 4  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg03), regs->SwReg03.sw_dec_mode        );
	printf("SwReg04.sw_ref_frames         : 5  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg04), regs->SwReg04.sw_ref_frames      );
	printf("SwReg04.reserve0              : 6  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg04), regs->SwReg04.reserve0           );
	printf("SwReg04.sw_pic_mb_height_p    : 8  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg04), regs->SwReg04.sw_pic_mb_height_p );
	printf("SwReg04.reserve1              : 4  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg04), regs->SwReg04.reserve1           );
	printf("SwReg04.sw_pic_mb_width       : 9  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg04), regs->SwReg04.sw_pic_mb_width    );
	printf("SwReg05.sw_fieldpic_flag_e    : 1  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg05), regs->SwReg05.sw_fieldpic_flag_e );
	printf("SwReg05.reserve0              : 13 (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg05), regs->SwReg05.reserve0           );
	printf("SwReg05.sw_ch_qp_offset2      : 5  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg05), regs->SwReg05.sw_ch_qp_offset2   );
	printf("SwReg05.sw_ch_qp_offset       : 5  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg05), regs->SwReg05.sw_ch_qp_offset    );
	printf("SwReg05.sw_type1_quant_e      : 1  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg05), regs->SwReg05.sw_type1_quant_e   );
	printf("SwReg05.sw_sync_marker_e      : 1  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg05), regs->SwReg05.sw_sync_marker_e   );
	printf("SwReg05.sw_strm_start_bit     : 6  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg05), regs->SwReg05.sw_strm_start_bit  );
	printf("SwReg06.sw_stream_len         : 24 (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg06), regs->SwReg06.sw_stream_len      );
	printf("SwReg06.sw_ch_8pix_ileav_e    : 1  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg06), regs->SwReg06.sw_ch_8pix_ileav_e );
	printf("SwReg06.sw_init_qp            : 6  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg06), regs->SwReg06.sw_init_qp         );
	printf("SwReg06.sw_start_code_e       : 1  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg06), regs->SwReg06.sw_start_code_e    );
	printf("SwReg07.sw_framenum           : 16 (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg07), regs->SwReg07.sw_framenum        );
	printf("SwReg07.sw_framenum_len       : 5  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg07), regs->SwReg07.sw_framenum_len    );
	printf("SwReg07.reserve0              : 5  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg07), regs->SwReg07.reserve0           );
	printf("SwReg07.sw_weight_bipr_idc    : 2  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg07), regs->SwReg07.sw_weight_bipr_idc );
	printf("SwReg07.sw_weight_pred_e      : 1  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg07), regs->SwReg07.sw_weight_pred_e   );
	printf("SwReg07.sw_dir_8x8_infer_e    : 1  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg07), regs->SwReg07.sw_dir_8x8_infer_e );
	printf("SwReg07.sw_blackwhite_e       : 1  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg07), regs->SwReg07.sw_blackwhite_e    );
	printf("SwReg07.sw_cabac_e            : 1  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg07), regs->SwReg07.sw_cabac_e         );
	printf("SwReg08.sw_idr_pic_id         : 16 (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg08), regs->SwReg08.sw_idr_pic_id      );
	printf("SwReg08.sw_idr_pic_e          : 1  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg08), regs->SwReg08.sw_idr_pic_e       );
	printf("SwReg08.sw_refpic_mk_len      : 11 (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg08), regs->SwReg08.sw_refpic_mk_len   );
	printf("SwReg08.sw_8x8trans_flag_e    : 1  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg08), regs->SwReg08.sw_8x8trans_flag_e );
	printf("SwReg08.sw_rdpic_cnt_pres     : 1  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg08), regs->SwReg08.sw_rdpic_cnt_pres  );
	printf("SwReg08.sw_filt_ctrl_pres     : 1  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg08), regs->SwReg08.sw_filt_ctrl_pres  );
	printf("SwReg08.sw_const_intra_e      : 1  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg08), regs->SwReg08.sw_const_intra_e   );
	printf("SwReg09.sw_poc_length         : 8  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg09), regs->SwReg09.sw_poc_length      );
	printf("SwReg09.reserve0              : 6  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg09), regs->SwReg09.reserve0           );
	printf("SwReg09.sw_refidx0_active     : 5  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg09), regs->SwReg09.sw_refidx0_active  );
	printf("SwReg09.sw_refidx1_active     : 5  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg09), regs->SwReg09.sw_refidx1_active  );
	printf("SwReg09.sw_pps_id             : 8  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg09), regs->SwReg09.sw_pps_id          );
	printf("SwReg10.sw_pinit_rlist_f4     : 5  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg10), regs->SwReg10.sw_pinit_rlist_f4  );
	printf("SwReg10.sw_pinit_rlist_f5     : 5  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg10), regs->SwReg10.sw_pinit_rlist_f5  );
	printf("SwReg10.sw_pinit_rlist_f6     : 5  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg10), regs->SwReg10.sw_pinit_rlist_f6  );
	printf("SwReg10.sw_pinit_rlist_f7     : 5  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg10), regs->SwReg10.sw_pinit_rlist_f7  );
	printf("SwReg10.sw_pinit_rlist_f8     : 5  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg10), regs->SwReg10.sw_pinit_rlist_f8  );
	printf("SwReg10.sw_pinit_rlist_f9     : 5  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg10), regs->SwReg10.sw_pinit_rlist_f9  );
	printf("SwReg10.reserve0              : 2  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg10), regs->SwReg10.reserve0           );
	printf("SwReg11.sw_pinit_rlist_f10    : 5  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg11), regs->SwReg11.sw_pinit_rlist_f10 );
	printf("SwReg11.sw_pinit_rlist_f11    : 5  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg11), regs->SwReg11.sw_pinit_rlist_f11 );
	printf("SwReg11.sw_pinit_rlist_f12    : 5  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg11), regs->SwReg11.sw_pinit_rlist_f12 );
	printf("SwReg11.sw_pinit_rlist_f13    : 5  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg11), regs->SwReg11.sw_pinit_rlist_f13 );
	printf("SwReg11.sw_pinit_rlist_f14    : 5  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg11), regs->SwReg11.sw_pinit_rlist_f14 );
	printf("SwReg11.sw_pinit_rlist_f15    : 5  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg11), regs->SwReg11.sw_pinit_rlist_f15 );
	printf("SwReg11.sw_i4x4_or_dc_base    : 2  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg11), regs->SwReg11.sw_i4x4_or_dc_base );
	printf("SwReg12.rlc_vlc_st_adr        : 32 (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg12), regs->SwReg12.rlc_vlc_st_adr     );
	printf("SwReg13.dec_out_st_adr        : 32 (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg13), regs->SwReg13.dec_out_st_adr     );
	printf("SwReg14.sw_refer0_base        : 32 (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg14), regs->SwReg14.sw_refer0_base     );
	printf("SwReg15.sw_refer1_base        : 32 (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg15), regs->SwReg15.sw_refer1_base     );
	printf("SwReg16.sw_refer2_base        : 32 (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg16), regs->SwReg16.sw_refer2_base     );
	printf("SwReg17.sw_refer3_base        : 32 (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg17), regs->SwReg17.sw_refer3_base     );
	printf("SwReg18.sw_refer4_base        : 32 (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg18), regs->SwReg18.sw_refer4_base     );
	printf("SwReg19.sw_refer5_base        : 32 (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg19), regs->SwReg19.sw_refer5_base     );
	printf("SwReg20.sw_refer6_base        : 32 (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg20), regs->SwReg20.sw_refer6_base     );
	printf("SwReg21.sw_refer7_base        : 32 (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg21), regs->SwReg21.sw_refer7_base     );
	printf("SwReg22.sw_refer8_base        : 32 (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg22), regs->SwReg22.sw_refer8_base     );
	printf("SwReg23.sw_refer9_base        : 32 (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg23), regs->SwReg23.sw_refer9_base     );
	printf("SwReg24.sw_refer10_base       : 32 (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg24), regs->SwReg24.sw_refer10_base    );
	printf("SwReg25.sw_refer11_base       : 32 (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg25), regs->SwReg25.sw_refer11_base    );
	printf("SwReg26.sw_refer12_base       : 32 (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg26), regs->SwReg26.sw_refer12_base    );
	printf("SwReg27.sw_refer13_base       : 32 (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg27), regs->SwReg27.sw_refer13_base    );
	printf("SwReg28.sw_refer14_base       : 32 (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg28), regs->SwReg28.sw_refer14_base    );
	printf("SwReg29.sw_refer15_base       : 32 (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg29), regs->SwReg29.sw_refer15_base    );
	printf("SwReg30.sw_refer0_nbr         : 16 (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg30), regs->SwReg30.sw_refer0_nbr      );
	printf("SwReg30.sw_refer1_nbr         : 16 (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg30), regs->SwReg30.sw_refer1_nbr      );
	printf("SwReg31.sw_refer2_nbr         : 16 (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg31), regs->SwReg31.sw_refer2_nbr      );
	printf("SwReg31.sw_refer3_nbr         : 16 (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg31), regs->SwReg31.sw_refer3_nbr      );
	printf("SwReg32.sw_refer4_nbr         : 16 (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg32), regs->SwReg32.sw_refer4_nbr      );
	printf("SwReg32.sw_refer5_nbr         : 16 (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg32), regs->SwReg32.sw_refer5_nbr      );
	printf("SwReg33.sw_refer6_nbr         : 16 (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg33), regs->SwReg33.sw_refer6_nbr      );
	printf("SwReg33.sw_refer7_nbr         : 16 (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg33), regs->SwReg33.sw_refer7_nbr      );
	printf("SwReg34.sw_refer8_nbr         : 16 (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg34), regs->SwReg34.sw_refer8_nbr      );
	printf("SwReg34.sw_refer9_nbr         : 16 (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg34), regs->SwReg34.sw_refer9_nbr      );
	printf("SwReg35.sw_refer10_nbr        : 16 (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg35), regs->SwReg35.sw_refer10_nbr     );
	printf("SwReg35.sw_refer11_nbr        : 16 (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg35), regs->SwReg35.sw_refer11_nbr     );
	printf("SwReg36.sw_refer12_nbr        : 16 (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg36), regs->SwReg36.sw_refer12_nbr     );
	printf("SwReg36.sw_refer13_nbr        : 16 (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg36), regs->SwReg36.sw_refer13_nbr     );
	printf("SwReg37.sw_refer14_nbr        : 16 (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg37), regs->SwReg37.sw_refer14_nbr     );
	printf("SwReg37.sw_refer15_nbr        : 16 (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg37), regs->SwReg37.sw_refer15_nbr     );
	printf("SwReg38.refpic_term_flag      : 32 (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg38), regs->SwReg38.refpic_term_flag   );
	printf("SwReg39.refpic_valid_flag     : 32 (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg39), regs->SwReg39.refpic_valid_flag  );
	printf("SwReg40.qtable_st_adr         : 32 (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg40), regs->SwReg40.qtable_st_adr      );
	printf("SwReg41.dmmv_st_adr           : 32 (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg41), regs->SwReg41.dmmv_st_adr        );
	printf("SwReg42.sw_binit_rlist_f0     : 5  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg42), regs->SwReg42.sw_binit_rlist_f0  );
	printf("SwReg42.sw_binit_rlist_b0     : 5  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg42), regs->SwReg42.sw_binit_rlist_b0  );
	printf("SwReg42.sw_binit_rlist_f1     : 5  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg42), regs->SwReg42.sw_binit_rlist_f1  );
	printf("SwReg42.sw_binit_rlist_b1     : 5  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg42), regs->SwReg42.sw_binit_rlist_b1  );
	printf("SwReg42.sw_binit_rlist_f2     : 5  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg42), regs->SwReg42.sw_binit_rlist_f2  );
	printf("SwReg42.sw_binit_rlist_b2     : 5  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg42), regs->SwReg42.sw_binit_rlist_b2  );
	printf("SwReg42.reserve0              : 2  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg42), regs->SwReg42.reserve0           );
	printf("SwReg43.sw_binit_rlist_f3     : 5  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg43), regs->SwReg43.sw_binit_rlist_f3  );
	printf("SwReg43.sw_binit_rlist_b3     : 5  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg43), regs->SwReg43.sw_binit_rlist_b3  );
	printf("SwReg43.sw_binit_rlist_f4     : 5  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg43), regs->SwReg43.sw_binit_rlist_f4  );
	printf("SwReg43.sw_binit_rlist_b4     : 5  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg43), regs->SwReg43.sw_binit_rlist_b4  );
	printf("SwReg43.sw_binit_rlist_f5     : 5  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg43), regs->SwReg43.sw_binit_rlist_f5  );
	printf("SwReg43.sw_binit_rlist_b5     : 5  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg43), regs->SwReg43.sw_binit_rlist_b5  );
	printf("SwReg43.reserve0              : 2  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg43), regs->SwReg43.reserve0           );
	printf("SwReg44.sw_binit_rlist_f6     : 5  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg44), regs->SwReg44.sw_binit_rlist_f6  );
	printf("SwReg44.sw_binit_rlist_b6     : 5  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg44), regs->SwReg44.sw_binit_rlist_b6  );
	printf("SwReg44.sw_binit_rlist_f7     : 5  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg44), regs->SwReg44.sw_binit_rlist_f7  );
	printf("SwReg44.sw_binit_rlist_b7     : 5  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg44), regs->SwReg44.sw_binit_rlist_b7  );
	printf("SwReg44.sw_binit_rlist_f8     : 5  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg44), regs->SwReg44.sw_binit_rlist_f8  );
	printf("SwReg44.sw_binit_rlist_b8     : 5  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg44), regs->SwReg44.sw_binit_rlist_b8  );
	printf("SwReg44.reserve0              : 2  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg44), regs->SwReg44.reserve0           );
	printf("SwReg45.sw_binit_rlist_f9     : 5  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg45), regs->SwReg45.sw_binit_rlist_f9  );
	printf("SwReg45.sw_binit_rlist_b9     : 5  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg45), regs->SwReg45.sw_binit_rlist_b9  );
	printf("SwReg45.sw_binit_rlist_f10    : 5  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg45), regs->SwReg45.sw_binit_rlist_f10 );
	printf("SwReg45.sw_binit_rlist_b10    : 5  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg45), regs->SwReg45.sw_binit_rlist_b10 );
	printf("SwReg45.sw_binit_rlist_f11    : 5  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg45), regs->SwReg45.sw_binit_rlist_f11 );
	printf("SwReg45.sw_binit_rlist_b11    : 5  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg45), regs->SwReg45.sw_binit_rlist_b11 );
	printf("SwReg45.reserve0              : 2  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg45), regs->SwReg45.reserve0           );
	printf("SwReg46.sw_binit_rlist_f12    : 5  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg46), regs->SwReg46.sw_binit_rlist_f12 );
	printf("SwReg46.sw_binit_rlist_b12    : 5  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg46), regs->SwReg46.sw_binit_rlist_b12 );
	printf("SwReg46.sw_binit_rlist_f13    : 5  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg46), regs->SwReg46.sw_binit_rlist_f13 );
	printf("SwReg46.sw_binit_rlist_b13    : 5  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg46), regs->SwReg46.sw_binit_rlist_b13 );
	printf("SwReg46.sw_binit_rlist_f14    : 5  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg46), regs->SwReg46.sw_binit_rlist_f14 );
	printf("SwReg46.sw_binit_rlist_b14    : 5  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg46), regs->SwReg46.sw_binit_rlist_b14 );
	printf("SwReg46.reserve0              : 2  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg46), regs->SwReg46.reserve0           );
	printf("SwReg47.sw_binit_rlist_f15    : 5  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg47), regs->SwReg47.sw_binit_rlist_f15 );
	printf("SwReg47.sw_binit_rlist_b15    : 5  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg47), regs->SwReg47.sw_binit_rlist_b15 );
	printf("SwReg47.sw_pinit_rlist_f0     : 5  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg47), regs->SwReg47.sw_pinit_rlist_f0  );
	printf("SwReg47.sw_pinit_rlist_f1     : 5  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg47), regs->SwReg47.sw_pinit_rlist_f1  );
	printf("SwReg47.sw_pinit_rlist_f2     : 5  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg47), regs->SwReg47.sw_pinit_rlist_f2  );
	printf("SwReg47.sw_pinit_rlist_f3     : 5  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg47), regs->SwReg47.sw_pinit_rlist_f3  );
	printf("SwReg47.reserve0              : 2  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg47), regs->SwReg47.reserve0           );
	printf("SwReg48.reserve0              : 15 (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg48), regs->SwReg48.reserve0           );
	printf("SwReg48.sw_startmb_y          : 8  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg48), regs->SwReg48.sw_startmb_y       );
	printf("SwReg48.sw_startmb_x          : 9  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg48), regs->SwReg48.sw_startmb_x       );
	printf("SwReg49.reserve0              : 2  (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg49), regs->SwReg49.reserve0           );
	printf("SwReg49.sw_pred_bc_tap_0_2    : 10 (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg49), regs->SwReg49.sw_pred_bc_tap_0_2 );
	printf("SwReg49.sw_pred_bc_tap_0_1    : 10 (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg49), regs->SwReg49.sw_pred_bc_tap_0_1 );
	printf("SwReg49.sw_pred_bc_tap_0_0    : 10 (struct offset : %02x) → %d\n", offsetof(H264dVdpu1Regs_t, SwReg49), regs->SwReg49.sw_pred_bc_tap_0_0 );
	for (int i=0; i < 5; i++)
		printf("SwReg50_54[%d]                : 32 (struct offset : %02x) → %d\n", i, offsetof(H264dVdpu1Regs_t, SwReg50_54), regs->SwReg50_54[i]);
	printf("SwReg55.sw_apf_threshold      : 14 (struct offset : %02x) →　%d\n", offsetof(H264dVdpu1Regs_t, SwReg55), regs->SwReg55.sw_apf_threshold  );
	printf("SwReg55.sw_refbu2_picid       : 5  (struct offset : %02x) →　%d\n", offsetof(H264dVdpu1Regs_t, SwReg55), regs->SwReg55.sw_refbu2_picid   );
	printf("SwReg55.sw_refbu2_thr         : 12 (struct offset : %02x) →　%d\n", offsetof(H264dVdpu1Regs_t, SwReg55), regs->SwReg55.sw_refbu2_thr     );
	printf("SwReg55.sw_refbu2_buf_e       : 1  (struct offset : %02x) →　%d\n", offsetof(H264dVdpu1Regs_t, SwReg55), regs->SwReg55.sw_refbu2_buf_e   );
	printf("SwReg56                       : 32 (struct offset : %02x) →　%d\n", offsetof(H264dVdpu1Regs_t, SwReg56), regs->SwReg56                   );
	printf("SwReg57.sw_stream_len_hi      : 1  (struct offset : %02x) →　%d\n", offsetof(H264dVdpu1Regs_t, SwReg57), regs->SwReg57.sw_stream_len_hi  );
	printf("SwReg57.sw_inter_dblspeed     : 1  (struct offset : %02x) →　%d\n", offsetof(H264dVdpu1Regs_t, SwReg57), regs->SwReg57.sw_inter_dblspeed );
	printf("SwReg57.sw_intra_dblspeed     : 1  (struct offset : %02x) →　%d\n", offsetof(H264dVdpu1Regs_t, SwReg57), regs->SwReg57.sw_intra_dblspeed );
	printf("SwReg57.sw_intra_dbl3t        : 1  (struct offset : %02x) →　%d\n", offsetof(H264dVdpu1Regs_t, SwReg57), regs->SwReg57.sw_intra_dbl3t    );
	printf("SwReg57.sw_paral_bus          : 1  (struct offset : %02x) →　%d\n", offsetof(H264dVdpu1Regs_t, SwReg57), regs->SwReg57.sw_paral_bus      );
	printf("SwReg57.sw_axiwr_sel          : 1  (struct offset : %02x) →　%d\n", offsetof(H264dVdpu1Regs_t, SwReg57), regs->SwReg57.sw_axiwr_sel      );
	printf("SwReg57.sw_pref_sigchan       : 1  (struct offset : %02x) →　%d\n", offsetof(H264dVdpu1Regs_t, SwReg57), regs->SwReg57.sw_pref_sigchan   );
	printf("SwReg57.sw_cache_en           : 1  (struct offset : %02x) →　%d\n", offsetof(H264dVdpu1Regs_t, SwReg57), regs->SwReg57.sw_cache_en       );
	printf("SwReg57.reserve0              : 24 (struct offset : %02x) →　%d\n", offsetof(H264dVdpu1Regs_t, SwReg57), regs->SwReg57.reserve0          );
	for (int i=0; i < 43; i++)
		printf("SwReg58_100[%d]               : 32 (struct offset : %02x) → %d\n", i, offsetof(H264dVdpu1Regs_t, SwReg58_100), regs->SwReg58_100[i]);
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		print_usage(argv[0]);
		exit(-EINVAL);
	}

	char const * __restrict const file_name =
		argv[1];


	int fd = open(file_name, O_RDONLY);
	if (fd < 0)
		exit(errno); // In this case, fd is 

	struct stat filestats;
	fstat(fd, &filestats);

	H264dVdpu1Regs_t regs;
	ssize_t const read_bytes = 
		read(fd, (uint8_t * __restrict) &regs, sizeof(regs));

	print_regs(&regs);

	if(filestats.st_size > sizeof(regs))
	{
		fprintf(stderr,
			"\n\n[NOTE]\n"
			"The file contains way more than expected.\n"
			"Expected %lu bytes. The file is %zu bytes long\n",
			sizeof(regs), filestats.st_size);
	}
	if(read_bytes < sizeof(regs))
	{
		fprintf(stderr,
			"\n\n[NOTE]\n"
			"Incomplete reg file...\n"
			"Expected %lu bytes, got %zu bytes\n"
			"Expect garbage output...\n",
			sizeof(regs), read_bytes);
	}

out:
	close(fd);
	return 0;
}
