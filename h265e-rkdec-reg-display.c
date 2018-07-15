#include <stdio.h>

struct H265d_REGS {
    struct swreg_id {
        RK_U32    minor_ver           : 8  ;
        RK_U32    major_ver           : 8  ;
        RK_U32    prod_num            : 16 ;
    } sw_id;

    struct swreg_int {
        RK_U32    sw_dec_e                     : 1  ;
        RK_U32    sw_dec_clkgate_e             : 1  ;
        RK_U32    reserve0                     : 2  ;
        RK_U32    sw_dec_irq_dis               : 1  ;
        RK_U32    sw_dec_timeout_e             : 1  ;
        RK_U32    sw_buf_empty_en              : 1  ;
        RK_U32    reserve1                     : 1  ;
        RK_U32    sw_dec_irq                   : 1  ;
        RK_U32    sw_dec_irq_raw               : 1  ;
        RK_U32    reserve2                     : 2  ;
        RK_U32    sw_dec_rdy_sta               : 1  ;
        RK_U32    sw_dec_bus_sta               : 1  ;
        RK_U32    sw_dec_error_sta             : 1  ;
        RK_U32    sw_dec_empty_sta             : 1  ;
        RK_U32    reserve4                     : 4  ;
        RK_U32    sw_softrst_en_p              : 1  ;
        RK_U32    sw_force_softreset_valid     : 1 ;
        RK_U32    sw_softreset_rdy             : 1  ;
        RK_U32    sw_wr_ddr_align_en           : 1;
        RK_U32    sw_scl_down_en               : 1;
        RK_U32    sw_allow_not_wr_unref_bframe : 1;
    } sw_interrupt; ///<- zrh: do nothing in C Model

    struct swreg_sysctrl {
        RK_U32    sw_in_endian        : 1  ;
        RK_U32    sw_in_swap32_e      : 1  ;
        RK_U32    sw_in_swap64_e      : 1  ;
        RK_U32    sw_str_endian       : 1  ;
        RK_U32    sw_str_swap32_e     : 1  ;
        RK_U32    sw_str_swap64_e     : 1  ;
        RK_U32    sw_out_endian       : 1  ;
        RK_U32    sw_out_swap32_e     : 1  ;
        RK_U32    sw_out_cbcr_swap    : 1  ;
        RK_U32    reserve             : 1  ;
        RK_U32    sw_rlc_mode_direct_write : 1;
        RK_U32    sw_rlc_mode         : 1  ;
        RK_U32    sw_strm_start_bit   : 7  ;
    } sw_sysctrl; ///<- zrh: do nothing in C Model

    struct swreg_pic {
        RK_U32    sw_y_hor_virstride  : 9 ;
        RK_U32    reserve             : 3 ;
        RK_U32    sw_uv_hor_virstride : 9 ;
        RK_U32    sw_slice_num        : 8 ;
    } sw_picparameter;

    RK_U32        sw_strm_rlc_base        ;///<- zrh: do nothing in C Model
    RK_U32        sw_stream_len           ;///<- zrh: do nothing in C Model
    RK_U32        sw_cabactbl_base        ;///<- zrh: do nothing in C Model
    RK_U32        sw_decout_base          ;
    RK_U32        sw_y_virstride          ;
    RK_U32        sw_yuv_virstride        ;
    RK_U32        sw_refer_base[15]       ;
    RK_S32        sw_refer_poc[15]        ;
    RK_S32        sw_cur_poc              ;
    RK_U32        sw_rlcwrite_base        ;///<- zrh: do nothing in C Model
    RK_U32        sw_pps_base             ;///<- zrh: do nothing in C Model
    RK_U32        sw_rps_base             ;///<- zrh: do nothing in C Model
    RK_U32        cabac_error_en          ;///<- zrh add
    RK_U32        cabac_error_status      ;///<- zrh add

    struct cabac_error_ctu      {
        RK_U32   sw_cabac_error_ctu_xoffset    : 8;
        RK_U32   sw_cabac_error_ctu_yoffset    : 8;
        RK_U32   sw_streamfifo_space2full      : 7;
        RK_U32   reversed0                     : 9;
    } cabac_error_ctu;

    struct sao_ctu_position     {
        RK_U32   sw_saowr_xoffset              : 9;
        RK_U32   reversed0                     : 7;
        RK_U32   sw_saowr_yoffset             : 10;
        RK_U32   reversed1                     : 6;
    } sao_ctu_position;

    RK_U32        sw_ref_valid;   //this is not same with hardware
    RK_U32        sw_refframe_index[15];

    RK_U32 reg_not_use0[16];

    RK_U32        performance_cycle;
    RK_U32        axi_ddr_rdata;
    RK_U32        axi_ddr_wdata;
    RK_U32        fpgadebug_reset;
    RK_U32        reserve[9];

    RK_U32 extern_error_en;

} ;

unsigned int regs[94] = {
	0x00000000, 0x00000021, 0x00000000, 0x00310110,
	0x0000001d, 0x00000120, 0x00000017, 0x00000022,
	0x00073d00, 0x000adb80, 0x00003c2f, 0x0000002e,
	0x0000002c, 0x00000028, 0x00000022, 0x00000022,
	0x00000022, 0x00000022, 0x00000022, 0x00000022,
	0x00000022, 0x00000022, 0x00000022, 0x00000022,
	0x00000022, 0x00000046, 0x00000048, 0x00000044,
	0x00000040, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000047, 0x00000000, 0x00000019, 0x0000001a,
	0xfdfffffd, 0x00000000, 0x00000000, 0x00000000,
	0x0000000f, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000};

struct H265d_REGS * interpreted_regs = (struct H265d_REGS *) regs;

int main() {

	fprintf(stderr, "struct {\n"
		"    struct swreg_id {\n"
		"        RK_U32    minor_ver           : %d ;\n"
		"        RK_U32    major_ver           : %d ;\n"
		"        RK_U32    prod_num            : %d ;\n"
		"    } sw_id;\n"
		"\n"
		"    struct swreg_int {\n"
		"        RK_U32    sw_dec_e                     : %d  ;\n"
		"        RK_U32    sw_dec_clkgate_e             : %d  ;\n"
		"        RK_U32    reserve0                     : %d  ;\n"
		"        RK_U32    sw_dec_irq_dis               : %d  ;\n"
		"        RK_U32    sw_dec_timeout_e             : %d  ;\n"
		"        RK_U32    sw_buf_empty_en              : %d  ;\n"
		"        RK_U32    reserve1                     : %d  ;\n"
		"        RK_U32    sw_dec_irq                   : %d  ;\n"
		"        RK_U32    sw_dec_irq_raw               : %d  ;\n"
		"        RK_U32    reserve2                     : %d  ;\n"
		"        RK_U32    sw_dec_rdy_sta               : %d  ;\n"
		"        RK_U32    sw_dec_bus_sta               : %d  ;\n"
		"        RK_U32    sw_dec_error_sta             : %d  ;\n"
		"        RK_U32    sw_dec_empty_sta             : %d  ;\n"
		"        RK_U32    reserve4                     : %d  ;\n"
		"        RK_U32    sw_softrst_en_p              : %d  ;\n"
		"        RK_U32    sw_force_softreset_valid     : %d ;\n"
		"        RK_U32    sw_softreset_rdy             : %d  ;\n"
		"        RK_U32    sw_wr_ddr_align_en           : %d;\n"
		"        RK_U32    sw_scl_down_en               : %d;\n"
		"        RK_U32    sw_allow_not_wr_unref_bframe : %d;\n"
		"    } sw_interrupt; ///<- zrh: do nothing in C Model\n"
		"\n"
		"    struct swreg_sysctrl {\n"
		"        RK_U32    sw_in_endian             : %d  ;\n"
		"        RK_U32    sw_in_swap32_e           : %d  ;\n"
		"        RK_U32    sw_in_swap64_e           : %d  ;\n"
		"        RK_U32    sw_str_endian            : %d  ;\n"
		"        RK_U32    sw_str_swap32_e          : %d  ;\n"
		"        RK_U32    sw_str_swap64_e          : %d  ;\n"
		"        RK_U32    sw_out_endian            : %d  ;\n"
		"        RK_U32    sw_out_swap32_e          : %d  ;\n"
		"        RK_U32    sw_out_cbcr_swap         : %d  ;\n"
		"        RK_U32    reserve                  : %d  ;\n"
		"        RK_U32    sw_rlc_mode_direct_write : %d;\n"
		"        RK_U32    sw_rlc_mode              : %d  ;\n"
		"        RK_U32    sw_strm_start_bit        : %d  ;\n"
		"    } sw_sysctrl; ///<- zrh: do nothing in C Model\n"
		"\n"
		"    struct swreg_pic {\n"
		"        RK_U32    sw_y_hor_virstride  : %d ;\n"
		"        RK_U32    reserve             : %d ;\n"
		"        RK_U32    sw_uv_hor_virstride : %d ;\n"
		"        RK_U32    sw_slice_num        : %d ;\n"
		"    } sw_picparameter;\n"
		"\n"
		"    RK_U32        sw_strm_rlc_base    : %d   ;///<- zrh: do nothing in C Model\n"
		"    RK_U32        sw_stream_len       : %d   ;///<- zrh: do nothing in C Model\n"
		"    RK_U32        sw_cabactbl_base    : %d   ;///<- zrh: do nothing in C Model\n"
		"    RK_U32        sw_decout_base      : %d   ;\n"
		"    RK_U32        sw_y_virstride      : %d   ;\n"
		"    RK_U32        sw_yuv_virstride    : %d   ;\n"
		"    RK_U32        sw_refer_base[15]   : %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d   ;\n"
		"    RK_S32        sw_refer_poc[15]    : %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d   ;\n"
		"    RK_S32        sw_cur_poc          : %d   ;\n"
		"    RK_U32        sw_rlcwrite_base    : %d   ;///<- zrh: do nothing in C Model\n"
		"    RK_U32        sw_pps_base         : %d   ;///<- zrh: do nothing in C Model\n"
		"    RK_U32        sw_rps_base         : %d   ;///<- zrh: do nothing in C Model\n"
		"    RK_U32        cabac_error_en      : %d   ;///<- zrh add\n"
		"    RK_U32        cabac_error_status  : %d   ;///<- zrh add\n"
		"\n"
		"    struct cabac_error_ctu      {\n"
		"        RK_U32   sw_cabac_error_ctu_xoffset    : %d;\n"
		"        RK_U32   sw_cabac_error_ctu_yoffset    : %d;\n"
		"        RK_U32   sw_streamfifo_space2full      : %d;\n"
		"        RK_U32   reversed0                     : %d;\n"
		"    } cabac_error_ctu;\n"
		"\n"
		"    struct sao_ctu_position     {\n"
		"        RK_U32   sw_saowr_xoffset              : %d;\n"
		"        RK_U32   reversed0                     : %d;\n"
		"        RK_U32   sw_saowr_yoffset              : %d;\n"
		"        RK_U32   reversed1                     : %d;\n"
		"    } sao_ctu_position;\n"
		"\n"
		"    RK_U32        sw_ref_valid          : %d;   //this is not same with hardware\n"
		"    RK_U32        sw_refframe_index[15] : %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d;\n"
		"\n"
		"    RK_U32        reg_not_use0[16] : %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d;\n"
		"\n"
		"    RK_U32        performance_cycle : %d;\n"
		"    RK_U32        axi_ddr_rdata     : %d;\n"
		"    RK_U32        axi_ddr_wdata     : %d;\n"
		"    RK_U32        fpgadebug_reset   : %d;\n"
		"    RK_U32        reserve[9]        : %d,%d,%d,%d,%d,%d,%d,%d,%d;\n"
		"\n"                         
		"    RK_U32        extern_error_en   : %D;\n"
		"\n"
		"} H265d_REGS_t;\n",
		interpreted_regs->swreg_id.minor_ver,
		interpreted_regs->swreg_id.major_ver,
		interpreted_regs->swreg_id.prod_num,
		interpreted_regs->swreg_int.sw_dec_e                    ,
		interpreted_regs->swreg_int.sw_dec_clkgate_e            ,
		interpreted_regs->swreg_int.reserve0                    ,
		interpreted_regs->swreg_int.sw_dec_irq_dis              ,
		interpreted_regs->swreg_int.sw_dec_timeout_e            ,
		interpreted_regs->swreg_int.sw_buf_empty_en             ,
		interpreted_regs->swreg_int.reserve1                    ,
		interpreted_regs->swreg_int.sw_dec_irq                  ,
		interpreted_regs->swreg_int.sw_dec_irq_raw              ,
		interpreted_regs->swreg_int.reserve2                    ,
		interpreted_regs->swreg_int.sw_dec_rdy_sta              ,
		interpreted_regs->swreg_int.sw_dec_bus_sta              ,
		interpreted_regs->swreg_int.sw_dec_error_sta            ,
		interpreted_regs->swreg_int.sw_dec_empty_sta            ,
		interpreted_regs->swreg_int.reserve4                    ,
		interpreted_regs->swreg_int.sw_softrst_en_p             ,
		interpreted_regs->swreg_int.sw_force_softreset_valid    ,
		interpreted_regs->swreg_int.sw_softreset_rdy            ,
		interpreted_regs->swreg_int.sw_wr_ddr_align_en          ,
		interpreted_regs->swreg_int.sw_scl_down_en              ,
		interpreted_regs->swreg_int.sw_allow_not_wr_unref_bframe,
		interpreted_regs->swreg_sysctrl.sw_in_endian             ,
		interpreted_regs->swreg_sysctrl.sw_in_swap32_e           ,
		interpreted_regs->swreg_sysctrl.sw_in_swap64_e           ,
		interpreted_regs->swreg_sysctrl.sw_str_endian            ,
		interpreted_regs->swreg_sysctrl.sw_str_swap32_e          ,
		interpreted_regs->swreg_sysctrl.sw_str_swap64_e          ,
		interpreted_regs->swreg_sysctrl.sw_out_endian            ,
		interpreted_regs->swreg_sysctrl.sw_out_swap32_e          ,
		interpreted_regs->swreg_sysctrl.sw_out_cbcr_swap         ,
		interpreted_regs->swreg_sysctrl.reserve                  ,
		interpreted_regs->swreg_sysctrl.sw_rlc_mode_direct_write ,
		interpreted_regs->swreg_sysctrl.sw_rlc_mode              ,
		interpreted_regs->swreg_sysctrl.sw_strm_start_bit        ,
		interpreted_regs->swreg_pic.sw_y_hor_virstride ,
		interpreted_regs->swreg_pic.reserve            ,
		interpreted_regs->swreg_pic.sw_uv_hor_virstride,
		interpreted_regs->swreg_pic.sw_slice_num       ,
		interpreted_regs->sw_strm_rlc_base  ,
		interpreted_regs->sw_stream_len     ,
		interpreted_regs->sw_cabactbl_base  ,
		interpreted_regs->sw_decout_base    ,
		interpreted_regs->sw_y_virstride    ,
		interpreted_regs->sw_yuv_virstride  ,
		interpreted_regs->sw_refer_base[0], interpreted_regs->sw_refer_base[1], interpreted_regs->sw_refer_base[2], interpreted_regs->sw_refer_base[3], interpreted_regs->sw_refer_base[4], interpreted_regs->sw_refer_base[5], interpreted_regs->sw_refer_base[6], interpreted_regs->sw_refer_base[7], interpreted_regs->sw_refer_base[8], interpreted_regs->sw_refer_base[9], interpreted_regs->sw_refer_base[10], interpreted_regs->sw_refer_base[11], interpreted_regs->sw_refer_base[12], interpreted_regs->sw_refer_base[13], interpreted_regs->sw_refer_base[14],
		interpreted_regs->sw_refer_poc[0], interpreted_regs->sw_refer_poc[1], interpreted_regs->sw_refer_poc[2], interpreted_regs->sw_refer_poc[3], interpreted_regs->sw_refer_poc[4], interpreted_regs->sw_refer_poc[5], interpreted_regs->sw_refer_poc[6], interpreted_regs->sw_refer_poc[7], interpreted_regs->sw_refer_poc[8], interpreted_regs->sw_refer_poc[9], interpreted_regs->sw_refer_poc[10], interpreted_regs->sw_refer_poc[11], interpreted_regs->sw_refer_poc[12], interpreted_regs->sw_refer_poc[13], interpreted_regs->sw_refer_poc[14],
		interpreted_regs->sw_cur_poc        ,
		interpreted_regs->sw_rlcwrite_base  ,
		interpreted_regs->sw_pps_base       ,
		interpreted_regs->sw_rps_base       ,
		interpreted_regs->cabac_error_en    ,
		interpreted_regs->cabac_error_status,
		interpreted_regs->cabac_error_ctu.sw_cabac_error_ctu_xoffset,
		interpreted_regs->cabac_error_ctu.sw_cabac_error_ctu_yoffset,
		interpreted_regs->cabac_error_ctu.sw_streamfifo_space2full  ,
		interpreted_regs->cabac_error_ctu.reversed0,
		interpreted_regs->sao_ctu_position.sw_saowr_xoffset,
		interpreted_regs->sao_ctu_position.reversed0       ,
		interpreted_regs->sao_ctu_position.sw_saowr_yoffset,
		interpreted_regs->sao_ctu_position.reversed1     ,
		interpreted_regs->sw_ref_valid         ,
		interpreted_regs->frame_index[0], interpreted_regs->sw_refframe_index[1], interpreted_regs->sw_refframe_index[2], interpreted_regs->sw_refframe_index[3], interpreted_regs->sw_refframe_index[4], interpreted_regs->sw_refframe_index[5], interpreted_regs->sw_refframe_index[6], interpreted_regs->sw_refframe_index[7], interpreted_regs->sw_refframe_index[8], interpreted_regs->sw_refframe_index[9], interpreted_regs->sw_refframe_index[10], interpreted_regs->sw_refframe_index[11], interpreted_regs->sw_refframe_index[12], interpreted_regs->sw_refframe_index[13], interpreted_regs->sw_refframe_index[14],
		interpreted_regs->reg_not_use0[0], interpreted_regs->reg_not_use0[1], interpreted_regs->reg_not_use0[2], interpreted_regs->reg_not_use0[3], interpreted_regs->reg_not_use0[4], interpreted_regs->reg_not_use0[5], interpreted_regs->reg_not_use0[6], interpreted_regs->reg_not_use0[7], interpreted_regs->reg_not_use0[8], interpreted_regs->reg_not_use0[9], interpreted_regs->reg_not_use0[10], interpreted_regs->reg_not_use0[11], interpreted_regs->reg_not_use0[12], interpreted_regs->reg_not_use0[13], interpreted_regs->reg_not_use0[14], interpreted_regs->reg_not_use0[15],
		interpreted_regs->performance_cycle,
		interpreted_regs->axi_ddr_rdata,    
		interpreted_regs->axi_ddr_wdata,    
		interpreted_regs->fpgadebug_reset,  
		interpreted_regs->reserve[0], interpreted_regs->reserve[1], interpreted_regs->reserve[2], interpreted_regs->reserve[3], interpreted_regs->reserve[4], interpreted_regs->reserve[5], interpreted_regs->reserve[6], interpreted_regs->reserve[7], interpreted_regs->reserve[8],
		interpreted_regs->extern_error_en  
   		);

	return 0;
}
