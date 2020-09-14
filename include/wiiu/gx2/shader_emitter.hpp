#pragma once

/* info :
 * https://developer.amd.com/wordpress/media/2012/10/R600-R700-Evergreen_Assembly_Language_Format.pdf
 *	https://developer.amd.com/wordpress/media/2012/10/R700-Family_Instruction_Set_Architecture.pdf
 * http://developer.amd.com/wordpress/media/2013/10/evergreen_3D_registers_v2.pdf
 * https://github.com/decaf-emu/decaf-emu */

#include <vector>
#include <string>
#include <sstream>

#include <wiiu/os/memory.h>
#include "mem.h"
#include "shaders.h"
#include "shader_inst.hpp"

#ifndef _assert_
#include <cassert>
#define _assert_ assert
#endif

namespace GX2Gen {

enum class VSInput : u32;
enum class PSInput : u32;
enum class UB_Bindings : u32;

enum Channel : u32 {
	x = 0,
	y = 1,
	z = 2,
	w = 3,
	r = x,
	g = y,
	b = z,
	a = w,
	_0_ = 4,
	_1_ = 5,
	zero = _0_,
	one = _1_,
};
enum AluSrc : u32 {
	ALU_SRC_1_DBL_L = 0xF4,
	ALU_SRC_1_DBL_M = 0xF5,
	ALU_SRC_0_5_DBL_L = 0xF6,
	ALU_SRC_0_5_DBL_M = 0xF7,
	ALU_SRC_0 = 0xF8,
	ALU_SRC_1 = 0xF9,
	ALU_SRC_1_INT = 0xFA,
	ALU_SRC_M_1_INT = 0xFB,
	ALU_SRC_0_5 = 0xFC,
	ALU_SRC_LITERAL = 0xFD,
	ALU_SRC_PV = 0xFE,
	ALU_SRC_PS = 0xFF,
};
struct SrcReg {
	SrcReg(u32 id_, Channel ch_) : id(id_), ch(ch_) {}
	u32 id;
	Channel ch;
};
struct DstReg {
	DstReg(u32 id_, Channel ch_, bool write_mask_ = true) : id(id_), ch(ch_), write_mask(write_mask_) {}
	u32 id;
	Channel ch;
	bool write_mask;
};
struct PARAM {
	PARAM(PSInput id) : type((ExportType)(EXPORT_TYPE_PARAM + (u32)id)) {}
	ExportType type;
	operator ExportType() { return type; }
};
struct RegChannel {
	RegChannel(u32 id_, Channel ch_, bool writeMask_ = true) : id(id_), ch(ch_), writeMask(writeMask_) {}
	u32 id;
	bool writeMask;
	Channel ch;
	operator SrcReg() { return SrcReg(id, ch); }
	operator DstReg() { return DstReg(id, ch, writeMask); }
};
struct Reg {
	Reg(u32 id_, bool writeMask_ = true) :
		id(id_),
		writeMask(writeMask_),
		ch{ Channel::x, Channel::y, Channel::z, Channel::w, Channel::_0_, Channel::_1_ } {}
	u32 id;
	bool writeMask;
	union {
		struct {
			Channel x, y, z, w, zero, one;
		};
		Channel ch[6];
	};
	Reg operator()(Channel c0, Channel c1, Channel c2, Channel c3) const {
		Reg r(id, writeMask);
		r.x = ch[c0];
		r.y = ch[c1];
		r.z = ch[c2];
		r.w = ch[c3];
		return r;
	}
	RegChannel operator()(Channel c) const { return RegChannel(id, ch[c], writeMask); }
};

static const Reg PV(ALU_SRC_PV);
static const SrcReg PS(ALU_SRC_PS, x);
static const Reg ___(0, false);

class Constant;
class GX2Emitter;

class GX2Emitter {
public:
	class Constant {
	public:
		Constant(UB_Bindings binding, size_t offset, GX2Emitter *emitter) :
			binding_((int)binding), offset_(offset >> 4), emitter_(emitter) {
			_assert_(!(offset & 0xF)); // vec4 aligned
		}
		Reg operator[](int index) { return emitter_->lockKCacheLane((int)binding_, offset_ + index); }
		SrcReg operator()(Channel c) { return operator[](0)(c); }

	private:
		int binding_;
		size_t offset_;
		GX2Emitter *emitter_;
	};
	enum : u32 {
		INVALID_SEMANTIC = ~0u,
		TEMP_GPR_FIRST = 0x78,
		TEMP_GPR_LAST = 0x7f,

		VALID_PIX = 1,
		ALU_SRC_KCACHE_SIZE = 0x20,
		ALU_SRC_KCACHE0_BASE = 0x80,
		ALU_SRC_KCACHE1_BASE = ALU_SRC_KCACHE0_BASE + ALU_SRC_KCACHE_SIZE,
	};

protected:
	GX2Emitter() {}

public:
	struct KCACHE {
		u32 binding;
		KCacheMode mode;
		u32 addr;
	};
	KCACHE kCache[2] = {};
	Reg lockKCacheLane(int binding, size_t offset) {
		int i;
		size_t addr = offset >> 4;
		for (i = 0; i < 2; i++) {
			if (kCache[i].mode == CF_KCACHE_BANK_LOCK_NONE) {
				kCache[i].mode = CF_KCACHE_BANK_LOCK_1;
				kCache[i].binding = binding;
				kCache[i].addr = addr;
				break;
			}
			if (binding == kCache[i].binding) {
				if (addr == kCache[i].addr)
					break;
				if (addr == kCache[i].addr + 1) {
					kCache[i].mode = CF_KCACHE_BANK_LOCK_2;
					break;
				}
			}
		}
		_assert_(i < 2);
		return Reg(offset - (kCache[i].addr << 4) + ALU_SRC_KCACHE0_BASE + i * ALU_SRC_KCACHE_SIZE);
	}
	~GX2Emitter() {}
	void CALL_FS(bool barrier = false) { emitCF(CF_DWORD0(0), CF_DWORD1(CF_INST::CALL_FS, barrier)); }

	void END_OF_PROGRAM() {
		program.clear();
		cf_.back() = cf_.back() | QWORD(0, 1 << 21);
		for (u64 word : cf_)
			program.push_back(word);
		// align to 32 Qwords
		while (program.size() & 0x1F)
			program.push_back(0);
		for (patch_ALU patch : patch_alu_)
			program[patch.pos] = ALU(patch.addr + program.size(), patch.cnt, patch.kCache0, patch.kCache1);

		for (u64 word : alu_)
			program.push_back(word);
		// align to 16 Qwords
		while (program.size() & 0xF)
			program.push_back(0);

		for (patch_TEX patch : patch_tex_)
			program[patch.pos] = TEX(patch.addr + program.size(), patch.cnt);

		for (u64 word : tex_)
			program.push_back(word);
		// align to 16 Qwords
		while (program.size() & 0xF)
			program.push_back(0);
	}

	u64 ALU(u32 addr, u32 count, KCACHE kCache0, KCACHE kCache1, u32 altConst = 0, bool barrier = true) {
		return QWORD(CF_ALU_WORD0(addr, kCache0.binding, kCache1.binding, kCache0.mode),
						 CF_ALU_WORD1(kCache1.mode, kCache0.addr, kCache1.addr, count, altConst, CF_INST_ALU::ALU, barrier));
	}
	u64 TEX(u32 addr, u32 count, bool barrier = true) {
		_assert_(count > 0);
		return QWORD(CF_DWORD0(addr), CF_DWORD1(CF_INST::TEX, barrier, count - 1, VALID_PIX));
	}

	void ALU_LAST() { alu_.back() |= QWORD((1ull << 31), 0); }

#define EMIT_ALU_OP2(op2) \
	void op2(DstReg dst, SrcReg src0, SrcReg src1 = SrcReg(ALU_SRC_0, x)) { \
		ALU_OP2(ALU_OP2_INST::op2, dst, src0, src1, ALU_OMOD::OFF); \
	} \
	void op2##x2(DstReg dst, SrcReg src0, SrcReg src1 = SrcReg(ALU_SRC_0, x)) { \
		ALU_OP2(ALU_OP2_INST::op2, dst, src0, src1, ALU_OMOD::M2); \
	} \
	void op2##x4(DstReg dst, SrcReg src0, SrcReg src1 = SrcReg(ALU_SRC_0, x)) { \
		ALU_OP2(ALU_OP2_INST::op2, dst, src0, src1, ALU_OMOD::M4); \
	} \
	void op2##d2(DstReg dst, SrcReg src0, SrcReg src1 = SrcReg(ALU_SRC_0, x)) { \
		ALU_OP2(ALU_OP2_INST::op2, dst, src0, src1, ALU_OMOD::D2); \
	}

	EMIT_ALU_OP2(ADD)
	EMIT_ALU_OP2(MUL)
	EMIT_ALU_OP2(MUL_IEEE)
	EMIT_ALU_OP2(MIN)
	EMIT_ALU_OP2(MAX)
	EMIT_ALU_OP2(MAX_DX10)
	EMIT_ALU_OP2(FRACT)
	EMIT_ALU_OP2(SETGT)
	EMIT_ALU_OP2(SETE_DX10)
	EMIT_ALU_OP2(SETGT_DX10)
	EMIT_ALU_OP2(FLOOR)
	EMIT_ALU_OP2(MOV)
	EMIT_ALU_OP2(PRED_SETGT)
	EMIT_ALU_OP2(PRED_SETE_INT)
	EMIT_ALU_OP2(DOT4)
	EMIT_ALU_OP2(DOT4_IEEE)
	EMIT_ALU_OP2(RECIP_IEEE)
	EMIT_ALU_OP2(RECIPSQRT_IEEE)
	EMIT_ALU_OP2(SQRT_IEEE)
	EMIT_ALU_OP2(SIN)
	EMIT_ALU_OP2(COS)

#undef EMIT_ALU_OP2

#define EMIT_ALU_OP3(op3) \
	void op3(DstReg dst, SrcReg src0, SrcReg src1, SrcReg src2) { ALU_OP3(ALU_OP3_INST::op3, dst, src0, src1, src2); }

	EMIT_ALU_OP3(MULADD)
	EMIT_ALU_OP3(CNDGT)
	EMIT_ALU_OP3(CNDE_INT)

#undef EMIT_ALU_OP3

	void SAMPLE(Reg dst, Reg src, u32 resourceID, u32 samplerID) {
		emitTEX(TEX_WORD0(TEX_INST::SAMPLE, 0x0, 0x0, resourceID, src.id, 0x0, 0x0),
				  TEX_WORD1(dst.id, 0x0, dst.x, dst.y, dst.z, dst.w, 0x0, TEX_NORMALIZED, TEX_NORMALIZED, TEX_NORMALIZED,
								TEX_NORMALIZED),
				  TEX_WORD2(0x0, 0x0, 0x0, samplerID, src.x, src.y, src.z, src.w));
	}

	Reg allocReg(u32 semantic = INVALID_SEMANTIC) {
		u32 id;
		if (semantic == INVALID_SEMANTIC) {
			id = TEMP_GPR_FIRST + num_temp_gprs++;
			if (id > TEMP_GPR_LAST)
				abort();
		} else {
			id = num_gprs++;
			semantics_.push_back({ id, semantic });
		}
		return Reg(id);
	}

	Constant makeConstant(UB_Bindings binding, size_t offset) { return Constant(binding, offset, this); }

	SrcReg C(float val) {
		if (val == 0.0f) {
			return SrcReg(ALU_SRC_0, x);
		} else if (val == 1.0f) {
			return SrcReg(ALU_SRC_1, x);
		} else if (val == 0.5f) {
			return SrcReg(ALU_SRC_0_5, x);
		}
		alu_literals.push_back(__builtin_bswap32(*(u32 *)&val));
		_assert_(alu_literals.size() < 5);
		return SrcReg(ALU_SRC_LITERAL, (Channel)(alu_literals.size() - 1));
	}
	//   SrcReg PV() { return SrcReg(ALU_SRC_PV, x); }

	std::vector<u64> cf_;
	std::vector<u64> alu_;
	std::vector<u64> tex_;

	std::vector<u32> alu_literals;
	std::vector<u64> program;
	std::vector<u32> regs;

	void EXP_DONE(ExportType dstReg_and_type, Reg srcReg, bool barrier = true) {
		emitCF(CF_EXP_WORD0(dstReg_and_type, srcReg.id, 0x0, 0x0, 0x0),
				 CF_EXP_WORD1(srcReg.x, srcReg.y, srcReg.z, srcReg.w, 0x0, CF_INST_EXP::EXP_DONE, barrier));
	}

	void EXP(ExportType dstReg_and_type, Reg srcReg, bool barrier = true) {
		emitCF(CF_EXP_WORD0(dstReg_and_type, srcReg.id, 0x0, 0x0, 0x0),
				 CF_EXP_WORD1(srcReg.x, srcReg.y, srcReg.z, srcReg.w, 0x0, CF_INST_EXP::EXP, barrier));
	}

protected:
	u32 num_gprs = 0;
	u32 num_temp_gprs = 0;
	struct Semantic {
		u32 reg;
		u32 value;
	};
	std::vector<Semantic> semantics_;

private:
	struct patch_ALU {
		size_t pos;
		size_t addr;
		size_t cnt;
		KCACHE kCache0;
		KCACHE kCache1;
	};
	struct patch_TEX {
		size_t pos;
		size_t addr;
		size_t cnt;
	};
	std::vector<patch_ALU> patch_alu_;
	std::vector<patch_TEX> patch_tex_;

	enum class Mode { CF, ALU, TEX, VTX };
	Mode mode_ = Mode::CF;

#ifdef __BIG_ENDIAN__
	u64 QWORD(u32 word0, u32 word1 = 0) { return __builtin_bswap64((u64)word0 | ((u64)word1 << 32ull)); }
#else
	u64 QWORD(u32 word0, u32 word1 = 0) { return (u64)word0 | ((u64)word1 << 32ull); }
#endif
	void checkCF(Mode newMode) {
		if (newMode == mode_)
			return;
		size_t last = 0;
		switch (mode_) {
		case Mode::CF: break;
		case Mode::ALU: {
			if (patch_alu_.size())
				last = patch_alu_.back().addr + patch_alu_.back().cnt;
			patch_alu_.push_back({ cf_.size(), last, alu_.size() - last, kCache[0], kCache[1] });
			cf_.push_back(0);
			kCache[0] = {};
			kCache[1] = {};

			break;
		}
		case Mode::TEX: {
			if (patch_tex_.size())
				last = patch_tex_.back().addr + patch_tex_.back().cnt;
			patch_tex_.push_back({ cf_.size(), last, (tex_.size() - last) >> 1 });
			cf_.push_back(0);
			break;
		}
		}
		mode_ = newMode;
	}
	void emitCF(u32 word0, u32 word1) {
		checkCF(Mode::CF);
		cf_.push_back(QWORD(word0, word1));
	}
	void emitALU(u32 word0, u32 word1) {
		checkCF(Mode::ALU);
		alu_.push_back(QWORD(word0, word1));
	}
	void emitTEX(u32 word0, u32 word1, u32 word2) {
		checkCF(Mode::TEX);
		tex_.push_back(QWORD(word0, word1));
		tex_.push_back(QWORD(word2));
	}

	u32 CF_DWORD0(u32 addr) { return addr; }

	u32 CF_DWORD1(CF_INST inst, bool barrier, u32 count = 0, u32 validPixelMode = 0, u32 cond = 0, u32 popCount = 0,
					  u32 cfConst = 0, u32 callCount = 0) {
		return (popCount | (cfConst << 3) | (cond << 8) | (count << 10) | (callCount << 13) | (validPixelMode << 22) |
				  ((u32)inst << 23) | ((u32) !!barrier << 31));
	}

	u32 CF_ALU_WORD0(u32 addr, u32 kcacheBank0, u32 kcacheBank1, KCacheMode kcacheMode0) {
		return (addr | ((u32)kcacheBank0 << 22) | ((u32)kcacheBank1 << 26) | ((u32)kcacheMode0 << 30));
	}
	u32 CF_ALU_WORD1(KCacheMode kcacheMode1, u32 kcacheAddr0, u32 kcacheAddr1, u32 count, u32 altConst, CF_INST_ALU inst,
						  bool barrier) {
		return ((u32)kcacheMode1 | (kcacheAddr0 << 2) | (kcacheAddr1 << 10) | ((count - 1) << 18) | (altConst << 25) |
				  ((u32)inst << 26) | ((u32) !!barrier << 31));
	}

	u32 CF_EXP_WORD0(ExportType dstReg_and_type, u32 srcReg, u32 srcRel, u32 indexGpr, u32 elemSize) {
		return ((u32)dstReg_and_type | (srcReg << 15) | (srcRel << 22) | (indexGpr << 23) | (elemSize << 30));
	}

	u32 CF_EXP_WORD1(u32 srcSelX, u32 srcSelY, u32 srcSelZ, u32 srcSelW, u32 validPixelMode, CF_INST_EXP inst,
						  bool barrier) {
		return (srcSelX | (srcSelY << 3) | (srcSelZ << 6) | (srcSelW << 9) | (validPixelMode << 22) | ((u32)inst << 23) |
				  ((u32) !!barrier << 31));
	}

	u32 CF_ALLOC_EXPORT_WORD0(u32 arrayBase, u32 type, u32 dstReg, u32 dstRel, u32 indexGpr, u32 elemSize) {
		return (arrayBase | (type << 13) | (dstReg << 15) | (dstRel << 22) | (indexGpr << 23) | (elemSize << 30));
	}

	u32 CF_ALLOC_EXPORT_WORD1_BUF(u32 arraySize, u32 writeMask, u32 inst, bool barrier) {
		return (arraySize | (writeMask << 12) | (inst << 23) | ((u32) !!barrier << 31));
	}

	u32 ALU_WORD0(u32 src0Sel, u32 src0Rel, u32 src0Chan, u32 src0Neg, u32 src1Sel, u32 src1Rel, u32 src1Chan,
					  u32 src1Neg, u32 indexMode, u32 predSel) {
		return (src0Sel | ((src0Rel) << 9) | ((src0Chan) << 10) | ((src0Neg) << 12) | ((src1Sel) << 13) |
				  ((src1Rel) << 22) | ((src1Chan) << 23) | ((src1Neg) << 25) | ((indexMode) << 26) | ((predSel) << 29));
	}

	u32 ALU_WORD1_OP2(u32 src0Abs, u32 src1Abs, u32 updateExecuteMask, u32 updatePred, u32 writeMask, ALU_OMOD omod,
							ALU_OP2_INST inst, u32 encoding, u32 bindingSwizzle, u32 dstGpr, u32 dstRel, u32 dstChan,
							u32 clamp) {
		return (src0Abs | (src1Abs << 1) | (updateExecuteMask << 2) | (updatePred << 3) | (writeMask << 4) |
				  ((u32)omod << 5) | ((u32)inst << 7) | (encoding << 15) | (bindingSwizzle << 18) |
				  ((dstGpr & 0x7F) << 21) | (dstRel << 28) | ((dstChan & 0x3) << 29) | (clamp << 31));
	}

	u32 ALU_WORD1_OP3(u32 src2Sel, u32 src2Rel, u32 src2Chan, u32 src2Neg, ALU_OP3_INST inst, u32 bindingSwizzle,
							u32 dstGpr, u32 dstRel, u32 dstChan, u32 clamp) {
		return (src2Sel | (src2Rel << 9) | (src2Chan << 10) | (src2Neg << 12) | ((u32)inst << 13) |
				  (bindingSwizzle << 18) | ((dstGpr & 0x7F) << 21) | (dstRel << 28) | ((dstChan & 0x3) << 29) |
				  (clamp << 31));
	}

	u32 TEX_WORD0(TEX_INST inst, u32 bcFracMode, u32 fetchWholeQuad, u32 resourceID, u32 srcReg, u32 srcRel,
					  u32 altConst) {
		return ((u32)inst | (bcFracMode << 5) | (fetchWholeQuad << 7) | (resourceID << 8) | (srcReg << 16) |
				  (srcRel << 23) | (altConst << 24));
	}

	u32 TEX_WORD1(u32 dstReg, u32 dstRel, u32 dstSelX, u32 dstSelY, u32 dstSelZ, u32 dstSelW, u32 lodBias,
					  CoordType coordTypeX, CoordType coordTypeY, CoordType coordTypeZ, CoordType coordTypeW) {
		return (dstReg | (dstRel << 7) | (dstSelX << 9) | (dstSelY << 12) | (dstSelZ << 15) | (dstSelW << 18) |
				  (lodBias << 21) | ((u32)coordTypeX << 28) | ((u32)coordTypeY << 29) | ((u32)coordTypeZ << 30) |
				  ((u32)coordTypeW << 31));
	}

	u32 TEX_WORD2(u32 offsetX, u32 offsetY, u32 offsetZ, u32 samplerID, u32 srcSelX, u32 srcSelY, u32 srcSelZ,
					  u32 srcSelW) {
		return (offsetX | (offsetY << 5) | (offsetZ << 10) | (samplerID << 15) | (srcSelX << 20) | (srcSelY << 23) |
				  (srcSelZ << 26) | (srcSelW << 29));
	}

	u32 VTX_WORD0(u32 inst, u32 type, u32 buffer_id, u32 srcReg, u32 srcSelX, u32 mega) {
		return (inst | (type << 5) | (buffer_id << 8) | (srcReg << 16) | (srcSelX << 24) | (mega << 26));
	}

	u32 VTX_WORD1(u32 dstReg, u32 dstSelX, u32 dstSelY, u32 dstSelZ, u32 dstSelW) {
		return (dstReg | (dstSelX << 9) | (dstSelY << 12) | (dstSelZ << 15) | (dstSelW << 18) | (1 << 21));
	}

	u32 VTX_WORD2(u32 offset, u32 ismega) { return (offset | (ismega << 19)); }
	void ALU_OP(ALU_OP2_INST inst, DstReg dst, SrcReg src0, SrcReg src1, ALU_OMOD omod) {
		ALU_OP2(inst, dst, src0, src1, omod);
	}
	void ALU_OP(ALU_OP3_INST inst, DstReg dst, SrcReg src0, SrcReg src1, SrcReg src2) {
		ALU_OP3(inst, dst, src0, src1, src2);
	}
	void ALU_OP2(ALU_OP2_INST inst, DstReg dst, SrcReg src0, SrcReg src1, ALU_OMOD omod) {
		emitALU(ALU_WORD0(src0.id, 0x0, src0.ch, 0x0, ((src1.id) & ((1 << 13) - 1)), 0x0, src1.ch, 0x0, 0x0, 0x0),
				  ALU_WORD1_OP2(0x0, 0x0, 0x0, 0x0, dst.write_mask, omod, inst, 0x0, 0x0, dst.id, 0x0, dst.ch, 0x0));
	}

	void ALU_OP3(ALU_OP3_INST inst, DstReg dst, SrcReg src0, SrcReg src1, SrcReg src2) {
		_assert_(dst.write_mask);
		emitALU(ALU_WORD0(src0.id, 0x0, src0.ch, 0x0, src1.id, 0x0, src1.ch, 0x0, 0x0, 0x0),
				  ALU_WORD1_OP3(src2.id, 0x0, src2.ch, 0x0, inst, 0x0, dst.id, 0x0, dst.ch, 0x0));
	}
};

class GX2VertexShaderEmitter : public GX2Emitter {
public:
	GX2VertexShaderEmitter() {
		num_gprs = 1;
		CALL_FS(false);
	}
	void EXP_DONE_POS(Reg srcReg, bool barrier = true) { EXP_DONE((ExportType)(POS0 + pos_exports++), srcReg, barrier); }
	void EXP_POS(Reg srcReg, bool barrier = true) { EXP((ExportType)(POS0 + pos_exports++), srcReg, barrier); }
	void EXP_DONE_PARAM(PSInput param, Reg srcReg, bool barrier = true) {
		EXP_DONE((ExportType)(PARAM0 + param_exports.size()), srcReg, barrier);
		param_exports.push_back(param);
	}
	void EXP_PARAM(PSInput param, Reg srcReg, bool barrier = true) {
		EXP((ExportType)(PARAM0 + param_exports.size()), srcReg, barrier);
		param_exports.push_back(param);
	}
	Reg allocReg(VSInput semantic = (VSInput)INVALID_SEMANTIC) { return GX2Emitter::allocReg((u32)semantic); }
	void END_OF_PROGRAM(GX2VertexShader *vs) {
		GX2Emitter::END_OF_PROGRAM();

		/* code */
		vs->size = program.size() * sizeof(*program.data());
		if (vs->program && !(vs->gx2rBuffer.flags & GX2R_RESOURCE_LOCKED_READ_ONLY))
			MEM2_free(vs->program);
		vs->program = (u8 *)MEM2_alloc(vs->size, GX2_SHADER_ALIGNMENT);
		vs->gx2rBuffer.flags = (GX2RResourceFlags)(vs->gx2rBuffer.flags & ~GX2R_RESOURCE_LOCKED_READ_ONLY);
		memcpy(vs->program, program.data(), vs->size);
		GX2Invalidate(GX2_INVALIDATE_MODE_CPU_SHADER, vs->program, vs->size);

		/* regs */
		vs->regs.sq_pgm_resources_vs.num_gprs = num_gprs;
		vs->regs.sq_pgm_resources_vs.stack_size = 1;
		vs->regs.spi_vs_out_config.vs_export_count = param_exports.size() >> 1; /* ? */
		vs->regs.num_spi_vs_out_id = (param_exports.size() + 3) >> 2;
		memset(vs->regs.spi_vs_out_id, 0xFF, sizeof(vs->regs.spi_vs_out_id));
		for (int i = 0; i < param_exports.size(); i++) {
			switch (i & 0x3) {
			case 0: vs->regs.spi_vs_out_id[i >> 2].semantic_0 = (u8)param_exports[i]; break;
			case 1: vs->regs.spi_vs_out_id[i >> 2].semantic_1 = (u8)param_exports[i]; break;
			case 2: vs->regs.spi_vs_out_id[i >> 2].semantic_2 = (u8)param_exports[i]; break;
			case 3: vs->regs.spi_vs_out_id[i >> 2].semantic_3 = (u8)param_exports[i]; break;
			}
		}
		vs->regs.sq_vtx_semantic_clear = ~0u;
		vs->regs.num_sq_vtx_semantic = semantics_.size();

		for (int i = 0; i < countof(vs->regs.sq_vtx_semantic); i++)
			vs->regs.sq_vtx_semantic[i] = 0xFF;
		for (Semantic s : semantics_) {
			vs->regs.sq_vtx_semantic[s.reg - 1] = s.value;
			vs->regs.sq_vtx_semantic_clear &= ~(1 << (s.reg - 1));
		}

		vs->regs.vgt_vertex_reuse_block_cntl.vtx_reuse_depth = 0xE;
		vs->regs.vgt_hos_reuse_depth.reuse_depth = 0x10;
	}

private:
	std::vector<PSInput> param_exports;
	u32 pos_exports = 0;
};
class GX2PixelShaderEmitter : public GX2Emitter {
public:
	GX2PixelShaderEmitter() {}
	Reg allocReg(PSInput semantic = (PSInput)INVALID_SEMANTIC) { return GX2Emitter::allocReg((u32)semantic); }
	void EXP_DONE_PIX(Reg srcReg, bool barrier = true) { EXP_DONE((ExportType)(PIX0 + exports++), srcReg, barrier); }
	void EXP_PIX(Reg srcReg, bool barrier = true) { EXP((ExportType)(PIX0 + exports++), srcReg, barrier); }

	void END_OF_PROGRAM(GX2PixelShader *ps) {
		GX2Emitter::END_OF_PROGRAM();

		/* code */
		ps->size = program.size() * sizeof(*program.data());
		if (ps->program && !(ps->gx2rBuffer.flags & GX2R_RESOURCE_LOCKED_READ_ONLY))
			MEM2_free(ps->program);

		ps->program = (u8 *)MEM2_alloc(ps->size, GX2_SHADER_ALIGNMENT);
		ps->gx2rBuffer.flags = (GX2RResourceFlags)(ps->gx2rBuffer.flags & ~GX2R_RESOURCE_LOCKED_READ_ONLY);
		memcpy(ps->program, program.data(), ps->size);
		GX2Invalidate(GX2_INVALIDATE_MODE_CPU_SHADER, ps->program, ps->size);

		/* regs */
		ps->regs.sq_pgm_resources_ps.num_gprs = num_gprs;
		ps->regs.sq_pgm_resources_ps.stack_size = 0;
		ps->regs.sq_pgm_exports_ps.export_mode = 0x2;
		ps->regs.spi_ps_in_control_0.num_interp = semantics_.size();
		ps->regs.spi_ps_in_control_0.persp_gradient_ena = TRUE;
		ps->regs.spi_ps_in_control_0.baryc_sample_cntl = spi_baryc_cntl_centers_only;
		ps->regs.num_spi_ps_input_cntl = semantics_.size();
		for (Semantic s : semantics_) {
			ps->regs.spi_ps_input_cntls[s.reg].semantic = s.value;
			ps->regs.spi_ps_input_cntls[s.reg].default_val = 1;
		}
		ps->regs.cb_shader_mask.output0_enable = 0xF;
		ps->regs.cb_shader_control.rt0_enable = TRUE;
		ps->regs.db_shader_control.z_order = db_z_order_early_z_then_late_z;
	}

private:
	u32 exports = 0;
};

} // namespace GX2Gen
