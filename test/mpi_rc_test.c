/*
 * Copyright 2015 Rockchip Electronics Co. LTD
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#if defined(_WIN32)
#include "vld.h"
#endif

#define MODULE_TAG "mpi_rc_test"

#include <string.h>
#include <math.h>
#include "rk_mpi.h"

#include "mpp_log.h"
#include "mpp_env.h"
#include "mpp_time.h"
#include "mpp_common.h"
#include "mpp_mem.h"

#include "utils.h"

#include "vpu_api.h"

#define MPI_RC_IO_COUNT             1
#define MPI_RC_FILE_NAME_LEN        256

#define MPI_BIT_DEPTH               8
#define MPI_PIXEL_MAX               ((1 << MPI_BIT_DEPTH) - 1)

#define MPI_RC_ITEM_BPS     0x00000001
#define MPI_RC_ITEM_FPS     0x00000002
#define MPI_RC_ITEM_GOP     0x00000004
#define MPI_RC_ITEM_FORCE_I 0x00000008
#define MPI_RC_ITEM_ROI     0x00000010

typedef RK_U8 pixel;

typedef struct {
    RK_U32          bitrate;
} MpiRcCfg;

typedef struct {
    double          psnr_y;
    double          ssim_y;
    RK_U32          avg_bitrate; // Every sequence, byte per second
    RK_U32          ins_bitrate; // Every second, byte per second
    RK_U32          frame_size; // Every frame, byte
} MpiRcStat;

typedef struct {
    FILE *fp_input;
    FILE *fp_enc_out;
    FILE *fp_dec_out;
    FILE *fp_stat;
} MpiRcFile;

typedef struct {
    char            file_input[MPI_RC_FILE_NAME_LEN];
    char            file_enc_out[MPI_RC_FILE_NAME_LEN];
    char            file_dec_out[MPI_RC_FILE_NAME_LEN];
    char            file_stat[MPI_RC_FILE_NAME_LEN];
    MppCodingType   type;
    RK_U32          width;
    RK_U32          height;
    MppFrameFormat  format;
    RK_U32          debug;
    RK_U32          num_frames;
    RK_U32          item_flag;

    RK_U32          have_input;
    RK_U32          have_enc_out;
    RK_U32          have_dec_out;
    RK_U32          have_stat_out;
} MpiRcTestCmd;

typedef struct {
    MpiRcTestCmd    cmd;
    MpiRcCfg        cfg;
    MpiRcStat       stat;
    MpiRcFile       file;
    RK_U8           *com_buf;
} MpiRcTestCtx;

static OptionInfo mpi_rc_cmd[] = {
    {"i",               "input_file",           "input bitstream file"},
    {"o",               "output_file",          "output bitstream file, "},
    {"w",               "width",                "the width of input picture"},
    {"h",               "height",               "the height of input picture"},
    {"f",               "format",               "the format of input picture"},
    {"t",               "type",                 "output stream coding type"},
    {"n",               "max frame number",     "max encoding frame number"},
    {"d",               "debug",                "debug flag"},
    {"s",               "stat_file",            "stat output file name"},
    {"c",               "rc test item",         "rc test item flags, one bit each item: roi|force_intra|gop|fps|bps"},
};

static MPP_RET mpi_rc_read_yuv_image(RK_U8 *buf, MppEncPrepCfg *prep_cfg, FILE *fp)
{
    MPP_RET ret = MPP_OK;
    RK_U32 read_size;
    RK_U32 row = 0;
    RK_U32 width        = prep_cfg->width;
    RK_U32 height       = prep_cfg->height;
    RK_U32 hor_stride   = prep_cfg->hor_stride;
    RK_U32 ver_stride   = prep_cfg->ver_stride;
    MppFrameFormat fmt  = prep_cfg->format;
    RK_U8 *buf_y = buf;
    RK_U8 *buf_u = buf_y + hor_stride * ver_stride; // NOTE: diff from gen_yuv_image
    RK_U8 *buf_v = buf_u + hor_stride * ver_stride / 4; // NOTE: diff from gen_yuv_image

    switch (fmt) {
    case MPP_FMT_YUV420SP : {
        for (row = 0; row < height; row++) {
            read_size = fread(buf_y + row * hor_stride, 1, width, fp);
            if (read_size != width) {
                mpp_err_f("read ori yuv file luma failed");
                ret  = MPP_NOK;
                goto err;
            }
        }

        for (row = 0; row < height / 2; row++) {
            read_size = fread(buf_u + row * hor_stride, 1, width, fp);
            if (read_size != width) {
                mpp_err_f("read ori yuv file cb failed");
                ret  = MPP_NOK;
                goto err;
            }
        }
    } break;
    case MPP_FMT_YUV420P : {
        for (row = 0; row < height; row++) {
            read_size = fread(buf_y + row * hor_stride, 1, width, fp);
            if (read_size != width) {
                mpp_err_f("read ori yuv file luma failed");
                ret  = MPP_NOK;
                goto err;
            }
        }

        for (row = 0; row < height / 2; row++) {
            read_size = fread(buf_u + row * hor_stride / 2, 1, width / 2, fp);
            if (read_size != width / 2) {
                mpp_err_f("read ori yuv file cb failed");
                ret  = MPP_NOK;
                goto err;
            }
        }

        for (row = 0; row < height / 2; row++) {
            read_size = fread(buf_v + row * hor_stride / 2, 1, width / 2, fp);
            if (read_size != width / 2) {
                mpp_err_f("read ori yuv file cr failed");
                ret  = MPP_NOK;
                goto err;
            }
        }
    } break;
    default : {
        mpp_err_f("read image do not support fmt %d\n", fmt);
        ret = MPP_NOK;
    } break;
    }

err:

    return ret;
}

static MPP_RET gen_yuv_image(RK_U8 *buf, MppEncPrepCfg *prep_cfg, RK_U32 frame_count)
{
    MPP_RET ret = MPP_OK;
    RK_U32 width        = prep_cfg->width;
    RK_U32 height       = prep_cfg->height;
    RK_U32 hor_stride   = prep_cfg->hor_stride;
    RK_U32 ver_stride   = prep_cfg->ver_stride;
    MppFrameFormat fmt  = prep_cfg->format;
    RK_U8 *buf_y = buf;
    RK_U8 *buf_c = buf + hor_stride * ver_stride;
    RK_U32 x, y;

    switch (fmt) {
    case MPP_FMT_YUV420SP : {
        RK_U8 *p = buf_y;

        for (y = 0; y < height; y++, p += hor_stride) {
            for (x = 0; x < width; x++) {
                p[x] = x + y + frame_count * 3;
            }
        }

        p = buf_c;
        for (y = 0; y < height / 2; y++, p += hor_stride) {
            for (x = 0; x < width / 2; x++) {
                p[x * 2 + 0] = 128 + y + frame_count * 2;
                p[x * 2 + 1] = 64  + x + frame_count * 5;
            }
        }
    } break;
    case MPP_FMT_YUV420P : {
        RK_U8 *p = buf_y;

        for (y = 0; y < height; y++, p += hor_stride) {
            for (x = 0; x < width; x++) {
                p[x] = x + y + frame_count * 3;
            }
        }

        p = buf_c;
        for (y = 0; y < height / 2; y++, p += hor_stride / 2) {
            for (x = 0; x < width / 2; x++) {
                p[x] = 128 + y + frame_count * 2;
            }
        }

        p = buf_c + hor_stride * ver_stride / 4;
        for (y = 0; y < height / 2; y++, p += hor_stride / 2) {
            for (x = 0; x < width / 2; x++) {
                p[x] = 64 + x + frame_count * 5;
            }
        }
    } break;
    default : {
        mpp_err_f("filling function do not support type %d\n", fmt);
        ret = MPP_NOK;
    } break;
    }

    return ret;
}

static void mpi_rc_deinit(MpiRcTestCtx *ctx)
{
    MpiRcFile *file = &ctx->file;

    if (file->fp_enc_out) {
        fclose(file->fp_enc_out);
        file->fp_enc_out = NULL;
    }
    if (file->fp_dec_out) {
        fclose(file->fp_dec_out);
        file->fp_dec_out = NULL;
    }

    if (file->fp_stat) {
        fclose(file->fp_stat);
        file->fp_stat = NULL;
    }

    if (file->fp_input) {
        fclose(file->fp_input);
        file->fp_input = NULL;
    }

    MPP_FREE(ctx->com_buf);
}

static MPP_RET mpi_rc_init(MpiRcTestCtx *ctx)
{
    MPP_RET ret = MPP_OK;
    MpiRcTestCmd *cmd = &ctx->cmd;
    MpiRcFile *file = &ctx->file;

    if (cmd->have_input) {
        file->fp_input = fopen(cmd->file_input, "rb");
        if (NULL == file->fp_input) {
            mpp_err("failed to open input file %s\n", cmd->file_input);
            mpp_err("create default yuv image for test\n");
        }
    }

    if (cmd->have_enc_out) {
        file->fp_enc_out = fopen(cmd->file_enc_out, "w+b");
        if (NULL == file->fp_enc_out) {
            mpp_err("failed to open enc output file %s\n", cmd->file_enc_out);
            ret = MPP_ERR_OPEN_FILE;
            goto err;
        }
    }

    if (cmd->have_dec_out) {
        file->fp_dec_out = fopen(cmd->file_dec_out, "w+b");
        if (NULL == file->fp_dec_out) {
            mpp_err("failed to open dec output file %s\n", cmd->file_dec_out);
            ret = MPP_ERR_OPEN_FILE;
            goto err;
        }
    }

    if (cmd->have_stat_out) {
        file->fp_stat = fopen(cmd->file_stat, "w+b");
        if (NULL == file->fp_stat) {
            mpp_err("failed to open stat file %s\n", cmd->file_stat);
            ret = MPP_ERR_OPEN_FILE;
            goto err;
        }
        fprintf(file->fp_stat, "frame,size(bit),psnr,ssim,ins_bitrate(bit/s),avg_bitrate(bit/s)\n");
    }

    ctx->com_buf = mpp_malloc(RK_U8, cmd->width * cmd->height * 2);
    if (ctx->com_buf == NULL) {
        mpp_err_f("ctx->com_buf malloc failed");
        ret = MPP_NOK;
        goto err;
    }

err:

    return ret;
}

static MPP_RET mpi_rc_cmp_frame(MppFrame frame_in, MppFrame frame_out)
{
    RK_U8 *enc_buf = (RK_U8 *)mpp_buffer_get_ptr(mpp_frame_get_buffer(frame_in));
    RK_U8 *dec_buf = (RK_U8 *)mpp_buffer_get_ptr(mpp_frame_get_buffer(frame_out));
    RK_U32 enc_width  = mpp_frame_get_width(frame_in);
    RK_U32 enc_height  = mpp_frame_get_height(frame_in);
    RK_U32 dec_width  = mpp_frame_get_width(frame_out);
    RK_U32 dec_height  = mpp_frame_get_height(frame_out);

    if (!enc_buf) {
        mpp_err_f("enc buf is NULL");
        return MPP_NOK;
    }
    if (!dec_buf) {
        mpp_err_f("dec buf is NULL");
        return MPP_NOK;
    }

    if (enc_width != dec_width) {
        mpp_err_f("enc_width %d != dec_width %d", enc_width, dec_width);
        return MPP_NOK;
    }

    if (enc_height != dec_height) {
        mpp_err_f("enc_height %d != dec_height %d", enc_height, dec_height);
        return MPP_NOK;
    }

    return MPP_OK;
}

static void mpi_rc_calc_psnr(MpiRcStat *stat, MppFrame frame_in, MppFrame frame_out)
{
    RK_U32 x, y;
    RK_S32 tmp;
    double ssd_y = 0.0;
    double mse_y = 0.0;
    double psnr_y = 0.0;

    RK_U32 width  = mpp_frame_get_width(frame_in);
    RK_U32 height  = mpp_frame_get_height(frame_in);
    RK_U32 luma_size = width * height;
    RK_U32 enc_hor_stride = mpp_frame_get_hor_stride(frame_in);
    RK_U32 dec_hor_stride = mpp_frame_get_hor_stride(frame_out);
    RK_U8 *enc_buf = (RK_U8 *)mpp_buffer_get_ptr(mpp_frame_get_buffer(frame_in));
    RK_U8 *dec_buf = (RK_U8 *)mpp_buffer_get_ptr(mpp_frame_get_buffer(frame_out));
    RK_U8 *enc_buf_y = enc_buf;
    RK_U8 *dec_buf_y = dec_buf;

    for (y = 0 ; y < height; y++) {
        for (x = 0; x < width; x++) {
            tmp = enc_buf_y[x + y * enc_hor_stride] - dec_buf_y[x + y * dec_hor_stride];
            tmp *= tmp;
            ssd_y += tmp;
        }
    }
    // NOTE: should be 65025.0 rather than 65025, because 65025*luma_size may exceed
    // 1 << 32.
    mse_y = ssd_y / (65025.0 * luma_size); // 65025=255*255
    if (mse_y <= 0.0000000001)
        psnr_y = 100;
    else
        psnr_y = -10.0 * log10f(mse_y);

    stat->psnr_y = psnr_y;
}

static float ssim_end1( int s1, int s2, int ss, int s12  )
{
    /* Maximum value for 10-bit is: ss*64 = (2^10-1)^2*16*4*64 = 4286582784, which will overflow in some cases.
     * s1*s1, s2*s2, and s1*s2 also obtain this value for edge cases: ((2^10-1)*16*4)^2 = 4286582784.
     * Maximum value for 9-bit is: ss*64 = (2^9-1)^2*16*4*64 = 1069551616, which will not overflow. */
    static const int ssim_c1 = (int)(.01 * .01 * MPI_PIXEL_MAX * MPI_PIXEL_MAX * 64 + .5);
    static const int ssim_c2 = (int)(.03 * .03 * MPI_PIXEL_MAX * MPI_PIXEL_MAX * 64 * 63 + .5);
    int fs1 = s1;
    int fs2 = s2;
    int fss = ss;
    int fs12 = s12;
    int vars = fss * 64 - fs1 * fs1 - fs2 * fs2;
    int covar = fs12 * 64 - fs1 * fs2;

    return (float)(2 * fs1 * fs2 + ssim_c1) * (float)(2 * covar + ssim_c2)
           / ((float)(fs1 * fs1 + fs2 * fs2 + ssim_c1) * (float)(vars + ssim_c2));
}

static float ssim_end4( int sum0[5][4], int sum1[5][4], int width  )
{
    double ssim = 0.0;
    int i  = 0;
    for (i = 0; i < width; i++  )
        ssim += ssim_end1( sum0[i][0] + sum0[i + 1][0] + sum1[i][0] + sum1[i + 1][0],
                           sum0[i][1] + sum0[i + 1][1] + sum1[i][1] + sum1[i + 1][1],
                           sum0[i][2] + sum0[i + 1][2] + sum1[i][2] + sum1[i + 1][2],
                           sum0[i][3] + sum0[i + 1][3] + sum1[i][3] + sum1[i + 1][3] );
    return ssim;
}

static void ssim_4x4x2_core( const pixel *pix1, intptr_t stride1,
                             const pixel *pix2, intptr_t stride2,
                             int sums[2][4]  )
{
    int a = 0;
    int b = 0;
    int x = 0;
    int y = 0;
    int z = 0;
    for (z = 0; z < 2; z++  ) {
        uint32_t s1 = 0, s2 = 0, ss = 0, s12 = 0;
        for (y = 0; y < 4; y++  )
            for ( x = 0; x < 4; x++  ) {
                a = pix1[x + y * stride1];
                b = pix2[x + y * stride2];
                s1  += a;
                s2  += b;
                ss  += a * a;
                ss  += b * b;
                s12 += a * b;

            }
        sums[z][0] = s1;
        sums[z][1] = s2;
        sums[z][2] = ss;
        sums[z][3] = s12;
        pix1 += 4;
        pix2 += 4;
    }
}


static float calc_ssim(pixel *pix1, RK_U32 stride1,
                       pixel *pix2, RK_U32 stride2,
                       int width, int height, void *buf, int *cnt)
{
    int x = 0;
    int y = 0;
    int z = 0;
    float ssim = 0.0;
    int (*sum0)[4] = buf;
    int (*sum1)[4] = sum0 + (width >> 2) + 3;
    width >>= 2;
    height >>= 2;
    for ( y = 1; y < height; y++  ) {
        for ( ; z <= y; z++  ) {
            MPP_SWAP( void*, sum0, sum1  );
            for ( x = 0; x < width; x += 2  )
                ssim_4x4x2_core( &pix1[4 * (x + z * stride1)], stride1, &pix2[4 * (x + z * stride2)], stride2, &sum0[x]  );

        }
        for ( x = 0; x < width - 1; x += 4  )
            ssim += ssim_end4( sum0 + x, sum1 + x, MPP_MIN(4, width - x - 1)  );

    }
    *cnt = (height - 1) * (width - 1);
    return ssim;
}

static void mpi_rc_calc_ssim(MpiRcTestCtx *ctx, MppFrame frame_in, MppFrame frame_out)
{
    int cnt = 0;
    float ssim = 0;
    MpiRcStat *stat = &ctx->stat;
    RK_U32 width  = mpp_frame_get_width(frame_in);
    RK_U32 height  = mpp_frame_get_height(frame_in);
    RK_U32 enc_hor_stride = mpp_frame_get_hor_stride(frame_in);
    RK_U32 dec_hor_stride = mpp_frame_get_hor_stride(frame_out);
    pixel *enc_buf = (RK_U8 *)mpp_buffer_get_ptr(mpp_frame_get_buffer(frame_in));
    pixel *dec_buf = (RK_U8 *)mpp_buffer_get_ptr(mpp_frame_get_buffer(frame_out));
    pixel *enc_buf_y = enc_buf;
    pixel *dec_buf_y = dec_buf;

    ssim = calc_ssim(enc_buf_y, enc_hor_stride, dec_buf_y, dec_hor_stride, width - 2, height - 2, ctx->com_buf, &cnt);
    ssim /= (float)cnt;

    stat->ssim_y = (double)ssim;

}

static MPP_RET mpi_rc_calc_stat(MpiRcTestCtx *ctx, MppFrame frame_in, MppFrame frame_out)
{
    MPP_RET ret = MPP_OK;
    MpiRcStat *stat = &ctx->stat;

    ret = mpi_rc_cmp_frame(frame_in, frame_out);
    if (MPP_OK != ret) {
        mpp_err_f("mpi_rc_cmp_frame failed ret %d", ret);
        return MPP_NOK;
    }

    mpi_rc_calc_psnr(stat, frame_in, frame_out);
    mpi_rc_calc_ssim(ctx, frame_in, frame_out);

    return ret;
}

static void mpi_rc_log_stat(MpiRcTestCtx *ctx, RK_U32 frame_count, RK_U32 one_second, RK_U32 seq_end)
{
    //static char rc_log_str[1024] = {0};
    MpiRcStat *stat = &ctx->stat;
    MpiRcFile *file  = &ctx->file;
    FILE *fp  = file->fp_stat;

    mpp_log("frame %3d | frame_size %6d bits | psnr %5.2f | ssim %5.5f",
            frame_count, stat->frame_size, stat->psnr_y, stat->ssim_y);
    if (one_second)
        mpp_log("ins_bitrate %d bit/s", stat->ins_bitrate);

    if (fp) {
        fprintf(fp, "%3d,%6d,%5.2f,%5.5f,",
                frame_count, stat->frame_size, stat->psnr_y, stat->ssim_y);
        if (one_second)
            fprintf(fp, "%d,", stat->ins_bitrate);
        if (!seq_end)
            fprintf(fp, "\n");
    }
}

static MPP_RET mpi_rc_codec(MpiRcTestCtx *ctx)
{
    MPP_RET ret             = MPP_OK;
    MpiRcTestCmd       *cmd = &ctx->cmd;
    RK_U32 frm_eos          = 0;
    RK_U32 pkt_eos          = 0;
    MpiRcFile         *file = &ctx->file;
    MpiRcStat         *stat = &ctx->stat;
    FILE *fp_input          = file->fp_input;
    FILE *fp_enc_out        = file->fp_enc_out;
    FILE *fp_dec_out        = file->fp_dec_out;
    FILE *fp_stat           = file->fp_stat;
    RK_U8 *dec_in_buf       = NULL;
    RK_U32 need_split       = 0;
    RK_U32 block_input      = 0;
    RK_U32 block_output     = 0;

    // base flow context
    MppCtx enc_ctx          = NULL;
    MppApi *enc_mpi         = NULL;
    MppCtx dec_ctx          = NULL;
    MppApi *dec_mpi         = NULL;

    // input / output
    RK_S32 i;
    MppBufferGroup frm_grp  = NULL;
    MppBufferGroup pkt_grp  = NULL;
    MppFrame  frame_in      = NULL;
    MppFrame  frame_out     = NULL;
    MppFrame  frame_tmp     = NULL;
    MppPacket packet        = NULL;
    MppPacket dec_packet    = NULL;
    MppBuffer frm_buf[MPI_RC_IO_COUNT] = { NULL };
    MppBuffer pkt_buf[MPI_RC_IO_COUNT] = { NULL };
    MppEncSeiMode sei_mode = MPP_ENC_SEI_MODE_ONE_SEQ;

    // paramter for resource malloc
    RK_U32 width        = cmd->width;
    RK_U32 height       = cmd->height;
    RK_U32 hor_stride   = MPP_ALIGN(width,  16);
    RK_U32 ver_stride   = MPP_ALIGN(height, 16);
    MppFrameFormat fmt  = cmd->format;
    MppCodingType type  = cmd->type;
    RK_U32 num_frames   = cmd->num_frames;

    // resources
    size_t frame_size   = hor_stride * ver_stride * 3 / 2;
    /* NOTE: packet buffer may overflow */
    size_t packet_size  = width * height;
    RK_U32 frame_count  = 0;
    RK_U64 stream_size  = 0;
    RK_U64 stream_size_1s = 0;

    // runtime config
    MppEncCfgSet cfg;
    MppEncRcCfg *rc_cfg = &cfg.rc;
    MppEncPrepCfg *prep_cfg = &cfg.prep;
    MppEncCodecCfg *codec_cfg = &cfg.codec;
    RK_S32 fps = 20;

    mpp_log_f("test start width %d height %d codingtype %d\n", width, height, type);
    ret = mpp_buffer_group_get_internal(&frm_grp, MPP_BUFFER_TYPE_ION);
    if (ret) {
        mpp_err("failed to get buffer group for input frame ret %d\n", ret);
        goto MPP_TEST_OUT;
    }

    ret = mpp_buffer_group_get_internal(&pkt_grp, MPP_BUFFER_TYPE_ION);
    if (ret) {
        mpp_err("failed to get buffer group for output packet ret %d\n", ret);
        goto MPP_TEST_OUT;
    }

    for (i = 0; i < MPI_RC_IO_COUNT; i++) {
        ret = mpp_buffer_get(frm_grp, &frm_buf[i], frame_size);
        if (ret) {
            mpp_err("failed to get buffer for input frame ret %d\n", ret);
            goto MPP_TEST_OUT;
        }

        ret = mpp_buffer_get(pkt_grp, &pkt_buf[i], packet_size);
        if (ret) {
            mpp_err("failed to get buffer for input frame ret %d\n", ret);
            goto MPP_TEST_OUT;
        }
    }

    dec_in_buf = mpp_calloc(RK_U8, packet_size);
    if (NULL == dec_in_buf) {
        mpp_err("mpi_dec_test malloc input stream buffer failed\n");
        goto MPP_TEST_OUT;
    }

    // decoder init
    ret = mpp_create(&dec_ctx, &dec_mpi);
    if (MPP_OK != ret) {
        mpp_err("mpp_create decoder failed\n");
        goto MPP_TEST_OUT;
    }

    ret = mpp_packet_init(&dec_packet, dec_in_buf, packet_size);
    if (ret) {
        mpp_err("mpp_packet_init failed\n");
        goto MPP_TEST_OUT;
    }

    ret = dec_mpi->control(dec_ctx, MPP_DEC_SET_PARSER_SPLIT_MODE, &need_split);
    if (MPP_OK != ret) {
        mpp_err("dec_mpi->control failed\n");
        goto MPP_TEST_OUT;
    }
    block_input = 0;
    block_output = 1;
    ret = dec_mpi->control(dec_ctx, MPP_SET_INPUT_BLOCK, (MppParam)&block_input);
    if (MPP_OK != ret) {
        mpp_err("dec_mpi->control dec MPP_SET_INPUT_BLOCK failed\n");
        goto MPP_TEST_OUT;
    }
    ret = dec_mpi->control(dec_ctx, MPP_SET_OUTPUT_BLOCK, (MppParam)&block_output);
    if (MPP_OK != ret) {
        mpp_err("dec_mpi->control MPP_SET_OUTPUT_BLOCK failed\n");
        goto MPP_TEST_OUT;
    }

    ret = mpp_init(dec_ctx, MPP_CTX_DEC, type);
    if (MPP_OK != ret) {
        mpp_err("mpp_init dec failed\n");
        goto MPP_TEST_OUT;
    }

    mpp_frame_init(&frame_tmp);
    mpp_frame_set_width(frame_tmp, width);
    mpp_frame_set_height(frame_tmp, height);
    // NOTE: only for current version, otherwise there will be info change
    mpp_frame_set_hor_stride(frame_tmp, /*(MPP_ALIGN(hor_stride, 256) | 256)*/hor_stride);
    mpp_frame_set_ver_stride(frame_tmp, ver_stride);
    mpp_frame_set_fmt(frame_tmp, MPP_FMT_YUV420SP); // dec out frame only support 420sp
    ret = dec_mpi->control(dec_ctx, MPP_DEC_SET_FRAME_INFO, (MppParam)frame_tmp);
    if (MPP_OK != ret) {
        mpp_err("dec_mpi->control set frame info failed");
        goto MPP_TEST_OUT;
    }

    // encoder init
    ret = mpp_create(&enc_ctx, &enc_mpi);
    if (MPP_OK != ret) {
        mpp_err("mpp_create encoder failed\n");
        goto MPP_TEST_OUT;
    }
    block_input = 0;
    block_output = 0;
    ret = enc_mpi->control(enc_ctx, MPP_SET_INPUT_BLOCK, (MppParam)&block_input);
    if (MPP_OK != ret) {
        mpp_err("enc_mpi->control MPP_SET_INPUT_BLOCK failed\n");
        goto MPP_TEST_OUT;
    }
    ret = enc_mpi->control(enc_ctx, MPP_SET_OUTPUT_BLOCK, (MppParam)&block_output);
    if (MPP_OK != ret) {
        mpp_err("enc_mpi->control MPP_SET_OUTPUT_BLOCK failed\n");
        goto MPP_TEST_OUT;
    }

    ret = mpp_init(enc_ctx, MPP_CTX_ENC, type);
    if (MPP_OK != ret) {
        mpp_err("mpp_init enc failed\n");
        goto MPP_TEST_OUT;
    }

    ret = enc_mpi->control(enc_ctx, MPP_ENC_SET_SEI_CFG, &sei_mode);
    if (MPP_OK != ret) {
        mpp_err("mpi control enc set sei cfg failed\n");
        goto MPP_TEST_OUT;
    }

    rc_cfg->change = MPP_ENC_RC_CFG_CHANGE_ALL;
    rc_cfg->rc_mode = MPP_ENC_RC_MODE_CBR;
    rc_cfg->quality = MPP_ENC_RC_QUALITY_MEDIUM;
    rc_cfg->bps_target = 800000;
    rc_cfg->bps_max =  800000;
    rc_cfg->bps_min =  800000;
    rc_cfg->fps_in_denorm = 1;
    rc_cfg->fps_out_denorm = 1;
    rc_cfg->fps_in_num = fps;
    rc_cfg->fps_out_num = fps;
    rc_cfg->fps_in_flex = 0;
    rc_cfg->fps_out_flex = 0;
    rc_cfg->gop = fps;
    rc_cfg->skip_cnt = 0;

    ret = enc_mpi->control(enc_ctx, MPP_ENC_SET_RC_CFG, rc_cfg);
    if (ret) {
        mpp_err("mpi control enc set rc cfg failed ret %d\n", ret);
        goto MPP_TEST_OUT;
    }

    prep_cfg->change        = MPP_ENC_PREP_CFG_CHANGE_INPUT |
                              MPP_ENC_PREP_CFG_CHANGE_FORMAT;
    prep_cfg->width         = width;
    prep_cfg->height        = height;
    prep_cfg->hor_stride    = hor_stride;
    prep_cfg->ver_stride    = ver_stride;
    prep_cfg->format        = fmt;

    ret = enc_mpi->control(enc_ctx, MPP_ENC_SET_PREP_CFG, prep_cfg);
    if (ret) {
        mpp_err("mpi control enc set prep cfg failed ret %d\n", ret);
        goto MPP_TEST_OUT;
    }

    codec_cfg->coding = type;
    codec_cfg->h264.change = MPP_ENC_H264_CFG_CHANGE_PROFILE |
                             MPP_ENC_H264_CFG_CHANGE_ENTROPY |
                             MPP_ENC_H264_CFG_CHANGE_QP_LIMIT;
    /*
     * H.264 profile_idc parameter
     * 66  - Baseline profile
     * 77  - Main profile
     * 100 - High profile
     */
    codec_cfg->h264.profile  = 100;
    /*
     * H.264 level_idc parameter
     * 10 / 11 / 12 / 13    - qcif@15fps / cif@7.5fps / cif@15fps / cif@30fps
     * 20 / 21 / 22         - cif@30fps / half-D1@@25fps / D1@12.5fps
     * 30 / 31 / 32         - D1@25fps / 720p@30fps / 720p@60fps
     * 40 / 41 / 42         - 1080p@30fps / 1080p@30fps / 1080p@60fps
     * 50 / 51 / 52         - 4K@30fps
     */
    codec_cfg->h264.level    = 40;
    codec_cfg->h264.entropy_coding_mode  = 1;
    codec_cfg->h264.cabac_init_idc  = 0;

    codec_cfg->h264.qp_init = 0;
    if (rc_cfg->rc_mode == MPP_ENC_RC_MODE_CBR) {
        /* constant bitrate do not limit qp range */
        codec_cfg->h264.qp_max   = 48;
        codec_cfg->h264.qp_min   = 4;
        codec_cfg->h264.qp_max_step  = 16;
    } else if (rc_cfg->rc_mode == MPP_ENC_RC_MODE_VBR) {
        if (rc_cfg->quality == MPP_ENC_RC_QUALITY_CQP) {
            /* constant QP mode qp is fixed */
            codec_cfg->h264.qp_init  = 26;
            codec_cfg->h264.qp_max   = 26;
            codec_cfg->h264.qp_min   = 26;
            codec_cfg->h264.qp_max_step  = 0;
        } else {
            /* variable bitrate has qp min limit */
            codec_cfg->h264.qp_max   = 40;
            codec_cfg->h264.qp_min   = 12;
            codec_cfg->h264.qp_max_step  = 8;
        }
    }
    ret = enc_mpi->control(enc_ctx, MPP_ENC_SET_CODEC_CFG, codec_cfg);
    if (ret) {
        mpp_err("mpi control enc set codec cfg failed ret %d\n", ret);
        goto MPP_TEST_OUT;
    }

    ret = enc_mpi->control(enc_ctx, MPP_ENC_GET_EXTRA_INFO, &packet);
    if (MPP_OK != ret) {
        mpp_err("mpi control enc get extra info failed\n");
        goto MPP_TEST_OUT;
    }

    /* get and write sps/pps for H.264 */
    if (packet) {
        void *ptr   = mpp_packet_get_pos(packet);
        size_t len  = mpp_packet_get_length(packet);
        MppPacket tmp_pkt = NULL;

        // write extra data to dec packet and send
        mpp_packet_init(&tmp_pkt, ptr, len);
        mpp_packet_set_extra_data(tmp_pkt);
        dec_mpi->decode_put_packet(dec_ctx, tmp_pkt);
        mpp_packet_deinit(&tmp_pkt);

        if (fp_enc_out)
            fwrite(ptr, 1, len, fp_enc_out);

        packet = NULL;
    }

    ret = mpp_frame_init(&frame_in);
    if (MPP_OK != ret) {
        mpp_err("mpp_frame_init failed\n");
        goto MPP_TEST_OUT;
    }

    mpp_frame_set_width(frame_in, width);
    mpp_frame_set_height(frame_in, height);
    mpp_frame_set_hor_stride(frame_in, hor_stride);
    mpp_frame_set_ver_stride(frame_in, ver_stride);
    mpp_frame_set_fmt(frame_in, fmt);

    i = 0;
    while (!pkt_eos) {
        MppTask enc_task = NULL;
        RK_S32 index = i++;
        MppBuffer frm_buf_in  = frm_buf[index];
        MppBuffer pkt_buf_out = pkt_buf[index];
        void *buf = mpp_buffer_get_ptr(frm_buf_in);

        if (i == MPI_RC_IO_COUNT)
            i = 0;

        if (fp_input) {
            ret = mpi_rc_read_yuv_image(buf, prep_cfg, fp_input);
            if (MPP_OK != ret || feof(fp_input)) {
                mpp_log("found last frame\n");
                frm_eos = 1;
            }
        } else {
            ret = gen_yuv_image(buf, prep_cfg, frame_count);
            if (ret)
                goto MPP_TEST_OUT;
        }

        mpp_frame_set_buffer(frame_in, frm_buf_in);
        mpp_frame_set_eos(frame_in, frm_eos);

        mpp_packet_init_with_buffer(&packet, pkt_buf_out);

        ret = enc_mpi->poll(enc_ctx, MPP_PORT_INPUT, MPP_POLL_BLOCK);
        if (ret) {
            mpp_err("mpp input poll failed\n");
            goto MPP_TEST_OUT;
        }

        ret = enc_mpi->dequeue(enc_ctx, MPP_PORT_INPUT, &enc_task);
        if (ret) {
            mpp_err("mpp task input dequeue failed\n");
            goto MPP_TEST_OUT;
        }

        mpp_assert(enc_task);
        {
            MppFrame tmp_frm = NULL;
            mpp_task_meta_get_frame(enc_task, KEY_INPUT_FRAME,  &tmp_frm);
            if (tmp_frm)
                mpp_assert(tmp_frm == frame_in);
        }

        mpp_task_meta_set_frame (enc_task, KEY_INPUT_FRAME,  frame_in);
        mpp_task_meta_set_packet(enc_task, KEY_OUTPUT_PACKET, packet);

        ret = enc_mpi->enqueue(enc_ctx, MPP_PORT_INPUT, enc_task);
        if (ret) {
            mpp_err("mpp task input enqueue failed\n");
            goto MPP_TEST_OUT;
        }

        ret = enc_mpi->poll(enc_ctx, MPP_PORT_OUTPUT, MPP_POLL_BLOCK);
        if (ret) {
            mpp_err("mpp task output poll failed ret %d\n", ret);
            goto MPP_TEST_OUT;
        }

        ret = enc_mpi->dequeue(enc_ctx, MPP_PORT_OUTPUT, &enc_task);
        if (ret || NULL == enc_task) {
            mpp_err("mpp task output dequeue failed ret %d task %p\n", ret, enc_task);
            goto MPP_TEST_OUT;
        }

        if (enc_task) {
            MppFrame packet_out = NULL;

            mpp_task_meta_get_packet(enc_task, KEY_OUTPUT_PACKET, &packet_out);

            mpp_assert(packet_out == packet);
            if (packet) {
                // write packet to file here
                void *ptr   = mpp_packet_get_pos(packet);
                size_t len  = mpp_packet_get_length(packet);
                RK_U32 dec_pkt_done = 0;

                pkt_eos = mpp_packet_get_eos(packet);

                if (fp_enc_out)
                    fwrite(ptr, 1, len, fp_enc_out);

                //mpp_log_f("encoded frame %d size %d bits\n", frame_count, len * 8);
                stream_size += len * 8;
                stream_size_1s += len * 8;
                stat->frame_size = len * 8;
                if ((frame_count + 1) % fps == 0) {
                    stat->ins_bitrate = stream_size_1s;
                    stream_size_1s = 0;
                }

                if (pkt_eos) {
                    mpp_log("found last packet\n");
                    mpp_assert(frm_eos);
                }

                /* decode one frame */
                // write packet to dec input
                memset(dec_in_buf, 0, packet_size);
                mpp_packet_write(dec_packet, 0, ptr, len);
                // reset pos
                mpp_packet_set_pos(dec_packet, dec_in_buf);
                mpp_packet_set_length(dec_packet, len);
                mpp_packet_set_size(dec_packet, len);
                frame_out = NULL;

                do {
                    // send the packet first if packet is not done
                    if (!dec_pkt_done) {
                        ret = dec_mpi->decode_put_packet(dec_ctx, dec_packet);
                        if (MPP_OK == ret)
                            dec_pkt_done = 1;
                    }

                    // then get all available frame and release
                    do {
                        RK_S32 dec_get_frm = 0;
                        RK_U32 dec_frm_eos = 0;

                        ret = dec_mpi->decode_get_frame(dec_ctx, &frame_out);
                        if (MPP_OK != ret) {
                            mpp_err("decode_get_frame failed ret %d\n", ret);
                            break;
                        }

                        if (frame_out) {
                            if (mpp_frame_get_info_change(frame_out)) {
                                mpp_log("decode_get_frame get info changed found\n");
                                dec_mpi->control(dec_ctx, MPP_DEC_SET_INFO_CHANGE_READY, NULL);
                            } else {
                                if (fp_dec_out)
                                    dump_mpp_frame_to_file(frame_out, fp_dec_out);
                                mpi_rc_calc_stat(ctx, frame_in, frame_out);
                                mpi_rc_log_stat(ctx, frame_count, (frame_count + 1) % fps == 0,
                                                frame_count + 1 == num_frames);
                                dec_get_frm = 1;
                            }
                            dec_frm_eos = mpp_frame_get_eos(frame_out);
                            mpp_frame_deinit(&frame_out);
                            frame_out = NULL;
                        }

                        // if last packet is send but last frame is not found continue
                        if (pkt_eos && dec_pkt_done && !dec_frm_eos)
                            continue;

                        if (dec_get_frm)
                            break;
                    } while (1);

                    if (dec_pkt_done)
                        break;

                    msleep(5);
                } while (1);

                mpp_packet_deinit(&packet); // encoder packet deinit
            }
            frame_count++;

            ret = enc_mpi->enqueue(enc_ctx, MPP_PORT_OUTPUT, enc_task);
            if (ret) {
                mpp_err("mpp task output enqueue failed\n");
                goto MPP_TEST_OUT;
            }
        }

        if (num_frames && frame_count >= num_frames) {
            mpp_log_f("codec max %d frames", frame_count);
            break;
        }
        if (frm_eos && pkt_eos)
            break;
    }

    ret = enc_mpi->reset(enc_ctx);
    if (MPP_OK != ret) {
        mpp_err("mpi->reset enc failed\n");
        goto MPP_TEST_OUT;
    }

    ret = dec_mpi->reset(dec_ctx);
    if (MPP_OK != ret) {
        mpp_err("mpi->reset dec failed\n");
        goto MPP_TEST_OUT;
    }

    stat->avg_bitrate = (RK_U64)stream_size * fps / frame_count;
    mpp_log_f("avg_bitrate %d bit/s", stat->avg_bitrate);
    if (fp_stat)
        fprintf(fp_stat, "%d\n", stat->avg_bitrate);

MPP_TEST_OUT:

    if (frame_tmp)
        mpp_frame_deinit(&frame_tmp);
    // encoder deinit
    if (enc_ctx) {
        mpp_destroy(enc_ctx);
        enc_ctx = NULL;
    }

    if (frame_in) {
        mpp_frame_deinit(&frame_in);
        frame_in = NULL;
    }

    for (i = 0; i < MPI_RC_IO_COUNT; i++) {
        if (frm_buf[i]) {
            mpp_buffer_put(frm_buf[i]);
            frm_buf[i] = NULL;
        }

        if (pkt_buf[i]) {
            mpp_buffer_put(pkt_buf[i]);
            pkt_buf[i] = NULL;
        }
    }

    if (frm_grp) {
        mpp_buffer_group_put(frm_grp);
        frm_grp = NULL;
    }

    if (pkt_grp) {
        mpp_buffer_group_put(pkt_grp);
        pkt_grp = NULL;
    }

    // decoder deinit
    if (dec_packet) {
        mpp_packet_deinit(&dec_packet);
        dec_packet = NULL;
    }

    if (frame_out) {
        mpp_frame_deinit(&frame_out);
        frame_out = NULL;
    }

    if (dec_ctx) {
        mpp_destroy(dec_ctx);
        dec_ctx = NULL;
    }

    MPP_FREE(dec_in_buf);

    if (MPP_OK == ret)
        mpp_log("test success total frame %d bps %d\n",
                frame_count, stat->avg_bitrate);
    else
        mpp_err_f("failed ret %d\n", ret);

    return ret;
}

static void mpi_rc_test_help()
{
    mpp_log("usage: mpi_rc_test [options]\n");
    show_options(mpi_rc_cmd);
    mpp_show_support_format();
}

static RK_S32 mpi_enc_test_parse_options(int argc, char **argv, MpiRcTestCmd* cmd)
{
    const char *opt;
    const char *next;
    RK_S32 optindex = 1;
    RK_S32 handleoptions = 1;
    RK_S32 err = MPP_NOK;

    if ((argc < 2) || (cmd == NULL)) {
        err = 1;
        return err;
    }

    /* parse options */
    while (optindex < argc) {
        opt  = (const char*)argv[optindex++];
        next = (const char*)argv[optindex];

        if (handleoptions && opt[0] == '-' && opt[1] != '\0') {
            if (opt[1] == '-') {
                if (opt[2] != '\0') {
                    opt++;
                } else {
                    handleoptions = 0;
                    continue;
                }
            }

            opt++;

            switch (*opt) {
            case 'i':
                if (next) {
                    strncpy(cmd->file_input, next, MPI_RC_FILE_NAME_LEN);
                    cmd->file_input[strlen(next)] = '\0';
                    cmd->have_input = 1;
                } else {
                    mpp_err("input file is invalid\n");
                    goto PARSE_OPINIONS_OUT;
                }
                break;
            case 'o':
                if (next) {
                    strncpy(cmd->file_enc_out, next, MPI_RC_FILE_NAME_LEN);
                    cmd->file_enc_out[strlen(next)] = '\0';
                    cmd->have_enc_out = 1;
                } else {
                    mpp_log("output file is invalid\n");
                    goto PARSE_OPINIONS_OUT;
                }
                break;
            case 'd':
                if (next) {
                    cmd->debug = atoi(next);;
                } else {
                    mpp_err("invalid debug flag\n");
                    goto PARSE_OPINIONS_OUT;
                }
                break;
            case 'w':
                if (next) {
                    cmd->width = atoi(next);
                } else {
                    mpp_err("invalid input width\n");
                    goto PARSE_OPINIONS_OUT;
                }
                break;
            case 'h':
                if ((*(opt + 1) != '\0') && !strncmp(opt, "help", 4)) {
                    mpi_rc_test_help();
                    err = 1;
                    goto PARSE_OPINIONS_OUT;
                } else if (next) {
                    cmd->height = atoi(next);
                } else {
                    mpp_log("input height is invalid\n");
                    goto PARSE_OPINIONS_OUT;
                }
                break;
            case 'f':
                if (next) {
                    cmd->format = (MppFrameFormat)atoi(next);
                    err = ((cmd->format >= MPP_FMT_YUV_BUTT && cmd->format < MPP_FRAME_FMT_RGB) ||
                           cmd->format >= MPP_FMT_RGB_BUTT);
                }

                if (!next || err) {
                    mpp_err("invalid input format %d\n", cmd->format);
                    goto PARSE_OPINIONS_OUT;
                }
                break;
            case 't':
                if (next) {
                    cmd->type = (MppCodingType)atoi(next);
                    err = mpp_check_support_format(MPP_CTX_ENC, cmd->type);
                }

                if (!next || err) {
                    mpp_err("invalid input coding type\n");
                    goto PARSE_OPINIONS_OUT;
                }
                break;
            case 'n':
                if (next) {
                    cmd->num_frames = atoi(next);
                } else {
                    mpp_err("invalid input max number of frames\n");
                    goto PARSE_OPINIONS_OUT;
                }
                break;
            case 'y':
                if (next) {
                    strncpy(cmd->file_dec_out, next, MPI_RC_FILE_NAME_LEN);
                    cmd->file_dec_out[strlen(next)] = '\0';
                    cmd->have_dec_out = 1;
                } else {
                    mpp_log("dec output file is invalid\n");
                    goto PARSE_OPINIONS_OUT;
                }
                break;
            case 's':
                if (next) {
                    strncpy(cmd->file_stat, next, MPI_RC_FILE_NAME_LEN);
                    cmd->file_stat[strlen(next)] = '\0';
                    cmd->have_stat_out = 1;
                } else {
                    mpp_log("stat file is invalid\n");
                    goto PARSE_OPINIONS_OUT;
                }
                break;
            case 'c':
                if (next) {
                    cmd->item_flag = strtol(next, NULL, 0);
                } else {
                    mpp_err("invalid input item_flag\n");
                    goto PARSE_OPINIONS_OUT;
                }
                break;
            default:
                goto PARSE_OPINIONS_OUT;
                break;
            }

            optindex++;
        }
    }

    err = 0;

PARSE_OPINIONS_OUT:
    return err;
}

static void mpi_rc_test_show_options(MpiRcTestCmd* cmd)
{
    mpp_log("cmd parse result:\n");
    mpp_log("input   file name: %s\n", cmd->file_input);
    mpp_log("enc out file name: %s\n", cmd->file_enc_out);
    mpp_log("dec out file name: %s\n", cmd->file_dec_out);
    mpp_log("stat    file name: %s\n", cmd->file_stat);
    mpp_log("width            : %d\n", cmd->width);
    mpp_log("height           : %d\n", cmd->height);
    mpp_log("type             : %d\n", cmd->type);
    mpp_log("debug flag       : %x\n", cmd->debug);
    mpp_log("format           : %d\n", cmd->format);
    mpp_log("num frames       : %d\n", cmd->num_frames);
    mpp_log("item flag        : 0x%x\n", cmd->item_flag);
}

int main(int argc, char **argv)
{
    MPP_RET ret = MPP_OK;
    MpiRcTestCtx ctx;
    MpiRcTestCmd* cmd = &ctx.cmd;

    mpp_log("=========== mpi rc test start ===========\n");

    memset(&ctx, 0, sizeof(ctx));

    // parse the cmd option
    ret = mpi_enc_test_parse_options(argc, argv, cmd);
    if (ret) {
        if (ret < 0) {
            mpp_err("mpi_rc_test_parse_options: input parameter invalid\n");
        }

        mpi_rc_test_help();
        return ret;
    }

    mpi_rc_test_show_options(cmd);

    mpp_env_set_u32("mpi_rc_debug", cmd->debug);

    ret = mpi_rc_init(&ctx);
    if (ret != MPP_OK) {
        mpp_err("mpi_rc_init failded ret %d", ret);
        goto err;
    }

    ret = mpi_rc_codec(&ctx);
    if (ret != MPP_OK) {
        mpp_err("mpi_rc_codec failded ret %d", ret);
        goto err;
    }

err:
    mpi_rc_deinit(&ctx);

    mpp_log("=========== mpi rc test end ===========\n");

    return (int)ret;
}

