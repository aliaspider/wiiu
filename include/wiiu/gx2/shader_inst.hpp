#pragma once

#include <wiiu/types.h>
#include "shader_enums.hpp"

#ifdef __BIG_ENDIAN__
struct CF_WORD0 {
	u32 addr;
};

struct CF_WORD1 {
	bool barrier       :1;
	u32 wholeQuadMode  :1;
	u32 instIsAlu      :1;
	u32 instIsExport   :1;
	CF_INST cfInst     :5;
	u32 validPixelMode :1;
	u32 endOfProgram   :1;
	u32 _reserved      :1;
	u32 count_3        :1;
	u32 callCount      :6;
	u32 count          :3;
	u32 cond           :2;
	u32 cfConst        :5;
	u32 popCount       :3;
};

struct CF_ALU_WORD0 {
	KCacheMode kcacheMode0 :2;
	u32 kcacheBank1        :4;
	u32 kcacheBank0        :4;
	u32 addr               :22;
};

struct CF_ALU_WORD1 {
	bool barrier           :1;
	u32 wholeQuadMode      :1;
	CF_INST_ALU cfInst     :4;
	u32 altConst           :1;
	u32 count              :7;
	u32 kcacheAddr1        :8;
	u32 kcacheAddr0        :8;
	KCacheMode kcacheMode1 :2;
};

struct CF_ALLOC_EXPORT_WORD0 {
	u32 elemSize  :2;
	u32 indexGPR  :7;
	u32 RWRel     :1;
	u32 RWGPR     :7;
	u32 type      :2;
	u32 arrayBase :13;
};

struct CF_ALLOC_EXPORT_WORD1 {
	bool barrier       :1;
	u32 wholeQuadMode  :1;
	CF_INST_EXP cfInst :7;
	u32 validPixelMode :1;
	u32 endOfProgram   :1;
	u32 burstCount     :4;
	u32 _reserved      :1;
	union {
		struct {
			u16 compMask  :4;
			u16 arraySize :12;
		} /*BUF*/;
		struct {
			u16 _reserved2 :4;
			u16 selW       :3;
			u16 selZ       :3;
			u16 selY       :3;
			u16 selX       :3;
		} /*SWIZ*/;
	};
};
static_assert(sizeof(CF_ALLOC_EXPORT_WORD1) == 4, "");

struct ALU_WORD0 {
	u32 last      :1;
	u32 predSel   :2;
	u32 indexMode :3;
	u32 src1Neg   :1;
	u32 src1Chan  :2;
	u32 src1Rel   :1;
	u32 src1Sel   :9;
	u32 src0Neg   :1;
	u32 src0Chan  :2;
	u32 src0Rel   :1;
	u32 src0Sel   :9;
};

struct ALU_WORD1_OP2 {
	u32 clamp               :1;
	u32 dstElem             :2;
	u32 dstRel              :1;
	u32 dstGPR              :7;
	BankSwizzle bankSwizzle :3;
	u32 is_op3              :4;
	ALU_OP2_INST inst       :7;
	ALU_OMOD omod           :2;
	u32 writeMask           :1;
	u32 updatePred          :1;
	u32 updateExecuteMask   :1;
	u32 src1Abs             :1;
	u32 src0Abs             :1;
};

struct ALU_WORD1_OP3 {
	u32 clamp               :1;
	u32 dstElem             :2;
	u32 dstRel              :1;
	u32 dstGPR              :7;
	BankSwizzle bankSwizzle :3;
	ALU_OP3_INST inst       :5;
	u32 src2Neg             :1;
	u32 src2Chan            :2;
	u32 src2Rel             :1;
	u32 src2Sel             :9;
};

struct VTX_WORD0 {
	u32 megaFetchCount :6;
	u32 srcSelX        :2;
	u32 srcRel         :1;
	u32 srcGPR         :7;
	u32 buffer_id      :8;
	u32 fetchWholeQuad :1;
	u32 fetchType      :2;
	u32 inst           :5;
};

struct VTX_WORD1 {
	u32 SRFModeAll     :1;
	u32 formatCompAll  :1;
	u32 numFormatAll   :2;
	u32 dataFormat     :6;
	u32 useConstFields :1;
	u32 dstSelW        :3;
	u32 dstSelZ        :3;
	u32 dstSelY        :3;
	u32 dstSelX        :3;
	u32 _reserved      :1;
	union {
		u8 semanticID; /*SEM*/
		struct {
			u8 dstRel :1;
			u8 dstGPR :7;
		} /*GPR*/;
	};
};
static_assert(sizeof(VTX_WORD1) == 4, "");

struct VTX_WORD2 {
	u32 _reserved        :11;
	u32 alConst          :1;
	u32 megaFetch        :1;
	u32 constBufNoStride :1;
	u32 endianSwap       :2;
	u32 offset           :16;
};

struct TEX_WORD0 {
	u32 _reserved2     :7;
	u32 altConst       :1;
	u32 srcRel         :1;
	u32 srcGPR         :7;
	u32 resourceID     :8;
	u32 fetchWholeQuad :1;
	u32 _reserved      :1;
	u32 bcFracMode     :1;
	TEX_INST inst      :5;
};

struct TEX_WORD1 {
	CoordType coordTypeW :1;
	CoordType coordTypeZ :1;
	CoordType coordTypeY :1;
	CoordType coordTypeX :1;
	u32 lodBias          :7;
	u32 dstSelW          :3;
	u32 dstSelZ          :3;
	u32 dstSelY          :3;
	u32 dstSelX          :3;
	u32 _reserved        :1;
	u32 dstRel           :1;
	u32 dstGPR           :7;
};

struct TEX_WORD2 {
	u32 srcSelW   :3;
	u32 srcSelZ   :3;
	u32 srcSelY   :3;
	u32 srcSelX   :3;
	u32 samplerID :5;
	u32 offsetZ   :5;
	u32 offsetY   :5;
	u32 offsetX   :5;
};

struct MEM_RD_WORD0 {
	u32 _reserved      :2;
	u32 burstCnt       :4;
	u32 srcSelX        :2;
	u32 srcRel         :1;
	u32 srcGPR         :7;
	u32 memReqSize     :2;
	u32 indexed        :1;
	u32 uncached       :1;
	u32 memOp          :3;
	u32 fetchWholeQuad :1;
	u32 elemSize       :2;
	u32 vtxInst        :5;
};

struct MEM_RD_WORD1 {
	u32 srfModeAll    :1;
	u32 formatCompAll :1;
	u32 numFormatAll  :2;
	u32 dataFormat    :6;
	u32 reserved      :21;
	u32 dstSel_w      :3;
	u32 dstSel_z      :3;
	u32 dstSel_y      :3;
	u32 dstSel_x      :3;
	u32 dstRel        :1;
	u32 dstGPR        :7;
};

struct MEM_RD_WORD2 {
	u32 arraySize  :12;
	u32 megaFetch  :1;
	u32 _reserved2 :1;
	u32 endianSwap :2;
	u32 _reserved  :3;
	u32 arrayBase  :13;
};

struct MEM_DSW_WORD0 {
	u32 srcSelW    :3;
	u32 srcSelZ    :3;
	u32 srcSelY    :3;
	u32 srcSelX    :3;
	u32 srcRelMode :2;
	u32 srcGPR     :7;
	u32 memOp      :3;
	u32 reserved   :3;
	u32 vtxInst    :5;
};

struct MEM_DSW_WORD1 {
	u32 simdWaveRel  :1;
	u32 _reserved2   :6;
	u32 dstStrideExt :2;
	u32 dstStride    :7;
	u32 _reserved    :8;
	u32 dstIndexExt  :2;
	u32 dstIndex     :6;
};

struct MEM_DSW_WORD2 {
	u32 Reserved  :28;
	u32 burst_cnt :4;
};

struct MEM_DSR_WORD0 {
	u32 rdOffset   :6;
	u32 srcSelY    :3;
	u32 srcSelX    :3;
	u32 srcRelMode :2;

	u32 srcGPR   :7;
	u32 memOp    :3;
	u32 reserved :3;
	u32 vtxInst  :5;
};

struct MEM_DSR_WORD1 {
	u32 _reserved2 :9;
	u32 muxCntl    :3;
	u32 reserved   :2;
	u32 waterfall  :1;
	u32 broadcast  :1;
	u32 _reserved  :3;
	u32 dstWRW     :1;
	u32 dstWRZ     :1;
	u32 dstWRY     :1;
	u32 dstWRX     :1;
	u32 dstRelMode :2;
	u32 dstGPR     :7;
};

struct MEM_DSR_WORD2 {
	u32 _reserved :28;
	u32 burstCnt  :4;
};
#else
struct CF_WORD0 {
	u32 addr;
};

struct CF_WORD1 {
	u32 popCount       :3;
	u32 cfConst        :5;
	u32 cond           :2;
	u32 count          :3;
	u32 callCount      :6;
	u32 count_3        :1;
	u32 _reserved      :1;
	u32 endOfProgram   :1;
	u32 validPixelMode :1;
	CF_INST cfInst     :5;
	u32 instIsExport   :1;
	u32 instIsAlu      :1;
	u32 wholeQuadMode  :1;
	bool barrier       :1;
};

struct CF_ALU_WORD0 {
	u32 addr               :22;
	u32 kcacheBank0        :4;
	u32 kcacheBank1        :4;
	KCacheMode kcacheMode0 :2;
};

struct CF_ALU_WORD1 {
	KCacheMode kcacheMode1 :2;
	u32 kcacheAddr0        :8;
	u32 kcacheAddr1        :8;
	u32 count              :7;
	u32 altConst           :1;
	CF_INST_ALU cfInst     :4;
	u32 wholeQuadMode      :1;
	bool barrier           :1;
};

struct CF_ALLOC_EXPORT_WORD0 {
	u32 arrayBase :13;
	u32 type      :2;
	u32 RWGPR     :7;
	u32 RWRel     :1;
	u32 indexGPR  :7;
	u32 elemSize  :2;
};

struct CF_ALLOC_EXPORT_WORD1 {
	union {
		struct {
			u16 arraySize :12;
			u16 compMask  :4;
		} /*BUF*/;
		struct {
			u16 selX :3;
			u16 selY :3;
			u16 selZ :3;
			u16 selW :3;
		} /*SWIZ*/;
	};
	u32 _reserved      :1;
	u32 burstCount     :4;
	u32 endOfProgram   :1;
	u32 validPixelMode :1;
	CF_INST_EXP cfInst :7;
	u32 wholeQuadMode  :1;
	bool barrier       :1;
};
static_assert(sizeof(CF_ALLOC_EXPORT_WORD1) == 4, "");

struct ALU_WORD0 {
	u32 src0Sel   :9;
	u32 src0Rel   :1;
	u32 src0Chan  :2;
	u32 src0Neg   :1;
	u32 src1Sel   :9;
	u32 src1Rel   :1;
	u32 src1Chan  :2;
	u32 src1Neg   :1;
	u32 indexMode :3;
	u32 predSel   :2;
	u32 last      :1;
};

struct ALU_WORD1_OP2 {
	u32 src0Abs             :1;
	u32 src1Abs             :1;
	u32 updateExecuteMask   :1;
	u32 updatePred          :1;
	u32 writeMask           :1;
	ALU_OMOD omod           :2;
	ALU_OP2_INST inst       :7;
	u32 is_op3              :4;
	BankSwizzle bankSwizzle :3;
	u32 dstGPR              :7;
	u32 dstRel              :1;
	u32 dstElem             :2;
	u32 clamp               :1;
};

struct ALU_WORD1_OP3 {
	u32 src2Sel             :9;
	u32 src2Rel             :1;
	u32 src2Chan            :2;
	u32 src2Neg             :1;
	ALU_OP3_INST inst       :5;
	BankSwizzle bankSwizzle :3;
	u32 dstGPR              :7;
	u32 dstRel              :1;
	u32 dstElem             :2;
	u32 clamp               :1;
};

struct VTX_WORD0 {
	u32 inst           :5;
	u32 fetchType      :2;
	u32 fetchWholeQuad :1;
	u32 buffer_id      :8;
	u32 srcGPR         :7;
	u32 srcRel         :1;
	u32 srcSelX        :2;
	u32 megaFetchCount :6;
};

struct VTX_WORD1 {
	union {
		u8 semanticID; /*SEM*/
		struct {
			u8 dstGPR :7;
			u8 dstRel :1;
		} /*GPR*/;
	};
	u32 _reserved      :1;
	u32 dstSelX        :3;
	u32 dstSelY        :3;
	u32 dstSelZ        :3;
	u32 dstSelW        :3;
	u32 useConstFields :1;
	u32 dataFormat     :6;
	u32 numFormatAll   :2;
	u32 formatCompAll  :1;
	u32 SRFModeAll     :1;
};
static_assert(sizeof(VTX_WORD1) == 4, "");

struct VTX_WORD2 {
	u32 offset           :16;
	u32 endianSwap       :2;
	u32 constBufNoStride :1;
	u32 megaFetch        :1;
	u32 alConst          :1;
	//	u32 _reserved     :11;
};

struct TEX_WORD0 {
	TEX_INST inst      :5;
	u32 bcFracMode     :1;
	u32 _reserved      :1;
	u32 fetchWholeQuad :1;
	u32 resourceID     :8;
	u32 srcGPR         :7;
	u32 srcRel         :1;
	u32 altConst       :1;
	//	u32 _reserved   :7;
};

struct TEX_WORD1 {
	u32 dstGPR           :7;
	u32 dstRel           :1;
	u32 _reserved        :1;
	u32 dstSelX          :3;
	u32 dstSelY          :3;
	u32 dstSelZ          :3;
	u32 dstSelW          :3;
	u32 lodBias          :7;
	CoordType coordTypeX :1;
	CoordType coordTypeY :1;
	CoordType coordTypeZ :1;
	CoordType coordTypeW :1;
};

struct TEX_WORD2 {
	u32 offsetX   :5;
	u32 offsetY   :5;
	u32 offsetZ   :5;
	u32 samplerID :5;
	u32 srcSelX   :3;
	u32 srcSelY   :3;
	u32 srcSelZ   :3;
	u32 srcSelW   :3;
};

struct MEM_RD_WORD0 {
	u32 vtxInst        :5;
	u32 elemSize       :2;
	u32 fetchWholeQuad :1;
	u32 memOp          :3;
	u32 uncached       :1;
	u32 indexed        :1;
	u32 memReqSize     :2;
	u32 srcGPR         :7;
	u32 srcRel         :1;
	u32 srcSelX        :2;
	u32 burstCnt       :4;
	u32 _reserved      :2;
};

struct MEM_RD_WORD1 {
	u32 dstGPR        :7;
	u32 dstRel        :1;
	u32 dstSel_x      :3;
	u32 dstSel_y      :3;
	u32 dstSel_z      :3;
	u32 dstSel_w      :3;
	u32 reserved      :21;
	u32 dataFormat    :6;
	u32 numFormatAll  :2;
	u32 formatCompAll :1;
	u32 srfModeAll    :1;
};

struct MEM_RD_WORD2 {
	u32 arrayBase  :13;
	u32 _reserved  :3;
	u32 endianSwap :2;
	u32 _reserved2 :1;
	u32 megaFetch  :1;
	u32 arraySize  :12;
};

struct MEM_DSW_WORD0 {
	u32 vtxInst    :5;
	u32 reserved   :3;
	u32 memOp      :3;
	u32 srcGPR     :7;
	u32 srcRelMode :2;
	u32 srcSelX    :3;
	u32 srcSelY    :3;
	u32 srcSelZ    :3;
	u32 srcSelW    :3;
};

struct MEM_DSW_WORD1 {
	u32 dstIndex     :6;
	u32 dstIndexExt  :2;
	u32 _reserved    :8;
	u32 dstStride    :7;
	u32 dstStrideExt :2;
	u32 _reserved2   :6;
	u32 simdWaveRel  :1;
};

struct MEM_DSW_WORD2 {
	u32 burst_cnt :4;
	//	u32 Reserved :28;
};

struct MEM_DSR_WORD0 {
	u32 vtxInst    :5;
	u32 reserved   :3;
	u32 memOp      :3;
	u32 srcGPR     :7;
	u32 srcRelMode :2;
	u32 srcSelX    :3;
	u32 srcSelY    :3;
	u32 rdOffset   :6;
};

struct MEM_DSR_WORD1 {
	u32 dstGPR     :7;
	u32 dstRelMode :2;
	u32 dstWRX     :1;
	u32 dstWRY     :1;
	u32 dstWRZ     :1;
	u32 dstWRW     :1;
	u32 _reserved  :3;
	u32 broadcast  :1;
	u32 waterfall  :1;
	u32 reserved   :2;
	u32 muxCntl    :3;
	//	u32 _reserved2 :9;
};

struct MEM_DSR_WORD2 {
	u32 burstCnt :4;
	//	u32 _reserved :28;
};
#endif
