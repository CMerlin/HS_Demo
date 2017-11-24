/******************************************************************************
  A simple program of Hisilicon HI3521A video input and output implementation.
  Copyright (C), 2014-2015, Hisilicon Tech. Co., Ltd.
 ******************************************************************************
Modification:  2015-1 Created
 ******************************************************************************/

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

//#include "sample_comm.h"

#include <sys/sem.h>
#include "hi_common.h"
#include "hi_comm_sys.h"
#include "hi_comm_vb.h"
#include "hi_comm_vi.h"
#include "hi_comm_vo.h"
#include "hi_comm_venc.h"
#include "hi_comm_vpss.h"
#include "hi_comm_vdec.h"
#include "hi_comm_vda.h"
#include "hi_comm_region.h"
#include "hi_comm_adec.h"
#include "hi_comm_aenc.h"
#include "hi_comm_ai.h"
#include "hi_comm_ao.h"
#include "hi_comm_aio.h"
#include "hi_comm_hdmi.h"
#include "hi_defines.h"

#include "mpi_sys.h"
#include "mpi_vb.h"
#include "mpi_vi.h"
#include "mpi_vo.h"
#include "mpi_venc.h"
#include "mpi_vpss.h"
#include "mpi_vdec.h"
#include "mpi_vda.h"
#include "mpi_region.h"
#include "mpi_adec.h"
#include "mpi_aenc.h"
#include "mpi_ai.h"
#include "mpi_ao.h"
#include "mpi_hdmi.h"

#include "tlv320aic31.h"
#include "tp2823.h"

#include "loadbmp.h"

#define HDMI_SUPPORT 
//#include "tw2865.h"
//#include "tw2960.h"

/***************************
* Description:此例子程序感觉很不多，可以多学一下
*******************************/
static VI_DEV_ATTR_S DEV_ATTR_BT656D1_4MUX =
{
	/*接口模式*/
	VI_MODE_BT656,
	/*1、2、4路工作模式*/
	VI_WORK_MODE_4Multiplex,
	/* r_mask    g_mask    b_mask*/
	{0xFF000000,    0x0},

	/* 双沿输入时必须设置 */
	VI_CLK_EDGE_SINGLE_UP,

	/*AdChnId*/
	{-1, -1, -1, -1},
	/*enDataSeq, 仅支持YUV格式*/
	VI_INPUT_DATA_YVYU,
	/*同步信息，对应reg手册的如下配置, --bt1120时序无效*/
	{
		/*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
		VI_VSYNC_FIELD, VI_VSYNC_NEG_HIGH, VI_HSYNC_VALID_SINGNAL,VI_HSYNC_NEG_HIGH,VI_VSYNC_VALID_SINGAL,VI_VSYNC_VALID_NEG_HIGH,

		/*timing信息，对应reg手册的如下配置*/
		/*hsync_hfb    hsync_act    hsync_hhb*/
		{0,            0,        0,
			/*vsync0_vhb vsync0_act vsync0_hhb*/
			0,            0,        0,
			/*vsync1_vhb vsync1_act vsync1_hhb*/
			0,            0,            0}
	},
	/*使用内部ISP*/
	VI_PATH_BYPASS,
	/*输入数据类型*/
	VI_DATA_TYPE_YUV
};

static VI_DEV_ATTR_S DEV_ATTR_TP2823_720P_2MUX_BASE =
{
	/*接口模式*/
	VI_MODE_BT656,
	/*1、2、4路工作模式*/
	VI_WORK_MODE_2Multiplex,
	/* r_mask    g_mask    b_mask*/
	{0xFF000000,    0x0},

	/* 双沿输入时必须设置 */
	VI_CLK_EDGE_SINGLE_UP,

	/*AdChnId*/
	{-1, -1, -1, -1},
	/*enDataSeq, 仅支持YUV格式*/
	VI_INPUT_DATA_YVYU,
	/*同步信息，对应reg手册的如下配置, --bt1120时序无效*/
	{
		/*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
		VI_VSYNC_FIELD, VI_VSYNC_NEG_HIGH, VI_HSYNC_VALID_SINGNAL,VI_HSYNC_NEG_HIGH,VI_VSYNC_VALID_SINGAL,VI_VSYNC_VALID_NEG_HIGH,

		/*timing信息，对应reg手册的如下配置*/
		/*hsync_hfb    hsync_act    hsync_hhb*/
		{0,            0,        0,
			/*vsync0_vhb vsync0_act vsync0_hhb*/
			0,            0,        0,
			/*vsync1_vhb vsync1_act vsync1_hhb*/
			0,            0,            0}
	},
	/*使用内部ISP*/
	VI_PATH_BYPASS,
	/*输入数据类型*/
	VI_DATA_TYPE_YUV
};


static VI_DEV_ATTR_S DEV_ATTR_TP2823_720P_1MUX_BASE =
{
	/*接口模式*/
	VI_MODE_BT656,
	/*1、2、4路工作模式*/
	VI_WORK_MODE_1Multiplex,
	/* r_mask    g_mask    b_mask*/
	{0xFF000000,    0x0},

	/* 双沿输入时必须设置 */
	VI_CLK_EDGE_SINGLE_UP,

	/*AdChnId*/
	{-1, -1, -1, -1},
	/*enDataSeq, 仅支持YUV格式*/
	VI_INPUT_DATA_YVYU,
	/*同步信息，对应reg手册的如下配置, --bt1120时序无效*/
	{
		/*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
		VI_VSYNC_FIELD, VI_VSYNC_NEG_HIGH, VI_HSYNC_VALID_SINGNAL,VI_HSYNC_NEG_HIGH,VI_VSYNC_VALID_SINGAL,VI_VSYNC_VALID_NEG_HIGH,

		/*timing信息，对应reg手册的如下配置*/
		/*hsync_hfb    hsync_act    hsync_hhb*/
		{0,            0,        0,
			/*vsync0_vhb vsync0_act vsync0_hhb*/
			0,            0,        0,
			/*vsync1_vhb vsync1_act vsync1_hhb*/
			0,            0,            0}
	},
	/*使用内部ISP*/
	VI_PATH_BYPASS,
	/*输入数据类型*/
	VI_DATA_TYPE_YUV
};

static VI_DEV_ATTR_S DEV_ATTR_7441_BT1120_STANDARD_BASE =
/* 典型时序3:7441 BT1120 1080P@60fps典型时序 (对接时序: 时序)*/
{
	/*接口模式*/
	VI_MODE_BT1120_STANDARD,
	/*1、2、4路工作模式*/
	VI_WORK_MODE_1Multiplex,
	/* r_mask    g_mask    b_mask*/
	{0xFF000000,    0xFF0000},
	/*逐行or隔行输入*/
	//VI_SCAN_PROGRESSIVE,

	/* 双沿输入时必须设置 */
	VI_CLK_EDGE_SINGLE_UP,

	/*AdChnId*/
	{-1, -1, -1, -1},
	/*enDataSeq, 仅支持YUV格式*/
	VI_INPUT_DATA_UVUV,

	/*同步信息，对应reg手册的如下配置, --bt1120时序无效*/
	{
		/*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
		VI_VSYNC_PULSE, VI_VSYNC_NEG_HIGH, VI_HSYNC_VALID_SINGNAL,VI_HSYNC_NEG_HIGH,VI_VSYNC_NORM_PULSE,VI_VSYNC_VALID_NEG_HIGH,

		/*timing信息，对应reg手册的如下配置*/
		/*hsync_hfb    hsync_act    hsync_hhb*/
		{0,            1920,        0,
			/*vsync0_vhb vsync0_act vsync0_hhb*/
			0,            1080,        0,
			/*vsync1_vhb vsync1_act vsync1_hhb*/
			0,            0,            0}
	},
	/*使用内部ISP*/
	VI_PATH_BYPASS,
	/*输入数据类型*/
	VI_DATA_TYPE_YUV

};

static VI_DEV_ATTR_S DEV_ATTR_BT1120I =
/* 典型时序3:7441 BT1120 1080P@60fps典型时序 (对接时序: 时序)*/
{
	/*接口模式*/
	VI_MODE_BT1120_INTERLEAVED,
	//    VI_MODE_BT656,
	/*1、2、4路工作模式*/
	VI_WORK_MODE_1Multiplex,
	/* r_mask    g_mask    b_mask*/
	{0xFF000000,    0xFF0000},

	/* 双沿输入时必须设置 */
	VI_CLK_EDGE_SINGLE_UP,

	/*AdChnId*/
	{-1, -1, -1, -1},
	/*enDataSeq, 仅支持YUV格式*/
	VI_INPUT_DATA_UVUV,
	/*同步信息，对应reg手册的如下配置, --bt1120时序无效*/
	{
		/*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
		VI_VSYNC_PULSE, VI_VSYNC_NEG_HIGH, VI_HSYNC_VALID_SINGNAL, VI_HSYNC_NEG_HIGH,VI_VSYNC_NORM_PULSE,VI_VSYNC_VALID_NEG_HIGH,

		/*timing信息，对应reg手册的如下配置*/
		/*hsync_hfb    hsync_act    hsync_hhb*/
		{0,            1920,        0,
			/*vsync0_vhb vsync0_act vsync0_hhb*/
			0,            1080,        0,
			/*vsync1_vhb vsync1_act vsync1_hhb*/
			0,            0,            0}
	},
	/*使用内部ISP*/
	VI_PATH_BYPASS,
	/*输入数据类型*/
	VI_DATA_TYPE_YUV,

	HI_FALSE
};

/*******************************************************
  macro define 
 *******************************************************/
#define CHECK_CHN_RET(express,Chn,name)\
	do{\
		HI_S32 Ret;\
		Ret = express;\
		if (HI_SUCCESS != Ret)\
		{\
			printf("\033[0;31m%s chn %d failed at %s: LINE: %d with %#x!\033[0;39m\n", name, Chn, __FUNCTION__, __LINE__, Ret);\
			fflush(stdout);\
			return Ret;\
		}\
	}while(0)

#define CHECK_RET(express,name)\
	do{\
		HI_S32 Ret;\
		Ret = express;\
		if (HI_SUCCESS != Ret)\
		{\
			printf("\033[0;31m%s failed at %s: LINE: %d with %#x!\033[0;39m\n", name, __FUNCTION__, __LINE__, Ret);\
			return Ret;\
		}\
	}while(0)


#define DEMO_VO_DEV_DHD0 0
#define DEMO_VO_DEV_DHD1 1
#define DEMO_VO_DEV_DSD0 2
#define DEMO_VO_DEV_VIRT0 3
#define DEMO_VO_DEV_DSD1 -1

#define DEMO_VO_LAYER_VHD0 0
#define DEMO_VO_LAYER_VHD1 1
#define DEMO_VO_LAYER_VPIP 2
#define DEMO_VO_LAYER_VSD0 3
#define DEMO_VO_LAYER_VIRT0 4


#define DEMO_VO_LAYER_PIP      2
#define DEMO_VO_LAYER_PIP_STA	2
#define DEMO_VO_LAYER_PIP_END	2
#define DEMO_VO_DEV_HD_END	2

#define DEMO_HD_WIDTH                1920
#define DEMO_HD_HEIGHT               1080

//#define HD_WIDTH                1280
//#define HD_HEIGHT               720

//#define HD_WIDTH                720
//#define HD_HEIGHT               576

#define DEMO__4K_WIDTH                3840
#define DEMO__4K_HEIGHT               2160

#define DEMO_D1_WIDTH                720
#define DEMO_D1_HEIGHT               576

#define DEMO__720P_WIDTH              1280
#define DEMO__720P_HEIGHT             720

#define DEMO_ALIGN_UP(x, a)              ((x+a-1)&(~(a-1)))
#define DEMO_ALIGN_BACK(x, a)              ((a) * (((x) / (a))))

#define write_yuv 1

typedef enum vi_mode_e
{
	DEMO_VI_MODE_32_D1,
	DEMO_VI_MODE_32_960H,
	DEMO_VI_MODE_32_1280H,
	DEMO_VI_MODE_32_HALF720P,

	DEMO_VI_MODE_8_720P,
	DEMO_VI_MODE_16_720P,

	DEMO_VI_MODE_8_1080P,
	DEMO_VI_MODE_16_1080P,

	DEMO_VI_MODE_4_4Kx2K,
}DEMO_VI_MODE_E;

typedef struct vi_param_s
{
	HI_S32 s32ViDevCnt;        // VI Dev Total Count    
	HI_S32 s32ViDevInterval;   // Vi Dev Interval    
	HI_S32 s32ViChnCnt;        // Vi Chn Total Count    
	HI_S32 s32ViChnInterval;   // VI Chn Interval
}DEMO_VI_PARAM_S;


typedef enum vi_chn_set_e
{
	DEMO_VI_CHN_SET_NORMAL = 0, /* mirror, filp close */ 
	DEMO_VI_CHN_SET_MIRROR,      /* open MIRROR */   
	DEMO_VI_CHN_SET_FILP        /* open filp */
}DEMO_VI_CHN_SET_E;

typedef enum vo_mode_e
{
	DEMO_VO_MODE_1MUX  = 0,
	DEMO_VO_MODE_4MUX = 1,
	DEMO_VO_MODE_9MUX = 2,
	DEMO_VO_MODE_16MUX = 3,    
	DEMO_VO_MODE_BUTT
}DEMO_VO_MODE_E;


typedef enum hiVdecThreadCtrlSignal_E
{
	DEMO_VDEC_CTRL_START,
	DEMO_VDEC_CTRL_PAUSE,
	DEMO_VDEC_CTRL_STOP,	
}DEMOVdecThreadCtrlSignal_E;

typedef struct hiVdecThreadParam
{
	HI_S32 s32ChnId;	
	PAYLOAD_TYPE_E enType;
	HI_CHAR cFileName[100];
	HI_S32 s32StreamMode;
	HI_S32 s32MilliSec;
	HI_S32 s32MinBufSize;
	HI_S32 s32IntervalTime;
	DEMOVdecThreadCtrlSignal_E eCtrlSinal;
	HI_U64	u64PtsInit;
	HI_U64	u64PtsIncrease;
	HI_BOOL bLoopSend;
	HI_BOOL bManuSend;
	HI_CHAR cUserCmd;
}DEMOVdecThreadParam;

typedef enum rc_e
{
	DEMO_RC_CBR = 0,
	DEMO_RC_VBR,
	DEMO_RC_FIXQP
}DEMO_RC_E;

typedef struct venc_getstream_s
{
	HI_BOOL bThreadStart;
	HI_S32  s32Cnt;
}DEMO_VENC_GETSTREAM_PARA_S;

//global
DEMO_VI_MODE_E g_enViMode = DEMO_VI_MODE_8_1080P;//VI_MODE_8_1080P VI_MODE_8_720P
VIDEO_NORM_E g_enNorm = VIDEO_ENCODING_MODE_AUTO; /*视频输入制式类型*/
VO_DEV g_VoDev = DEMO_VO_DEV_DHD0;
VO_LAYER g_VoLayer = DEMO_VO_LAYER_VHD0;
VO_LAYER g_VoLayerPip = DEMO_VO_LAYER_VPIP;
VPSS_CHN g_VpssChn_VoHD0 = VPSS_CHN1;	
VPSS_CHN g_VpssChn_VoHD1 = VPSS_CHN2;
VPSS_CHN g_VpssChn_VoSD0 = VPSS_CHN3;
static VB_POOL g_hPool  = VB_INVALID_POOLID;
static DEMO_VENC_GETSTREAM_PARA_S gs_stPara;

#if 1
/*3531A VI 的一些全局信息*/
#define VI_MAX_CH 33 /*3531A中VI的最大通道数*/
/*3531A VO 的一些全局信息*/
#define VO_AMX_LAYER_CH 64 /*3531A中,VO的每个视频层支持的最大通道数*/
/*3531A vapass 的一些全局信息*/
#define MAX_VP_GRP_CNT 256 /*vpass的最大组数目*/

VI_CHN_ATTR_S AIChAttr3531A[VI_MAX_CH]; /*所有VI通道属性信息*/


/******************************************************************************
 * Description：获取进程和线程ID
 * Output id：已将格式化好的ID 
 * Return：0-成功 -1-失败
 * *****************************************************************************/
int get_pid_ttid(char *id)
{
	pid_t pid = getpid();
	sprintf(id, "%d.%lu", pid, pthread_self());
	return 0;
}

/**************************************************************************************
* Description:禁止掉指定的VI设备
* Input:-1-所有的VI通道 0>=指定的通道号
**************************************************************************************/
int disVICHNum(VI_CHN chNum){
	HI_S32 s32Ret = 0, count = 0, ch = 0, num = 0;

	printf("[%s]:chNum=%d line:%d\n", __func__, chNum, __LINE__);
	/*禁止掉某通道*/
	if((-1) != chNum){
		s32Ret = HI_MPI_VI_DisableChn(chNum);
		if (HI_SUCCESS != s32Ret){
			printf("[%s]:failed with %#x\n", __func__, s32Ret);
			return HI_FAILURE;
		}
		return 0;
	}
	/*禁止掉所有的通道*/
	for(ch = 0; ch < VI_MAX_CH; ch++){
		num += 4; /*0 4 8 12 16 20 24 28 输出的是1080P的视频*/
		s32Ret = HI_MPI_VI_DisableChn(num);
		if (HI_SUCCESS != s32Ret){
			printf("[%s]:failed with %#x\n", __func__, s32Ret);
			return HI_FAILURE;
		}
	}
	return 0;
}

/****************************************************************************************
* Description:保留某个VI通道有输入
* Input:需要保留的通道
* Return:0-成功 0<-失败
***************************************************************************************/
int onlyVICHNum(VI_CHN chNum){
	HI_S32 s32Ret = 0,  ch = 0;
	
	printf("[%s]:chNum=%d line:%d\n", __func__, chNum, __LINE__);
	for(ch = 0; ch < VI_MAX_CH; ch++){
		if(ch == chNum){continue;} /*0 4 8 12 16 20 24 28 输出的是1080P的视频*/
		s32Ret = HI_MPI_VI_DisableChn(ch);
		if (HI_SUCCESS != s32Ret){
			printf("[%s]:failed with %#x\n", __func__, s32Ret);
			return HI_FAILURE;
		}
	}
	return 0;
}

/****************************************************************************************
* Description:禁掉某个视频层上的某个通道
* Input:layer-层号
* Input chNum:0>=禁止掉对应的通道号 -1-禁止掉所有的通道号
* Return:0-成功 0<-失败
***************************************************************************************/
int disVOCHNum(VO_LAYER layer, VO_CHN chNum){
	HI_S32 s32Ret = 0, ch = 0;

	printf("[%s]:layer=%d chNUm=%d line:%d\n", __func__, layer, chNum, __LINE__);
	s32Ret= HI_MPI_VO_SetAttrBegin(layer); /*设置视频层上的通道的设置属性开始*/
	/*禁掉某通道*/
	if((-1) != chNum){
		s32Ret = HI_MPI_VO_DisableChn(layer, chNum); /*禁用指定的视频输出通道*/
		if (s32Ret != HI_SUCCESS){
			printf("[%s]:failed with %#x!\n", __func__, s32Ret);
			return HI_FAILURE;
		}
		return 0;
	}
	else{
		/*禁掉所有通道*/
		for(ch = 0; ch < VO_AMX_LAYER_CH; ch++){
			s32Ret = HI_MPI_VO_DisableChn(layer, ch); /*禁用指定的视频输出通道*/
			if (s32Ret != HI_SUCCESS){
				printf("[%s]:failed with %#x!\n", __func__, s32Ret);
				return HI_FAILURE;
			}
		}
	}
	s32Ret= HI_MPI_VO_SetAttrEnd(layer); /*设置视频层上的通道的设置属性结束*/
	if (HI_SUCCESS != s32Ret){
		printf("[%s]:failed with %#x!\n", __func__, s32Ret);
		return 0;
	}	
	return 0;
}

/****************************************************************************************
* Description:保留某个VO通道有输入
* Input:需要保留的通道
* Return:0-成功 0<-失败
***************************************************************************************/
int onlyVOCHNum(VO_LAYER layer, VO_CHN chNum){
	HI_S32 s32Ret = 0,  ch = 0, num = 0;
	
	printf("[%s]:layer=%d chNUm=%d line:%d\n", __func__, layer, chNum, __LINE__);
	for(ch = 0; ch < VO_AMX_LAYER_CH; ch++){
		if(chNum == ch){continue;}
		s32Ret = HI_MPI_VO_DisableChn(layer, ch); /*禁用指定的视频输出通道*/
		if (HI_SUCCESS != s32Ret){
			printf("[%s]:failed with %#x\n", __func__, s32Ret);
			return HI_FAILURE;
		}
	}
	return 0;
}
#endif

/*********************************************************************
 * Description:开启VGA设备X视频层上X通道
 **********************************************************************/
int openVOLayerCH(VO_LAYER VoLayer, HI_S32 VOChNum){
	VO_DEV VoDev;
	VO_PUB_ATTR_S stVoPubAttr_hd0;
	VO_VIDEO_LAYER_ATTR_S stLayerAttr;
	VO_CHN_ATTR_S stChnAttr;
	HI_S32 s32Ret = HI_SUCCESS;
	DEMO_VO_MODE_E enVoMode;
	//VO_LAYER VoLayer;

	/*开启VGA设备*/
	//printf("[%s]:open VGA device line:%d\n", __func__, __LINE__);
	VoDev = g_VoDev; //SAMPLE_VO_DEV_DHD0;
	stVoPubAttr_hd0.enIntfSync = VO_OUTPUT_1080P60; /*Vo接口时序类型*/
	//stVoPubAttr_hd0.enIntfType = VO_INTF_BT1120|VO_INTF_VGA; /*Vo 接口类型*/
	stVoPubAttr_hd0.enIntfType = VO_INTF_BT1120|VO_INTF_VGA | VO_INTF_HDMI; /*Vo 接口类型*/
	stVoPubAttr_hd0.u32BgColor = 0x000000ff; /*0x00ff60 设备背景色 RGB表示*/
	s32Ret = SAMPLE_COMM_VO_StartDev(VoDev, &stVoPubAttr_hd0); /*设置并使能VO设备*/
	if (HI_SUCCESS != s32Ret){
		printf("Start SAMPLE_COMM_VO_StartDev failed!\n");
		return HI_FAILURE;
	}
	/*设置并使能视频层属性*/
	memset(&(stLayerAttr), 0 , sizeof(VO_VIDEO_LAYER_ATTR_S));
	s32Ret = SAMPLE_COMM_VO_GetWH(stVoPubAttr_hd0.enIntfSync, \
			&stLayerAttr.stImageSize.u32Width, \
			&stLayerAttr.stImageSize.u32Height, \
			&stLayerAttr.u32DispFrmRt);
	if (HI_SUCCESS != s32Ret)
	{
		printf("[%s]:Start SAMPLE_COMM_VO_GetWH failed!\n", __func__);
		return HI_FAILURE;
	}
	/*视频显示的坐标还有大小*/
	stLayerAttr.enPixFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_420; //SAMPLE_PIXEL_FORMAT; /*视频层使用的像素格式*/
	stLayerAttr.stDispRect.s32X       = 0;
	stLayerAttr.stDispRect.s32Y       = 0;
	stLayerAttr.stDispRect.u32Width   = stLayerAttr.stImageSize.u32Width;
	stLayerAttr.stDispRect.u32Height  = stLayerAttr.stImageSize.u32Height;
	//VoLayer = SAMPLE_VO_LAYER_VHD0; /*视频层*/
	s32Ret = SAMPLE_COMM_VO_StartLayer(VoLayer, &stLayerAttr); /*设置并使能视频层属性*/
	if (HI_SUCCESS != s32Ret){
		printf("Start SAMPLE_COMM_VO_StartLayer failed!\n");
		//goto END_8_1080P_4;
		return HI_FAILURE;
	}
	/*设置并使能视频通道*/
	memset(&stLayerAttr, 0, sizeof(stLayerAttr));
	s32Ret = HI_MPI_VO_GetVideoLayerAttr(VoLayer, &stLayerAttr); /*获取视频层属性*/
	if (s32Ret != HI_SUCCESS){
		printf("failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}
	stChnAttr.stRect.s32X       = 0; //ALIGN_BACK((u32Width/u32Square) * (i%u32Square), 2);
	stChnAttr.stRect.s32Y       = 0; //ALIGN_BACK((u32Height/u32Square) * (i/u32Square), 2);
	stChnAttr.stRect.u32Width   = stLayerAttr.stDispRect.u32Width; //ALIGN_BACK(u32Width/u32Square, 2);
	stChnAttr.stRect.u32Height  = stLayerAttr.stDispRect.u32Height; //ALIGN_BACK(u32Height/u32Square, 2);
	stChnAttr.u32Priority       = 0; /*视频层优先级*/
	stChnAttr.bDeflicker        = HI_FALSE; /*是否开启抗闪烁*/
	printf("[%s]:enable VO=%d-%d  line:%d\n", __func__, VoLayer , VOChNum,  __LINE__);
	s32Ret = HI_MPI_VO_SetChnAttr(VoLayer, VOChNum, &stChnAttr); /*配置指定视频输出通道的属性*/
	if (s32Ret != HI_SUCCESS){
		printf("%s(%d):failed with %#x!\n",__FUNCTION__,__LINE__,  s32Ret);
		return HI_FAILURE;
	}
	 /*启用指定的视频输出通道*/
	s32Ret = HI_MPI_VO_EnableChn(VoLayer, VOChNum);
	if (s32Ret != HI_SUCCESS){
		printf("failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}

#ifdef HDMI_SUPPORT
		/* 如果支持HDMI接口，初始化HDMI数据 */
		if (stVoPubAttr_hd0.enIntfType & VO_INTF_HDMI)
		{
			if (HI_SUCCESS != SAMPLE_COMM_VO_HdmiStart(stVoPubAttr_hd0.enIntfSync))
			{
				printf("[%s]:Start SAMPLE_COMM_VO_HdmiStart failed!\n", __func__);
				//goto END_8_1080P_7;
				return -1;
			}
		}
#endif

	//SAMPLE_COMM_VO_StartChn();
	return 0;
}

/*******************************************************************************
* Description:开启某个VO通道
* Input VoLayer:视频层号 VOChNum-通道号
* Return：0-成功 0<-失败
*******************************************************************************/
int startVOCHx(VO_LAYER VoLayer, HI_S32 VOChNum){
	HI_S32 s32Ret = HI_SUCCESS;
	VO_VIDEO_LAYER_ATTR_S stLayerAttr;
	VO_CHN_ATTR_S stChnAttr;

	s32Ret= HI_MPI_VO_SetAttrBegin(VoLayer); /*设置视频层上的通道的设置属性开始*/
	if (HI_SUCCESS != s32Ret){
		printf("[%s]:failed with %#x!\n", __func__, s32Ret);
		return 0;
	}
	/*设置并使能视频通道*/
	memset(&stLayerAttr, 0, sizeof(stLayerAttr));
	s32Ret = HI_MPI_VO_GetVideoLayerAttr(VoLayer, &stLayerAttr); /*获取视频层属性*/
	if (s32Ret != HI_SUCCESS){
		printf("failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}
	/*下面的这些值决定，此通到的显示位置和图像的大小*/
	stChnAttr.stRect.s32X       = 0; //ALIGN_BACK((u32Width/u32Square) * (i%u32Square), 2);
	stChnAttr.stRect.s32Y       = 0; //ALIGN_BACK((u32Height/u32Square) * (i/u32Square), 2);
	stChnAttr.stRect.u32Width   = 1080;//1080; //720;//stLayerAttr.stDispRect.u32Width; //ALIGN_BACK(u32Width/u32Square, 2);
	stChnAttr.stRect.u32Height  = 720; //480;//stLayerAttr.stDispRect.u32Height; //ALIGN_BACK(u32Height/u32Square, 2);
	stChnAttr.u32Priority       = 0; /*视频层优先级*/
	stChnAttr.bDeflicker        = HI_TRUE; /*是否开启抗闪烁*/
	printf("[%s]:enable VO=%d-%d size=%d-%d  line:%d\n", __func__, VoLayer , VOChNum, (stChnAttr.stRect.u32Width), (stChnAttr.stRect.u32Height),  __LINE__);
	s32Ret = HI_MPI_VO_SetChnAttr(VoLayer, VOChNum, &stChnAttr); /*配置指定视频输出通道的属性*/
	if (s32Ret != HI_SUCCESS){
		printf("[%s]:Start HI_MPI_VO_SetChnAttr failed!\n", __func__);
		return HI_FAILURE;
	}
	 /*启用指定的视频输出通道*/
	s32Ret = HI_MPI_VO_EnableChn(VoLayer, VOChNum);
	if (s32Ret != HI_SUCCESS){
		printf("failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}
	s32Ret= HI_MPI_VO_SetAttrEnd(VoLayer); /*设置视频层上的通道的设置属性结束*/
	if (HI_SUCCESS != s32Ret){
		printf("[%s]:failed with %#x!\n", __func__, s32Ret);
		return 0;
	}
	return 0;
}

#if 0
/***************************************************************************************
* Description:设置 VO中 VGA
*****************************************************************************************/
int setVOAttrVGA(){
	VPSS_CHN VpssChn_VoHD0 = VPSS_CHN1;
	VPSS_GRP VpssGrp;
	HI_S32 s32Ret = HI_SUCCESS, i = 0;
	VO_DEV VoDev;
	VO_LAYER VoLayer;
	VO_CHN VoChn;
	HI_U32 u32WndNum;
	SAMPLE_VO_MODE_E enVoMode;
	VO_PUB_ATTR_S stVoPubAttr_hd0;
	VO_VIDEO_LAYER_ATTR_S stLayerAttr;
	#if 1 /*输出设备：J10 VGA输出*/	
	/******************************************
	 step 5: start vo HD0 (bt1120+VGA), multi-screen, you can switch mode
	******************************************/
	printf("\n[%s]:****** start step 5: start vo HD0 (bt1120+VGA). ****** line:%d\n", __func__, __LINE__);
	VoDev = SAMPLE_VO_DEV_DHD0;
	stVoPubAttr_hd0.enIntfSync = VO_OUTPUT_1080P60; /*Vo接口时序类型*/
	stVoPubAttr_hd0.enIntfType = VO_INTF_BT1120|VO_INTF_VGA; /*Vo 接口类型*/
	stVoPubAttr_hd0.u32BgColor = 0x000000ff; /*0x00ff60 设备背景色 RGB表示*/
	s32Ret = SAMPLE_COMM_VO_StartDev(VoDev, &stVoPubAttr_hd0); /*设置并使能VO设备*/
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartDev failed!\n");
		//goto END_8_1080P_3;
		return HI_FAILURE;
	}
	/*视频层相关属性*/
	memset(&(stLayerAttr), 0 , sizeof(VO_VIDEO_LAYER_ATTR_S));
	s32Ret = SAMPLE_COMM_VO_GetWH(stVoPubAttr_hd0.enIntfSync, \
		&stLayerAttr.stImageSize.u32Width, \
		&stLayerAttr.stImageSize.u32Height, \
		&stLayerAttr.u32DispFrmRt);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_GetWH failed!\n");
		//goto END_8_1080P_4;
		return HI_FAILURE;
	}
	/*视频显示的坐标还有大小*/
	stLayerAttr.enPixFormat = SAMPLE_PIXEL_FORMAT; /*视频层使用的像素格式*/
    stLayerAttr.stDispRect.s32X       = 0;
    stLayerAttr.stDispRect.s32Y       = 0;
    stLayerAttr.stDispRect.u32Width   = stLayerAttr.stImageSize.u32Width;
    stLayerAttr.stDispRect.u32Height  = stLayerAttr.stImageSize.u32Height;
	VoLayer = SAMPLE_VO_LAYER_VHD0;
	s32Ret = SAMPLE_COMM_VO_StartLayer(VoLayer, &stLayerAttr); /*设置并使能视频层属性*/
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartLayer failed!\n");
		//goto END_8_1080P_4;
		return HI_FAILURE;
	}
	 /*设置并使能视频通道*/
	enVoMode = VO_MODE_9MUX;
	s32Ret = SAMPLE_COMM_VO_StartChn(VoLayer, enVoMode);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartChn failed!\n");
		//goto END_8_1080P_5;
		return HI_FAILURE;
	}
	/*vpass 和 VO设备之间建立bind关系*/
	u32WndNum = 8;
	printf("\n[%s]:bind vpass and VO:", __func__);
	for(i=0;i<u32WndNum;i++)
	{
		VoChn = i;
		VpssGrp = i;
		printf("vpas=%d-%d VO=%d-%d  ", VpssGrp, VpssChn_VoHD0, VoDev, VoChn);
		s32Ret = SAMPLE_COMM_VO_BindVpss(VoDev,VoChn,VpssGrp,VpssChn_VoHD0);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("Start VO failed!\n");
			//goto END_8_1080P_5;
			return HI_FAILURE;
		}
	}
#endif
	return 0;
}
#endif

#if 0
/*****************************************************************************
* Description:mpp的初始化
*****************************************************************************/
int initMpp(){
	
	HI_S32 s32Ret = HI_SUCCESS;
	HI_U32 u32BlkSize = 0;
	HI_U32 u32ViChnCnt = 8;
	VB_CONF_S stVbConf; /*定义视频缓存池属性结构体*/
	//SAMPLE_VI_MODE_E enViMode = SAMPLE_VI_MODE_8_1080P; //SAMPLE_VI_MODE_8_720P; //SAMPLE_VI_MODE_8_1080P;
	VIDEO_NORM_E enNorm = VIDEO_ENCODING_MODE_AUTO; /*视频输入制式类型*/
	
	printf("[%s]:Begin line:%d\n", __func__, __LINE__);
	/******************************************
	 step  1: init variable 
	******************************************/	
	printf("[%s]:****** step  1: init variable ****** line:%d\n", __func__, __LINE__);
	memset(&stVbConf,0,sizeof(VB_CONF_S));
	u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(enNorm,\
				PIC_HD1080, SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH,COMPRESS_MODE_SEG);
	stVbConf.u32MaxPoolCnt = 128; /*系统所能容纳的缓存池的个数*/

	/* video buffer*/
	//todo: vb=15
	stVbConf.astCommPool[0].u32BlkSize = u32BlkSize; /*公共缓存快的0的大小*/
	stVbConf.astCommPool[0].u32BlkCnt = u32ViChnCnt * 8; /*缓存块的个数*/

	/******************************************
	 step 2: mpp system init. 
	******************************************/
	printf("[%s]:******  step 2: mpp system init.  ****** line:%d\n", __func__, __LINE__);
	s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf); /*mpp的初始化设置*/
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("system init failed with %d!\n", s32Ret);
		//goto END_8_1080P_0;
		return -1;
	}
	return 0;
}
#endif

/******************************************************************************
* Description：解bind某路AI和AO0的bind关系
* Input:type-操作类型1-bind或是0-unbind 
* Input:pstSrcChn-源通道道路 pstDestChn-目标通道
******************************************************************************/
HI_S32 bindORunBindAIAO(HI_S32 type, MPP_CHN_S *pstSrcChn, MPP_CHN_S *pstDestChn){
	HI_S32 s32Ret = 0; 

	printf("[%s]:type=%d VI=%d-%d VO-%d-%d line:%d\n", __func__, type, (pstSrcChn->s32DevId), \
		(pstSrcChn->s32ChnId), (pstDestChn->s32DevId), (pstDestChn->s32ChnId), __LINE__);
	if(1 == type){	
		s32Ret = HI_MPI_SYS_Bind(pstSrcChn, pstDestChn); /*bind操作*/
		if (s32Ret != HI_SUCCESS){
			printf("[%s]:failed with bind %#x!\n", __func__, s32Ret);
			return HI_FAILURE;
		}
		return 0;
	}
	s32Ret = HI_MPI_SYS_UnBind(pstSrcChn, pstDestChn); /*解bind操作*/
	if (s32Ret != HI_SUCCESS){
		printf("[%s]:failed with Unbind %#x!\n", __func__, s32Ret);
		return HI_FAILURE;
	}
	return 0;
}

/******************************************************************************
* Description：bind或是解bind某两个模块之间的关系
* Input:type-操作类型1-bind或是0-unbind 
* Input:pstSrcChn-源通道道路 pstDestChn-目标通道
******************************************************************************/
HI_S32 bindORunBindModule(HI_S32 type, MPP_CHN_S *pstSrcChn, MPP_CHN_S *pstDestChn){
	HI_S32 s32Ret = 0; 
#if 0
	printf("[%s]:type=%d SRC=%d-%d DEST-%d-%d line:%d\n", __func__, type, (pstSrcChn->s32DevId), \
		(pstSrcChn->s32ChnId), (pstDestChn->s32DevId), (pstDestChn->s32ChnId), __LINE__);
#endif
	if(1 == type){	
		s32Ret = HI_MPI_SYS_Bind(pstSrcChn, pstDestChn); /*bind操作*/
		if (s32Ret != HI_SUCCESS){
			printf("[%s]:failed with bind %#x! SRC=%d-%d DEST=%d-%d line:%d\n", __func__, s32Ret, \
				(pstSrcChn->s32DevId), (pstSrcChn->s32ChnId), (pstDestChn->s32DevId), (pstDestChn->s32ChnId), __LINE__);
			return HI_FAILURE;
		}
		return 0;
	}
	s32Ret = HI_MPI_SYS_UnBind(pstSrcChn, pstDestChn); /*解bind操作*/
	if (s32Ret != HI_SUCCESS){
		printf("[%s]:failed with Unbind %#x! SRC=%d-%d DEST=%d-%d line:%d\n", __func__, s32Ret,\
		(pstSrcChn->s32DevId), (pstSrcChn->s32ChnId), (pstDestChn->s32DevId), (pstDestChn->s32ChnId), __LINE__);
		return HI_FAILURE;
	}
	return 0;
}


#if 0
/*********************************************************************************************************
* func:获取某路VI输入，直接通过VGA输出
*********************************************************************************************************/
int funcVOFromVI(){
	HI_S32 s32VpssGrpCnt = 8;
	SIZE_S stSize;
	HI_S32 s32Ret = HI_FAILURE;
	VO_VIDEO_LAYER_ATTR_S stLayerAttr;
	VPSS_GRP_ATTR_S stGrpAttr; /*vpass 静态属性信息*/
	SAMPLE_VI_MODE_E enViMode = SAMPLE_VI_MODE_8_1080P; //SAMPLE_VI_MODE_8_720P; //SAMPLE_VI_MODE_8_1080P;
	VIDEO_NORM_E enNorm = VIDEO_ENCODING_MODE_AUTO; /*视频输入制式类型*/
	VO_LAYER VoLayer = SAMPLE_VO_LAYER_VHD0; /*视频层*/
	HI_S32 VOCHNum = 0; /*VO通道号*/
	MPP_CHN_S stSrcChn;
	MPP_CHN_S stDestChn;
	HI_S32 VIChNum = 0; /*VI通道号*/
	HI_S32 VIDevNum = 0; /*VI设备号*/

	initMpp();
	printf("[%s]:****** step 3: start vi dev & chn ****** line:%d\n", __func__, __LINE__);
	s32Ret = SAMPLE_COMM_VI_Start(enViMode, enNorm); /*开启VI设备*/
	if (HI_SUCCESS != s32Ret){
		SAMPLE_PRT("[%s]:start vi failed!\n", __func__);
		return -1;
	}
	/*设置VGA的属性*/
	VOCHNum = 0;
	openVOLayerCH(VoLayer, VOCHNum); /*打开VO设备X视频层的X通道*/
	/* VI和VO进行通道bind     , 源通道指针*/
	VIDevNum = 0;
	VIChNum = 12;
	stSrcChn.enModId = HI_ID_VIU; /*模块号*/
	stSrcChn.s32DevId = VIDevNum; /*设备号*/
	stSrcChn.s32ChnId = VIChNum; /*通道号*/
	/*目的通道指针*/
	stDestChn.enModId = HI_ID_VOU;
	stDestChn.s32DevId = VoLayer;
	stDestChn.s32ChnId = VOCHNum;
	/*数据源到数据接收者绑定*/
	printf("[%s]:VI=%d-%d VO=%d-%d line:%d\n", __func__, VIDevNum, VIChNum, VoLayer, VOCHNum, __LINE__);
	s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
	if (s32Ret != HI_SUCCESS){
		SAMPLE_PRT("[%s]:failed with %#x!\n", __func__, s32Ret);
		return HI_FAILURE;
	}
	
	return 0;
}
#endif

int getVP(){
	//HI_MPI_VPSS_GetGrpFrameRate();
}

/***************************************************************************************
* Description:获取Vpass模块的一些属性信息
* Input:s32DevId-组号 s32ChnId-通道号
****************************************************************************************/
int printVPAttr(HI_S32 s32DevId, HI_S32 s32ChnId){
	HI_S32 s32Ret = 0, i = 0, devID = 0, chID = 0;
	VPSS_FRAME_RATE_S frameRate;
	VPSS_SIZER_INFO_S sizeInfo;
	VPSS_CHN_MODE_S chMode;

	printf("[%s]:****** VP=%d-%d ****** line:%d\n", __func__, s32DevId, s32ChnId, __LINE__);
	/*************************************************/
	memset(&frameRate, 0, sizeof(frameRate));
	s32Ret = HI_MPI_VPSS_GetGrpFrameRate(s32DevId, &frameRate); /*获取 VPSS 帧率控制信息(源帧率和目标帧率)*/
	if (HI_SUCCESS != s32Ret){
		printf("[%s]:failed with %#x! line:%d\n", __func__, s32Ret, __LINE__);
		return 0;
	}
	printf("[%s]:s32SrcFrmRate=%d s32DstFrmRate=%d line:%d\n", __func__, (frameRate.s32SrcFrmRate), (frameRate.s32DstFrmRate), __LINE__);

	/*************************************************/
	memset(&sizeInfo, 0, sizeof(sizeInfo));
	s32Ret = HI_MPI_VPSS_GetGrpSizer(s32DevId, &sizeInfo); /*获取 VPSS 尺寸筛选信息*/
	if (HI_SUCCESS != s32Ret){
		printf("[%s]:failed with %#x! line:%d\n", __func__, s32Ret, __LINE__);
		return 0;
	}
	printf("[%s]:bSizer=%d size=%d*%d line:%d\n", __func__, (int)(sizeInfo.bSizer), (sizeInfo.stSize.u32Width), (sizeInfo.stSize.u32Height), __LINE__);

	/*************************************************/
	memset(&chMode, 0, sizeof(chMode));
	s32Ret = HI_MPI_VPSS_GetChnMode(s32DevId, s32ChnId, &chMode); /*获取VPSS通道工作模式(包含分辨率信息)*/
	if (HI_SUCCESS != s32Ret){
		printf("[%s]:failed with %#x! line:%d\n", __func__, s32Ret, __LINE__);
		return 0;
	}
	printf("[%s]:enChnMode=%d size=%d*%d@%d stFrameRate=%d line:%d\n", __func__, (chMode.enChnMode), (chMode.u32Width), (chMode.u32Height), (chMode.stFrameRate), (chMode.enPixelFormat), __LINE__);
	
	return 0;
}

/************************************************************************************
* Description:解出VPass 和 VO的bind 关系
*************************************************************************************/
int unBindVPAndVO(HI_S32 VPCh, HI_S32 VOLayer){
	HI_S32 i = 0;
	HI_S32 VoChID = VOLayer, VoDevID = 0;
	HI_S32 VPDevID = 0, VPChID = VPCh;
	MPP_CHN_S sSrcChn, sDestChn;

	memset(&sSrcChn, 0, sizeof(sSrcChn));
	memset(&sDestChn, 0, sizeof(sDestChn));
	sSrcChn.enModId = HI_ID_VPSS; /*模块号*/
	sSrcChn.s32DevId = VPDevID; /*设备号*/
	sSrcChn.s32ChnId = VPChID; /*通道号*/
	/*目的通道指针*/
	sDestChn.enModId = HI_ID_VOU;
	sDestChn.s32DevId = VoChID;
	sDestChn.s32ChnId = VoDevID;
	for(i=0; i<9; i++){
		VoChID = i;
		VPDevID = i;
		sSrcChn.s32DevId = VPDevID;
		sDestChn.s32ChnId = VoChID;
		bindORunBindModule(0, &sSrcChn, &sDestChn); /*unbind*/
	}
	
	return 0;
}

/******************************************************************************************
* Description:打印获取到的VO视频层属性信息
*******************************************************************************************/
int printVOLayerAttr(const VO_VIDEO_LAYER_ATTR_S *p_attr){
	printf("[%s]:DispRect=%d*%d@%d ImageSize=%d*%d enPixFormat=%d line:%d\n", __func__, (p_attr->stDispRect.u32Width),
		(p_attr->stDispRect.u32Height), (p_attr->u32DispFrmRt),
		(p_attr->stImageSize.u32Width), (p_attr->stImageSize.u32Height), (p_attr->enPixFormat), __LINE__);
	return 0;
}

/*****************************************************************************************
* Description:让某通道画面全屏显示（X视频层的X通道画面全屏显示）通过修改VO模块的相关参数实现
* Input:s32DevId-VO视频层号 s32ChnId-通道号
* Return:0-成功 0<-失败
******************************************************************************************/
int fullScreen(HI_S32 s32DevId, HI_S32 s32ChnId){
	HI_S32 s32Ret = HI_SUCCESS;
	VO_VIDEO_LAYER_ATTR_S stLayerAttr;
	VO_CHN_ATTR_S stChnAttr;

	disVOCHNum(s32DevId, -1); /*将VO的0层的所有通道禁掉*/
	s32Ret= HI_MPI_VO_SetAttrBegin(s32DevId); /*设置视频层上的通道的设置属性开始*/
	if (HI_SUCCESS != s32Ret){
		printf("[%s]:failed with %#x! line:%d\n", __func__, s32Ret, __LINE__);
		return 0;
	}
	memset(&stLayerAttr, 0, sizeof(stLayerAttr));
	s32Ret = HI_MPI_VO_GetVideoLayerAttr(s32DevId, &stLayerAttr); /*获取视频层属性*/
	if (s32Ret != HI_SUCCESS){
		printf("[%s]:failed with %#x! line:%d\n", __func__, s32Ret, __LINE__);
		return HI_FAILURE;
	}
	printVOLayerAttr(&stLayerAttr);
	/*下面的这些值决定，此通到的显示位置和图像的大小*/
	memset(&stChnAttr, 0, sizeof(stChnAttr));
	stChnAttr.stRect.s32X       = 0; /*窗口的起始坐标*/
	stChnAttr.stRect.s32Y       = 0;
	stChnAttr.stRect.u32Width   = (stLayerAttr.stDispRect.u32Width); /*窗口的宽度*/
	stChnAttr.stRect.u32Height  = (stLayerAttr.stDispRect.u32Height); /*窗口的高度*/
	stChnAttr.u32Priority       = 0; /*视频层优先级*/
	stChnAttr.bDeflicker        = HI_TRUE; /*是否开启抗闪烁*/
	printf("[%s]:channel attr VO=%d-%d size=%d-%d  line:%d\n", __func__, s32DevId , s32ChnId, (stChnAttr.stRect.u32Width), (stChnAttr.stRect.u32Height),  __LINE__);
	s32Ret = HI_MPI_VO_SetChnAttr(s32DevId, s32ChnId, &stChnAttr); /*配置指定视频输出通道的属性*/
	if (s32Ret != HI_SUCCESS){
		printf("[%s]:Start HI_MPI_VO_SetChnAttr failed! line:%d\n", __func__, __LINE__);
		return HI_FAILURE;
	}
	s32Ret = HI_MPI_VO_EnableChn(s32DevId, s32ChnId); /*启用指定的视频输出通道*/
	if (s32Ret != HI_SUCCESS){
		printf("failed with %#x! line:%d\n", __func__, s32Ret, __LINE__);
		return HI_FAILURE;
	}
	s32Ret= HI_MPI_VO_SetAttrEnd(s32DevId); /*设置视频层上的通道的设置属性结束*/
	if (HI_SUCCESS != s32Ret){
		printf("[%s]:failed with %#x! line:%d\n", __func__, s32Ret, __LINE__);
		return 0;
	}
	
	return 0;
} 

/*************************************************************************************
* Description:设置VPSS通道工作模式(User 模式)
**************************************************************************************/
int setVPChModeUser(HI_S32 s32DevId, HI_S32 s32ChnId){
#if 1	
	HI_S32 s32Ret= 0;
	VPSS_CHN_MODE_S stVpssChnMode;
	HI_S32 VpssGrp = s32DevId;
		VpssGrp = s32DevId;
		s32Ret = HI_MPI_VPSS_GetChnMode(VpssGrp, 0, &stVpssChnMode);
		if (HI_SUCCESS != s32Ret)
		{
			printf("get Vpss chn mode failed!\n");
		}
		memset(&stVpssChnMode,0,sizeof(VPSS_CHN_MODE_S));
		//high
		stVpssChnMode.enChnMode = VPSS_CHN_MODE_USER;
		stVpssChnMode.u32Width  = 1920;
		stVpssChnMode.u32Height = 1080;
		stVpssChnMode.stFrameRate.s32DstFrmRate = -1;
		stVpssChnMode.stFrameRate.s32SrcFrmRate = -1;
		stVpssChnMode.enPixelFormat  = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
		stVpssChnMode.enCompressMode = COMPRESS_MODE_NONE;
		s32Ret = HI_MPI_VPSS_SetChnMode(VpssGrp, 0, &stVpssChnMode);
		if (HI_SUCCESS != s32Ret)
		{
			printf("Set Vpss chn mode failed!\n");
			return HI_FAILURE;
		}
		return 0;
#endif

#if 0
	HI_S32 s32Ret = HI_SUCCESS;
	VPSS_CHN_MODE_S chMode;

	memset(&chMode, 0, sizeof(chMode));
	chMode.enChnMode = VPSS_CHN_MODE_USER;
	chMode.u32Width  = 1920;
	chMode.u32Height = 1080;
	chMode.stFrameRate.s32DstFrmRate = -1;
	chMode.stFrameRate.s32SrcFrmRate = -1;
	chMode.enPixelFormat  = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
	chMode.enCompressMode = COMPRESS_MODE_NONE;
	s32Ret= HI_MPI_VPSS_SetChnMode(s32DevId, s32ChnId, &chMode); /*设置VPSS通道工作模式*/
	if (HI_SUCCESS != s32Ret){
		printf("[%s]:failed with %#x! line:%d\n", __func__, s32Ret, __LINE__);
		return -1;
	}
	return 0;
#endif
}

/*************************************************************************************
* Description:设置VPSS通道工作模式(Auto 模式)
**************************************************************************************/
int setVPChModeAuto(HI_S32 s32DevId, HI_S32 s32ChnId){
	HI_S32 s32Ret = HI_SUCCESS;
	VPSS_CHN_MODE_S chMode;

	memset(&chMode, 0, sizeof(chMode));
	chMode.enChnMode = VPSS_CHN_MODE_AUTO;
	chMode.u32Width  = 1920;
	chMode.u32Height = 1080;
	chMode.stFrameRate.s32DstFrmRate = -1;
	chMode.stFrameRate.s32SrcFrmRate = -1;
	chMode.enPixelFormat  = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
	chMode.enCompressMode = COMPRESS_MODE_NONE;
	s32Ret= HI_MPI_VPSS_SetChnMode(s32DevId, s32ChnId, &chMode); /*设置VPSS通道工作模式*/
	if (HI_SUCCESS != s32Ret){
		printf("[%s]:failed with %#x! line:%d\n", __func__, s32Ret, __LINE__);
		return -1;
	}
	return 0;
}


/****************************************************************************************
* Description:将不同的VI通过VGA的视频层0通道0输出
****************************************************************************************/
int showVINum(HI_S32 chNum){
#if 1
	MPP_CHN_S sSrcChn, sDestChn;

	disVOCHNum(0, 0); /*将VO的0层的0通道禁掉*/
	memset(&sSrcChn, 0, sizeof(sSrcChn));
	memset(&sDestChn, 0, sizeof(sDestChn));
	sSrcChn.enModId = HI_ID_VIU; /*模块号*/
	sSrcChn.s32DevId = 0; /*设备号*/
	sSrcChn.s32ChnId = chNum; /*通道号*/
	/*目的通道指针*/
	sDestChn.enModId = HI_ID_VOU;
	sDestChn.s32DevId = g_VoLayer; //SAMPLE_VO_LAYER_VHD0;
	sDestChn.s32ChnId = 0;
	printf("[%s]:show VI=%d-%d VO=%d-%d line:%d\n", __func__, (sSrcChn.s32DevId), (sSrcChn.s32ChnId), \
		(sDestChn.s32DevId), (sDestChn.s32ChnId),  __LINE__);
	/*数据源到数据接收者绑定*/
	//openVOLayerCH((sDestChn.s32DevId), (sDestChn.s32ChnId)); /*打开VO设备X视频层的X通道*/
	startVOCHx((sDestChn.s32DevId), (sDestChn.s32ChnId)); /*打开VO设备X视频层的X通道*/
	bindORunBindAIAO(0, &sSrcChn, &sDestChn); /*bind*/
	bindORunBindAIAO(1, &sSrcChn, &sDestChn); /*unbind*/

#endif
#if 0
	MPP_CHN_S sSrcChn, sDestChn;

	disVOCHNum(0, 0); /*将VO的0层的0通道禁掉*/
	memset(&sSrcChn, 0, sizeof(sSrcChn));
	memset(&sDestChn, 0, sizeof(sDestChn));
	sSrcChn.enModId = HI_ID_VIU; /*模块号*/
	sSrcChn.s32DevId = 0; /*设备号*/
	sSrcChn.s32ChnId = chNum; /*通道号*/
	/*目的通道指针*/
	sDestChn.enModId = HI_ID_VOU;
	sDestChn.s32DevId = g_VoLayer; //SAMPLE_VO_LAYER_VHD0;
	sDestChn.s32ChnId = 0;
	printf("[%s]:show VI=%d-%d VO=%d-%d line:%d\n", __func__, (sSrcChn.s32DevId), (sSrcChn.s32ChnId), \
		(sDestChn.s32DevId), (sDestChn.s32ChnId),  __LINE__);
	/*数据源到数据接收者绑定*/
	//openVOLayerCH((sDestChn.s32DevId), (sDestChn.s32ChnId)); /*打开VO设备X视频层的X通道*/
	startVOCHx((sDestChn.s32DevId), (sDestChn.s32ChnId)); /*打开VO设备X视频层的X通道*/
	bindORunBindAIAO(0, &sSrcChn, &sDestChn); /*bind*/
	bindORunBindAIAO(1, &sSrcChn, &sDestChn); /*unbind*/
#endif
	return 0;
}

/****************************************************************************************
* Description:将VI 0-0通0-X通道输出
****************************************************************************************/
int showVONum(HI_S32 chNum){
	MPP_CHN_S sSrcChn, sDestChn;
	
	disVOCHNum(0, -1); /*将VO的0层的所有通道禁掉*/
	memset(&sSrcChn, 0, sizeof(sSrcChn));
	memset(&sDestChn, 0, sizeof(sDestChn));
	sSrcChn.enModId = HI_ID_VIU; /*模块号*/
	sSrcChn.s32DevId = 0; /*设备号*/
	sSrcChn.s32ChnId = 12; /*通道号*/
	/*目的通道指针*/
	sDestChn.enModId = HI_ID_VOU;
	sDestChn.s32DevId = g_VoLayer; //SAMPLE_VO_LAYER_VHD0;
	sDestChn.s32ChnId = chNum;
	/*数据源到数据接收者绑定*/
	//openVOLayerCH((sDestChn.s32DevId), (sDestChn.s32ChnId)); /*打开VO设备X视频层的X通道*/
	startVOCHx((sDestChn.s32DevId), (sDestChn.s32ChnId)); /*打开VO设备X视频层的X通道*/
	bindORunBindAIAO(0, &sSrcChn, &sDestChn); /*unbind*/
	bindORunBindAIAO(1, &sSrcChn, &sDestChn); /*bind*/
	return 0;
}


/******************************************************************************
 * function : show usage
 ******************************************************************************/
void Usage(char *sPrgNm)
{
	fprintf(stderr,"Usage : %s + <index> (eg: %s 0)\n", sPrgNm,sPrgNm);
	fprintf(stderr,"index:\n");
	fprintf(stderr,"\t0:  VI 8Mux 1080P input, HD0(VGA+BT1120),HD1(HDMI)/SD VO \n");	
	fprintf(stderr,"\t1:  VI 1Mux 1080p input, HD ZoomIn\n");	
	fprintf(stderr,"\tq:  quit\n");

	return;
}


/******************************************************************************
 * function : to process abnormal case                                         
 ******************************************************************************/
void HandleSig(HI_S32 signo)
{
	HI_S32 i;
	if (SIGINT == signo || SIGTSTP == signo)
	{

		HI_MPI_SYS_Exit();
		for(i=0;i<VB_MAX_USER;i++)
		{
			HI_MPI_VB_ExitModCommPool(i);
		}
		for(i=0; i<VB_MAX_POOLS; i++)
		{
			HI_MPI_VB_DestroyPool(i);
		}	
		HI_MPI_VB_Exit();
		fprintf(stderr,"\033[0;31mprogram termination abnormally!\033[0;39m\n");
	}
	exit(-1);
}






/******************************************************************************
 * function : vb init & MPI system init
 ******************************************************************************/
HI_S32 sys_init()
{
	VB_CONF_S stVbConf;
	HI_S32 i;
	HI_S32 s32Ret = HI_FAILURE;
	HI_U32 u32BlkSize;
	SIZE_S stSize;

	HI_U32 u32Width;
	HI_U32 u32Height;
	HI_S32 u32AlignWidth;
	COMPRESS_MODE_E enCompFmt = 0;
	HI_U32 u32ViChnCnt = 8;
	HI_U32 u32HeaderSize 	= 0;

	////init mpp system and  common VB 
	memset(&stVbConf,0,sizeof(VB_CONF_S));	
	//vb format:PIXEL_FORMAT_YUV_SEMIPLANAR_422 PIXEL_FORMAT_YUV_SEMIPLANAR_420
	//vb align_width 16 32 64
	PIXEL_FORMAT_E enPixFmt = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
	stSize.u32Width = 1920;
	stSize.u32Height = 1080;
	u32AlignWidth = 16;
	u32Width  = CEILING_2_POWER(stSize.u32Width, u32AlignWidth);
	u32Height = CEILING_2_POWER(stSize.u32Height,u32AlignWidth);
	fprintf(stderr,"[%s]:u32Width*u32Height:%d*%d\n", __func__, u32Width,u32Height);	
	if (PIXEL_FORMAT_YUV_SEMIPLANAR_422 == enPixFmt)
	{
		u32BlkSize = u32Width * u32Height * 2;
	}
	else
	{
		u32BlkSize = u32Width * u32Height * 3 / 2;
	}

	if(COMPRESS_MODE_SEG == enCompFmt)
	{
		VB_PIC_HEADER_SIZE(u32Width,u32Height,enPixFmt,u32HeaderSize);
	}
	u32BlkSize += u32HeaderSize;
	stVbConf.u32MaxPoolCnt = 128;
	/* video buffer*/
	//todo: vb=15   to vi 模块
	stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
	stVbConf.astCommPool[0].u32BlkCnt = u32ViChnCnt * 8;


	MPP_SYS_CONF_S stSysConf = {0};

	HI_MPI_SYS_Exit();

	for(i=0;i<VB_MAX_USER;i++)
	{
		HI_MPI_VB_ExitModCommPool(i);
	}
	for(i=0; i<VB_MAX_POOLS; i++)
	{
		HI_MPI_VB_DestroyPool(i);
	}
	HI_MPI_VB_Exit();

	if (NULL == &stVbConf)
	{
		fprintf(stderr,"input parameter is null, it is invaild!\n");
		return HI_FAILURE;
	}
	s32Ret = HI_MPI_VB_SetConf(&stVbConf);
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"HI_MPI_VB_SetConf failed!\n");
		return HI_FAILURE;
	}

	s32Ret = HI_MPI_VB_Init();
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"HI_MPI_VB_Init failed!\n");
		return HI_FAILURE;
	}

	stSysConf.u32AlignWidth = 16;
	s32Ret = HI_MPI_SYS_SetConf(&stSysConf);
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"HI_MPI_SYS_SetConf failed\n");
		return HI_FAILURE;
	}

	s32Ret = HI_MPI_SYS_Init();
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"HI_MPI_SYS_Init failed!\n");
		return HI_FAILURE;
	}

	return HI_SUCCESS;
}







/******************************************************************************
 * function : vb init & MPI system init
 ******************************************************************************/
HI_S32 sys_dec_init()
{
	VB_CONF_S stVbConf;
	HI_S32 i;
	HI_S32 s32Ret = HI_FAILURE;
	HI_U32 u32BlkSize;
	SIZE_S stSize;

	HI_U32 u32Width;
	HI_U32 u32Height;
	HI_S32 u32AlignWidth;
	COMPRESS_MODE_E enCompFmt = 0;
	HI_U32 u32ViChnCnt = 8;
	HI_U32 u32HeaderSize 	= 0;

	////init mpp system and  common VB 
	memset(&stVbConf,0,sizeof(VB_CONF_S));	
	//vb format:PIXEL_FORMAT_YUV_SEMIPLANAR_422 PIXEL_FORMAT_YUV_SEMIPLANAR_420
	//vb align_width 16 32 64
	PIXEL_FORMAT_E enPixFmt = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
	stSize.u32Width = 1920;
	stSize.u32Height = 1080;
	fprintf(stderr,"u32Width*u32Height:%d*%d\n",u32Width,u32Height);	
	if (PIXEL_FORMAT_YUV_SEMIPLANAR_422 == enPixFmt)
	{
		u32BlkSize = u32Width * u32Height * 2;
	}
	else
	{
		u32BlkSize = u32Width * u32Height * 3 / 2;
	}
	stVbConf.u32MaxPoolCnt = 2;
	stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
	stVbConf.astCommPool[0].u32BlkCnt = 20;


	MPP_SYS_CONF_S stSysConf = {0};

	HI_MPI_SYS_Exit();

	for(i=0;i<VB_MAX_USER;i++)
	{
		HI_MPI_VB_ExitModCommPool(i);
	}
	for(i=0; i<VB_MAX_POOLS; i++)
	{
		HI_MPI_VB_DestroyPool(i);
	}
	HI_MPI_VB_Exit();

	if (NULL == &stVbConf)
	{
		fprintf(stderr,"input parameter is null, it is invaild!\n");
		return HI_FAILURE;
	}
	s32Ret = HI_MPI_VB_SetConf(&stVbConf);
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"HI_MPI_VB_SetConf failed!\n");
		return HI_FAILURE;
	}

	s32Ret = HI_MPI_VB_Init();
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"HI_MPI_VB_Init failed!\n");
		return HI_FAILURE;
	}

	stSysConf.u32AlignWidth = 16;
	s32Ret = HI_MPI_SYS_SetConf(&stSysConf);
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"HI_MPI_SYS_SetConf failed\n");
		return HI_FAILURE;
	}

	s32Ret = HI_MPI_SYS_Init();
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"HI_MPI_SYS_Init failed!\n");
		return HI_FAILURE;
	}

	return HI_SUCCESS;
}


/*****************************************************************************
 * function : set vi mask.
 *****************************************************************************/
HI_VOID VI_SetMask(VI_DEV ViDev, VI_DEV_ATTR_S *pstDevAttr)
{
	switch (ViDev % 2)
	{
	case 0: 
		pstDevAttr->au32CompMask[0] = 0xFF0000;
		pstDevAttr->au32CompMask[1] = 0x0;
		break;

	case 1:
		pstDevAttr->au32CompMask[0] = 0xFF000000;
		pstDevAttr->au32CompMask[1] = 0x0;              
		break;
	default:
		HI_ASSERT(0);
	}
}

/*****************************************************************************
 * function : get vi parameter, according to vi type
 *****************************************************************************/
HI_S32 VI_Mode2Param(DEMO_VI_MODE_E enViMode, DEMO_VI_PARAM_S *pstViParam)
{
	switch (enViMode)
	{
	case DEMO_VI_MODE_32_D1:
	case DEMO_VI_MODE_32_960H:
	case DEMO_VI_MODE_32_1280H:
	case DEMO_VI_MODE_32_HALF720P:
		pstViParam->s32ViDevCnt = 4;
		pstViParam->s32ViDevInterval = 1;
		pstViParam->s32ViChnCnt = 16;
		pstViParam->s32ViChnInterval = 1;
		break;
	case DEMO_VI_MODE_16_720P:
	case DEMO_VI_MODE_16_1080P:
		/* use chn 0,2,4,6,8,10,12,14,16,18,20,22,24,28 */
		pstViParam->s32ViDevCnt = 8;
		pstViParam->s32ViDevInterval = 1;
		pstViParam->s32ViChnCnt = 16;
		pstViParam->s32ViChnInterval = 2;
		break;
	case DEMO_VI_MODE_8_720P:
	case DEMO_VI_MODE_8_1080P:        
		/* use chn 0,4,8,12,16,20,24,28 */
		pstViParam->s32ViDevCnt = 8;
		pstViParam->s32ViDevInterval = 1;
		pstViParam->s32ViChnCnt = 8;
		pstViParam->s32ViChnInterval = 4;
		break;
	default:
		fprintf(stderr,"ViMode invaild!\n");
		return HI_FAILURE;
	}
	return HI_SUCCESS;
}

/*****************************************************************************
 * function : get vi size, according to vi type
 *****************************************************************************/
HI_S32 VI_Mode2Size(DEMO_VI_MODE_E enViMode, VIDEO_NORM_E enNorm, RECT_S *pstCapRect, SIZE_S *pstDestSize)
{
	pstCapRect->s32X = 0;
	pstCapRect->s32Y = 0;
	switch (enViMode)
	{
	case DEMO_VI_MODE_32_D1:
		pstDestSize->u32Width = DEMO_D1_WIDTH;
		pstDestSize->u32Height = (VIDEO_ENCODING_MODE_PAL==enNorm)?576:480;
		pstCapRect->u32Width = DEMO_D1_WIDTH;
		pstCapRect->u32Height = (VIDEO_ENCODING_MODE_PAL==enNorm)?576:480;
		break;
	case DEMO_VI_MODE_32_960H:
		pstDestSize->u32Width = 960;
		pstDestSize->u32Height = (VIDEO_ENCODING_MODE_PAL==enNorm)?576:480;
		pstCapRect->u32Width = 960;
		pstCapRect->u32Height = (VIDEO_ENCODING_MODE_PAL==enNorm)?576:480;
		break;
	case DEMO_VI_MODE_8_720P:
	case DEMO_VI_MODE_16_720P:
		pstDestSize->u32Width  = DEMO__720P_WIDTH;
		pstDestSize->u32Height = DEMO__720P_HEIGHT;
		pstCapRect->u32Width  = DEMO__720P_WIDTH;
		pstCapRect->u32Height = DEMO__720P_HEIGHT;
		break;

	case DEMO_VI_MODE_8_1080P:
	case DEMO_VI_MODE_16_1080P:
		pstDestSize->u32Width  = DEMO_HD_WIDTH;
		pstDestSize->u32Height = DEMO_HD_HEIGHT;
		pstCapRect->u32Width  = DEMO_HD_WIDTH;
		pstCapRect->u32Height = DEMO_HD_HEIGHT;
		break;           
	case DEMO_VI_MODE_4_4Kx2K:
		pstDestSize->u32Width  = DEMO__4K_WIDTH;
		pstDestSize->u32Height = DEMO__4K_HEIGHT;
		pstCapRect->u32Width  = DEMO__4K_WIDTH;
		pstCapRect->u32Height = DEMO__4K_HEIGHT;
		break;

	default:
		fprintf(stderr,"vi mode invaild!\n");
		return HI_FAILURE;
	}

	return HI_SUCCESS;
}


/*****************************************************************************
 * function : star vi dev (cfg vi_dev_attr; set_dev_cfg; enable dev)
 *****************************************************************************/
HI_S32 VI_StartDev(VI_DEV ViDev, DEMO_VI_MODE_E enViMode)
{
	HI_S32 s32Ret;
	VI_DEV_ATTR_S stViDevAttr;
	memset(&stViDevAttr,0,sizeof(stViDevAttr));

	switch (enViMode)
	{
	case DEMO_VI_MODE_32_D1:
		memcpy(&stViDevAttr,&DEV_ATTR_BT656D1_4MUX,sizeof(stViDevAttr));
		VI_SetMask(ViDev,&stViDevAttr);
		break;
	case DEMO_VI_MODE_32_960H:
		memcpy(&stViDevAttr,&DEV_ATTR_BT656D1_4MUX,sizeof(stViDevAttr));
		VI_SetMask(ViDev,&stViDevAttr);
		break;
	case DEMO_VI_MODE_8_720P:
	case DEMO_VI_MODE_8_1080P:
		memcpy(&stViDevAttr,&DEV_ATTR_BT1120I,sizeof(stViDevAttr));
		VI_SetMask(ViDev,&stViDevAttr);
		break;
	case DEMO_VI_MODE_16_720P:      
		memcpy(&stViDevAttr,&DEV_ATTR_TP2823_720P_2MUX_BASE,sizeof(stViDevAttr));
		VI_SetMask(ViDev,&stViDevAttr);
		break;
	case DEMO_VI_MODE_16_1080P:
		memcpy(&stViDevAttr,&DEV_ATTR_7441_BT1120_STANDARD_BASE,sizeof(stViDevAttr));
		VI_SetMask(ViDev,&stViDevAttr);
		break;    
	default:
		printf("[%s][Error]:vi input type[%d] is invalid!\n", __func__, enViMode);
		return HI_FAILURE;
	}

	s32Ret = HI_MPI_VI_SetDevAttr(ViDev, &stViDevAttr);
	if (s32Ret != HI_SUCCESS){
		printf("[%s][Error]:HI_MPI_VI_SetDevAttr failed with %#x!\n", __func__, s32Ret);
		return HI_FAILURE;
	}
	s32Ret = HI_MPI_VI_EnableDev(ViDev);
	if (s32Ret != HI_SUCCESS){
		printf("[%s][Error]:HI_MPI_VI_EnableDev failed with %#x!\n", __func__, s32Ret);
		return HI_FAILURE;
	}

	return HI_SUCCESS;
}


/*****************************************************************************
 * function : star vi chn
 *****************************************************************************/
HI_S32 VI_StartChn(VI_CHN ViChn, RECT_S *pstCapRect, SIZE_S *pstTarSize, 
		DEMO_VI_MODE_E enViMode, DEMO_VI_CHN_SET_E enViChnSet)
{
	HI_S32 s32Ret;
	VI_CHN_ATTR_S stChnAttr;

	/* step  5: config & start vicap dev */
	memcpy(&stChnAttr.stCapRect, pstCapRect, sizeof(RECT_S));
	/* to show scale. this is a sample only, we want to show dist_size = D1 only */
	stChnAttr.stDestSize.u32Width = pstTarSize->u32Width;
	stChnAttr.stDestSize.u32Height = pstTarSize->u32Height;
	stChnAttr.enCapSel = VI_CAPSEL_BOTH;
	stChnAttr.enPixFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_420;   /* sp420 or sp422 */
	stChnAttr.bMirror = (DEMO_VI_CHN_SET_MIRROR == enViChnSet)?HI_TRUE:HI_FALSE;
	stChnAttr.bFlip = (DEMO_VI_CHN_SET_FILP == enViChnSet)?HI_TRUE:HI_FALSE;
	stChnAttr.s32SrcFrameRate = -1;
	stChnAttr.s32DstFrameRate = -1;
	switch (enViMode)
	{
	case DEMO_VI_MODE_32_D1:
	case DEMO_VI_MODE_32_960H:
	case DEMO_VI_MODE_32_1280H:
		stChnAttr.enScanMode = VI_SCAN_INTERLACED;
		break;
	case DEMO_VI_MODE_32_HALF720P:
	case DEMO_VI_MODE_8_720P:
	case DEMO_VI_MODE_16_720P:        
	case DEMO_VI_MODE_8_1080P:
	case DEMO_VI_MODE_16_1080P:
	case DEMO_VI_MODE_4_4Kx2K:
		stChnAttr.enScanMode = VI_SCAN_PROGRESSIVE;
		break;
	default:
		fprintf(stderr,"ViMode invaild!\n");
		return HI_FAILURE;
	}

	AIChAttr3531A[ViChn] =  stChnAttr;
	s32Ret = HI_MPI_VI_SetChnAttr(ViChn, &stChnAttr); /*设置 VI 通道属性*/
	if (s32Ret != HI_SUCCESS)
	{
		fprintf(stderr,"failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}

	s32Ret = HI_MPI_VI_EnableChn(ViChn); /*启用 VI 通道*/
	if (s32Ret != HI_SUCCESS)
	{
		fprintf(stderr,"failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}

	return HI_SUCCESS;
}

/******************************************************************************
 *function:将vi模块的原始数据提取出来sp420 转存为 p420 ; sp422 转存为 p422
 *SP420:Y... VUVU...，P420:Y... UU... VV...
 *需要四字节对齐，多余的补零---跨度stride
 * width * height + width * height / 4 + width * height / 4 = width * height *3 /2
 ******************************************************************************/
static void yuv_dump(VIDEO_FRAME_S * pVBuf, FILE *fp)
{
	unsigned int w, h;
	char * pVBufVirt_Y;
	char * pVBufVirt_C;
	char * pMemContent;
	unsigned char TmpBuff[8192];                
	HI_U32 phy_addr,size;
	HI_CHAR *pUserPageAddr[2];
	PIXEL_FORMAT_E  enPixelFormat = pVBuf->enPixelFormat;
	HI_U32 u32UvHeight;/* uv height when saved for planar type */

	if (PIXEL_FORMAT_YUV_SEMIPLANAR_420 == enPixelFormat)
	{
		size = (pVBuf->u32Stride[0])*(pVBuf->u32Height)*3/2;    
		u32UvHeight = pVBuf->u32Height/2;
	}
	else
	{
		size = (pVBuf->u32Stride[0])*(pVBuf->u32Height)*2;   
		u32UvHeight = pVBuf->u32Height;
	}

	phy_addr = pVBuf->u32PhyAddr[0];

	fprintf(stderr,"phy_addr:%x, size:%d\n", phy_addr, size);
	pUserPageAddr[0] = (HI_CHAR *) HI_MPI_SYS_Mmap(phy_addr, size); 
	if (NULL == pUserPageAddr[0])
	{
		return;
	}
	fprintf(stderr,"stride: %d,%d\n",pVBuf->u32Stride[0],pVBuf->u32Stride[1] );

	pVBufVirt_Y = pUserPageAddr[0]; 
	pVBufVirt_C = pVBufVirt_Y + (pVBuf->u32Stride[0])*(pVBuf->u32Height);
	fprintf(stderr,"pVBufVirt_Y pVBufVirt_C:%p %p\n",pVBufVirt_Y,pVBufVirt_C);

#ifdef write_yuv
	/* save Y ----------------------------------------------------------------*/
	fprintf(stderr, "saving......Y......");
	fflush(stderr);

	for(h=0; h<pVBuf->u32Height; h++)
	{
		pMemContent = pVBufVirt_Y + h*pVBuf->u32Stride[0];
		fwrite(pMemContent, pVBuf->u32Width, 1, fp);
	}
	fflush(fp);


	/* save U ----------------------------------------------------------------*/
	fprintf(stderr, "U......");
	fflush(stderr);
	for(h=0; h<u32UvHeight; h++)
	{
		pMemContent = pVBufVirt_C + h*pVBuf->u32Stride[1];

		pMemContent += 1;

		for(w=0; w<pVBuf->u32Width/2; w++)
		{
			TmpBuff[w] = *pMemContent;
			pMemContent += 2;
		}
		fwrite(TmpBuff, pVBuf->u32Width/2, 1, fp);
	}
	fflush(fp);

	/* save V ----------------------------------------------------------------*/
	fprintf(stderr, "V......");
	fflush(stderr);
	for(h=0; h<u32UvHeight; h++)    
	{
		pMemContent = pVBufVirt_C + h*pVBuf->u32Stride[1];

		for(w=0; w<pVBuf->u32Width/2; w++)
		{
			TmpBuff[w] = *pMemContent;
			pMemContent += 2;
		}
		fwrite(TmpBuff, pVBuf->u32Width/2, 1, fp);
	}
	fflush(fp);

	fprintf(stderr, "done %d!\n", pVBuf->u32TimeRef);
	fflush(stderr);
#endif
	HI_MPI_SYS_Munmap(pUserPageAddr[0], size);    
}


/******************************************************************************
 * function : 通过vi的通道描述符获取原始数据
 ******************************************************************************/
void *get_vi_rawdata(void *parm)
{
	VI_CHN ViChn;
	HI_S32 s32ViFd[32] ;
	HI_S32 i,j,s32Ret;
	HI_S32 maxfd = 0;

	fd_set read_fds;
	struct timeval TimeoutVal;
	VI_CHN_STAT_S stStat;
	HI_U32 u32Depth;
	VIDEO_FRAME_INFO_S stFrameInfo;


	DEMO_VI_PARAM_S stViParam ;
	s32Ret = VI_Mode2Param(g_enViMode, &stViParam);
	fprintf(stderr,"stViParam.s32ViChnCnt:%d %d %d %d\n",stViParam.s32ViChnCnt,
			stViParam.s32ViDevCnt,
			stViParam.s32ViChnInterval,
			stViParam.s32ViDevInterval);
	for(i=0; i<stViParam.s32ViDevCnt; i++)
	{
		ViChn = i * stViParam.s32ViChnInterval;

		s32ViFd[i] = HI_MPI_VI_GetFd(ViChn);
		if (s32ViFd[i] > 0)
		{
			//fprintf(stderr,"s32ViFd:%d %d\n",s32ViFd[i],stViParam.s32ViChnCnt); /*注释掉防止刷屏*/
		}
		if (maxfd <= s32ViFd[i])
		{
			maxfd = s32ViFd[i];
		}
		//需要在setect之前设置可获取的 VI 图像最大深度才会获取得到图像
		//最大深度需要根据系统初始化是设置的vb大小来设置
		HI_MPI_VI_SetFrameDepth(ViChn,1);
		if (s32Ret != HI_SUCCESS)
		{
			fprintf(stderr,"failed with %#x!\n", s32Ret);
			//return HI_FAILURE;
		}

	}




	HI_MPI_VI_GetFrameDepth(12,&u32Depth);
	//fprintf(stderr,"u32Depth:%d\n",u32Depth); /*注释减少打印 add_merlin*/

	FILE *fp = NULL;
	//if(!fp)
		//fp = fopen("./yuv420p.yuv","w+");

	while(1)
	{
		FD_ZERO(&read_fds);
		for (i = 0; i < stViParam.s32ViChnCnt; i++)
		{
			FD_SET(s32ViFd[i], &read_fds);
		}
		TimeoutVal.tv_sec  = 2;
		TimeoutVal.tv_usec = 0;
		s32Ret = select(maxfd + 1, &read_fds, NULL, NULL, &TimeoutVal);
		if (s32Ret < 0)
		{
			fprintf(stderr,"select failed!\n");
			break;
		}
		else if (s32Ret == 0)
		{
			fprintf(stderr,"get vi stream time out\n");
			continue;
		}
		else
		{
			for (i = 0; i < stViParam.s32ViChnCnt; i++)
			{
				ViChn = i * stViParam.s32ViChnInterval;
				s32Ret = HI_MPI_VI_Query(ViChn, &stStat);
#if 0 /*助十掉，减少打印 add_merlin*/
				fprintf(stderr,"ViChn:%d stStat:%d %d %d %d %d %d %d\n",
						ViChn,stStat.bEnable,stStat.u32IntCnt,stStat.u32FrmRate, stStat.u32LostInt,
						stStat.u32VbFail,stStat.u32PicWidth,stStat.u32PicHeight);
#endif
				if (HI_SUCCESS != s32Ret)
				{
					/*经常会进入此函数，所以先注释掉*/
					//fprintf(stderr,"HI_MPI_VI_Query chn[%d] failed with %#x!\n", i, s32Ret);
					continue;
				}
				if (FD_ISSET(s32ViFd[i], &read_fds))
				{
					/*注释掉减少打印 Add_merlin */
					//fprintf(stderr,"FD_ISSET :s32ViFd[i]:%d ViChn %d\n",s32ViFd[i],ViChn);

					/* set max depth */
					u32Depth = 1;
					s32Ret = HI_MPI_VI_SetFrameDepth(ViChn, u32Depth);
					HI_MPI_VI_GetFrameDepth(12,&u32Depth);
					//fprintf(stderr,"u32Depth:%d\n",u32Depth); /*注释掉减少打印 add_merlin*/
					for(j = 0;j < 1;j++)
					{
						/* get video frame from vi chn */
						s32Ret = HI_MPI_VI_GetFrame(ViChn, &stFrameInfo,-1);
						if (HI_SUCCESS != s32Ret)
						{
							fprintf(stderr,"get vi frame err:0x%x\n", s32Ret);

						}
						/* deal with video frame ... 需要映射地址到用户空间*/
						/*先注释掉减少打印输出 add_merlin*/
						//fprintf(stderr,"HI_MPI_VI_GetFrame :%d %d\n",stFrameInfo.stVFrame.u32Width,stFrameInfo.stVFrame.u32Height);


						//PIXEL_FORMAT_YUV_SEMIPLANAR_420 == 23
						//save_yuvsp420_to_file 濂界殑
						//yuv_dump(&stFrameInfo.stVFrame,fp);

						/* release video frame */
						HI_MPI_VI_ReleaseFrame(12, &stFrameInfo);
					}

				}
				else{
					//fprintf(stderr,"no source ViChn:%d\n",ViChn);
				}
					
			}
		}


	}

}

/******************************************************************************
 * function :vi chn 设置用户图片---用于无源时显示
 ******************************************************************************/
HI_S32 set_userpic_to_vichn(VI_CHN vichn)
{
	HI_S32 s32ret;
	HI_U32 u32Width;
	HI_U32 u32Height;
	HI_U32 u32LStride;
	HI_U32 u32CStride;
	HI_U32 u32LumaSize;
	HI_U32 u32ChrmSize;
	HI_U32 u32Size;
	VB_BLK VbBlk;
	HI_U32 u32PhyAddr;
	HI_U8 *pVirAddr;
	VIDEO_FRAME_INFO_S stVFrameInfo;
	VIDEO_FRAME_INFO_S *pstVFrameInfo = &stVFrameInfo;
	unsigned char TmpBuff[8192];           

	/* you need get width and height of pictrue */
	u32Width = 1280;
	u32Height = 720;
	u32LStride = 1280;
	u32CStride = 1280;
	u32LumaSize = (u32LStride * u32Height);
	u32ChrmSize = (u32CStride * u32Height) >> 2;/* 420*/
	u32Size = u32LumaSize + (u32ChrmSize << 1);
	/* get video buffer block form common pool */

	g_hPool   = HI_MPI_VB_CreatePool( u32Size, 2,NULL);
	VbBlk = HI_MPI_VB_GetBlock(g_hPool, u32Size,NULL);
	if (VB_INVALID_HANDLE == VbBlk)
	{
		fprintf(stderr,"HI_MPI_VB_GetBlock fail\n");
		return -1;
	}
	/* get physical address*/
	u32PhyAddr = HI_MPI_VB_Handle2PhysAddr(VbBlk);
	if (0 == u32PhyAddr)
	{
		fprintf(stderr,"HI_MPI_VB_Handle2PhysAddr fail\n");
		return -1;
	}
	/* mmap physical address to virtual address*/
	pVirAddr = (HI_U8 *) HI_MPI_SYS_Mmap( u32PhyAddr, u32Size );
	fprintf(stderr,"u32PhyAddr,pVirAddr:%p %p\n",u32PhyAddr,pVirAddr);	

	/* get pool id */
	pstVFrameInfo->u32PoolId = HI_MPI_VB_Handle2PoolId(VbBlk);
	fprintf(stderr,"pstVFrameInfo->u32PoolId :%d %d\n",pstVFrameInfo->u32PoolId,g_hPool);
	if (VB_INVALID_POOLID == pstVFrameInfo->u32PoolId)
	{
		fprintf(stderr,"HI_MPI_VB_Handle2PoolId fail\n");
		return -1;
	}

	memset(pstVFrameInfo,0,sizeof(pstVFrameInfo));
	pstVFrameInfo->stVFrame.u32PhyAddr[0] = u32PhyAddr;
	pstVFrameInfo->stVFrame.u32PhyAddr[1] = pstVFrameInfo->stVFrame.u32PhyAddr[0] + u32LumaSize;
	pstVFrameInfo->stVFrame.u32PhyAddr[2] = pstVFrameInfo->stVFrame.u32PhyAddr[1] + u32ChrmSize;
	pstVFrameInfo->stVFrame.pVirAddr[0] = pVirAddr;
	pstVFrameInfo->stVFrame.pVirAddr[1] = pstVFrameInfo->stVFrame.pVirAddr[0] + u32LumaSize;
	pstVFrameInfo->stVFrame.pVirAddr[2] = pstVFrameInfo->stVFrame.pVirAddr[1] + u32ChrmSize;
	pstVFrameInfo->stVFrame.u32Width = u32Width;
	pstVFrameInfo->stVFrame.u32Height = u32Height;
	pstVFrameInfo->stVFrame.u32Stride[0] = u32LStride;
	pstVFrameInfo->stVFrame.u32Stride[1] = u32CStride;
	pstVFrameInfo->stVFrame.u32Stride[2] = u32CStride;
	pstVFrameInfo->stVFrame.enPixelFormat =
		PIXEL_FORMAT_YUV_SEMIPLANAR_420;
	//pstVFrameInfo->u32PoolId = g_hPool;
	//<3>[    vi] [Func]:ViuSetUserPic [Line]:760 [Info]:<3>[    vi] u32Field invalid, only support TOP/BOTTOM/INTERLACED/FRAME.
	pstVFrameInfo->stVFrame.u32Field = VIDEO_FIELD_FRAME;
	/* now you need get YUV Semi Planar Data ,fill them to the virtual
	   address */
	//黑色
	memset(pVirAddr,0,u32Size);
	memset(pVirAddr + u32LumaSize,128,u32ChrmSize << 1 );


	/* enabel VI channel ... ... */
	//之前已经使能过了
	//HI_MPI_VI_EnableChn(vichn);

	/* first set user pic info*/
	VI_USERPIC_ATTR_S stUsrPic;

	VIDEO_FRAME_INFO_S stUsrPicFrm;
	stUsrPic.bPub = HI_TRUE;
	stUsrPic.enUsrPicMode = VI_USERPIC_MODE_PIC;

	memcpy(&(stUsrPic.unUsrPic.stUsrPicFrm), pstVFrameInfo,sizeof(VIDEO_FRAME_INFO_S));
	fprintf(stderr,"stUsrPic.unUsrPic.stUsrPicFrm.stVFrame.enPixelFormat:%d\n",stUsrPic.unUsrPic.stUsrPicFrm.stVFrame.enPixelFormat);
#if 0
	VI_USERPIC_BGC_S stUsrPicBg;
	stUsrPic.bPub = HI_TRUE;
	stUsrPic.enUsrPicMode = VI_USERPIC_MODE_BGC;
	stUsrPic.unUsrPic.stUsrPicBg = stUsrPicBg; //黑屏
#endif

	s32ret = HI_MPI_VI_SetUserPic(vichn,&stUsrPic);
	if (s32ret)
	{
		fprintf(stderr,"HI_MPI_VI_SetUserPic fail:%x\n",s32ret);
		return -1;
	}


	/* enable insert user pic if you need */
	s32ret = HI_MPI_VI_EnableUserPic(vichn);
	if (s32ret)
	{
		fprintf(stderr,"HI_MPI_VI_EnableUserPic fail\n");
		return -1;
	}

#if 0
	/* disable insert user pic if you don't need */
	s32ret = HI_MPI_VI_DisableUserPic(0, 0);
	if (s32ret)
	{
		return -1;
	}
#endif
	return 0;
}




/******************************************************************************
 * function : start vi dev & chn 
 * 需要根据驱动获取到的分辨率重新配置vi的enViMode
 * Input enNorm:视频输入制式类型
 ******************************************************************************/
HI_S32 vi_start(DEMO_VI_MODE_E enViMode,VIDEO_NORM_E enNorm)
{

	//DEMO_COMM_VI_Start(enViMode, enNorm);
	VI_DEV ViDev;
	VI_CHN ViChn;
	HI_S32 i;
	HI_S32 s32Ret;
	DEMO_VI_PARAM_S stViParam;
	SIZE_S stTargetSize;
	RECT_S stCapRect;
	VI_CHN_BIND_ATTR_S stChnBindAttr;

	/*** get parameter and size from enViMode ***/
	s32Ret = VI_Mode2Param(g_enViMode, &stViParam);
	if (HI_SUCCESS !=s32Ret){
		printf("[%s][Error]:vi get param failed! line:%d\n", __func__, __LINE__);
		return HI_FAILURE;
	}
	s32Ret = VI_Mode2Size(g_enViMode, g_enNorm, &stCapRect, &stTargetSize);
	if (HI_SUCCESS !=s32Ret){
		printf("[%s][Error]:vi get size failed! line:%d\n", __func__, __LINE__);
		return HI_FAILURE;
	}

	/*** Start VI Dev ***/
	printf("[%s]:ViDev=", __func__);
	for(i=0; i<stViParam.s32ViDevCnt; i++)
	{
		/*VI_MODE_8_1080P:   use dev 0 1 2 3 4 5 6 7*/
		ViDev = i * stViParam.s32ViDevInterval;
		printf("%d ", ViDev);
		s32Ret = VI_StartDev(ViDev, g_enViMode);
		if (HI_SUCCESS != s32Ret){
			printf("[%s][Error]:DEMO_COMM_VI_StartDev failed with %#x line:%d\n", __func__, s32Ret, __LINE__);
			return HI_FAILURE;
		}
	}
	printf("\n");
	//dev-->way(默认为0)-->chn
	/*** Start VI Chn ***/
	printf("[%s][Error]:stViParam.s32ViChnCnt:%d line:%d\n", __func__, stViParam.s32ViChnCnt, __LINE__);
	for(i=0; i<stViParam.s32ViChnCnt; i++)
	{
		/*DEMO_VI_MODE_8_1080P:   use chn 0 4 8 12 16 20 24 28*/
		ViChn = i * stViParam.s32ViChnInterval;

		if (DEMO_VI_MODE_16_1080P == g_enViMode
				|| DEMO_VI_MODE_16_720P == g_enViMode)
		{
			/* When in the 16x1080p mode, bind chn 2,6,10,14 to way1 is needed */
			if (ViChn%4 != 0)
			{
				s32Ret = HI_MPI_VI_GetChnBind(ViChn, &stChnBindAttr);
				if (HI_ERR_VI_FAILED_NOTBIND == s32Ret)
				{
					stChnBindAttr.ViDev = ViChn/4;
					stChnBindAttr.ViWay = 1;

					// 通常不需调用该接口，特殊情况，需要调整默认绑定关系，或使多个通道绑定到
					//	同一个{Dev，Way}时，可调用此接口来实现。
					s32Ret = HI_MPI_VI_BindChn(ViChn, &stChnBindAttr);
					if (HI_SUCCESS != s32Ret){
						printf("[%s][Error]:call HI_MPI_VI_BindChn failed with %#x line:%d\n", __func__, s32Ret, __LINE__);
						return HI_FAILURE;
					} 
				} 
			}
		}

		s32Ret = VI_StartChn(ViChn, &stCapRect, &stTargetSize, g_enViMode, DEMO_VI_CHN_SET_NORMAL);
		if (HI_SUCCESS != s32Ret){
			printf("[%s][Error]:call DEMO_COMM_VI_StarChn failed with %#x\n", __func__, s32Ret, __LINE__);
			return HI_FAILURE;
		} 

		//只留chn 12 显示，其他chn 显示黑屏
		if(ViChn != 12)
			s32Ret = set_userpic_to_vichn(ViChn);
		if (HI_SUCCESS != s32Ret){
			printf("[%s][Error]:set_userpic_to_vichn fail\n", __func__, __LINE__);
			return HI_FAILURE;
		} 

	}
	//fprintf(stderr,"stViParam.s32ViChnCnt:%d\n",stViParam.s32ViChnCnt);

	//创建线程接收vi原始数据
	pthread_t vitid;
	pthread_create(&vitid,NULL,get_vi_rawdata,NULL);
	
	return HI_SUCCESS;
}



/*****************************************************************************
 * function : Vi chn to vpss group
 *****************************************************************************/
HI_S32 vi_to_vpss(DEMO_VI_MODE_E enViMode)
{
	HI_S32 j, s32Ret;
	VPSS_GRP VpssGrp;
	MPP_CHN_S stSrcChn;
	MPP_CHN_S stDestChn;
	DEMO_VI_PARAM_S stViParam;
	VI_CHN ViChn;

	s32Ret = VI_Mode2Param(enViMode, &stViParam);
	if (HI_SUCCESS !=s32Ret){
		printf("[%s][Error]:DEMO_COMM_VI_Mode2Param failed! line:%d\n", __func__, __LINE__);
		return HI_FAILURE;
	}

	VpssGrp = 0;
	printf("[%s]:s32ViChnCnt=%d VI bind VPass: ", __func__, (stViParam.s32ViChnCnt), __LINE__);
	for (j=0; j<stViParam.s32ViChnCnt; j++)
	{
		/*VI_MODE_8_1080P:  use chn 0,4,8,12,16,20,24,28 */
		ViChn = j * stViParam.s32ViChnInterval;   
		stSrcChn.enModId = HI_ID_VIU;
		stSrcChn.s32DevId = 0;
		stSrcChn.s32ChnId = ViChn;
		/*use group 0 1 2 3 4 5 6 7*/
		stDestChn.enModId = HI_ID_VPSS;
		stDestChn.s32DevId = VpssGrp;
		stDestChn.s32ChnId = 0;
		printf("VI=%d-%d VP=%d-%d ", (stSrcChn.s32DevId), (stSrcChn.s32ChnId), (stDestChn.s32DevId), (stDestChn.s32ChnId));
		s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
		if (s32Ret != HI_SUCCESS){
			printf("[%s][Error]:failed with %#x! line:%d\n", __func__, s32Ret, __LINE__);
			return HI_FAILURE;
		}

		VpssGrp ++;
	}
	printf("\n");
	return HI_SUCCESS;
}


/******************************************************************************
 * function : send send stream to vpss
 ******************************************************************************/
HI_VOID * VDEC_SendStream(HI_VOID *pArgs)
{
	DEMOVdecThreadParam *pstVdecThreadParam =(DEMOVdecThreadParam *)pArgs;
	FILE *fpStrm=NULL;
	HI_U8 *pu8Buf = NULL;
	VDEC_STREAM_S stStream;
	HI_BOOL bFindStart, bFindEnd;
	HI_S32 s32Ret,  i,  start = 0;
	HI_S32 s32UsedBytes = 0, s32ReadLen = 0;
	HI_U64 u64pts = 0;
	HI_S32 len;
	HI_BOOL sHasReadStream = HI_FALSE; 

	if(pstVdecThreadParam->cFileName != 0)
	{
		fpStrm = fopen(pstVdecThreadParam->cFileName, "rb");
		if(fpStrm == NULL)
		{
			printf("DEMO_TEST:can't open file %s in send stream thread:%d\n",pstVdecThreadParam->cFileName, pstVdecThreadParam->s32ChnId);
			return (HI_VOID *)(HI_FAILURE);
		}
	}
	//printf("DEMO_TEST:chn %d, stream file:%s, bufsize: %d\n", 
	//pstVdecThreadParam->s32ChnId, pstVdecThreadParam->cFileName, pstVdecThreadParam->s32MinBufSize);

	pu8Buf = malloc(pstVdecThreadParam->s32MinBufSize);
	if(pu8Buf == NULL)
	{
		printf("DEMO_TEST:can't alloc %d in send stream thread:%d\n", pstVdecThreadParam->s32MinBufSize, pstVdecThreadParam->s32ChnId);
		fclose(fpStrm);
		return (HI_VOID *)(HI_FAILURE);
	}     
	fflush(stdout);
	sleep(2);
	u64pts = pstVdecThreadParam->u64PtsInit;
	while (1)
	{
		//printf("send file data\n");
		if (pstVdecThreadParam->eCtrlSinal == DEMO_VDEC_CTRL_STOP)
		{
			break;
		}
		else if (pstVdecThreadParam->eCtrlSinal == DEMO_VDEC_CTRL_PAUSE)
		{
			sleep(MIN2(pstVdecThreadParam->s32IntervalTime,1000));
			continue;
		}

		if ( (pstVdecThreadParam->s32StreamMode==VIDEO_MODE_FRAME) && (pstVdecThreadParam->enType == PT_MP4VIDEO) )
		{
			bFindStart = HI_FALSE;  
			bFindEnd   = HI_FALSE;
			fseek(fpStrm, s32UsedBytes, SEEK_SET);
			s32ReadLen = fread(pu8Buf, 1, pstVdecThreadParam->s32MinBufSize, fpStrm);
			if (s32ReadLen == 0)
			{
				if (pstVdecThreadParam->bLoopSend)
				{
					s32UsedBytes = 0;
					fseek(fpStrm, 0, SEEK_SET);
					s32ReadLen = fread(pu8Buf, 1, pstVdecThreadParam->s32MinBufSize, fpStrm);
				}
				else
				{
					break;
				}
			}

			for (i=0; i<s32ReadLen-4; i++)
			{
				if (pu8Buf[i] == 0 && pu8Buf[i+1] == 0 && pu8Buf[i+2] == 1 && pu8Buf[i+3] == 0xB6)
				{
					bFindStart = HI_TRUE;
					i += 4;
					break;
				}
			}

			for (; i<s32ReadLen-4; i++)
			{
				if (pu8Buf[i  ] == 0 && pu8Buf[i+1] == 0 && pu8Buf[i+2] == 1 && pu8Buf[i+3] == 0xB6)
				{
					bFindEnd = HI_TRUE;
					break;
				}
			}

			s32ReadLen = i;
			if (bFindStart == HI_FALSE)
			{
				printf("DEMO_TEST: chn %d can not find start code! s32ReadLen %d, s32UsedBytes %d. \n", 
						pstVdecThreadParam->s32ChnId, s32ReadLen, s32UsedBytes);
			}
			else if (bFindEnd == HI_FALSE)
			{
				s32ReadLen = i+4;
			}

		}
		else if ( (pstVdecThreadParam->s32StreamMode==VIDEO_MODE_FRAME) && (pstVdecThreadParam->enType == PT_H264) )
		{
			bFindStart = HI_FALSE;  
			bFindEnd   = HI_FALSE;
			fseek(fpStrm, s32UsedBytes, SEEK_SET);
			s32ReadLen = fread(pu8Buf, 1, pstVdecThreadParam->s32MinBufSize, fpStrm);
			if (s32ReadLen == 0)
			{
				if (pstVdecThreadParam->bLoopSend)
				{
					s32UsedBytes = 0;
					fseek(fpStrm, 0, SEEK_SET);
					s32ReadLen = fread(pu8Buf, 1, pstVdecThreadParam->s32MinBufSize, fpStrm);
				}
				else
				{
					break;
				}
			}

			for (i=0; i<s32ReadLen-5; i++)
			{
				if (  pu8Buf[i  ] == 0 && pu8Buf[i+1] == 0 && pu8Buf[i+2] == 1 && 
						( (pu8Buf[i+3]&0x1F) == 0x5 || (pu8Buf[i+3]&0x1F) == 0x1 ) &&
						( (pu8Buf[i+4]&0x80) == 0x80)
				   )                 
				{
					bFindStart = HI_TRUE;
					i += 4;
					break;
				}
			}

			for (; i<s32ReadLen-5; i++)
			{
				if (  pu8Buf[i  ] == 0 && pu8Buf[i+1] == 0 && pu8Buf[i+2] == 1 && 
						( ((pu8Buf[i+3]&0x1F) == 0x7) || ((pu8Buf[i+3]&0x1F) == 0x8) || ((pu8Buf[i+3]&0x1F) == 0x6)
						  || (((pu8Buf[i+3]&0x1F) == 0x5 || (pu8Buf[i+3]&0x1F) == 0x1) &&((pu8Buf[i+4]&0x80) == 0x80))
						)
				   )
				{
					bFindEnd = HI_TRUE;
					break;
				}
			}

			if(i > 0) s32ReadLen = i;
			if (bFindStart == HI_FALSE)
			{
				printf("DEMO_TEST: chn %d can not find start code!s32ReadLen %d, s32UsedBytes %d. \n", 
						pstVdecThreadParam->s32ChnId, s32ReadLen, s32UsedBytes);
			}
			else if (bFindEnd == HI_FALSE)
			{
				s32ReadLen = i+5;
			}

		}
		else if ((pstVdecThreadParam->enType == PT_MJPEG) )
		{
			bFindStart = HI_FALSE;  
			bFindEnd   = HI_FALSE;          
			fseek(fpStrm, s32UsedBytes, SEEK_SET);
			s32ReadLen = fread(pu8Buf, 1, pstVdecThreadParam->s32MinBufSize, fpStrm);
			if (s32ReadLen == 0)
			{
				if (pstVdecThreadParam->bLoopSend)
				{
					s32UsedBytes = 0;
					fseek(fpStrm, 0, SEEK_SET);
					s32ReadLen = fread(pu8Buf, 1, pstVdecThreadParam->s32MinBufSize, fpStrm);
				}
				else
				{
					break;
				}
			}


			for (i=0; i<s32ReadLen-2; i++)
			{
				if (pu8Buf[i] == 0xFF && pu8Buf[i+1] == 0xD8) 
				{  
					start = i;
					bFindStart = HI_TRUE;
					i = i + 2;
					break;
				}  
			}

			for (; i<s32ReadLen-4; i++)
			{
				if ( (pu8Buf[i] == 0xFF) && (pu8Buf[i+1]& 0xF0) == 0xE0 )
				{   
					len = (pu8Buf[i+2]<<8) + pu8Buf[i+3];                    
					i += 1 + len;                  
				}
				else
				{
					break;
				}
			}

			for (; i<s32ReadLen-2; i++)
			{
				if (pu8Buf[i] == 0xFF && pu8Buf[i+1] == 0xD8)
				{
					bFindEnd = HI_TRUE;
					break;
				} 
			}                    
			s32ReadLen = i;
			if (bFindStart == HI_FALSE)
			{
				printf("DEMO_TEST: chn %d can not find start code! s32ReadLen %d, s32UsedBytes %d. \n", 
						pstVdecThreadParam->s32ChnId, s32ReadLen, s32UsedBytes);
			}
			else if (bFindEnd == HI_FALSE)
			{
				s32ReadLen = i+2;
			}
		}
		else if ((pstVdecThreadParam->enType == PT_JPEG) )
		{
			if (HI_TRUE != sHasReadStream)
			{               

				bFindStart = HI_FALSE;  
				bFindEnd   = HI_FALSE; 

				fseek(fpStrm, s32UsedBytes, SEEK_SET);
				s32ReadLen = fread(pu8Buf, 1, pstVdecThreadParam->s32MinBufSize, fpStrm);
				if (s32ReadLen == 0)
				{
					if (pstVdecThreadParam->bLoopSend)
					{
						s32UsedBytes = 0;
						fseek(fpStrm, 0, SEEK_SET);
						s32ReadLen = fread(pu8Buf, 1, pstVdecThreadParam->s32MinBufSize, fpStrm);
					}
					else
					{
						break;
					}
				}


				for (i=0; i<s32ReadLen-2; i++)
				{
					if (pu8Buf[i] == 0xFF && pu8Buf[i+1] == 0xD8) 
					{  
						start = i;
						bFindStart = HI_TRUE;
						i = i + 2;
						break;
					}  
				}

				for (; i<s32ReadLen-4; i++)
				{
					if ( (pu8Buf[i] == 0xFF) && (pu8Buf[i+1]& 0xF0) == 0xE0 )
					{   
						len = (pu8Buf[i+2]<<8) + pu8Buf[i+3];                    
						i += 1 + len;                  
					}
					else
					{
						break;
					}
				}

				for (; i<s32ReadLen-2; i++)
				{
					if (pu8Buf[i] == 0xFF && pu8Buf[i+1] == 0xD8)
					{                    
						bFindEnd = HI_TRUE;
						break;
					} 
				}                    
				s32ReadLen = i;
				if (bFindStart == HI_FALSE)
				{
					printf("DEMO_TEST: chn %d can not find start code! s32ReadLen %d, s32UsedBytes %d. \n", 
							pstVdecThreadParam->s32ChnId, s32ReadLen, s32UsedBytes);
				}
				else if (bFindEnd == HI_FALSE)
				{
					s32ReadLen = i+2;
				}
				sHasReadStream = HI_TRUE;
			}
		}
		else
		{
			fseek(fpStrm, s32UsedBytes, SEEK_SET);
			s32ReadLen = fread(pu8Buf, 1, pstVdecThreadParam->s32MinBufSize, fpStrm);
			if (s32ReadLen == 0)
			{
				if (pstVdecThreadParam->bLoopSend)
				{
					s32UsedBytes = 0;
					fseek(fpStrm, 0, SEEK_SET);
					s32ReadLen = fread(pu8Buf, 1, pstVdecThreadParam->s32MinBufSize, fpStrm);
				}
				else
				{
					break;
				}
			}
		}

		stStream.u64PTS  = u64pts;
		stStream.pu8Addr = pu8Buf + start;
		stStream.u32Len  = s32ReadLen; 
		stStream.bEndOfFrame  = (pstVdecThreadParam->s32StreamMode==VIDEO_MODE_FRAME)? HI_TRUE: HI_FALSE;
		stStream.bEndOfStream = HI_FALSE;                   


		//printf("Send One Frame");
		//fflush(stdout);   

		s32Ret=HI_MPI_VDEC_SendStream(pstVdecThreadParam->s32ChnId, &stStream, pstVdecThreadParam->s32MilliSec);
		pstVdecThreadParam->cUserCmd = 0;
		if (HI_SUCCESS != s32Ret)
		{
			//printf("HI_MPI_VDEC_SendStream fail,0x%x\n",s32Ret);
			usleep(100);
		}
		else
		{
			s32UsedBytes = s32UsedBytes +s32ReadLen + start;			
			u64pts += pstVdecThreadParam->u64PtsIncrease;            
		}
		usleep(1000);
	}

	/* send the flag of stream end */
	memset(&stStream, 0, sizeof(VDEC_STREAM_S) );
	stStream.bEndOfStream = HI_TRUE;
	HI_MPI_VDEC_SendStream(pstVdecThreadParam->s32ChnId, &stStream, -1);

	//printf("DEMO_TEST:send steam thread %d return ...\n", pstVdecThreadParam->s32ChnId);
	fflush(stdout);
	if (pu8Buf != HI_NULL)
	{
		free(pu8Buf);
	}
	fclose(fpStrm);

	return (HI_VOID *)HI_SUCCESS;
}


/******************************************************************************
 * function : start vdec chn 
 ******************************************************************************/
HI_S32 VDEC_StartChn(HI_S32 s32ChnNum,VDEC_CHN_ATTR_S *pstVdecChnAttr,PAYLOAD_TYPE_E enType, SIZE_S *pstSize)
{
	HI_S32 i;

	for(i=0; i<s32ChnNum; i++)
	{
		pstVdecChnAttr[i].enType       = enType;
		pstVdecChnAttr[i].u32BufSize   = 3 * pstSize->u32Width * pstSize->u32Height;
		pstVdecChnAttr[i].u32Priority  = 5;
		pstVdecChnAttr[i].u32PicWidth  = pstSize->u32Width;
		pstVdecChnAttr[i].u32PicHeight = pstSize->u32Height;
		if (PT_H264 == enType || PT_MP4VIDEO == enType)
		{
			pstVdecChnAttr[i].stVdecVideoAttr.enMode=VIDEO_MODE_FRAME;
			pstVdecChnAttr[i].stVdecVideoAttr.u32RefFrameNum = 1;
			pstVdecChnAttr[i].stVdecVideoAttr.bTemporalMvpEnable = 0;
		}
	}

	for(i=0; i<s32ChnNum; i++)
	{			
		CHECK_CHN_RET(HI_MPI_VDEC_CreateChn(i, &pstVdecChnAttr[i]), i, "HI_MPI_VDEC_CreateChn");
		CHECK_CHN_RET(HI_MPI_VDEC_StartRecvStream(i), i, "HI_MPI_VDEC_StartRecvStream");
	}


	return HI_SUCCESS;

}



/******************************************************************************
 * function : start vdec VBSource & chn 
 *0：ModuleVB
 *1：PrivateVB
 *2：UserVB
 ******************************************************************************/
HI_S32 vdec_start(HI_VOID)
{
	HI_S32 i;
	HI_S32 s32Ret;
	VDEC_MOD_PARAM_S stModParam;
	VB_CONF_S stModVbConf;
	HI_MPI_VDEC_GetModParam(&stModParam);
	printf("[%s]:stModParam:%d %d line:%d\n", __func__, stModParam.u32MiniBufMode,stModParam.u32VBSource, __LINE__);
	MPP_CHN_S stSrcChn;
	MPP_CHN_S stDestChn;
	//init VBSource
	HI_S32 PicSize;
	HI_S32 s32ChnNum = 1;
	SIZE_S stSize;
	stSize.u32Height = 1920;
	stSize.u32Width = 1080;
	memset(&stModVbConf, 0, sizeof(VB_CONF_S));
	stModVbConf.u32MaxPoolCnt = 2;

	VB_PIC_BLK_SIZE(stSize.u32Height, stSize.u32Width, PT_H264, PicSize);	
	stModVbConf.astCommPool[0].u32BlkSize = PicSize;
	stModVbConf.astCommPool[0].u32BlkCnt  = 4;



	HI_MPI_VB_ExitModCommPool(VB_UID_VDEC);

	CHECK_RET(HI_MPI_VB_SetModPoolConf(VB_UID_VDEC, &stModVbConf), "HI_MPI_VB_SetModPoolConf");
	CHECK_RET(HI_MPI_VB_InitModCommPool(VB_UID_VDEC), "HI_MPI_VB_InitModCommPool");


	VDEC_CHN_ATTR_S stVdecChnAttr[VDEC_MAX_CHN_NUM];
	VDEC_StartChn(s32ChnNum,&stVdecChnAttr[0],PT_H264,&stSize);


	DEMOVdecThreadParam stVdecSend[VDEC_MAX_CHN_NUM];
	for(i=0; i<s32ChnNum; i++)
	{
		sprintf(stVdecSend[i].cFileName, "./1080P.h264");
		stVdecSend[i].s32MilliSec     = 10;
		stVdecSend[i].s32ChnId        = i;  
		stVdecSend[i].s32IntervalTime = 1;
		stVdecSend[i].u64PtsInit      = 0;
		stVdecSend[i].u64PtsIncrease  = 0;
		stVdecSend[i].eCtrlSinal      = DEMO_VDEC_CTRL_START;
		stVdecSend[i].bLoopSend       = HI_TRUE;
		stVdecSend[i].bManuSend       = HI_FALSE;
		stVdecSend[i].enType          = PT_H264;
		stVdecSend[i].s32MinBufSize   = (stVdecChnAttr[i].u32PicWidth * stVdecChnAttr[i].u32PicHeight * 3)>>1;
		if (PT_H264 == stVdecSend[i].enType  || PT_MP4VIDEO == stVdecSend[i].enType)
		{
			stVdecSend[i].s32StreamMode  = stVdecChnAttr[i].stVdecVideoAttr.enMode;
		}
		else
		{
			stVdecSend[i].s32StreamMode = VIDEO_MODE_FRAME;
		}

		pthread_t tid[s32ChnNum];

		for(i=0; i<s32ChnNum; i++)
		{
			pthread_create(&tid[i], 0, VDEC_SendStream, (HI_VOID *)&stVdecSend[i]);
		}



		return HI_SUCCESS;
	}
}


/******************************************************************************
 * function : get picture size(w*h), according Norm and enPicSize
 ******************************************************************************/
HI_S32 SYS_GetPicSize(VIDEO_NORM_E enNorm, PIC_SIZE_E enPicSize, SIZE_S *pstSize)
{
	switch (enPicSize)
	{
	case PIC_QCIF:
		pstSize->u32Width = DEMO_D1_WIDTH / 4;
		pstSize->u32Height = (VIDEO_ENCODING_MODE_PAL==enNorm)?144:120;
		break;
	case PIC_CIF:
		pstSize->u32Width = DEMO_D1_WIDTH / 2;
		pstSize->u32Height = (VIDEO_ENCODING_MODE_PAL==enNorm)?288:240;
		break;
	case PIC_D1:
		pstSize->u32Width = DEMO_D1_WIDTH;
		pstSize->u32Height = (VIDEO_ENCODING_MODE_PAL==enNorm)?576:480;
		break;
	case PIC_960H:
		pstSize->u32Width = 960;
		pstSize->u32Height = (VIDEO_ENCODING_MODE_PAL==enNorm)?576:480;
		break;			
	case PIC_2CIF:
		pstSize->u32Width = DEMO_D1_WIDTH / 2;
		pstSize->u32Height = (VIDEO_ENCODING_MODE_PAL==enNorm)?576:480;
		break;
	case PIC_QVGA:    /* 320 * 240 */
		pstSize->u32Width = 320;
		pstSize->u32Height = 240;
		break;
	case PIC_VGA:     /* 640 * 480 */
		pstSize->u32Width = 640;
		pstSize->u32Height = 480;
		break;
	case PIC_XGA:     /* 1024 * 768 */
		pstSize->u32Width = 1024;
		pstSize->u32Height = 768;
		break;
	case PIC_SXGA:    /* 1400 * 1050 */
		pstSize->u32Width = 1400;
		pstSize->u32Height = 1050;
		break;
	case PIC_UXGA:    /* 1600 * 1200 */
		pstSize->u32Width = 1600;
		pstSize->u32Height = 1200;
		break;
	case PIC_QXGA:    /* 2048 * 1536 */
		pstSize->u32Width = 2048;
		pstSize->u32Height = 1536;
		break;
	case PIC_WVGA:    /* 854 * 480 */
		pstSize->u32Width = 854;
		pstSize->u32Height = 480;
		break;
	case PIC_WSXGA:   /* 1680 * 1050 */
		pstSize->u32Width = 1680;
		pstSize->u32Height = 1050;
		break;
	case PIC_WUXGA:   /* 1920 * 1200 */
		pstSize->u32Width = 1920;
		pstSize->u32Height = 1200;
		break;
	case PIC_WQXGA:   /* 2560 * 1600 */
		pstSize->u32Width = 2560;
		pstSize->u32Height = 1600;
		break;
	case PIC_HD720:   /* 1280 * 720 */
		pstSize->u32Width = 1280;
		pstSize->u32Height = 720;
		break;
	case PIC_HD1080:  /* 1920 * 1080 */
		pstSize->u32Width = 1920;
		pstSize->u32Height = 1080;
		break;
	default:
		return HI_FAILURE;
	}
	return HI_SUCCESS;
}


/******************************************************************************
 * function : start vpss group and chn
 * Input enNorm:视频输入制式类型
 * Input s32VpssGrpCnt:vpass组的总数
 ******************************************************************************/
HI_S32 vpss_start(VIDEO_NORM_E enNorm,HI_S32 s32VpssGrpCnt)
{
	SIZE_S stSize;
	HI_S32 s32Ret = HI_FAILURE;


	VPSS_GRP VpssGrp;
	VPSS_CHN VpssChn;
	VPSS_GRP_ATTR_S stGrpAttr = {0};
	VPSS_CHN_ATTR_S stChnAttr = {0};
	VPSS_GRP_PARAM_S stVpssParam = {0};
	HI_S32 i, j;

	/*vpass group 的数量不能操作上限值*/
	if(s32VpssGrpCnt > MAX_VP_GRP_CNT){
		printf("[%s][Error]:vpass group > max value(%d) line:%d\n", __func__, MAX_VP_GRP_CNT, __LINE__);
		return 0;
	}
	s32Ret = SYS_GetPicSize(enNorm, PIC_HD1080, &stSize);//PIC_HD1080 PIC_HD720
	if (HI_SUCCESS != s32Ret){
		printf("[%s][Error]:DEMO_COMM_SYS_GetPicSize failed! line:%d\n", __func__, __LINE__);
	}
	
	memset(&stGrpAttr,0,sizeof(VPSS_GRP_ATTR_S));
	stGrpAttr.u32MaxW = stSize.u32Width; /*最大图像的大小*/
	stGrpAttr.u32MaxH = stSize.u32Height;
	stGrpAttr.bNrEn = HI_TRUE;
	stGrpAttr.enDieMode = VPSS_DIE_MODE_NODIE;
	stGrpAttr.enPixFmt = PIXEL_FORMAT_YUV_SEMIPLANAR_420; /*像素格式*/

	printf("[%s]:Vpass grp attr=MaxSize=%d*%d line:%d\n", __func__, (stGrpAttr.u32MaxW), (stGrpAttr.u32MaxH), __LINE__);
	for(i=0; i<s32VpssGrpCnt ; i++)
	{
		VpssGrp = i;
		/*** create vpss group ***/
		s32Ret = HI_MPI_VPSS_CreateGrp(VpssGrp, &stGrpAttr); /*创建vpass grop*/
		if (s32Ret != HI_SUCCESS){
			printf("[%s][Error]:HI_MPI_VPSS_CreateGrp failed with %#x! line:%d\n", __func__, s32Ret, __LINE__);
			return HI_FAILURE;
		}
		/*** set vpss param ***/
		s32Ret = HI_MPI_VPSS_GetGrpParam(VpssGrp, &stVpssParam); /*获取 VPSS GROUP 参数*/
		if (s32Ret != HI_SUCCESS){
			printf("[%s][Error]:failed with %#x! line:%d\n", __func__, s32Ret, __LINE__);
			return HI_FAILURE;
		}
		stVpssParam.u32IeStrength = 0;  /*画面锐利度，越大瑞锐利度越高，噪声越明显*/
		s32Ret = HI_MPI_VPSS_SetGrpParam(VpssGrp, &stVpssParam);
		if (s32Ret != HI_SUCCESS){
			printf("[%d][Error]:failed with %#x! line:%d\n", __func__, s32Ret, __LINE__);
			return HI_FAILURE;
		}
		/*** enable vpss chn, with frame ***/
		for(j=0; j<VPSS_MAX_CHN_NUM; j++)
		{
			VpssChn = j; /*vpass 组的通道号*/
			/* Set Vpss Chn attr */
			stChnAttr.bSpEn = HI_FALSE;
			stChnAttr.bUVInvert = HI_FALSE;
			stChnAttr.bBorderEn = HI_TRUE;
			stChnAttr.stBorder.u32Color = 0xff00; /*边框颜色*/
			stChnAttr.stBorder.u32LeftWidth = 2; /*边框的大小*/
			stChnAttr.stBorder.u32RightWidth = 2;
			stChnAttr.stBorder.u32TopWidth = 2;
			stChnAttr.stBorder.u32BottomWidth = 2;
			s32Ret = HI_MPI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stChnAttr); /*设置通道*/
			if (s32Ret != HI_SUCCESS){
				printf("[%d][Error]:HI_MPI_VPSS_SetChnAttr failed with %#x line:%d\n", __func__, s32Ret, __LINE__);
				return HI_FAILURE;
			}
			s32Ret = HI_MPI_VPSS_EnableChn(VpssGrp, VpssChn); /*使能通道*/
			if (s32Ret != HI_SUCCESS){
				printf("[%d][Error]:HI_MPI_VPSS_EnableChn failed with %#x line:%d\n", __func__, s32Ret, __LINE__);
				return HI_FAILURE;
			}
		}
		/*** start vpss group ***/
		s32Ret = HI_MPI_VPSS_StartGrp(VpssGrp); /*使能组*/
		if (s32Ret != HI_SUCCESS){
			printf("[%d][Error]:HI_MPI_VPSS_StartGrp failed with %#x line:%d\n", __func__, s32Ret, __LINE__);
			return HI_FAILURE;
		}
	}

	if (HI_SUCCESS != s32Ret)
	{
		printf("[%d][Error]:Start Vpss failed! line:%d\n", __func__, __LINE__);
	}
	return HI_SUCCESS;
}



/*****************************************************************************
 * function : Vdec chn to vpss group
 *****************************************************************************/
HI_S32 vdec_to_vpss(VPSS_GRP VpssGrp)
{
	HI_S32 s32Ret;
	MPP_CHN_S stSrcChn;
	MPP_CHN_S stDestChn;
	//将vdec 的第0 路绑定到vpss 的group8 上(支持ch0 ch2 ch3)
	// vdec to vpss
	stSrcChn.enModId = HI_ID_VDEC;
	stSrcChn.s32DevId = 0;
	stSrcChn.s32ChnId = 0;
	stDestChn.enModId = HI_ID_VPSS;
	stDestChn.s32DevId = VpssGrp;//8
	stDestChn.s32ChnId = 0;
#if 1
	printf("[%s]:Dec Bind Vppass:Dec=%d-%d VP=%d-%d line:%d\n", __func__, (stSrcChn.s32DevId), (stSrcChn.s32ChnId),\
		(stDestChn.s32DevId), (stDestChn.s32ChnId), __LINE__);
#endif
	s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
	if (s32Ret != HI_SUCCESS){
		printf("[%s][Error]:failed with %#x!\n", __func__, s32Ret, __LINE__);
		return HI_FAILURE;
	}

	return HI_SUCCESS;
}


/******************************************************************************
* Description:打印H264的编码参数
*********************************************************************************/
int PrintVencAttrH264(const VENC_ATTR_H264_S     *p_H264Attr, const VENC_CHN_ATTR_S *p_vencAttr){
#if 1
	/*H264的一些编码参数*/
	VENC_ATTR_H264_CBR_S *p_stH264Cbr = &(p_vencAttr->stRcAttr.stAttrH264Cbr);
	printf("[%s][H264]:mode=%d size=%d*%d statTime=%d FrmRate=%d-%d BitRate=%d line:%d\n", __func__, \
		(p_vencAttr->stRcAttr.enRcMode), (p_H264Attr->u32PicWidth), (p_H264Attr->u32MaxPicHeight), \
		(p_stH264Cbr->u32StatTime), (p_stH264Cbr->u32SrcFrmRate), (p_stH264Cbr->fr32DstFrmRate), (p_stH264Cbr->u32BitRate),\
		__LINE__);
#endif
	return 0;
}

/*****************************************************************************
 * function : Venc chn start
 *****************************************************************************/
HI_S32 VENC_StartChn(VENC_CHN VencChn, PAYLOAD_TYPE_E enType, VIDEO_NORM_E enNorm, PIC_SIZE_E enSize, DEMO_RC_E enRcMode,HI_U32 u32Profile)
{
	VENC_PARAM_MOD_S stModParam;
	#if 0
	HI_MPI_VENC_GetModParam(&stModParam);
	printf("stModParam:%d %d %d %d %d\n",stModParam.enVencModType,
			stModParam.stH264eModParam.u32H264eMiniBufMode,
			stModParam.stH264eModParam.u32H264eRcnEqualRef,
			stModParam.stH264eModParam.u32H264eVBSource,
			stModParam.stH264eModParam.u32OneStreamBuffer);
	#endif

	HI_S32 s32Ret;
	VENC_CHN_ATTR_S stVencChnAttr;
	VENC_ATTR_H264_S stH264Attr;
	VENC_ATTR_H264_CBR_S    stH264Cbr;
	VENC_ATTR_H264_VBR_S    stH264Vbr;
	VENC_ATTR_H264_FIXQP_S  stH264FixQp;
	VENC_ATTR_MJPEG_S stMjpegAttr;
	VENC_ATTR_MJPEG_FIXQP_S stMjpegeFixQp;
	VENC_ATTR_JPEG_S stJpegAttr;
	SIZE_S stPicSize;

	s32Ret = SYS_GetPicSize(g_enNorm, enSize, &stPicSize);
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"Get picture size failed!\n");
		return HI_FAILURE;
	}

	/******************************************
	  step 1:  Create Venc Channel
	 ******************************************/
	stVencChnAttr.stVeAttr.enType = enType;
	switch(enType)
	{
	case PT_H264:
		{
			stH264Attr.u32MaxPicWidth  = stPicSize.u32Width; /*编码图像的最大宽度*/
			stH264Attr.u32MaxPicHeight = stPicSize.u32Height; /*编码图像的最大高度*/
			stH264Attr.u32PicWidth     = stPicSize.u32Width;/*the picture width 编码图像的宽度*/
			stH264Attr.u32PicHeight    = stPicSize.u32Height;/*the picture height 编码图像的高度*/
			stH264Attr.u32BufSize      = stPicSize.u32Width * stPicSize.u32Height * 2;/*stream buffer size 码流 buffer 大小*/
			stH264Attr.u32Profile      = u32Profile;/*0: baseline; 1:MP; 2:HP 3:svc-t 编码的等级 */
			stH264Attr.bByFrame        = HI_TRUE;/*get stream mode is slice mode or frame mode? 帧/包模式获取码流（按帧获取）*/
			memcpy(&stVencChnAttr.stVeAttr.stAttrH264e, &stH264Attr, sizeof(VENC_ATTR_H264_S));

			if(DEMO_RC_CBR == enRcMode)
			{
				stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264CBR; /*RC模式*/
				stH264Cbr.u32Gop            = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30; /*H.264 gop 值*/
				stH264Cbr.u32StatTime       = 1; /* stream rate statics time(s) CBR码率统计时间*/
				stH264Cbr.u32SrcFrmRate      = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;/*VI 输入帧率，以 fps 为单位 input (vi) frame rate */
				stH264Cbr.fr32DstFrmRate = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;/* 编码器输出帧率，以 fps 为单位 target frame rate */
				switch (enSize)
				{
				case PIC_QCIF:
					stH264Cbr.u32BitRate = 256; /* 平均码率 average bit rate */
					break;
				case PIC_QVGA:    /* 320 * 240 */
				case PIC_CIF: 
					stH264Cbr.u32BitRate = 512;
					break;
				case PIC_D1:
				case PIC_VGA:	   /* 640 * 480 */
					stH264Cbr.u32BitRate = 1024*2;
					break;
				case PIC_HD720:   /* 1280 * 720 */
					stH264Cbr.u32BitRate = 1024*3;
					break;
				case PIC_HD1080:  /* 1920 * 1080 */
					stH264Cbr.u32BitRate = 1024*6;
					break;
				default :
					stH264Cbr.u32BitRate = 1024*4;
					break;
				}

				stH264Cbr.u32FluctuateLevel = 0; /*  最大码率相对平均码率的波动等级 average bit rate */
				memcpy(&stVencChnAttr.stRcAttr.stAttrH264Cbr, &stH264Cbr, sizeof(VENC_ATTR_H264_CBR_S));
			}
			else if (DEMO_RC_FIXQP == enRcMode) 
			{
				stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264FIXQP;
				stH264FixQp.u32Gop = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
				stH264FixQp.u32SrcFrmRate = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
				stH264FixQp.fr32DstFrmRate = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
				stH264FixQp.u32IQp = 20;
				stH264FixQp.u32PQp = 23;
				memcpy(&stVencChnAttr.stRcAttr.stAttrH264FixQp, &stH264FixQp,sizeof(VENC_ATTR_H264_FIXQP_S));
			}
			else if (DEMO_RC_VBR == enRcMode) 
			{
				stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264VBR;
				stH264Vbr.u32Gop = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
				stH264Vbr.u32StatTime = 1;
				stH264Vbr.u32SrcFrmRate = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
				stH264Vbr.fr32DstFrmRate = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
				stH264Vbr.u32MinQp = 10;
				stH264Vbr.u32MaxQp = 40;
				switch (enSize)
				{
				case PIC_QCIF:
					stH264Vbr.u32MaxBitRate= 256*3; /* average bit rate */
					break;
				case PIC_QVGA:    /* 320 * 240 */
				case PIC_CIF:
					stH264Vbr.u32MaxBitRate = 512*3;
					break;
				case PIC_D1:
				case PIC_VGA:	   /* 640 * 480 */
					stH264Vbr.u32MaxBitRate = 1024*2;
					break;
				case PIC_HD720:   /* 1280 * 720 */
					stH264Vbr.u32MaxBitRate = 1024*3;
					break;
				case PIC_HD1080:  /* 1920 * 1080 */
					stH264Vbr.u32MaxBitRate = 1024*6;
					break;
				default :
					stH264Vbr.u32MaxBitRate = 1024*4*3;
					break;
				}
				memcpy(&stVencChnAttr.stRcAttr.stAttrH264Vbr, &stH264Vbr, sizeof(VENC_ATTR_H264_VBR_S));
			}
			else
			{
				return HI_FAILURE;
			}
			PrintVencAttrH264(&stH264Attr, &stVencChnAttr);
		}
		break;

	case PT_MJPEG:
		{
			stMjpegAttr.u32MaxPicWidth = stPicSize.u32Width;
			stMjpegAttr.u32MaxPicHeight = stPicSize.u32Height;
			stMjpegAttr.u32PicWidth = stPicSize.u32Width;
			stMjpegAttr.u32PicHeight = stPicSize.u32Height;
			stMjpegAttr.u32BufSize = stPicSize.u32Width * stPicSize.u32Height * 2;
			stMjpegAttr.bByFrame = HI_TRUE;  /*get stream mode is field mode  or frame mode*/
			memcpy(&stVencChnAttr.stVeAttr.stAttrMjpeg, &stMjpegAttr, sizeof(VENC_ATTR_MJPEG_S));

			if(DEMO_RC_FIXQP == enRcMode)
			{
				stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_MJPEGFIXQP;
				stMjpegeFixQp.u32Qfactor        = 90;
				stMjpegeFixQp.u32SrcFrmRate      = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
				stMjpegeFixQp.fr32DstFrmRate = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
				memcpy(&stVencChnAttr.stRcAttr.stAttrMjpegeFixQp, &stMjpegeFixQp,
						sizeof(VENC_ATTR_MJPEG_FIXQP_S));
			}
			else if (DEMO_RC_CBR == enRcMode)
			{
				stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_MJPEGCBR;
				stVencChnAttr.stRcAttr.stAttrMjpegeCbr.u32StatTime       = 1;
				stVencChnAttr.stRcAttr.stAttrMjpegeCbr.u32SrcFrmRate      = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
				stVencChnAttr.stRcAttr.stAttrMjpegeCbr.fr32DstFrmRate = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
				stVencChnAttr.stRcAttr.stAttrMjpegeCbr.u32FluctuateLevel = 0;
				switch (enSize)
				{
				case PIC_QCIF:
					stVencChnAttr.stRcAttr.stAttrMjpegeCbr.u32BitRate = 384*3; /* average bit rate */
					break;
				case PIC_QVGA:    /* 320 * 240 */
				case PIC_CIF:
					stVencChnAttr.stRcAttr.stAttrMjpegeCbr.u32BitRate = 768*3;
					break;
				case PIC_D1:
				case PIC_VGA:	   /* 640 * 480 */
					stVencChnAttr.stRcAttr.stAttrMjpegeCbr.u32BitRate = 1024*3*3;
					break;
				case PIC_HD720:   /* 1280 * 720 */
					stVencChnAttr.stRcAttr.stAttrMjpegeCbr.u32BitRate = 1024*5*3;
					break;
				case PIC_HD1080:  /* 1920 * 1080 */
					stVencChnAttr.stRcAttr.stAttrMjpegeCbr.u32BitRate = 1024*10*3;
					break;
				default :
					stVencChnAttr.stRcAttr.stAttrMjpegeCbr.u32BitRate = 1024*7*3;
					break;
				}
			}
			else if (DEMO_RC_VBR == enRcMode) 
			{
				stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_MJPEGVBR;
				stVencChnAttr.stRcAttr.stAttrMjpegeVbr.u32StatTime = 1;
				stVencChnAttr.stRcAttr.stAttrMjpegeVbr.u32SrcFrmRate = (VIDEO_ENCODING_MODE_PAL == enNorm)?25:30;
				stVencChnAttr.stRcAttr.stAttrMjpegeVbr.fr32DstFrmRate = 5;
				stVencChnAttr.stRcAttr.stAttrMjpegeVbr.u32MinQfactor = 50;
				stVencChnAttr.stRcAttr.stAttrMjpegeVbr.u32MaxQfactor = 95;
				switch (enSize)
				{
				case PIC_QCIF:
					stVencChnAttr.stRcAttr.stAttrMjpegeVbr.u32MaxBitRate= 256*3; /* average bit rate */
					break;
				case PIC_QVGA:    /* 320 * 240 */
				case PIC_CIF:
					stVencChnAttr.stRcAttr.stAttrMjpegeVbr.u32MaxBitRate = 512*3;
					break;
				case PIC_D1:
				case PIC_VGA:	   /* 640 * 480 */
					stVencChnAttr.stRcAttr.stAttrMjpegeVbr.u32MaxBitRate = 1024*2*3;
					break;
				case PIC_HD720:   /* 1280 * 720 */
					stVencChnAttr.stRcAttr.stAttrMjpegeVbr.u32MaxBitRate = 1024*3*3;
					break;
				case PIC_HD1080:  /* 1920 * 1080 */
					stVencChnAttr.stRcAttr.stAttrMjpegeVbr.u32MaxBitRate = 1024*6*3;
					break;
				default :
					stVencChnAttr.stRcAttr.stAttrMjpegeVbr.u32MaxBitRate = 1024*4*3;
					break;
				}
			}
			else 
			{
				fprintf(stderr,"cann't support other mode in this version!\n");
				return HI_FAILURE;
			}
		}
		break;

	case PT_JPEG:
		stJpegAttr.u32PicWidth  = stPicSize.u32Width;
		stJpegAttr.u32PicHeight = stPicSize.u32Height;
		stJpegAttr.u32BufSize = stPicSize.u32Width * stPicSize.u32Height * 2;
		stJpegAttr.bByFrame = HI_TRUE;/*get stream mode is field mode  or frame mode*/
		memcpy(&stVencChnAttr.stVeAttr.stAttrJpeg, &stJpegAttr, sizeof(stJpegAttr));
		break;
	default:
		return HI_ERR_VENC_NOT_SUPPORT;
	}

	s32Ret = HI_MPI_VENC_CreateChn(VencChn, &stVencChnAttr); /*创建编码通道*/
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"HI_MPI_VENC_CreateChn [%d] faild with %#x!\n",\
				VencChn, s32Ret);
		return s32Ret;
	}

	/******************************************
	  step 2:  Start Recv Venc Pictures
	 ******************************************/
	s32Ret = HI_MPI_VENC_StartRecvPic(VencChn); /*开启编码通道接收输入图像，超出指定的帧数后自动停止接收图像*/
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"HI_MPI_VENC_StartRecvPic faild with%#x!\n", s32Ret);
		return HI_FAILURE;
	}

	return HI_SUCCESS;

}



/*****************************************************************************
 * function :venc start chn
 *****************************************************************************/
HI_S32 venc_start(HI_U32 level,VIDEO_NORM_E enNorm,HI_U32 s32VpssGrpCnt)
{
	VENC_CHN VencChn;
	PAYLOAD_TYPE_E enPayLoad[3]= {PT_H264,PT_H264, PT_JPEG};
	PIC_SIZE_E enSize[3] = {PIC_HD1080, PIC_HD720,PIC_CIF};
	HI_U32 u32Profile = 1; /*0: baseline; 1:MP; 2:HP 3:svc-t */
	DEMO_RC_E enRcMode = DEMO_RC_CBR;
	HI_U32 i;
	HI_U32 VpssGrp;
	//start venc

	for (i=0; i<s32VpssGrpCnt; i++)
	{
		//group 0 1 2 3 4 5 6 7 8
		/*** main stream,H264**/// 0 2 4 6 8 10 12 14 16
		VencChn = i*2;
		VpssGrp = i;

		if(level == 0)
			VENC_StartChn(VencChn,enPayLoad[0] ,enNorm ,enSize[0],enRcMode,u32Profile);

		/*** Sub stream **/    // 1  3 5 7 9 11 13 15 17
		if(level == 1){
			VencChn ++;
			VENC_StartChn(VencChn,enPayLoad[1] ,enNorm ,enSize[1],enRcMode,u32Profile);
		}

	}

	return HI_SUCCESS;
}

/*****************************************************************************
 * function :vpss group to venc chn
 *****************************************************************************/
HI_S32 vpss_to_venc(HI_U32 level,HI_U32 s32VpssGrpCnt)
{

	HI_S32 s32Ret = HI_SUCCESS;
	MPP_CHN_S stSrcChn;
	MPP_CHN_S stDestChn;
	HI_U32 i;
	HI_U32 VpssGrp;
	VENC_CHN VencChn;
	VPSS_CHN VpssChn_High = 0;
	VPSS_CHN VpssChn_low = 1;

#if 1
	printf("[%s]:level=%d s32VpssGrpCnt=%d line:%d\n", __func__, level, s32VpssGrpCnt, __LINE__);
	printf("[%s]:Vpass bind Venc:", __func__);
	for (i=0; i<s32VpssGrpCnt; i++)
	{
		VencChn = i*2;
		VpssGrp = i;
		stSrcChn.enModId = HI_ID_VPSS;
		stSrcChn.s32DevId = VpssGrp;
		stSrcChn.s32ChnId = VpssChn_High;

		stDestChn.enModId = HI_ID_VENC;
		stDestChn.s32DevId = 0;
		stDestChn.s32ChnId = VencChn;

		if(level == 0){
			printf("VP=%d-%d Ven=%d-%d ", (stSrcChn.s32DevId), (stSrcChn.s32ChnId), (stDestChn.s32DevId), (stDestChn.s32ChnId));
			s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
			if (s32Ret != HI_SUCCESS){
				printf("[%s][Eror]:failed with %#x! line:%d\n",  __func__, s32Ret, __LINE__);
				return HI_FAILURE;
			}
		}
		if(level == 1){
			VencChn ++;
			stDestChn.s32ChnId = VencChn;
			stSrcChn.s32ChnId = VpssChn_low;
			printf("VP=%d-%d Ven=%d-%d ", (stSrcChn.s32DevId), (stSrcChn.s32ChnId), (stDestChn.s32DevId), (stDestChn.s32ChnId));
			s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
			if (s32Ret != HI_SUCCESS){
				printf("[%s][Eror]:failed with %#x! line:%d\n", __func__, s32Ret, __LINE__);
				return HI_FAILURE;
			}
		}
	}
#endif
	return HI_SUCCESS;

}


static HI_VOID VO_HdmiConvertSync(VO_INTF_SYNC_E enIntfSync,
		HI_HDMI_VIDEO_FMT_E *penVideoFmt)
{
	switch (enIntfSync)
	{
	case VO_OUTPUT_PAL:
		*penVideoFmt = HI_HDMI_VIDEO_FMT_PAL;
		break;
	case VO_OUTPUT_NTSC:
		*penVideoFmt = HI_HDMI_VIDEO_FMT_NTSC;
		break;
	case VO_OUTPUT_1080P24:
		*penVideoFmt = HI_HDMI_VIDEO_FMT_1080P_24;
		break;
	case VO_OUTPUT_1080P25:
		*penVideoFmt = HI_HDMI_VIDEO_FMT_1080P_25;
		break;
	case VO_OUTPUT_1080P30:
		*penVideoFmt = HI_HDMI_VIDEO_FMT_1080P_30;
		break;
	case VO_OUTPUT_720P50:
		*penVideoFmt = HI_HDMI_VIDEO_FMT_720P_50;
		break;
	case VO_OUTPUT_720P60:
		*penVideoFmt = HI_HDMI_VIDEO_FMT_720P_60;
		break;
	case VO_OUTPUT_1080I50:
		*penVideoFmt = HI_HDMI_VIDEO_FMT_1080i_50;
		break;
	case VO_OUTPUT_1080I60:
		*penVideoFmt = HI_HDMI_VIDEO_FMT_1080i_60;
		break;
	case VO_OUTPUT_1080P50:
		*penVideoFmt = HI_HDMI_VIDEO_FMT_1080P_50;
		break;
	case VO_OUTPUT_1080P60:
		*penVideoFmt = HI_HDMI_VIDEO_FMT_1080P_60;
		break;
	case VO_OUTPUT_576P50:
		*penVideoFmt = HI_HDMI_VIDEO_FMT_576P_50;
		break;
	case VO_OUTPUT_480P60:
		*penVideoFmt = HI_HDMI_VIDEO_FMT_480P_60;
		break;
	case VO_OUTPUT_800x600_60:
		*penVideoFmt = HI_HDMI_VIDEO_FMT_VESA_800X600_60;
		break;
	case VO_OUTPUT_1024x768_60:
		*penVideoFmt = HI_HDMI_VIDEO_FMT_VESA_1024X768_60;
		break;
	case VO_OUTPUT_1280x1024_60:
		*penVideoFmt = HI_HDMI_VIDEO_FMT_VESA_1280X1024_60;
		break;
	case VO_OUTPUT_1366x768_60:
		*penVideoFmt = HI_HDMI_VIDEO_FMT_VESA_1366X768_60;
		break;
	case VO_OUTPUT_1440x900_60:
		*penVideoFmt = HI_HDMI_VIDEO_FMT_VESA_1440X900_60;
		break;
	case VO_OUTPUT_1280x800_60:
		*penVideoFmt = HI_HDMI_VIDEO_FMT_VESA_1280X800_60;
		break;
	case VO_OUTPUT_1600x1200_60:
		*penVideoFmt = HI_HDMI_VIDEO_FMT_VESA_1600X1200_60;
		break;
	case VO_OUTPUT_2560x1440_30:
		*penVideoFmt = HI_HDMI_VIDEO_FMT_2560x1440_30;
		break;
	case VO_OUTPUT_2560x1600_60:
		*penVideoFmt = HI_HDMI_VIDEO_FMT_2560x1600_60;
		break;
	case VO_OUTPUT_3840x2160_30:
		*penVideoFmt = HI_HDMI_VIDEO_FMT_3840X2160P_30;
		break;
	default :
		fprintf(stderr,"Unkonw VO_INTF_SYNC_E value!\n");
		break;
	}

	return;
}


/******************************************************************************
 * function : vo hdmistart
 ******************************************************************************/
HI_S32 VO_HdmiStart(VO_INTF_SYNC_E enIntfSync)
{
	HI_HDMI_ATTR_S      stAttr;
	HI_HDMI_VIDEO_FMT_E enVideoFmt;
	HI_HDMI_INIT_PARA_S stHdmiPara;

	VO_HdmiConvertSync(enIntfSync, &enVideoFmt);

	stHdmiPara.pfnHdmiEventCallback = NULL;
	stHdmiPara.pCallBackArgs = NULL;
	stHdmiPara.enForceMode = HI_HDMI_FORCE_HDMI;
	HI_MPI_HDMI_Init(&stHdmiPara);

	HI_MPI_HDMI_Open(HI_HDMI_ID_0);

	HI_MPI_HDMI_GetAttr(HI_HDMI_ID_0, &stAttr);

	stAttr.bEnableHdmi = HI_TRUE;

	stAttr.bEnableVideo = HI_TRUE;
	stAttr.enVideoFmt = enVideoFmt;

	stAttr.enVidOutMode = HI_HDMI_VIDEO_MODE_YCBCR444;
	stAttr.enDeepColorMode = HI_HDMI_DEEP_COLOR_OFF;
	stAttr.bxvYCCMode = HI_FALSE;

	stAttr.bEnableAudio = HI_FALSE;
	stAttr.enSoundIntf = HI_HDMI_SND_INTERFACE_I2S;
	stAttr.bIsMultiChannel = HI_FALSE;

	stAttr.enBitDepth = HI_HDMI_BIT_DEPTH_16;

	stAttr.bEnableAviInfoFrame = HI_TRUE;
	stAttr.bEnableAudInfoFrame = HI_TRUE;
	stAttr.bEnableSpdInfoFrame = HI_FALSE;
	stAttr.bEnableMpegInfoFrame = HI_FALSE;

	stAttr.bDebugFlag = HI_FALSE;          
	stAttr.bHDCPEnable = HI_FALSE;

	stAttr.b3DEnable = HI_FALSE;

	HI_MPI_HDMI_SetAttr(HI_HDMI_ID_0, &stAttr);

	HI_MPI_HDMI_Start(HI_HDMI_ID_0);

	fprintf(stderr,"HDMI start success.\n");
	return HI_SUCCESS;
}


/******************************************************************************
 * function : vo start dev
 ******************************************************************************/
HI_S32 VO_StartDev(VO_DEV VoDev,VO_PUB_ATTR_S *pstVoPubAttr)
{
	HI_S32 s32Ret = HI_SUCCESS;
	s32Ret = HI_MPI_VO_SetPubAttr(VoDev, pstVoPubAttr);
	if (s32Ret != HI_SUCCESS)
	{
		fprintf(stderr,"failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}

	s32Ret = HI_MPI_VO_Enable(VoDev);
	if (s32Ret != HI_SUCCESS)
	{
		fprintf(stderr,"failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}

	return s32Ret;
}

HI_S32 VO_GetWH(VO_INTF_SYNC_E enIntfSync, HI_U32 *pu32W,HI_U32 *pu32H, HI_U32 *pu32Frm)
{
	switch (enIntfSync)
	{
	case VO_OUTPUT_PAL       :  *pu32W = 720;  *pu32H = 576;  *pu32Frm = 25; break;
	case VO_OUTPUT_NTSC      :  *pu32W = 720;  *pu32H = 480;  *pu32Frm = 30; break;        
	case VO_OUTPUT_576P50    :  *pu32W = 720;  *pu32H = 576;  *pu32Frm = 50; break;
	case VO_OUTPUT_480P60    :  *pu32W = 720;  *pu32H = 480;  *pu32Frm = 60; break;
	case VO_OUTPUT_800x600_60:  *pu32W = 800;  *pu32H = 600;  *pu32Frm = 60; break;
	case VO_OUTPUT_720P50    :  *pu32W = 1280; *pu32H = 720;  *pu32Frm = 50; break;
	case VO_OUTPUT_720P60    :  *pu32W = 1280; *pu32H = 720;  *pu32Frm = 60; break;        
	case VO_OUTPUT_1080I50   :  *pu32W = 1920; *pu32H = 1080; *pu32Frm = 50; break;
	case VO_OUTPUT_1080I60   :  *pu32W = 1920; *pu32H = 1080; *pu32Frm = 60; break;
	case VO_OUTPUT_1080P24   :  *pu32W = 1920; *pu32H = 1080; *pu32Frm = 24; break;        
	case VO_OUTPUT_1080P25   :  *pu32W = 1920; *pu32H = 1080; *pu32Frm = 25; break;
	case VO_OUTPUT_1080P30   :  *pu32W = 1920; *pu32H = 1080; *pu32Frm = 30; break;
	case VO_OUTPUT_1080P50   :  *pu32W = 1920; *pu32H = 1080; *pu32Frm = 50; break;
	case VO_OUTPUT_1080P60   :  *pu32W = 1920; *pu32H = 1080; *pu32Frm = 60; break;
	case VO_OUTPUT_1024x768_60:  *pu32W = 1024; *pu32H = 768;  *pu32Frm = 60; break;
	case VO_OUTPUT_1280x1024_60: *pu32W = 1280; *pu32H = 1024; *pu32Frm = 60; break;
	case VO_OUTPUT_1366x768_60:  *pu32W = 1366; *pu32H = 768;  *pu32Frm = 60; break;
	case VO_OUTPUT_1440x900_60:  *pu32W = 1440; *pu32H = 900;  *pu32Frm = 60; break;
	case VO_OUTPUT_1280x800_60:  *pu32W = 1280; *pu32H = 800;  *pu32Frm = 60; break;        
	case VO_OUTPUT_1600x1200_60: *pu32W = 1600; *pu32H = 1200; *pu32Frm = 60; break;
	case VO_OUTPUT_1680x1050_60: *pu32W = 1680; *pu32H = 1050; *pu32Frm = 60; break;
	case VO_OUTPUT_1920x1200_60: *pu32W = 1920; *pu32H = 1200; *pu32Frm = 60; break;
	case VO_OUTPUT_3840x2160_30: *pu32W = 3840; *pu32H = 2160; *pu32Frm = 30; break;
	case VO_OUTPUT_3840x2160_60: *pu32W = 3840; *pu32H = 2160; *pu32Frm = 60; break;
	case VO_OUTPUT_USER    :     *pu32W = 720;  *pu32H = 576;  *pu32Frm = 25; break;
	default: 
								 fprintf(stderr,"vo enIntfSync not support!\n");
								 return HI_FAILURE;
	}
	return HI_SUCCESS;
}

/******************************************************************************
 * function : vo start volayer
 ******************************************************************************/
HI_S32 VO_StartVideoLayer(VO_LAYER VoLayer,VO_INTF_SYNC_E enIntfSync)
{
	HI_S32 s32Ret = HI_SUCCESS;
	VO_VIDEO_LAYER_ATTR_S stLayerAttr;
	VO_PUB_ATTR_S stVoPubAttr_hd0;
	memset(&(stLayerAttr), 0 , sizeof(VO_VIDEO_LAYER_ATTR_S));

	//获取画布大小--- 图像分辨率
	s32Ret = VO_GetWH(enIntfSync, \
			&stLayerAttr.stImageSize.u32Width, \
			&stLayerAttr.stImageSize.u32Height, \
			&stLayerAttr.u32DispFrmRt);
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"Start DEMO_COMM_VO_GetWH failed!\n");

	}

	// 将显示分辨率设置为图像分辨率相同的大小
	if(VoLayer == g_VoLayerPip)
		stLayerAttr.bClusterMode = HI_TRUE;
	stLayerAttr.enPixFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
	stLayerAttr.stDispRect.s32X       = 0;
	stLayerAttr.stDispRect.s32Y       = 0;
	stLayerAttr.stDispRect.u32Width   = stLayerAttr.stImageSize.u32Width;
	stLayerAttr.stDispRect.u32Height  = stLayerAttr.stImageSize.u32Height;
	printf("[%s]:VoLayer=%d resolution=%d-%d line:%d\n", __func__, VoLayer, (stLayerAttr.stDispRect.u32Width), (stLayerAttr.stDispRect.u32Height), __LINE__);
	s32Ret = HI_MPI_VO_SetVideoLayerAttr(VoLayer, &stLayerAttr);
	if (s32Ret != HI_SUCCESS)
	{
		fprintf(stderr,"failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}

	s32Ret = HI_MPI_VO_EnableVideoLayer(VoLayer);
	if (s32Ret != HI_SUCCESS)
	{
		fprintf(stderr,"failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}

	return s32Ret;
}

/******************************************************************************
 * function : vo start chn
 ******************************************************************************/
HI_S32 VO_StartChn(VO_LAYER VoLayer,DEMO_VO_MODE_E enVoMode)
{
	HI_S32 i;
	HI_S32 s32Ret = HI_SUCCESS;
	HI_U32 u32WndNum = 0;
	HI_U32 u32Square = 0;
	HI_U32 u32Width = 0;
	HI_U32 u32Height = 0;
	VO_CHN_ATTR_S stChnAttr;
	VO_VIDEO_LAYER_ATTR_S stLayerAttr;
	switch (enVoMode)
	{
	case DEMO_VO_MODE_1MUX:
		u32WndNum = 1;
		u32Square = 1;
		break;
	case DEMO_VO_MODE_4MUX:
		u32WndNum = 4;
		u32Square = 2;
		break;
	case DEMO_VO_MODE_9MUX:
		u32WndNum = 9;
		u32Square = 3;
		break;
	case DEMO_VO_MODE_16MUX:
		u32WndNum = 16;
		u32Square = 4;
		break;            
	default:
		fprintf(stderr,"failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}

	s32Ret = HI_MPI_VO_GetVideoLayerAttr(VoLayer, &stLayerAttr);
	if (s32Ret != HI_SUCCESS)
	{
		fprintf(stderr,"failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}

	//得到画布大小
	u32Width = stLayerAttr.stImageSize.u32Width;
	u32Height = stLayerAttr.stImageSize.u32Height;

	//在画布内拼接各通道里的图像
	printf("[%s]:u32WndNum=%d VoLayer=%d ", __func__, u32WndNum, VoLayer);
	for (i=0; i<u32WndNum; i++)
	{
		stChnAttr.stRect.s32X       = DEMO_ALIGN_BACK((u32Width/u32Square) * (i%u32Square), 2);
		stChnAttr.stRect.s32Y       = DEMO_ALIGN_BACK((u32Height/u32Square) * (i/u32Square), 2);
		stChnAttr.stRect.u32Width   = DEMO_ALIGN_BACK(u32Width/u32Square, 2);
		stChnAttr.stRect.u32Height  = DEMO_ALIGN_BACK(u32Height/u32Square, 2);
		stChnAttr.u32Priority       = 0;
		stChnAttr.bDeflicker        = HI_TRUE; //HI_FALSE;
		
#if 1 /*坐标位置和分辨率*/
	if(2 == VoLayer){
		stChnAttr.stRect.s32X = 1000;
		stChnAttr.stRect.s32Y = 0;
	}
		printf("VO=%d-%d coordinate=%d-%d resolution=%d-%d ", (VoLayer), (i), \
		(stChnAttr.stRect.s32X), (stChnAttr.stRect.s32Y), \
		(stChnAttr.stRect.u32Width), (stChnAttr.stRect.u32Height));
#endif
		s32Ret = HI_MPI_VO_SetChnAttr(VoLayer, i, &stChnAttr);
		if (s32Ret != HI_SUCCESS)
		{
			printf("[ERROR]\n");
			fprintf(stderr,"%s(%d):failed with %#x!\n\n",\
					__FUNCTION__,__LINE__,  s32Ret);
			return HI_FAILURE;
		}
		//应该根据输入的源来使能通道,此时接了第四路--最左边一个SDI口
		s32Ret = HI_MPI_VO_EnableChn(VoLayer, i);
		if (s32Ret != HI_SUCCESS)
		{
			printf("[ERROR]\n");
			fprintf(stderr,"ch%d failed with %#x!\n\n",i,s32Ret);
			return HI_FAILURE;
		}
	}
	printf(" line:%d\n", __LINE__);
	return HI_SUCCESS;
}



/******************************************************************************
 * function : vo stop chn
 ******************************************************************************/
HI_S32 VO_StopChn(VO_LAYER VoLayer,DEMO_VO_MODE_E enVoMode)
{
	HI_S32 i;
	HI_S32 s32Ret = HI_SUCCESS;
	HI_U32 u32WndNum = 0;
	HI_U32 u32Square = 0;
	switch (enVoMode)
	{
	case DEMO_VO_MODE_1MUX:
		u32WndNum = 1;
		u32Square = 1;
		break;
	case DEMO_VO_MODE_4MUX:
		u32WndNum = 4;
		u32Square = 2;
		break;
	case DEMO_VO_MODE_9MUX:
		u32WndNum = 9;
		u32Square = 3;
		break;
	case DEMO_VO_MODE_16MUX:
		u32WndNum = 16;
		u32Square = 4;
		break;            
	default:
		fprintf(stderr,"failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}

	for (i=0; i<u32WndNum; i++)
	{
		s32Ret = HI_MPI_VO_DisableChn(VoLayer, i);
		if (s32Ret != HI_SUCCESS)
		{
			fprintf(stderr,"failed with %#x!\n", s32Ret);
			return HI_FAILURE;
		}
	}    
	return HI_SUCCESS;
}


/******************************************************************************
 * function : start vo dev & videolayer & chn 
 ******************************************************************************/
HI_S32 vo_start(HI_VOID)
{
	VO_LAYER VoLayer;
	VO_DEV VoDev;
	VO_PUB_ATTR_S stVoPubAttr_hd0;
	VO_VIDEO_LAYER_ATTR_S stLayerAttr;
	DEMO_VO_MODE_E enVoMode;
	HI_U32 s32Ret;
	HI_U32 u32WndNum;

	/******************************************
	  step 5: start vo HD0 (bt1120+VGA), multi-screen, you can switch mode
	 ******************************************/
	fprintf(stderr,"start vo HD0.\n");
	VoDev = DEMO_VO_DEV_DHD0;
	VoLayer = DEMO_VO_LAYER_VHD0;
	enVoMode = DEMO_VO_MODE_9MUX;

	printf("[%s]:Begin set layer=%d *********************************** line:%d\n", __func__, VoLayer, __LINE__);
	stVoPubAttr_hd0.enIntfSync = VO_OUTPUT_1080P60;
	stVoPubAttr_hd0.enIntfType = VO_INTF_BT1120|VO_INTF_VGA| VO_INTF_HDMI;
	stVoPubAttr_hd0.u32BgColor = 0x9999a7; //0x000000ff;
	s32Ret = VO_StartDev(VoDev,&stVoPubAttr_hd0);
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"Start DEMO_COMM_VO_StartDev failed!\n");

	}

	s32Ret = VO_StartVideoLayer(VoLayer,VO_OUTPUT_1080P60);
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"Start DEMO_COMM_VO_StartLayer failed!\n");

	}

	s32Ret = VO_StartChn(VoLayer,enVoMode);
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"Start DEMO_COMM_VO_StartChn failed!\n");

	}
#ifdef HDMI_SUPPORT
	/* if it's displayed on HDMI, we should start HDMI */
	if (stVoPubAttr_hd0.enIntfType & VO_INTF_HDMI)
	{
		if (HI_SUCCESS != VO_HdmiStart(stVoPubAttr_hd0.enIntfSync))
		{
			fprintf(stderr,"Start DEMO_COMM_VO_HdmiStart failed!\n");

		}
	}
#endif

	printf("[%s]:End set layer=%d *********************************** line:%d\n", __func__, VoLayer, __LINE__);

	return HI_SUCCESS;
}

/******************************************************************************
 * function : vpss group(chn0) to vo chn(volayer)
 ******************************************************************************/
HI_S32 vpss_to_vo(VPSS_CHN VpssChn_Vo,VO_LAYER VoLayer,HI_U32 s32VpssGrpCnt)
{

	HI_S32 s32Ret = HI_SUCCESS;
	MPP_CHN_S stSrcChn;
	MPP_CHN_S stDestChn;
	HI_S32 VpssGrp,VoChn;
	HI_S32 i;

	printf("[%s]:", __func__);
	for(i=0;i<s32VpssGrpCnt;i++)
	{
		VoChn = i;
		VpssGrp = i;

		stSrcChn.enModId = HI_ID_VPSS;
		stSrcChn.s32DevId = VpssGrp;
		stSrcChn.s32ChnId = VpssChn_Vo;

		stDestChn.enModId = HI_ID_VOU;
		stDestChn.s32DevId = VoLayer;
		stDestChn.s32ChnId = VoChn;
		printf("VP=%d-%d VO=%d-%d ", (stSrcChn.s32DevId), (stSrcChn.s32ChnId), (stDestChn.s32DevId), (stDestChn.s32ChnId));
		s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
		if (s32Ret != HI_SUCCESS){
			printf("[%s][Error]:failed with %#x! line:%d\n", __func__, s32Ret, __LINE__);
			return HI_FAILURE;
		}

	}
	printf("\n");

	return s32Ret;
}

/******************************************************************************
 * function : vi chn to vo chn(volayer)
 ******************************************************************************/
HI_S32 vi_to_vo(VO_LAYER VoLayer)
{
	HI_S32 j, s32Ret;
	VPSS_GRP VpssGrp;
	MPP_CHN_S stSrcChn;
	MPP_CHN_S stDestChn;
	DEMO_VI_PARAM_S stViParam;
	VI_CHN ViChn;

	s32Ret = VI_Mode2Param(g_enViMode, &stViParam);
	if (HI_SUCCESS !=s32Ret)
	{
		fprintf(stderr,"DEMO_COMM_VI_Mode2Param failed!\n");
		return HI_FAILURE;
	}

	printf("[%s]:VI bind VO: \n", __func__, __LINE__);
	for (j=0; j<stViParam.s32ViChnCnt; j++)
	{
		/*VI_MODE_8_1080P:  use chn 0,4,8,12,16,20,24,28 */
		ViChn = j * stViParam.s32ViChnInterval;   

		stSrcChn.enModId = HI_ID_VIU;
		stSrcChn.s32DevId = 0;
		stSrcChn.s32ChnId = ViChn;
		/*use chn 0 1 2 3 4 5 6 7*/
		stDestChn.enModId = HI_ID_VOU;
		stDestChn.s32DevId = VoLayer;
		stDestChn.s32ChnId = j;
		printf("VI=%d-%d VO=%d-%d ", (stSrcChn.s32DevId), (stSrcChn.s32ChnId), (stDestChn.s32DevId), (stDestChn.s32ChnId));
		s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
		if (s32Ret != HI_SUCCESS)
		{
			fprintf(stderr,"failed with %#x!\n", s32Ret);
			return HI_FAILURE;
		}
	}
	printf("\n");

#if 0
	//一般只显示8 路流
	//将第9 路也绑定到vi模块的chn 12 上
	stSrcChn.enModId = HI_ID_VIU;
	stSrcChn.s32DevId = 0;
	stSrcChn.s32ChnId = 12;
	/*use chn 0 1 2 3 4 5 6 7*/
	stDestChn.enModId = HI_ID_VOU;
	stDestChn.s32DevId = VoLayer;
	stDestChn.s32ChnId = 8;

	s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
	if (s32Ret != HI_SUCCESS)
	{
		fprintf(stderr,"failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}
#endif

	return HI_SUCCESS;
}

static HI_S32 s_s32MemDev = 0;

HI_S32 memopen( void )
{
	if (s_s32MemDev <= 0)
	{
		s_s32MemDev = open ("/dev/mem", O_CREAT|O_RDWR|O_SYNC);
		if (s_s32MemDev <= 0)
		{
			return -1;
		}
	}
	return 0;
}

HI_VOID memclose()
{
	close(s_s32MemDev);
}

/******************************************************************************
 * function : get vo dev data
 ******************************************************************************/
HI_S32 WbcDump(VO_DEV VoDev, HI_U32 u32Cnt)
{
	HI_S32 i, s32Ret;
	VIDEO_FRAME_INFO_S stFrame;
	//VIDEO_FRAME_INFO_S astFrame[256];
	HI_CHAR szYuvName[128];
	HI_CHAR szPixFrm[10];
	FILE *fp;
	HI_S32 s32MilliSec = 1000*2;

	VO_WBC_SOURCE_S stWbcSource;
	stWbcSource.enSourceType = VO_WBC_SOURCE_DEV;
	stWbcSource.u32SourceId = 0;
	HI_MPI_VO_SetWbcSource(0, &stWbcSource );

	VO_WBC_ATTR_S stWbcAttr;
	VO_WBC_MODE_E enWbcMode;
	stWbcAttr.stTargetSize.u32Width = 1920;
	stWbcAttr.stTargetSize.u32Height = 1080;
	stWbcAttr.enPixelFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
	stWbcAttr.u32FrameRate = 25;

	if (HI_SUCCESS != HI_MPI_VO_SetWbcAttr(0,&stWbcAttr))
	{
		printf("Set wbc attr failed!\n");
		return HI_FAILURE;
	}

	enWbcMode = VO_WBC_MODE_NOMAL;
	if (HI_SUCCESS != HI_MPI_VO_SetWbcMode(0, enWbcMode))
	{
		printf("Set wbc mode failed!\n");
		return HI_FAILURE;
	}

	HI_MPI_VO_EnableWbc(0);



	s32Ret = HI_MPI_VO_SetWbcDepth(VoDev, 5);
	if (HI_SUCCESS != s32Ret)
	{
		printf("Dev(%d) HI_MPI_VO_SetWbcDepth errno %#x\n", VoDev, s32Ret);
		return s32Ret;
	}

	/* Get Frame to make file name*/
	s32Ret = HI_MPI_VO_GetWbcFrame(VoDev, &stFrame, s32MilliSec);
	if (HI_SUCCESS != s32Ret)
	{
		printf("HI_MPI_VO(%d)_WbcGetScreenFrame errno %#x\n", VoDev, s32Ret);
		return -1;
	}

	/* make file name */
	strcpy(szPixFrm,
			(PIXEL_FORMAT_YUV_SEMIPLANAR_420 == stFrame.stVFrame.enPixelFormat)?"p420":"p422");
	sprintf(szYuvName, "./Wbc(%d)_%d_%d_%s_%d.yuv",VoDev,
			stFrame.stVFrame.u32Width, stFrame.stVFrame.u32Height,szPixFrm,u32Cnt);
	printf("Dump YUV frame of Wbc(%d) to file: \"%s\"\n",VoDev, szYuvName);

	s32Ret = HI_MPI_VO_ReleaseWbcFrame(VoDev, &stFrame);
	if (HI_SUCCESS != s32Ret)
	{
		printf("Dev(%d) HI_MPI_VO_ReleaseWbcFrame errno %#x\n", VoDev, s32Ret);
		return -1;
	}

	/* open file */
	fp = fopen(szYuvName, "wb");

	if (NULL == fp)
	{
		return -1;
	}

	//memopen();

	/* get VO frame  */
	for (i=0; i<u32Cnt; i++)
	{
		s32Ret = HI_MPI_VO_GetWbcFrame(VoDev, &stFrame, s32MilliSec);
		if (HI_SUCCESS != s32Ret)
		{
			printf("get Wbc(%d) frame err\n", VoDev);
			printf("only get %d frame\n", i);
			break;
		}
		/* save VO frame to file */
		yuv_dump(&stFrame.stVFrame, fp);

		/* release frame after using */
		s32Ret = HI_MPI_VO_ReleaseWbcFrame(VoDev, &stFrame);
		if (HI_SUCCESS != s32Ret)
		{
			printf("release Wbc(%d) frame err\n", VoDev);
			printf("only get %d frame\n", i);
			break;
		}
	}

	//memclose();

	fclose(fp);

	return 0;
}


/******************************************************************************
 * function : get vo dev data
 ******************************************************************************/
HI_S32 WbcDump1(VO_DEV VoDev, HI_U32 u32Cnt)
{
	HI_S32 i, s32Ret;
	VIDEO_FRAME_INFO_S stFrame;
	//VIDEO_FRAME_INFO_S astFrame[256];
	HI_CHAR szYuvName[128];
	HI_CHAR szPixFrm[10];
	FILE *fp;
	HI_S32 s32MilliSec = 1000*2;

	VO_WBC_SOURCE_S stWbcSource;
	stWbcSource.enSourceType = VO_WBC_SOURCE_DEV;
	stWbcSource.u32SourceId = 0;
	HI_MPI_VO_SetWbcSource(0, &stWbcSource );

	VO_WBC_ATTR_S stWbcAttr;
	VO_WBC_MODE_E enWbcMode;
	stWbcAttr.stTargetSize.u32Width = 1920;
	stWbcAttr.stTargetSize.u32Height = 1080;
	stWbcAttr.enPixelFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
	stWbcAttr.u32FrameRate = 25;

	if (HI_SUCCESS != HI_MPI_VO_SetWbcAttr(0,&stWbcAttr))
	{
		printf("Set wbc attr failed!\n");
		return HI_FAILURE;
	}

	enWbcMode = VO_WBC_MODE_NOMAL;
	if (HI_SUCCESS != HI_MPI_VO_SetWbcMode(0, enWbcMode))
	{
		printf("Set wbc mode failed!\n");
		return HI_FAILURE;
	}

	HI_MPI_VO_EnableWbc(0);



	s32Ret = HI_MPI_VO_SetWbcDepth(VoDev, 5);
	if (HI_SUCCESS != s32Ret)
	{
		printf("Dev(%d) HI_MPI_VO_SetWbcDepth errno %#x\n", VoDev, s32Ret);
		return s32Ret;
	}

	/* Get Frame to make file name*/
	s32Ret = HI_MPI_VO_GetWbcFrame(VoDev, &stFrame, s32MilliSec);
	if (HI_SUCCESS != s32Ret)
	{
		printf("HI_MPI_VO(%d)_WbcGetScreenFrame errno %#x\n", VoDev, s32Ret);
		return -1;
	}

	/* make file name */
	strcpy(szPixFrm,
			(PIXEL_FORMAT_YUV_SEMIPLANAR_420 == stFrame.stVFrame.enPixelFormat)?"p420":"p422");
	sprintf(szYuvName, "./Wbc(%d)_%d_%d_%s_%d.yuv",VoDev,
			stFrame.stVFrame.u32Width, stFrame.stVFrame.u32Height,szPixFrm,u32Cnt);
	printf("Dump YUV frame of Wbc(%d) to file: \"%s\"\n",VoDev, szYuvName);

	s32Ret = HI_MPI_VO_ReleaseWbcFrame(VoDev, &stFrame);
	if (HI_SUCCESS != s32Ret)
	{
		printf("Dev(%d) HI_MPI_VO_ReleaseWbcFrame errno %#x\n", VoDev, s32Ret);
		return -1;
	}

	/* open file */
	fp = fopen(szYuvName, "wb");

	if (NULL == fp)
	{
		return -1;
	}

	//memopen();

	/* get VO frame  */
	for (i=0; i<u32Cnt; i++)
	{
		s32Ret = HI_MPI_VO_GetWbcFrame(VoDev, &stFrame, s32MilliSec);
		if (HI_SUCCESS != s32Ret)
		{
			printf("get Wbc(%d) frame err\n", VoDev);
			printf("only get %d frame\n", i);
			break;
		}
		/* save VO frame to file */
		yuv_dump(&stFrame.stVFrame, fp);


		s32Ret = HI_MPI_VPSS_SendFrame(9, &stFrame, -1);
		if (HI_SUCCESS != s32Ret)
		{
			printf("HI_MPI_VPSS_SendFrame err\n");
		}


		/* release frame after using */
		s32Ret = HI_MPI_VO_ReleaseWbcFrame(VoDev, &stFrame);
		if (HI_SUCCESS != s32Ret)
		{
			printf("release Wbc(%d) frame err\n", VoDev);
			printf("only get %d frame\n", i);
			break;
		}
	}

	//memclose();

	fclose(fp);

	return 0;
}



/******************************************************************************
 * function : get vo dev data
 ******************************************************************************/
HI_S32 WbcDump2(VO_DEV VoDev)
{
	HI_S32 i, s32Ret;
	VIDEO_FRAME_INFO_S stFrame;
	//VIDEO_FRAME_INFO_S astFrame[256];
	HI_CHAR szYuvName[128];
	HI_CHAR szPixFrm[10];
	FILE *fp;
	HI_S32 s32MilliSec = 1000*2;

	VO_WBC_SOURCE_S stWbcSource;
	stWbcSource.enSourceType = VO_WBC_SOURCE_DEV;
	stWbcSource.u32SourceId = 0;
	HI_MPI_VO_SetWbcSource(0, &stWbcSource );

	VO_WBC_ATTR_S stWbcAttr;
	VO_WBC_MODE_E enWbcMode;
	stWbcAttr.stTargetSize.u32Width = 1920;
	stWbcAttr.stTargetSize.u32Height = 1080;
	stWbcAttr.enPixelFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
	stWbcAttr.u32FrameRate = 25;

	if (HI_SUCCESS != HI_MPI_VO_SetWbcAttr(0,&stWbcAttr))
	{
		printf("Set wbc attr failed!\n");
		return HI_FAILURE;
	}

	enWbcMode = VO_WBC_MODE_NOMAL;
	if (HI_SUCCESS != HI_MPI_VO_SetWbcMode(0, enWbcMode))
	{
		printf("Set wbc mode failed!\n");
		return HI_FAILURE;
	}

	HI_MPI_VO_EnableWbc(0);



	s32Ret = HI_MPI_VO_SetWbcDepth(VoDev, 1);
	if (HI_SUCCESS != s32Ret)
	{
		printf("Dev(%d) HI_MPI_VO_SetWbcDepth errno %#x\n", VoDev, s32Ret);
		return s32Ret;
	}

	/* Get Frame to make file name*/
	s32Ret = HI_MPI_VO_GetWbcFrame(VoDev, &stFrame, s32MilliSec);
	if (HI_SUCCESS != s32Ret)
	{
		printf("HI_MPI_VO(%d)_WbcGetScreenFrame errno %#x\n", VoDev, s32Ret);
		return -1;
	}

	/* make file name */
	strcpy(szPixFrm,
			(PIXEL_FORMAT_YUV_SEMIPLANAR_420 == stFrame.stVFrame.enPixelFormat)?"p420":"p422");
	sprintf(szYuvName, "./Wbc(%d)_%d_%d_%s_%d.yuv",VoDev,
			stFrame.stVFrame.u32Width, stFrame.stVFrame.u32Height,szPixFrm,1);
	printf("Dump YUV frame of Wbc(%d) to file: \"%s\"\n",VoDev, szYuvName);

	s32Ret = HI_MPI_VO_ReleaseWbcFrame(VoDev, &stFrame);
	if (HI_SUCCESS != s32Ret)
	{
		printf("Dev(%d) HI_MPI_VO_ReleaseWbcFrame errno %#x\n", VoDev, s32Ret);
		return -1;
	}

	/* open file */
	//fp = fopen(szYuvName, "wb");

	if (NULL == fp)
	{
		return -1;
	}

	//memopen();

	/* get VO frame  */
	while(1)
	{
		s32Ret = HI_MPI_VO_GetWbcFrame(VoDev, &stFrame, s32MilliSec);
		if (HI_SUCCESS != s32Ret)
		{
			printf("get Wbc(%d) frame err\n", VoDev);
			printf("only get %d frame\n", i);
			break;
		}
		/* save VO frame to file */
		//yuv_dump(&stFrame.stVFrame, fp);


		s32Ret = HI_MPI_VPSS_SendFrame(9, &stFrame, -1);
		if (HI_SUCCESS != s32Ret)
		{
			printf("HI_MPI_VPSS_SendFrame err\n");
		}


		/* release frame after using */
		s32Ret = HI_MPI_VO_ReleaseWbcFrame(VoDev, &stFrame);
		if (HI_SUCCESS != s32Ret)
		{
			printf("release Wbc(%d) frame err\n", VoDev);
			printf("only get %d frame\n", i);
			break;
		}
	}

	//memclose();

	fclose(fp);

	return 0;
}


/******************************************************************************
 * funciton : save H264 stream
 * Input fpH264File:文件描述符号
 * Input pstStream:需要保存的数据
 ******************************************************************************/
HI_S32 VENC_SaveH264(FILE* fpH264File, VENC_STREAM_S *pstStream)
{
	HI_S32 i;
	unsigned char buff[1920*1080];
	unsigned char *p = buff;
	HI_S32 len = 0;
#if 0
	if(pstStream->u32PackCount == 4)
		fprintf(stderr,"IDR frame\n");
#endif
	for (i = 0; i < pstStream->u32PackCount; i++)
	{
		memcpy(p + len,pstStream->pstPack[i].pu8Addr+pstStream->pstPack[i].u32Offset,
			pstStream->pstPack[i].u32Len-pstStream->pstPack[i].u32Offset);
		len += pstStream->pstPack[i].u32Len-pstStream->pstPack[i].u32Offset;
		
	}
	fwrite(buff,len, 1, fpH264File);

	fflush(fpH264File);
	//printf("msgqid:%d\n",gparm.msgqid[0]);

	return HI_SUCCESS;
}

/******************************************************************************
 * funciton : save H264 stream
 ******************************************************************************/
HI_S32 VENC_SaveH2641(FILE* fpH264File, VENC_STREAM_S *pstStream)
{
	HI_S32 i;
	unsigned char buff[1920*1080];
	unsigned char *p = buff;
	for (i = 0; i < pstStream->u32PackCount; i++)
	{
		fwrite(pstStream->pstPack[i].pu8Addr+pstStream->pstPack[i].u32Offset,
				pstStream->pstPack[i].u32Len-pstStream->pstPack[i].u32Offset, 1, fpH264File);

		fflush(fpH264File);
		//printf("pstStream->u32PackCount:%d\n",pstStream->u32PackCount);
	}
	//printf("msgqid:%d\n",gparm.msgqid[0]);

	return HI_SUCCESS;
}


/******************************************************************************
 * funciton : get file postfix according palyload_type.
 * Description:获取文件后缀名称
 ******************************************************************************/
HI_S32 VENC_GetFilePostfix(PAYLOAD_TYPE_E enPayload, char *szFilePostfix)
{
	if (PT_H264 == enPayload)
	{
		strcpy(szFilePostfix, ".h264");
	}
	else if (PT_JPEG == enPayload)
	{
		strcpy(szFilePostfix, ".jpg");
	}
	else if (PT_MJPEG == enPayload)
	{
		strcpy(szFilePostfix, ".mjp");
	}
	else if (PT_MP4VIDEO == enPayload)
	{
		strcpy(szFilePostfix, ".mp4");
	}
	else
	{
		fprintf(stderr,"payload type err!\n");
		return HI_FAILURE;
	}
	return HI_SUCCESS;
}



/******************************************************************************
 * funciton : get stream from each channels and save them
 * Description:获取码流数据，并将数据存储到本地文件中
 ******************************************************************************/
HI_VOID* GetVencStreamProc(HI_VOID *p)
{
	HI_S32 i;
	HI_S32 s32ChnTotal;
	VENC_CHN_ATTR_S stVencChnAttr;
	DEMO_VENC_GETSTREAM_PARA_S *pstPara;
	HI_S32 maxfd = 0;
	struct timeval TimeoutVal;
	fd_set read_fds;
	HI_S32 VencFd[VENC_MAX_CHN_NUM];
	HI_CHAR aszFileName[VENC_MAX_CHN_NUM][64];
	FILE *pFile[VENC_MAX_CHN_NUM];
	char szFilePostfix[10];
	VENC_CHN_STAT_S stStat;
	VENC_STREAM_S stStream;
	HI_S32 s32Ret;
	VENC_CHN VencChn;
	PAYLOAD_TYPE_E enPayLoadType[VENC_MAX_CHN_NUM];
	
	pstPara = (DEMO_VENC_GETSTREAM_PARA_S*)p;
	s32ChnTotal = pstPara->s32Cnt;

	char IDInfo[64] = {0};
	get_pid_ttid(IDInfo);
	/******************************************
	  step 1:  check & prepare save-file & venc-fd
	 ******************************************/
	if (s32ChnTotal >= VENC_MAX_CHN_NUM){
		printf("[%s][Error]:input count invaild line:%d\n", __func__, __LINE__);
		return NULL;
	}
	printf("[%s]:s32ChnTotal=%d line:%d\n", __func__, s32ChnTotal, __LINE__);
	for (i = 0; i < s32ChnTotal; i++)
	{
		/* decide the stream file name, and open file to save stream */
		VencChn = i;
		s32Ret = HI_MPI_VENC_GetChnAttr(VencChn, &stVencChnAttr); /*获取编码通道的编码属性*/
		if(s32Ret != HI_SUCCESS){
			printf("[%s][Error]:HI_MPI_VENC_GetChnAttr chn[%d] failed with %#x! line:%d\n", __func__, VencChn, s32Ret, __LINE__);
			return NULL;
		}
		enPayLoadType[i] = stVencChnAttr.stVeAttr.enType; /*编码类型*/
		s32Ret = VENC_GetFilePostfix(enPayLoadType[i], szFilePostfix); /*格式化文件后缀名称*/
		if(s32Ret != HI_SUCCESS){
			printf("[%s][Error]:DEMO_COMM_VENC_GetFilePostfix [%d] failed with %#x! line:%d\n", __func__, stVencChnAttr.stVeAttr.enType, s32Ret, __LINE__);
			return NULL;
		}
		sprintf(aszFileName[i], "stream_chn%d%s", i, szFilePostfix);
		printf("[%s]:VencChn=%d open file=%s line:%d\n", __func__, VencChn, aszFileName[i], __LINE__);
		pFile[i] = fopen(aszFileName[i], "wb");
		if (!pFile[i]){
			printf("[%s][Error]:open file[%s] failed! line:%d\n", __func__, aszFileName[i], __LINE__);
			return NULL;
		}

		/* Set Venc Fd. */
		VencFd[i] = HI_MPI_VENC_GetFd(i); /*获取编码通道对应的设备文件句柄*/
		if (VencFd[i] < 0){
			printf("[%s][Error]:HI_MPI_VENC_GetFd failed with %#x! line:%d\n", __func__, VencFd[i], __LINE__);
			return NULL;
		}
		if (maxfd <= VencFd[i])
		{
			maxfd = VencFd[i];
		}
	}

	/******************************************
	  step 2:  Start to get streams of each channel.
	 ******************************************/
	while (HI_TRUE == pstPara->bThreadStart)
	{
		FD_ZERO(&read_fds);
		for (i = 0; i < s32ChnTotal; i++){
			FD_SET(VencFd[i], &read_fds);
		}

		TimeoutVal.tv_sec  = 2;
		TimeoutVal.tv_usec = 0;
		s32Ret = select(maxfd + 1, &read_fds, NULL, NULL, &TimeoutVal);
		if (s32Ret < 0){
			printf("[%s][Error]:select failed! line:%d\n", __func__, __LINE__);
			break;  /*发生错误*/
		}
		else if (s32Ret == 0){
			printf("[%s][Error]:get venc stream time out, exit thread line:%d\n", __func__, __LINE__);
			continue; /*超时*/
		}
		else
		{
			for (i = 0; i < s32ChnTotal; i++)
			{
				if (FD_ISSET(VencFd[i], &read_fds))
				{
					/*******************************************************
					  step 2.1 : query how many packs in one-frame stream.
					 *******************************************************/
					memset(&stStream, 0, sizeof(stStream));
					s32Ret = HI_MPI_VENC_Query(i, &stStat); /*查询编码通道状态*/
					if (HI_SUCCESS != s32Ret){
						printf("[%s][Error]:HI_MPI_VENC_Query chn[%d] failed with %#x!\n", __func__, i, s32Ret, __LINE__);
						break;
					}
					/*******************************************************
					  step 2.2 : suggest to check both u32CurPacks and u32LeftStreamFrames at the same time，for example:
					  if(0 == stStat.u32CurPacks || 0 == stStat.u32LeftStreamFrames)
					  {
					  fprintf(stderr,"NOTE: Current  frame is NULL!\n");
					  continue;
					  }
					 *******************************************************/
					if(0 == stStat.u32CurPacks){
						printf("[%s][Error]:NOTE: Current  frame is NULL! line:%d\n", __func__, __LINE__);
						continue; /*编码器没有出数据*/
					}
					/*******************************************************
					  step 2.3 : malloc corresponding number of pack nodes.
					 *******************************************************/
					stStream.pstPack = (VENC_PACK_S*)malloc(sizeof(VENC_PACK_S) * stStat.u32CurPacks);
					if (NULL == stStream.pstPack){
						printf("[%s][Error]:malloc stream pack failed! line:%d\n", __func__, __LINE__);
						break;
					}
					/*******************************************************
					  step 2.4 : call mpi to get one-frame stream
					 *******************************************************/
					stStream.u32PackCount = stStat.u32CurPacks; /*当前帧的码流包个数*/
					s32Ret = HI_MPI_VENC_GetStream(i, &stStream, HI_TRUE); /*获取编码码流*/
					if (HI_SUCCESS != s32Ret){
						free(stStream.pstPack);
						stStream.pstPack = NULL;
						printf("[%s][Error]:HI_MPI_VENC_GetStream failed with %#x! line:%d\n", __func__, s32Ret, __LINE__);
						break;
					}

					/*******************************************************
					  step 2.5 : save frame to file
					 *******************************************************/
					s32Ret = VENC_SaveH264(pFile[i], &stStream);
					if (HI_SUCCESS != s32Ret){
						free(stStream.pstPack);
						stStream.pstPack = NULL;
						printf("[%s][Error]:save stream failed! line:%d\n", __func__, __LINE__);
						break;
					}
					/*******************************************************
					  step 2.6 : release stream
					 *******************************************************/
					s32Ret = HI_MPI_VENC_ReleaseStream(i, &stStream);
					if (HI_SUCCESS != s32Ret){
						free(stStream.pstPack);
						stStream.pstPack = NULL;
						printf("[%s][Error]:release venc stream failed! line:%d\n", __func__, __LINE__);
						break;
					}
					/*******************************************************
					  step 2.7 : free pack nodes
					 *******************************************************/
					free(stStream.pstPack);
					stStream.pstPack = NULL;
				}
			}
		}
	}

	/*******************************************************
	 * step 3 : close save-file
	 *******************************************************/
	for (i = 0; i < s32ChnTotal; i++)
	{
		fclose(pFile[i]);
	}

	return NULL;
}



HI_S32 test1(HI_VOID)
{
	HI_S32 s32VpssGrpCnt = 8;
	VO_DEV VoDev;
	VO_LAYER VoLayer;
	DEMO_VO_MODE_E enVoMode, enPreVoMode;
	HI_S32 i;
	HI_S32 s32Ret = HI_SUCCESS;
	HI_CHAR ch;
	SIZE_S stSize;
	HI_U32 u32WndNum;

	/******************************************
	  step 2: mpp system init. 
	 ******************************************/
	s32Ret = sys_init();
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"system init failed with %d!\n", s32Ret);
		return s32Ret;
	}


	/******************************************
	  step 3: start vi dev & chn
	 ******************************************/
	s32Ret = vi_start(g_enViMode, g_enNorm);
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"start vi failed!\n");
		return s32Ret;
	}

	/******************************************
	  step 4: start vpss and vi bind vpss
	 ******************************************/
	s32Ret = vpss_start(g_enNorm,9);
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"Start Vpss failed!\n");
		return s32Ret;
	}

	s32Ret = vi_to_vpss(g_enViMode);
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"vi_to_vpss failed!\n");
		return s32Ret;
	}

	s32Ret = vo_start();
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"Vo start failed!\n");
		return s32Ret;

	}
	s32Ret = vpss_to_vo(g_VpssChn_VoHD0,VoLayer,9);
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"vpss_to_vo failed!\n");
		return s32Ret;

	}
	while(1)
		sleep(1);



	return s32Ret;
}


HI_S32 test2(HI_VOID)
{
	HI_S32 s32VpssGrpCnt = 8;
	VO_DEV VoDev;
	VO_LAYER VoLayer;
	DEMO_VO_MODE_E enVoMode, enPreVoMode;
	HI_S32 i;
	HI_S32 s32Ret = HI_SUCCESS;
	HI_CHAR ch;
	SIZE_S stSize;
	HI_U32 u32WndNum;

	/******************************************
	  step 2: mpp system init. 
	 ******************************************/
	s32Ret = sys_init();
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"system init failed with %d!\n", s32Ret);
		return s32Ret;
	}


	/******************************************
	  step 3: start vi dev & chn
	 ******************************************/
	s32Ret = vi_start(g_enViMode, g_enNorm);
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"start vi failed!\n");
		return s32Ret;
	}

	s32Ret = vo_start();
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"Vo start failed!\n");
		return s32Ret;

	}
	s32Ret = vi_to_vo(VoLayer);
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"vpss_to_vo failed!\n");
		return s32Ret;

	}
	while(1)
		sleep(1);

	return s32Ret;
}

HI_S32 test3(HI_VOID)
{
	HI_S32 s32VpssGrpCnt = 8;
	VO_LAYER VoLayer = 0;
	DEMO_VO_MODE_E enVoMode, enPreVoMode;
	HI_S32 i = 0;
	HI_S32 s32Ret = HI_SUCCESS;
	HI_CHAR ch = 0;
	SIZE_S stSize = {0};
	HI_U32 u32WndNum;


	/******************************************
	  step 2: mpp system init. 
	 ******************************************/
	s32Ret = sys_init();
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"system init failed with %d!\n", s32Ret);
		return s32Ret;
	}


	/******************************************
	  step 3: start vi dev & chn
	 ******************************************/
	s32Ret = vi_start(g_enViMode, g_enNorm);
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"start vi failed!\n");
		return s32Ret;
	}

	s32Ret = vo_start(); /*VO标清层的的开启*/
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"Vo start failed!\n");
		return s32Ret;

	}
	s32Ret = vi_to_vo(VoLayer);
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"vpss_to_vo failed!\n");
		return s32Ret;

	}

	/******************************************
	  Clip process
	 ******************************************/
	printf("[%s]:press any key to show hd zoom. line:%d\n", __func__, __LINE__);
	getchar();

	//sleep(5);
	//PIP默认绑定设备0,注意切换bind设备前一点要将上一个设备解bind
	HI_MPI_VO_UnBindVideoLayer(g_VoLayerPip, 0); /*将PIP视频层从设备0上面解除绑定*/
	s32Ret = HI_MPI_VO_BindVideoLayer(g_VoLayerPip, g_VoDev); /*将PIP视频层绑定在设备0上*/
	if (HI_SUCCESS != s32Ret)
	{
		printf("[%s][Error]:HI_MPI_VO_BindVideoLayer failed with %#x! line:%d\n", __func__, s32Ret, __LINE__);
	}
	/* PIP视频层的开启和设置 */
	printf("[%s]:Begin set layer=%d *********************************** line:%d\n", __func__, g_VoLayerPip, __LINE__);
	VO_StartVideoLayer(g_VoLayerPip,VO_OUTPUT_1080P60);//VO_OUTPUT_PAL  VO_OUTPUT_1080P60
	if (HI_SUCCESS != s32Ret)
	{
		printf("[%s][Error]:VO_StartVideoLayer failed with %#x! line:%d\n", __func__, s32Ret, __LINE__);
	}

	printf("[%s]:begin start VO PIP layer. line:%d\n", __func__, __LINE__);
	VO_StartChn(g_VoLayerPip, DEMO_VO_MODE_9MUX);
	printf("[%s]:End layer=%d *********************************** line:%d\n", __func__, g_VoLayerPip, __LINE__);


	MPP_CHN_S stSrcChn;
	MPP_CHN_S stDestChn;
	//将vou 的第0 路绑定到viu 的chn 12 上
	stSrcChn.enModId = HI_ID_VIU;
	stSrcChn.s32DevId = 0;
	stSrcChn.s32ChnId = 12;
	stDestChn.enModId = HI_ID_VOU;
	stDestChn.s32DevId = g_VoLayerPip;
	stDestChn.s32ChnId = 0;

	printf("[%s]:VI bind VO:VI=%d-%d VO=%d-%d line:%d\n", __func__, (stSrcChn.s32DevId), (stSrcChn.s32ChnId), (stDestChn.s32DevId), (stDestChn.s32ChnId), __LINE__); 
	s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
	if (s32Ret != HI_SUCCESS)
	{
		fprintf(stderr,"failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}


	printf("[%s]:change to pip line:%d\n", __func__, __LINE__);



	while(1) 
		sleep(1);


	return s32Ret;
}

HI_S32 test4(HI_VOID)
{
	HI_S32 s32VpssGrpCnt = 8;
	VO_DEV VoDev;
	DEMO_VO_MODE_E enVoMode, enPreVoMode;
	HI_S32 i;
	HI_S32 s32Ret = HI_SUCCESS;
	HI_CHAR ch;
	SIZE_S stSize;
	HI_U32 u32WndNum;

	/******************************************
	  step 2: mpp system init. 
	 ******************************************/
	s32Ret = sys_init();
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"system init failed with %d!\n", s32Ret);
		return s32Ret;
	}


	/******************************************
	  step 3: start vi dev & chn
	 ******************************************/
	s32Ret = vi_start(g_enViMode, g_enNorm);
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"start vi failed!\n");
		return s32Ret;
	}

	/******************************************
	  step 4: start vpss and vi bind vpss
	 ******************************************/
	s32Ret = vpss_start(g_enNorm,9);
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"Start Vpss failed!\n");
		return s32Ret;
	}

	s32Ret = vi_to_vpss(g_enViMode);
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"vi_to_vpss failed!\n");
		return s32Ret;
	}

	s32Ret = vo_start();
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"Vo start failed!\n");
		return s32Ret;

	}
	s32Ret = vpss_to_vo(g_VpssChn_VoHD0,g_VoLayer,9);
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"vpss_to_vo failed!\n");
		return s32Ret;

	}

	/******************************************
	  Clip process
	 ******************************************/
	printf("press any key to show hd zoom.\n");
	getchar();

	//PIP默认绑定设备0
	HI_MPI_VO_UnBindVideoLayer(g_VoLayerPip, 0);
	s32Ret = HI_MPI_VO_BindVideoLayer(g_VoLayerPip, g_VoDev);
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"HI_MPI_VO_BindVideoLayer failed with %#x!\n", s32Ret);
	}

	VO_StartVideoLayer(g_VoLayerPip,VO_OUTPUT_1080P60);//VO_OUTPUT_PAL  VO_OUTPUT_1080P60
	if (HI_SUCCESS != s32Ret)
	{
		printf("VO_StartVideoLayer failed with %#x!\n", s32Ret);
	}

	VO_VIDEO_LAYER_ATTR_S stLayerAttr;
	VO_CHN_ATTR_S stChnAttr;
	u32WndNum = 9;
	HI_U32 u32Square = 3;
	HI_U32 u32Width = 0;
	HI_U32 u32Height = 0;
	s32Ret = HI_MPI_VO_GetVideoLayerAttr(g_VoLayer, &stLayerAttr);
	if (s32Ret != HI_SUCCESS)
	{
		fprintf(stderr,"failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}
	u32Width = stLayerAttr.stImageSize.u32Width;
	u32Height = stLayerAttr.stImageSize.u32Height;

	//设置pip视频层上chn0 在画布上的位置--初始位置
	stChnAttr.stRect.s32X       = DEMO_ALIGN_BACK((u32Width/u32Square) * (3%u32Square), 2);
	stChnAttr.stRect.s32Y       = DEMO_ALIGN_BACK((u32Height/u32Square) * (3/u32Square), 2);
	stChnAttr.stRect.u32Width   = DEMO_ALIGN_BACK(u32Width/u32Square/2, 2);
	stChnAttr.stRect.u32Height  = DEMO_ALIGN_BACK(u32Height/u32Square/2, 2);
	stChnAttr.u32Priority       = 0;
	stChnAttr.bDeflicker        = HI_FALSE;
	s32Ret = HI_MPI_VO_SetChnAttr(g_VoLayerPip, 0, &stChnAttr);
	if (s32Ret != HI_SUCCESS)
	{
		fprintf(stderr,"%s(%d):failed with %#x!\n",\
				__FUNCTION__,__LINE__,  s32Ret);
		return HI_FAILURE;
	}
	//应该根据输入的源来使能通道,此时接了第四路--最左边一个SDI口
	s32Ret = HI_MPI_VO_EnableChn(g_VoLayerPip, 0);
	if (s32Ret != HI_SUCCESS)
	{
		fprintf(stderr,"ch%d failed with %#x!\n",i,s32Ret);
		return HI_FAILURE;
	}

	MPP_CHN_S stSrcChn;
	MPP_CHN_S stDestChn;
	VPSS_GRP VpssGrp_Clip = 0;
	VPSS_CHN VpssChn_VoPIP = 3;
	//将vou 的第0 路绑定到vpss 的group0 ch3 上(支持ch0 ch2 ch3)
	stSrcChn.enModId = HI_ID_VPSS;
	stSrcChn.s32DevId = VpssGrp_Clip;
	stSrcChn.s32ChnId = VpssChn_VoPIP;
	stDestChn.enModId = HI_ID_VOU;
	stDestChn.s32DevId = g_VoLayerPip;
	stDestChn.s32ChnId = 0;  //只有通道0 有效
#if 1
	printf("[%s]:vpass bind PIP:VP=%d=%d PIP=%d-%d line:%d\n", __func__, (stSrcChn.s32DevId), (stSrcChn.s32ChnId),\
	(stDestChn.s32DevId), (stDestChn.s32ChnId), __LINE__);
#endif
	s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
	if (s32Ret != HI_SUCCESS)
	{
		fprintf(stderr,"failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}
	printf("change to pip1\n");

	printf("press any key to show hd zoom.\n");
	getchar();

	//改变pip视频层上chn0 在画布上的位置
	POINT_S stDispPos;
	stDispPos.s32X = 500;
	stDispPos.s32Y = 500;
	s32Ret = HI_MPI_VO_SetChnDispPos(g_VoLayerPip,0,&stDispPos);
	if (s32Ret != HI_SUCCESS)
	{
		fprintf(stderr,"failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}

	printf("change to pip2\n");

#if 0
	/*** enable vpss group clip ***/
	VPSS_CROP_INFO_S stVpssCropInfo;
	stVpssCropInfo.bEnable = HI_TRUE;
	stVpssCropInfo.enCropCoordinate = VPSS_CROP_ABS_COOR;
	stVpssCropInfo.stCropRect.s32X = 0;
	stVpssCropInfo.stCropRect.s32Y = 0;
	stVpssCropInfo.stCropRect.u32Height = 100;
	stVpssCropInfo.stCropRect.u32Width = 100;
	s32Ret = HI_MPI_VPSS_SetGrpCrop(VpssGrp_Clip, &stVpssCropInfo);
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"HI_MPI_VPSS_SetGrpCrop failed with %#x!\n", s32Ret);
	}
#endif
	VO_PART_MODE_E enPartMode;
	HI_MPI_VO_GetVideoLayerPartitionMode(g_VoLayer,&enPartMode);
	printf("VoLayer:enPartMode:%d\n",enPartMode);

	HI_MPI_VO_GetVideoLayerPartitionMode(g_VoLayerPip,&enPartMode);
	printf("VoLayerPip:enPartMode:%d\n",enPartMode);

	while(1)
		sleep(1);



	return s32Ret;
}



HI_S32 test5(HI_VOID)
{
	HI_S32 s32VpssGrpCnt = 8;
	VO_DEV VoDev;
	DEMO_VO_MODE_E enVoMode, enPreVoMode;
	HI_S32 i;
	HI_S32 s32Ret = HI_SUCCESS;
	HI_CHAR ch;
	SIZE_S stSize;
	HI_U32 u32WndNum;
	MPP_CHN_S stSrcChn;
	MPP_CHN_S stDestChn;
	/******************************************
	  step 2: mpp system init. 
	 ******************************************/
	s32Ret = sys_init();
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"system init failed with %d!\n", s32Ret);
		return s32Ret;
	}


	/******************************************
	  step 3: start vi dev & chn
	 ******************************************/
	s32Ret = vi_start(g_enViMode, g_enNorm);
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"start vi failed!\n");
		return s32Ret;
	}

	/******************************************
	  step 4: start vpss and vi bind vpss
	 ******************************************/
	s32Ret = vpss_start(g_enNorm,9);
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"Start Vpss failed!\n");
		return s32Ret;
	}

	//获取8路数据
	s32Ret = vi_to_vpss(g_enViMode);
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"vi_to_vpss failed!\n");
		return s32Ret;
	}

	//获取第9路数据
	s32Ret = vdec_start();
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"vi_to_vpss failed!\n");
		return s32Ret;
	}


	s32Ret = vdec_to_vpss(8);
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"vi_to_vpss failed!\n");
		return s32Ret;
	}

	s32Ret = vo_start();
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"Vo start failed!\n");
		return s32Ret;

	}
	s32Ret = vpss_to_vo(g_VpssChn_VoHD0,g_VoLayer,9);
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"vpss_to_vo failed!\n");
		return s32Ret;

	}

	/******************************************
	  Clip process
	 ******************************************/
	printf("[%s]:press any key to show hd zoom. line:%d\n", __func__, __LINE__);
	getchar();

	//PIP默认绑定设备0
	HI_MPI_VO_UnBindVideoLayer(g_VoLayerPip, 0);
	s32Ret = HI_MPI_VO_BindVideoLayer(g_VoLayerPip, g_VoDev);
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"HI_MPI_VO_BindVideoLayer failed with %#x!\n", s32Ret);
	}

	VO_StartVideoLayer(g_VoLayerPip,VO_OUTPUT_1080P60);//VO_OUTPUT_PAL  VO_OUTPUT_1080P60
	if (HI_SUCCESS != s32Ret)
	{
		printf("VO_StartVideoLayer failed with %#x!\n", s32Ret);
	}

	VO_VIDEO_LAYER_ATTR_S stLayerAttr;
	VO_CHN_ATTR_S stChnAttr;
	u32WndNum = 9;
	HI_U32 u32Square = 3;
	HI_U32 u32Width = 0;
	HI_U32 u32Height = 0;
	s32Ret = HI_MPI_VO_GetVideoLayerAttr(g_VoLayer, &stLayerAttr); /*获取视频层属性*/
	if (s32Ret != HI_SUCCESS)
	{
		fprintf(stderr,"failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}
	u32Width = stLayerAttr.stImageSize.u32Width;
	u32Height = stLayerAttr.stImageSize.u32Height;

	//设置pip视频层上chn0 在画布上的位置--初始位置
	stChnAttr.stRect.s32X       = DEMO_ALIGN_BACK((u32Width/u32Square) * (3%u32Square), 2);
	stChnAttr.stRect.s32Y       = DEMO_ALIGN_BACK((u32Height/u32Square) * (3/u32Square), 2);
	stChnAttr.stRect.u32Width   = DEMO_ALIGN_BACK(u32Width/u32Square/2, 2);
	stChnAttr.stRect.u32Height  = DEMO_ALIGN_BACK(u32Height/u32Square/2, 2);
	stChnAttr.u32Priority       = 0;
	stChnAttr.bDeflicker        = HI_FALSE;
#if 1
	printf("[%s]:PIP attr:coord=%d-%d line:%d\n", __func__, (stChnAttr.stRect.s32X), (stChnAttr.stRect.s32Y),\
		(stChnAttr.stRect.u32Width), (stChnAttr.stRect.u32Height), __LINE__);
#endif
	s32Ret = HI_MPI_VO_SetChnAttr(g_VoLayerPip, 0, &stChnAttr);
	if (s32Ret != HI_SUCCESS)
	{
		fprintf(stderr,"%s(%d):failed with %#x!\n",\
				__FUNCTION__,__LINE__,  s32Ret);
		return HI_FAILURE;
	}
	//应该根据输入的源来使能通道,此时接了第四路--最左边一个SDI口
	s32Ret = HI_MPI_VO_EnableChn(g_VoLayerPip, 0);
	if (s32Ret != HI_SUCCESS)
	{
		fprintf(stderr,"ch%d failed with %#x!\n",i,s32Ret);
		return HI_FAILURE;
	}

	//vpss to vo(volaypip)
	VPSS_GRP VpssGrp_Clip = 8;
	VPSS_CHN VpssChn_VoPIP = 0;
	//将vou 的第0 路绑定到vpss 的group8 ch3 上(支持ch0 ch2 ch3)
	stSrcChn.enModId = HI_ID_VPSS;
	stSrcChn.s32DevId = VpssGrp_Clip;
	//stSrcChn.s32ChnId = VpssChn_VoPIP;
	stSrcChn.s32ChnId = 2;
	stDestChn.enModId = HI_ID_VOU;
	stDestChn.s32DevId = g_VoLayerPip;
	stDestChn.s32ChnId = 0;  //只有通道0 有效
#if 1
	printf("[%s]:VPass bind PIP:VP=%d-%d PIP=%d-%d  line:%d\n", __func__,(stSrcChn.s32DevId), (stSrcChn.s32ChnId), \
		(stDestChn.s32DevId), (stDestChn.s32ChnId), __LINE__);
#endif
	s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
	if (s32Ret != HI_SUCCESS)
	{
		fprintf(stderr,"failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}
	printf("change to pip1\n");

	printf("press any key to show hd zoom.\n");
	getchar();

	//改变pip视频层上chn0 在画布上的位置
	POINT_S stDispPos;
	stDispPos.s32X = 500;
	stDispPos.s32Y = 500;
	s32Ret = HI_MPI_VO_SetChnDispPos(g_VoLayerPip,0,&stDispPos);
	if (s32Ret != HI_SUCCESS)
	{
		fprintf(stderr,"failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}

	printf("change to pip2\n");

#if 0
	/*** enable vpss group clip ***/
	VPSS_CROP_INFO_S stVpssCropInfo;
	stVpssCropInfo.bEnable = HI_TRUE;
	stVpssCropInfo.enCropCoordinate = VPSS_CROP_ABS_COOR;
	stVpssCropInfo.stCropRect.s32X = 0;
	stVpssCropInfo.stCropRect.s32Y = 0;
	stVpssCropInfo.stCropRect.u32Height = 100;
	stVpssCropInfo.stCropRect.u32Width = 100;
	s32Ret = HI_MPI_VPSS_SetGrpCrop(VpssGrp_Clip, &stVpssCropInfo);
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"HI_MPI_VPSS_SetGrpCrop failed with %#x!\n", s32Ret);
	}
#endif
	VO_PART_MODE_E enPartMode;
	HI_MPI_VO_GetVideoLayerPartitionMode(g_VoLayer,&enPartMode);
	printf("VoLayer:enPartMode:%d\n",enPartMode);

	HI_MPI_VO_GetVideoLayerPartitionMode(g_VoLayerPip,&enPartMode);
	printf("VoLayerPip:enPartMode:%d\n",enPartMode);



	WbcDump(0,5);

	printf("WbcDump success \n");

	while(1)
		sleep(1);




	return s32Ret;
}




HI_S32 test6(HI_VOID)
{
	HI_S32 s32VpssGrpCnt = 9;
	VO_DEV VoDev;
	DEMO_VO_MODE_E enVoMode, enPreVoMode;
	HI_S32 i;
	HI_S32 s32Ret = HI_SUCCESS;
	HI_CHAR ch;
	SIZE_S stSize;
	HI_U32 u32WndNum;
	MPP_CHN_S stSrcChn;
	MPP_CHN_S stDestChn;
	/******************************************
	  step 2: mpp system init. 
	 ******************************************/
	s32Ret = sys_init();
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"system init failed with %d!\n", s32Ret);
		return s32Ret;
	}


	/******************************************
	  step 3: start vi dev & chn
	 ******************************************/
	s32Ret = vi_start(g_enViMode, g_enNorm);
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"start vi failed!\n");
		return s32Ret;
	}

	/******************************************
	  step 4: start vpss and vi bind vpss
	 ******************************************/
	s32Ret = vpss_start(g_enNorm,9);
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"Start Vpss failed!\n");
		return s32Ret;
	}

	//获取8路数据
	s32Ret = vi_to_vpss(g_enViMode);
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"vi_to_vpss failed!\n");
		return s32Ret;
	}

#if 1
	//获取第9路数据
	s32Ret = vdec_start();
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"vdec_start failed!\n");
		return s32Ret;
	}


	s32Ret = vdec_to_vpss(8);
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"vdec_to_vpss failed!\n");
		return s32Ret;
	}
#endif

	s32Ret = vo_start();
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"Vo start failed!\n");
		return s32Ret;

	}
	s32Ret = vpss_to_vo(3,g_VoLayer,9);
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"vpss_to_vo failed!\n");
		return s32Ret;

	}

#if 1
	printf("[%s]:press any key to enable enc. line:%d\n", __func__, __LINE__);
	getchar();
	VPSS_CHN_MODE_S stVpssChnMode;
	VPSS_GRP VpssGrp;
	//VPSS_CHN_MODE_USER 模式下chn绑定输出,AUTO 模式下chn 才会有输出
	// VPSS_CHN_MODE_USER chn0 -enc
#if 1
	for (i=0; i<s32VpssGrpCnt ; i++)
	{
		VpssGrp = i;
		s32Ret = HI_MPI_VPSS_GetChnMode(VpssGrp, 0, &stVpssChnMode);
		if (HI_SUCCESS != s32Ret)
		{
			printf("get Vpss chn mode failed!\n");
		}
		memset(&stVpssChnMode,0,sizeof(VPSS_CHN_MODE_S));
		//high
		stVpssChnMode.enChnMode = VPSS_CHN_MODE_USER;
		stVpssChnMode.u32Width  = 1920;
		stVpssChnMode.u32Height = 1080;
		stVpssChnMode.stFrameRate.s32DstFrmRate = -1;
		stVpssChnMode.stFrameRate.s32SrcFrmRate = -1;
		stVpssChnMode.enPixelFormat  = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
		stVpssChnMode.enCompressMode = COMPRESS_MODE_NONE;
		s32Ret = HI_MPI_VPSS_SetChnMode(VpssGrp, 0, &stVpssChnMode);
		if (HI_SUCCESS != s32Ret)
		{
			printf("Set Vpss chn mode failed!\n");
			return HI_FAILURE;
		}

	}

#endif	
	for (i=0; i<s32VpssGrpCnt ; i++)
	{
		VpssGrp = i;
		s32Ret = HI_MPI_VPSS_GetChnMode(VpssGrp, 0, &stVpssChnMode);
		if (HI_SUCCESS != s32Ret)
		{
			printf("get Vpss chn mode failed!\n");
		}
		printf("stVpssChnMode:%d %d %d %d\n",stVpssChnMode.enChnMode
										,stVpssChnMode.enCompressMode
										,stVpssChnMode.u32Width
										,stVpssChnMode.u32Height
										);
	}

	
	s32Ret = venc_start(0,g_enNorm,9);
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"venc_start failed!\n");
		return s32Ret;
	}

	s32Ret = vpss_to_venc(0,9);
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"vpss_to_venc failed!\n");
		return s32Ret;
	}

	s32Ret = venc_start(1,g_enNorm,9);
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"venc_start failed!\n");
		return s32Ret;
	}

	
	s32Ret = vpss_to_venc(1,9);
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"vpss_to_venc failed!\n");
		return s32Ret;
	}

	//venc_to_file
	gs_stPara.bThreadStart = HI_TRUE;
	gs_stPara.s32Cnt = 9*2;
	pthread_t VencPid;
	pthread_create(&VencPid, 0, GetVencStreamProc, (HI_VOID*)&gs_stPara);

	
#endif


#if 0
	/******************************************
	  Clip process
	 ******************************************/
	printf("press any key to show pip\n");
	getchar();

	//PIP默认绑定设备0
	HI_MPI_VO_UnBindVideoLayer(g_VoLayerPip, 0);
	s32Ret = HI_MPI_VO_BindVideoLayer(g_VoLayerPip, g_VoDev);
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"HI_MPI_VO_BindVideoLayer failed with %#x!\n", s32Ret);
		printf("HI_MPI_VO_BindVideoLayer failed with %#x!\n", s32Ret);
	}

	VO_StartVideoLayer(g_VoLayerPip,VO_OUTPUT_1080P60);//VO_OUTPUT_PAL  VO_OUTPUT_1080P60
	if (HI_SUCCESS != s32Ret)
	{
		printf("VO_StartVideoLayer failed with %#x!\n", s32Ret);
	}

	VO_VIDEO_LAYER_ATTR_S stLayerAttr;
	VO_CHN_ATTR_S stChnAttr;
	u32WndNum = 9;
	HI_U32 u32Square = 3;
	HI_U32 u32Width = 0;
	HI_U32 u32Height = 0;
	s32Ret = HI_MPI_VO_GetVideoLayerAttr(g_VoLayer, &stLayerAttr);
	if (s32Ret != HI_SUCCESS)
	{
		fprintf(stderr,"failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}
	u32Width = stLayerAttr.stImageSize.u32Width;
	u32Height = stLayerAttr.stImageSize.u32Height;

	//设置pip视频层上chn0 在画布上的位置--初始位置
	stChnAttr.stRect.s32X       = DEMO_ALIGN_BACK((u32Width/u32Square) * (3%u32Square), 2);
	stChnAttr.stRect.s32Y       = DEMO_ALIGN_BACK((u32Height/u32Square) * (3/u32Square), 2);
	stChnAttr.stRect.u32Width   = DEMO_ALIGN_BACK(u32Width/u32Square/2, 2);
	stChnAttr.stRect.u32Height  = DEMO_ALIGN_BACK(u32Height/u32Square/2, 2);
	stChnAttr.u32Priority       = 0;
	stChnAttr.bDeflicker        = HI_FALSE;
	s32Ret = HI_MPI_VO_SetChnAttr(g_VoLayerPip, 0, &stChnAttr);
	if (s32Ret != HI_SUCCESS)
	{
		fprintf(stderr,"%s(%d):failed with %#x!\n",\
				__FUNCTION__,__LINE__,  s32Ret);
		return HI_FAILURE;
	}
	//应该根据输入的源来使能通道,此时接了第四路--最左边一个SDI口
	s32Ret = HI_MPI_VO_EnableChn(g_VoLayerPip, 0);
	if (s32Ret != HI_SUCCESS)
	{
		fprintf(stderr,"ch%d failed with %#x!\n",i,s32Ret);
		return HI_FAILURE;
	}

	//vpss to vo(volaypip)
	VPSS_GRP VpssGrp_Clip = 8;
	VPSS_CHN VpssChn_VoPIP = 2;
	//将vou 的第0 路绑定到vpss 的group8 ch3 上(支持ch0 ch2 ch3)
	stSrcChn.enModId = HI_ID_VPSS;
	stSrcChn.s32DevId = VpssGrp_Clip;
	stSrcChn.s32ChnId = VpssChn_VoPIP;
	stDestChn.enModId = HI_ID_VOU;
	stDestChn.s32DevId = g_VoLayerPip;
	stDestChn.s32ChnId = 0;  //只有通道0 有效

	s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
	if (s32Ret != HI_SUCCESS)
	{
		fprintf(stderr,"failed with %#x!\n", s32Ret);
		printf("failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}

	printf("press any key to change pip.\n");
	getchar();

	//改变pip视频层上chn0 在画布上的位置
	POINT_S stDispPos;
	stDispPos.s32X = 500;
	stDispPos.s32Y = 500;
	s32Ret = HI_MPI_VO_SetChnDispPos(g_VoLayerPip,0,&stDispPos);
	if (s32Ret != HI_SUCCESS)
	{
		fprintf(stderr,"failed with %#x!\n", s32Ret);
		printf("failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}
#endif

#if 0
	/*** enable vpss group clip ***/
	VPSS_CROP_INFO_S stVpssCropInfo;
	stVpssCropInfo.bEnable = HI_TRUE;
	stVpssCropInfo.enCropCoordinate = VPSS_CROP_ABS_COOR;
	stVpssCropInfo.stCropRect.s32X = 0;
	stVpssCropInfo.stCropRect.s32Y = 0;
	stVpssCropInfo.stCropRect.u32Height = 100;
	stVpssCropInfo.stCropRect.u32Width = 100;
	s32Ret = HI_MPI_VPSS_SetGrpCrop(VpssGrp_Clip, &stVpssCropInfo);
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"HI_MPI_VPSS_SetGrpCrop failed with %#x!\n", s32Ret);
	}
#endif
	VO_PART_MODE_E enPartMode;
	HI_MPI_VO_GetVideoLayerPartitionMode(g_VoLayer,&enPartMode);
	printf("VoLayer:enPartMode:%d\n",enPartMode);

	HI_MPI_VO_GetVideoLayerPartitionMode(g_VoLayerPip,&enPartMode);
	printf("VoLayerPip:enPartMode:%d\n",enPartMode);


	printf("press any key to enable wbc.\n");
	getchar();
	WbcDump(0,5);

	printf("WbcDump success \n");



	while(1)
		sleep(1);




	return s32Ret;
}





HI_S32 test7(HI_VOID)
{
	HI_S32 s32VpssGrpCnt = 10;
	VO_DEV VoDev;
	DEMO_VO_MODE_E enVoMode, enPreVoMode;
	HI_S32 i;
	HI_S32 s32Ret = HI_SUCCESS;
	HI_CHAR ch;
	SIZE_S stSize;
	HI_U32 u32WndNum;
	MPP_CHN_S stSrcChn;
	MPP_CHN_S stDestChn;
	/******************************************
	  step 2: mpp system init. 
	 ******************************************/
	s32Ret = sys_init();
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"system init failed with %d!\n", s32Ret);
		return s32Ret;
	}


	/******************************************
	  step 3: start vi dev & chn
	 ******************************************/
	s32Ret = vi_start(g_enViMode, g_enNorm);
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"start vi failed!\n");
		return s32Ret;
	}

	/******************************************
	  step 4: start vpss and vi bind vpss
	 ******************************************/
	s32Ret = vpss_start(g_enNorm,10);
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"Start Vpss failed!\n");
		return s32Ret;
	}

	//获取8路数据
	s32Ret = vi_to_vpss(g_enViMode);
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"vi_to_vpss failed!\n");
		return s32Ret;
	}

#if 1
	//获取第9路数据
	s32Ret = vdec_start();
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"vdec_start failed!\n");
		return s32Ret;
	}


	s32Ret = vdec_to_vpss(8);
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"vdec_to_vpss failed!\n");
		return s32Ret;
	}
#endif

	s32Ret = vo_start();
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"Vo start failed!\n");
		return s32Ret;

	}
	s32Ret = vpss_to_vo(3,g_VoLayer,9);
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"vpss_to_vo failed!\n");
		return s32Ret;

	}

#if 1
	printf("press any key to enable enc.\n");
	getchar();
	VPSS_CHN_MODE_S stVpssChnMode;
	VPSS_GRP VpssGrp;
	//VPSS_CHN_MODE_USER 模式下chn绑定输出,AUTO 模式下chn 才会有输出
	// VPSS_CHN_MODE_USER chn0 -enc
#if 1
	for (i=0; i<s32VpssGrpCnt ; i++)
	{
		VpssGrp = i;
		s32Ret = HI_MPI_VPSS_GetChnMode(VpssGrp, 0, &stVpssChnMode);
		if (HI_SUCCESS != s32Ret)
		{
			printf("get Vpss chn mode failed!\n");
		}
		memset(&stVpssChnMode,0,sizeof(VPSS_CHN_MODE_S));
		//high
		stVpssChnMode.enChnMode = VPSS_CHN_MODE_USER;
		stVpssChnMode.u32Width  = 1920;
		stVpssChnMode.u32Height = 1080;
		stVpssChnMode.stFrameRate.s32DstFrmRate = -1;
		stVpssChnMode.stFrameRate.s32SrcFrmRate = -1;
		stVpssChnMode.enPixelFormat  = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
		stVpssChnMode.enCompressMode = COMPRESS_MODE_NONE;
		s32Ret = HI_MPI_VPSS_SetChnMode(VpssGrp, 0, &stVpssChnMode);
		if (HI_SUCCESS != s32Ret)
		{
			printf("Set Vpss chn mode failed!\n");
			return HI_FAILURE;
		}

	}

#endif	
	for (i=0; i<s32VpssGrpCnt ; i++)
	{
		VpssGrp = i;
		s32Ret = HI_MPI_VPSS_GetChnMode(VpssGrp, 0, &stVpssChnMode);
		if (HI_SUCCESS != s32Ret)
		{
			printf("get Vpss chn mode failed!\n");
		}
		printf("stVpssChnMode:%d %d %d %d\n",stVpssChnMode.enChnMode
										,stVpssChnMode.enCompressMode
										,stVpssChnMode.u32Width
										,stVpssChnMode.u32Height
										);
	}

	
	s32Ret = venc_start(0,g_enNorm,10);
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"venc_start failed!\n");
		return s32Ret;
	}

	s32Ret = vpss_to_venc(0,10);
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"vpss_to_venc failed!\n");
		return s32Ret;
	}

	s32Ret = venc_start(1,g_enNorm,10);
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"venc_start failed!\n");
		return s32Ret;
	}

	
	s32Ret = vpss_to_venc(1,10);
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"vpss_to_venc failed!\n");
		return s32Ret;
	}

	//venc_to_file
	gs_stPara.bThreadStart = HI_TRUE;
	gs_stPara.s32Cnt = 10*2;
	pthread_t VencPid;
	pthread_create(&VencPid, 0, GetVencStreamProc, (HI_VOID*)&gs_stPara);

	
#endif



	/******************************************
	  Clip process
	 ******************************************/
	printf("press any key to show pip\n");
	getchar();

	//PIP默认绑定设备0
	HI_MPI_VO_UnBindVideoLayer(g_VoLayerPip, 0);
	s32Ret = HI_MPI_VO_BindVideoLayer(g_VoLayerPip, g_VoDev);
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"HI_MPI_VO_BindVideoLayer failed with %#x!\n", s32Ret);
		printf("HI_MPI_VO_BindVideoLayer failed with %#x!\n", s32Ret);
	}

	VO_StartVideoLayer(g_VoLayerPip,VO_OUTPUT_1080P60);//VO_OUTPUT_PAL  VO_OUTPUT_1080P60
	if (HI_SUCCESS != s32Ret)
	{
		printf("VO_StartVideoLayer failed with %#x!\n", s32Ret);
	}

	VO_VIDEO_LAYER_ATTR_S stLayerAttr;
	VO_CHN_ATTR_S stChnAttr;
	u32WndNum = 9;
	HI_U32 u32Square = 3;
	HI_U32 u32Width = 0;
	HI_U32 u32Height = 0;
	s32Ret = HI_MPI_VO_GetVideoLayerAttr(g_VoLayer, &stLayerAttr);
	if (s32Ret != HI_SUCCESS)
	{
		fprintf(stderr,"failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}
	u32Width = stLayerAttr.stImageSize.u32Width;
	u32Height = stLayerAttr.stImageSize.u32Height;

	//设置pip视频层上chn0 在画布上的位置--初始位置
	stChnAttr.stRect.s32X       = DEMO_ALIGN_BACK((u32Width/u32Square) * (3%u32Square), 2);
	stChnAttr.stRect.s32Y       = DEMO_ALIGN_BACK((u32Height/u32Square) * (3/u32Square), 2);
	stChnAttr.stRect.u32Width   = DEMO_ALIGN_BACK(u32Width/u32Square/2, 2);
	stChnAttr.stRect.u32Height  = DEMO_ALIGN_BACK(u32Height/u32Square/2, 2);
	stChnAttr.u32Priority       = 0;
	stChnAttr.bDeflicker        = HI_FALSE;
	s32Ret = HI_MPI_VO_SetChnAttr(g_VoLayerPip, 0, &stChnAttr);
	if (s32Ret != HI_SUCCESS)
	{
		fprintf(stderr,"%s(%d):failed with %#x!\n",\
				__FUNCTION__,__LINE__,  s32Ret);
		return HI_FAILURE;
	}
	//应该根据输入的源来使能通道,此时接了第四路--最左边一个SDI口
	s32Ret = HI_MPI_VO_EnableChn(g_VoLayerPip, 0);
	if (s32Ret != HI_SUCCESS)
	{
		fprintf(stderr,"ch%d failed with %#x!\n",i,s32Ret);
		return HI_FAILURE;
	}

	//vpss to vo(volaypip)
	VPSS_GRP VpssGrp_Clip = 8;
	VPSS_CHN VpssChn_VoPIP = 2;
	//将vou 的第0 路绑定到vpss 的group8 ch3 上(支持ch0 ch2 ch3)
	stSrcChn.enModId = HI_ID_VPSS;
	stSrcChn.s32DevId = VpssGrp_Clip;
	stSrcChn.s32ChnId = VpssChn_VoPIP;
	stDestChn.enModId = HI_ID_VOU;
	stDestChn.s32DevId = g_VoLayerPip;
	stDestChn.s32ChnId = 0;  //只有通道0 有效

	s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
	if (s32Ret != HI_SUCCESS)
	{
		fprintf(stderr,"failed with %#x!\n", s32Ret);
		printf("failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}

	printf("press any key to change pip.\n");
	getchar();

	//改变pip视频层上chn0 在画布上的位置
	POINT_S stDispPos;
	stDispPos.s32X = 500;
	stDispPos.s32Y = 500;
	s32Ret = HI_MPI_VO_SetChnDispPos(g_VoLayerPip,0,&stDispPos);
	if (s32Ret != HI_SUCCESS)
	{
		fprintf(stderr,"failed with %#x!\n", s32Ret);
		printf("failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}


#if 0
	/*** enable vpss group clip ***/
	VPSS_CROP_INFO_S stVpssCropInfo;
	stVpssCropInfo.bEnable = HI_TRUE;
	stVpssCropInfo.enCropCoordinate = VPSS_CROP_ABS_COOR;
	stVpssCropInfo.stCropRect.s32X = 0;
	stVpssCropInfo.stCropRect.s32Y = 0;
	stVpssCropInfo.stCropRect.u32Height = 100;
	stVpssCropInfo.stCropRect.u32Width = 100;
	s32Ret = HI_MPI_VPSS_SetGrpCrop(VpssGrp_Clip, &stVpssCropInfo);
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"HI_MPI_VPSS_SetGrpCrop failed with %#x!\n", s32Ret);
	}
#endif
	VO_PART_MODE_E enPartMode;
	HI_MPI_VO_GetVideoLayerPartitionMode(g_VoLayer,&enPartMode);
	printf("VoLayer:enPartMode:%d\n",enPartMode);

	HI_MPI_VO_GetVideoLayerPartitionMode(g_VoLayerPip,&enPartMode);
	printf("VoLayerPip:enPartMode:%d\n",enPartMode);


	printf("press any key to enable wbc.\n");
	getchar();
	WbcDump1(0,5);

	printf("WbcDump success \n");



	while(1)
		sleep(1);

	return s32Ret;
}






HI_S32 test8(HI_VOID)
{
	HI_S32 s32VpssGrpCnt = 10;
	VO_DEV VoDev;
	DEMO_VO_MODE_E enVoMode, enPreVoMode;
	HI_S32 i;
	HI_S32 s32Ret = HI_SUCCESS;
	HI_CHAR ch;
	SIZE_S stSize;
	HI_U32 u32WndNum;
	MPP_CHN_S stSrcChn;
	MPP_CHN_S stDestChn;

	printf("[%s]:Begin ***************************** line:%d\n", __func__, __LINE__);
	/******************************************
	  step 2: mpp system init. 
	 ******************************************/
	s32Ret = sys_init();
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"system init failed with %d!\n", s32Ret);
		return s32Ret;
	}


	/******************************************
	  step 3: start vi dev & chn
	 ******************************************/
	s32Ret = vi_start(g_enViMode, g_enNorm);
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"start vi failed!\n");
		return s32Ret;
	}

	/******************************************
	  step 4: start vpss and vi bind vpss
	 ******************************************/
	s32Ret = vpss_start(g_enNorm,10);
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"Start Vpss failed!\n");
		return s32Ret;
	}

	//获取8路数据
	s32Ret = vi_to_vpss(g_enViMode);
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"vi_to_vpss failed!\n");
		return s32Ret;
	}

#if 1
	//获取第9路数据
	s32Ret = vdec_start();
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"vdec_start failed!\n");
		return s32Ret;
	}


	s32Ret = vdec_to_vpss(8);
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"vdec_to_vpss failed!\n");
		return s32Ret;
	}
#endif

	s32Ret = vo_start();
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"Vo start failed!\n");
		return s32Ret;

	}
	s32Ret = vpss_to_vo(3,g_VoLayer,9);
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"vpss_to_vo failed!\n");
		return s32Ret;

	}

#if 1
	printf("[%s]:press any key to enable enc. s32VpssGrpCnt=%d line:%d\n", __func__, s32VpssGrpCnt, __LINE__);
	getchar();
	VPSS_CHN_MODE_S stVpssChnMode;
	VPSS_GRP VpssGrp;
	//VPSS_CHN_MODE_USER 模式下chn绑定输出,AUTO 模式下chn 才会有输出
	// VPSS_CHN_MODE_USER chn0 -enc
#if 1
	for (i=0; i<s32VpssGrpCnt ; i++)
	{
		VpssGrp = i;
		s32Ret = HI_MPI_VPSS_GetChnMode(VpssGrp, 0, &stVpssChnMode);
		if (HI_SUCCESS != s32Ret)
		{
			printf("get Vpss chn mode failed!\n");
		}
		memset(&stVpssChnMode,0,sizeof(VPSS_CHN_MODE_S));
		//high
		stVpssChnMode.enChnMode = VPSS_CHN_MODE_USER;
		stVpssChnMode.u32Width  = 1920;
		stVpssChnMode.u32Height = 1080;
		stVpssChnMode.stFrameRate.s32DstFrmRate = -1;
		stVpssChnMode.stFrameRate.s32SrcFrmRate = -1;
		stVpssChnMode.enPixelFormat  = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
		stVpssChnMode.enCompressMode = COMPRESS_MODE_NONE;
		s32Ret = HI_MPI_VPSS_SetChnMode(VpssGrp, 0, &stVpssChnMode);
		if (HI_SUCCESS != s32Ret)
		{
			printf("Set Vpss chn mode failed!\n");
			return HI_FAILURE;
		}

	}

#endif	
	for (i=0; i<s32VpssGrpCnt ; i++)
	{
		VpssGrp = i;
		s32Ret = HI_MPI_VPSS_GetChnMode(VpssGrp, 0, &stVpssChnMode);
		if (HI_SUCCESS != s32Ret)
		{
			printf("get Vpss chn mode failed!\n");
		}
		printf("stVpssChnMode:%d %d %d %d\n",stVpssChnMode.enChnMode
										,stVpssChnMode.enCompressMode
										,stVpssChnMode.u32Width
										,stVpssChnMode.u32Height
										);
	}

	
	s32Ret = venc_start(0,g_enNorm,10);
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"venc_start failed!\n");
		return s32Ret;
	}

	s32Ret = vpss_to_venc(0,10);
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"vpss_to_venc failed!\n");
		return s32Ret;
	}

	s32Ret = venc_start(1,g_enNorm,10);
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"venc_start failed!\n");
		return s32Ret;
	}

	
	s32Ret = vpss_to_venc(1,10);
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"vpss_to_venc failed!\n");
		return s32Ret;
	}

	/*创建一个线程保存编好码的视频数据*/
	gs_stPara.bThreadStart = HI_TRUE;
	gs_stPara.s32Cnt = 10*2;
	pthread_t VencPid;
	pthread_create(&VencPid, 0, GetVencStreamProc, (HI_VOID*)&gs_stPara); /*获取码流数据并存储到本地文件*/

	
#endif

#if 0
	/******************************************
	  Clip process
	 ******************************************/
	printf("[%s]:press any key to show pip line:%d\n", __func__, __LINE__);
	getchar();

	//PIP默认绑定设备0
	HI_MPI_VO_UnBindVideoLayer(g_VoLayerPip, 0);
	s32Ret = HI_MPI_VO_BindVideoLayer(g_VoLayerPip, g_VoDev);
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"HI_MPI_VO_BindVideoLayer failed with %#x!\n", s32Ret);
		printf("HI_MPI_VO_BindVideoLayer failed with %#x!\n", s32Ret);
	}

	VO_StartVideoLayer(g_VoLayerPip,VO_OUTPUT_1080P60);//VO_OUTPUT_PAL  VO_OUTPUT_1080P60
	if (HI_SUCCESS != s32Ret)
	{
		printf("VO_StartVideoLayer failed with %#x!\n", s32Ret);
	}

	VO_VIDEO_LAYER_ATTR_S stLayerAttr;
	VO_CHN_ATTR_S stChnAttr;
	u32WndNum = 9;
	HI_U32 u32Square = 3;
	HI_U32 u32Width = 0;
	HI_U32 u32Height = 0;
	s32Ret = HI_MPI_VO_GetVideoLayerAttr(g_VoLayer, &stLayerAttr);
	if (s32Ret != HI_SUCCESS)
	{
		fprintf(stderr,"failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}
	u32Width = stLayerAttr.stImageSize.u32Width;
	u32Height = stLayerAttr.stImageSize.u32Height;

	//设置pip视频层上chn0 在画布上的位置--初始位置
	stChnAttr.stRect.s32X       = DEMO_ALIGN_BACK((u32Width/u32Square) * (3%u32Square), 2);
	stChnAttr.stRect.s32Y       = DEMO_ALIGN_BACK((u32Height/u32Square) * (3/u32Square), 2);
	stChnAttr.stRect.u32Width   = DEMO_ALIGN_BACK(u32Width/u32Square/2, 2);
	stChnAttr.stRect.u32Height  = DEMO_ALIGN_BACK(u32Height/u32Square/2, 2);
	stChnAttr.u32Priority       = 0;
	stChnAttr.bDeflicker        = HI_FALSE;
	s32Ret = HI_MPI_VO_SetChnAttr(g_VoLayerPip, 0, &stChnAttr);
	if (s32Ret != HI_SUCCESS)
	{
		fprintf(stderr,"%s(%d):failed with %#x!\n",\
				__FUNCTION__,__LINE__,  s32Ret);
		return HI_FAILURE;
	}
	//应该根据输入的源来使能通道,此时接了第四路--最左边一个SDI口
	s32Ret = HI_MPI_VO_EnableChn(g_VoLayerPip, 0);
	if (s32Ret != HI_SUCCESS)
	{
		fprintf(stderr,"ch%d failed with %#x!\n",i,s32Ret);
		return HI_FAILURE;
	}

	//vpss to vo(volaypip)
	VPSS_GRP VpssGrp_Clip = 8;
	VPSS_CHN VpssChn_VoPIP = 2;
	//将vou 的第0 路绑定到vpss 的group8 ch3 上(支持ch0 ch2 ch3)
	stSrcChn.enModId = HI_ID_VPSS;
	stSrcChn.s32DevId = VpssGrp_Clip;
	stSrcChn.s32ChnId = VpssChn_VoPIP;
	stDestChn.enModId = HI_ID_VOU;
	stDestChn.s32DevId = g_VoLayerPip;
	stDestChn.s32ChnId = 0;  //只有通道0 有效

	s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
	if (s32Ret != HI_SUCCESS)
	{
		fprintf(stderr,"failed with %#x!\n", s32Ret);
		printf("failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}

	printf("press any key to change pip.\n");
	getchar();

	//改变pip视频层上chn0 在画布上的位置
	POINT_S stDispPos;
	stDispPos.s32X = 500;
	stDispPos.s32Y = 500;
	s32Ret = HI_MPI_VO_SetChnDispPos(g_VoLayerPip,0,&stDispPos);
	if (s32Ret != HI_SUCCESS)
	{
		fprintf(stderr,"failed with %#x!\n", s32Ret);
		printf("failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}
#endif

#if 0
	/*** enable vpss group clip ***/
	VPSS_CROP_INFO_S stVpssCropInfo;
	stVpssCropInfo.bEnable = HI_TRUE;
	stVpssCropInfo.enCropCoordinate = VPSS_CROP_ABS_COOR;
	stVpssCropInfo.stCropRect.s32X = 0;
	stVpssCropInfo.stCropRect.s32Y = 0;
	stVpssCropInfo.stCropRect.u32Height = 100;
	stVpssCropInfo.stCropRect.u32Width = 100;
	s32Ret = HI_MPI_VPSS_SetGrpCrop(VpssGrp_Clip, &stVpssCropInfo);
	if (HI_SUCCESS != s32Ret)
	{
		fprintf(stderr,"HI_MPI_VPSS_SetGrpCrop failed with %#x!\n", s32Ret);
	}
#endif
	VO_PART_MODE_E enPartMode;
	HI_MPI_VO_GetVideoLayerPartitionMode(g_VoLayer,&enPartMode);
	printf("VoLayer:enPartMode:%d\n",enPartMode);

	HI_MPI_VO_GetVideoLayerPartitionMode(g_VoLayerPip,&enPartMode);
	printf("VoLayerPip:enPartMode:%d\n",enPartMode);


	printf("press any key to enable wbc.\n");
	getchar();
	WbcDump2(0);

	printf("WbcDump success \n");



	while(1)
		sleep(1);

	return s32Ret;
}

#if 1
int test9(){
	HI_S32 s32VpssGrpCnt = 10;
	VO_DEV VoDev;
	DEMO_VO_MODE_E enVoMode, enPreVoMode;
	HI_S32 i;
	HI_S32 s32Ret = HI_SUCCESS;
	HI_CHAR ch;
	SIZE_S stSize;
	HI_U32 u32WndNum;
	MPP_CHN_S stSrcChn;
	MPP_CHN_S stDestChn;

	printf("[%s]:Begin ***************************** line:%d\n", __func__, __LINE__);
	/******************************************
	  step 2: mpp system init. 
	 ******************************************/
	s32Ret = sys_init();
	if (HI_SUCCESS != s32Ret){
		printf("[%s][Error]:system init failed with %d! line:%d\n", __func__, s32Ret, __LINE__);
		return s32Ret;
	}

	/******************************************
	  step 4: start vpss and vi bind vpss
	 ******************************************/
	//s32Ret = vpss_start(g_enNorm,10);
	s32Ret = vpss_start(g_enNorm,s32VpssGrpCnt);
	if (HI_SUCCESS != s32Ret){
		printf("[%s][Error]:Start Vpss failed! line:%d\n", __func__, __LINE__);
		return s32Ret;
	}
	
#if 1
	/******************************************
	  step 3: start vi dev & chn
	 ******************************************/
	s32Ret = vi_start(g_enViMode, g_enNorm);
	if (HI_SUCCESS != s32Ret){
		printf("[%s][Error]:start vi failed! line:%d\n", __func__, __LINE__);
		return s32Ret;
	}
	
	//获取8路数据
	s32Ret = vi_to_vpss(g_enViMode);
	if (HI_SUCCESS != s32Ret){
		printf("[%s][Error]:vi_to_vpss failed! line:%d\n", __func__, __LINE__);
		return s32Ret;
	}
#endif

#if 1
	//获取第9路数据
	s32Ret = vdec_start();
	if (HI_SUCCESS != s32Ret){
		printf("[%s][Error]:vdec_start failed! line:%d\n", __func__, __LINE__);
		return s32Ret;
	}

	s32Ret = vdec_to_vpss(8);
	//s32Ret = vdec_to_vpss(0);
	if (HI_SUCCESS != s32Ret){
		printf("[%s][Error]:vdec_to_vpss failed! line:%d\n", __func__, __LINE__);
		return s32Ret;
	}
#endif
	for(i=0; i< s32VpssGrpCnt; i++){
		setVPChModeUser(i, 0); /*设置VPSS通道工作模式(User模式)*/	
	}
	s32Ret = vo_start();
	if (HI_SUCCESS != s32Ret){
		printf("[%s][Error]:Vo start failed! line:%d\n", __func__, __LINE__);
		return s32Ret;
	}
	//s32Ret = vpss_to_vo(3,g_VoLayer,9);
	//s32Ret = vpss_to_vo(3,g_VoLayer,8);
	s32Ret = vpss_to_vo(0,g_VoLayer,9);
	if (HI_SUCCESS != s32Ret){
		printf("[%s][Error]:vpss_to_vo failed! line:%d\n", __func__, __LINE__);
		return s32Ret;
	}
	demoFunc();
	printf("[%s]:press any key to enable enc. s32VpssGrpCnt=%d line:%d\n", __func__, s32VpssGrpCnt, __LINE__);
	getchar();
	return 0;
}
#endif

/*************************************************************************************************
* Description:函数功能列表
***************************************************************************************************/
int printFuncList(){
	printf("1-printVPAttr          2-fullScreen          3-setVPChModeUser          4-setVPChModeAuto\n");
	return 0;
}

/**************************************************************************************************
 * Description:测试程序入口
 ****************************************************************************************************/
int demoFunc()
{
#if 0
	HI_S32 s32Ret = HI_FAILURE;
	VO_DEV VoDev = SAMPLE_VO_DEV_DHD0;
	VO_LAYER VoLayer = SAMPLE_VO_LAYER_VHD0;
	char buffer[32] = {0};
	//SAMPLE_VO_MODE_E enVoMode, enPreVoMode;

	/*异常信号处理*/
	signal(SIGINT, SAMPLE_VIO_HandleSig);
	signal(SIGTERM, SAMPLE_VIO_HandleSig);
	funcVOFromVI();
#endif
	HI_S32 devID = 0, chID = 0, cmdType = 0;
	HI_U8 buffer[64] = {0};
	HI_S32 s32Ret = HI_FAILURE;
	printf("[%s]:****** show plase enter numter:(such as:XX-XX-XX)=", __func__);
	while(1)
	{
		if(NULL == gets(buffer)){
			printf("[%s][Error]:wrong input data. line:%d\n", __func__, __LINE__);
			continue;
		}
		s32Ret = atoi(buffer);
		printf("[%s]:cmd=%d 100-quit VI=0 4...+4 VO=0 1....+1 line:%d\n", __func__, s32Ret, __LINE__);
		if(100 == s32Ret){
			break;
		}
		sscanf(buffer, "%d-%d-%d", &cmdType, &devID, &chID);
		switch(cmdType){
		case 1:
			printVPAttr(devID, chID); /*输出VP模块的相关信息*/
			break;
		case 2:
			fullScreen(devID, chID); /*让某视频层的某通道全屏显示*/			
			break;
		case 3:
			setVPChModeUser(devID, chID); /*设置VPSS通道工作模式(User模式)*/
			break;
		case 4:
			setVPChModeAuto(devID, chID); /*设置VPSS通道工作模式(Auto模式)*/
			break;
		default:
			printf("[%s]:default! line:%d\n", __func__, __LINE__);
			break;
		}

		//showVINum(s32Ret); /*将不同的VI输出到同一个VO通道上*/
		//showVONum(s32Ret); /*将同一个VI输出到不同的VO通道上*/
		//layersDisplay(s32Ret);
		//demoVIVpassVO();
		printFuncList();
		printf("[%s]:****** show plase enter numter:(such as:XX-XX-XX)=", __func__);
	}
	SAMPLE_COMM_SYS_Exit();
	return 0;
}


/******************************************************************************
 * function    : main()
 * Description : video preview sample
 * Description:鎵ц渚嬪瓙绋嬪簭鐨勭殑鍛戒护锛?/sample_test 6 2> log
 ******************************************************************************/
int main(int argc, char *argv[])
{
	HI_S32 s32Ret = HI_FAILURE;

	if ( (argc < 2) || (1 != strlen(argv[1])))
	{
		Usage(argv[0]);
		return HI_FAILURE;
	}
	signal(SIGINT,HandleSig);
	signal(SIGTERM, HandleSig);

	switch(*argv[1])
	{
	case '1':  
		/* VI:8*720P;VPSS GROUP(chn0);VO:DHD--VHD0(HDMI|VGA); */
		s32Ret = test1();
		break;	
	case '2':
		/* VI:8*720P; VO:DHD--VHD0*/
		s32Ret = test2();
		break;	
	case '3':
		/* VI:8*720P; VO:DHD--VHD0*/
		/* 	  (chn12); VO:DHD--PIP(chn0)*/
		s32Ret = test3();
		break;
	case '4':
		/* VI:8*720P; VPSS GROUP(0-7)(chn0)---VO:DHD--VHD0*/
		/*                        (group0 chn3)-crop--VO:DHD--PIP(chn0) */
		s32Ret = test4();
		break;
	case '5':
		/* VI:8*720P; VPSS GROUP(0-7)(chn0)---VO:DHD--VHD0*/
		/*    file;VDEC(chn0);VPSS(group8 chn0)---VO:DHD--VHD0  */
		/*    file;VDEC(chn0);VPSS(group8 chn3)-crop--VO:DHD--PIP(chn0)*/
		/*VO:DHD;WBC;file*/
		s32Ret = test5();
		break;
	case '6':
		/* VI:8*720P; VPSS GROUP(0-7)(chn3)---VO:DHD--VHD0*/
		/*    file;VDEC(chn0);VPSS(group8 chn3)---VO:DHD--VHD0  */
		/*    file;VDEC(chn0);VPSS(group8 chn2)-crop--VO:DHD--PIP(chn0)*/
		/*VPSS(0-8 ch0 ch1);VENC(HIGH,LOW);file*/
		/*VO:DHD;WBC;file*/
		s32Ret = test6();
		break;	
	case '7':
		/* VI:8*720P; VPSS GROUP(0-7)(chn3)---VO:DHD--VHD0*/
		/*    file;VDEC(chn0);VPSS(group8 chn3)---VO:DHD---VHD0  */
		/*    file;VDEC(chn0);VPSS(group8 chn2)-crop--VO:DHD--PIP(chn0)*/
		/*VO:DHD;WBC1(5帧);VPSS(group9)* /
		/*VPSS(0-9 ch0 ch1);VENC(HIGH,LOW);file*/
		s32Ret = test7(); /*merlin read*/
		break;		
	case '8':
		/* VI:8*720P; VPSS GROUP(0-7)(chn3)---VO:DHD--VHD0*/
		/*    file;VDEC(chn0);VPSS(group8 chn3)---VO:DHD---VHD0  */
		/*    file;VDEC(chn0);VPSS(group8 chn2)-crop--VO:DHD--PIP(chn0)*/
		/*VO:DHD;WBC2(while);VPSS(group9)* /
		/*VPSS(0-9 ch0 ch1);VENC(HIGH,LOW);file*/
		s32Ret = test8();
		break;
	case '9':
		s32Ret = test9();
		break;
	default:
		fprintf(stderr,"input invaild! please try again.\n");
		Usage(argv[0]);
		return HI_FAILURE;
	}

	if (HI_SUCCESS == s32Ret)
		fprintf(stderr,"program exit normally!\n");
	else
		fprintf(stderr,"program exit abnormally!\n");
	exit(s32Ret);

}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


