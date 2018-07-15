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

#define MODULE_TAG "mpp_device"

#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "mpp_env.h"
#include "mpp_log.h"

#include "mpp_device.h"
#include "mpp_platform.h"

#include "vpu.h"

#define VPU_IOC_MAGIC                       'l'

#define VPU_IOC_SET_CLIENT_TYPE             _IOW(VPU_IOC_MAGIC, 1, unsigned long)
#define VPU_IOC_SET_REG                     _IOW(VPU_IOC_MAGIC, 3, unsigned long)
#define VPU_IOC_GET_REG                     _IOW(VPU_IOC_MAGIC, 4, unsigned long)

#define VPU_IOC_SET_CLIENT_TYPE_U32         _IOW(VPU_IOC_MAGIC, 1, unsigned int)

#define VPU_IOC_WRITE(nr, size)             _IOC(_IOC_WRITE, VPU_IOC_MAGIC, (nr), (size))

typedef struct MppReq_t {
    RK_U32 *req;
    RK_U32  size;
} MppReq;

static RK_U32 mpp_device_debug = 1;

static RK_S32 mpp_device_set_client_type(int dev, RK_S32 client_type)
{
    static RK_S32 mpp_device_ioctl_version = -1;
    RK_S32 ret;

    if (mpp_device_ioctl_version < 0) {
        mpp_log_f("VPU_IOC_SET_CLIENT_TYPE - %lu\n", (unsigned long)client_type);
        ret = ioctl(dev, VPU_IOC_SET_CLIENT_TYPE, (unsigned long)client_type);
        if (!ret) {
            mpp_device_ioctl_version = 0;
        } else {
            mpp_log_f("VPU_IOC_SET_CLIENT_TYPE_U32 - %u\n", client_type);
            ret = ioctl(dev, VPU_IOC_SET_CLIENT_TYPE_U32, (RK_U32)client_type);
            if (!ret)
                mpp_device_ioctl_version = 1;
        }

        if (ret)
            mpp_err_f("can not find valid client type ioctl\n");

        mpp_assert(ret == 0);
    } else {
        RK_U32 cmd = (mpp_device_ioctl_version == 0) ?
                     (VPU_IOC_SET_CLIENT_TYPE) :
                     (VPU_IOC_SET_CLIENT_TYPE_U32);

        mpp_log_f("VPU_IOC_SET_CLIENT_TYPE == %d\n"
			"VPU_IOC_SET_CLIENT_TYPE_U32 == %d\n"
			"ioctl(dev, %d, %d)",
			VPU_IOC_SET_CLIENT_TYPE,
			VPU_IOC_SET_CLIENT_TYPE_U32,
			cmd, client_type);
        ret = ioctl(dev, cmd, client_type);
    }

    if (ret)
        mpp_err_f("set client type failed ret %d errno %d\n", ret, errno);

    return ret;
}

static RK_S32 mpp_device_get_client_type(MppDevCtx *ctx, MppCtxType coding, MppCodingType type)
{
    RK_S32 client_type = -1;

	mpp_log_f("(Myy) CLIENT_TYPES\n"
		"\tVPU_ENC     %d\n"
		"\tVPU_DEC     %d\n"
		"\tVPU_DEC_PP  %d\n",
		 VPU_ENC, VPU_DEC, VPU_DEC_PP);
    if (coding == MPP_CTX_ENC)
        client_type = VPU_ENC;
    else { /* MPP_CTX_DEC */
        client_type = VPU_DEC;
        if (ctx->pp_enable)
            client_type = VPU_DEC_PP;
    }
    (void)ctx;
    (void)type;
    (void)coding;

    return client_type;
}

RK_S32 mpp_device_init(MppDevCtx *ctx, MppCtxType coding, MppCodingType type)
{
    RK_S32 dev = -1;
    const char *name = NULL;

    ctx->coding = coding;
    ctx->type = type;
    if (ctx->platform)
        name = mpp_get_platform_dev_name(coding, type, ctx->platform);
    else
        name = mpp_get_vcodec_dev_name(coding, type);
    if (name) {
        dev = open(name, O_RDWR);
        if (dev > 0) {
            RK_S32 client_type = mpp_device_get_client_type(ctx, coding, type);
            RK_S32 ret = mpp_device_set_client_type(dev, client_type);

            if (ret) {
                close(dev);
                dev = -2;
            }
            ctx->client_type = client_type;
        } else
            mpp_err_f("failed to open device %s, errno %d, error msg: %s\n",
                      name, errno, strerror(errno));
    } else
        mpp_err_f("failed to find device for coding %d type %d\n", coding, type);

    return dev;
}

MPP_RET mpp_device_deinit(RK_S32 dev)
{
    if (dev > 0)
        close(dev);

    return MPP_OK;
}

MPP_RET mpp_device_send_reg(RK_S32 dev, RK_U32 *regs, RK_U32 nregs)
{
    MPP_RET ret;
    MppReq req;

    if (mpp_device_debug) {
        RK_U32 i;

        for (i = 0; i < nregs; i++) {
            mpp_log_f("set reg[%03d]: %08x\n", i, regs[i]);
        }
    }

    nregs *= sizeof(RK_U32);
    req.req     = regs;
    req.size    = nregs;
    ret = (RK_S32)ioctl(dev, VPU_IOC_SET_REG, &req);
    if (ret) {
        mpp_err_f("ioctl VPU_IOC_SET_REG failed ret %d errno %d %s\n",
                  ret, errno, strerror(errno));
        ret = errno;
    }

    return ret;
}

MPP_RET mpp_device_wait_reg(RK_S32 dev, RK_U32 *regs, RK_U32 nregs)
{
    MPP_RET ret;
    MppReq req;

    req.req     = regs;
    req.size    = nregs * sizeof(RK_U32);

    
    ret = (RK_S32)ioctl(dev, VPU_IOC_GET_REG, &req);
    if (ret) {
        mpp_err_f("ioctl VPU_IOC_GET_REG failed ret %d errno %d %s\n",
                  ret, errno, strerror(errno));
        ret = errno;
    }

    if (mpp_device_debug) {
        RK_U32 i;

        for (i = 0; i < nregs; i++) {
            mpp_log_f("get reg[%03d]: %08x\n", i, regs[i]);
        }
    }

    return ret;
}

MPP_RET mpp_device_send_reg_with_id(RK_S32 dev, RK_S32 id, void *param,
                                    RK_S32 size)
{
    MPP_RET ret = MPP_NOK;

    if (param == NULL) {
        mpp_err_f("input param is NULL");
        return ret;
    }

    mpp_log_f("(Myy) ioctl(dev, IOW(VPU_IOC_MAGIC, %d, %d), %p)\n",
		id, size, param);

    ret = (RK_S32)ioctl(dev, VPU_IOC_WRITE(id, size), param);
    if (ret) {
        mpp_err_f("ioctl VPU_IOC_WRITE failed ret %d errno %d %s\n",
                  ret, errno, strerror(errno));
        ret = errno;
    }

    return ret;
}

RK_S32 mpp_device_control(MppDevCtx *ctx, MppDevCmd cmd, void* param)
{
    switch (cmd) {
    case MPP_DEV_GET_MMU_STATUS : {
        ctx->mmu_status = 1;
        *((RK_U32 *)param) = ctx->mmu_status;
    } break;
    case MPP_DEV_ENABLE_POSTPROCCESS : {
        ctx->pp_enable = 1;
    } break;
    case MPP_DEV_SET_HARD_PLATFORM : {
        ctx->platform = *((RK_U32 *)param);
    } break;
    default : {
    } break;
    }

    return 0;
}

