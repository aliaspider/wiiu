
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <wiiu/gx2/shader_disasm.hpp>
#include <wiiu/gx2/shader_inst.hpp>

static const char channels[0x8] = { 'x', 'y', 'z', 'w', '0', '1', '?', '_' };

class TextEmitter {
public:
	void Emit(const char *s, ...);
	void EmitPrefix(const char *s, ...);
	void EmitSuffix(const char *s, ...);
	void Endl();
	void setOutputBuffer(char *dst);

protected:
	static constexpr int indent_width = 3;
	int indent_ = 0;

private:
	bool clauseStart_ = false;
	bool first_ = true;
	char *dst_;
	char *dstStart_;
	void CheckIndent();
};

void TextEmitter::CheckIndent() {
	if (first_) {
		memset(dst_, ' ', indent_ * indent_width);
		dst_ += indent_ * indent_width;
		first_ = false;
	} else if (clauseStart_)
		*dst_++ = ' ';

	*dst_ = '\0';

	clauseStart_ = false;
}

void TextEmitter::setOutputBuffer(char *dst) {
	dstStart_ = dst_ = dst;
	indent_ = 0;
	first_ = true;
	clauseStart_ = false;
}

void TextEmitter::Emit(const char *s, ...) {
	if (!*s)
		return;

	CheckIndent();

	va_list vl;
	va_start(vl, s);
	dst_ += vsprintf(dst_, s, vl);
	va_end(vl);

	clauseStart_ = true;
}

void TextEmitter::EmitPrefix(const char *s, ...) {
	CheckIndent();
	va_list vl;
	va_start(vl, s);
	dst_ += vsprintf(dst_, s, vl);
	va_end(vl);
}

void TextEmitter::EmitSuffix(const char *s, ...) {
	char *dst_org = dst_;
	while (dst_ > dstStart_ && dst_[-1] == ' ')
		dst_--;

	va_list vl;
	va_start(vl, s);
	dst_ += vsprintf(dst_, s, vl);
	va_end(vl);

	if (dst_ < dst_org) {
		*dst_ = ' ';
		dst_ = dst_org;
	}
}

void TextEmitter::Endl() {
	*dst_++ = '\n';
	*dst_ = '\0';
	first_ = true;
}

class GX2ShaderDisassembler : private TextEmitter {
public:
	GX2ShaderDisassembler() {}
	void disassemble(const void *data, u32 size, char *buffer);

private:
	const u64 *index_;
	const u32 *src_;
	const u32 *slotPtr_;
	int cf_cnt_ = 0;
	int groupIndex_ = 0;
	int aluLiterals_ = 0;

	bool CF();
	void TEX(CF_WORD0 w0, CF_WORD1 w1);
	void VTX(CF_WORD0 w0, CF_WORD1 w1);
	void JUMP(CF_WORD0 w0, CF_WORD1 w1);
	void ELSE(CF_WORD0 w0, CF_WORD1 w1);
	void CALL_FS(CF_WORD0 w0, CF_WORD1 w1);
	void EMIT_VERTEX(CF_WORD0 w0, CF_WORD1 w1);

	void CF_EXPORT(CF_ALLOC_EXPORT_WORD0 w0, CF_ALLOC_EXPORT_WORD1 w1);
	void ALU(CF_ALU_WORD0 w0, CF_ALU_WORD1 w1);
	void ALU_OP2(ALU_WORD0 w0, ALU_WORD1_OP2 w1);
	void ALU_OP3(ALU_WORD0 w0, ALU_WORD1_OP3 w1);

	void ALU_SRC(u32 sel, u32 chan, bool neg);
	void ALU_SRC_LITERAL(u32 chan);
};

void GX2ShaderDisassembler::TEX(CF_WORD0 w0, CF_WORD1 w1) {
	Emit("TEX:");
	Emit("ADDR(%i)", w0.addr);
	Emit("CNT(%i)", w1.count + 1);
	if (w1.validPixelMode)
		Emit("VALID_PIX");
	Endl();
	indent_++;
	slotPtr_ = (u32 *)&index_[w0.addr];
	for (int i = 0; i < w1.count + 1; i++) {
		TEX_WORD0 tex0 = *(TEX_WORD0 *)slotPtr_++;
		TEX_WORD1 tex1 = *(TEX_WORD1 *)slotPtr_++;
		TEX_WORD2 tex2 = *(TEX_WORD2 *)slotPtr_++;
		slotPtr_++;

		Emit("%2i", groupIndex_++);
		Emit("  ");
		Emit("%-15s", GetInstName(tex0.inst));

		Emit("R%i.%c%c%c%c,", tex1.dstGPR, channels[tex1.dstSelX], channels[tex1.dstSelY], channels[tex1.dstSelZ],
			  channels[tex1.dstSelW]);
		Emit("R%i.%c%c%c%c,", tex0.srcGPR, channels[tex2.srcSelX], channels[tex2.srcSelY], channels[tex2.srcSelZ],
			  channels[tex2.srcSelW]);
		Emit("t%i,", tex0.resourceID);
		Emit("s%i", tex2.samplerID);
		Endl();
	}
	slotPtr_ = nullptr;
	indent_--;
}
void GX2ShaderDisassembler::VTX(CF_WORD0 w0, CF_WORD1 w1) {
	Emit("VTX");
	if (w1.popCount)
		Emit("POP_CNT(%i)", w1.popCount);
	Emit("ADDR(%i)", w0.addr);
	Endl();
}
void GX2ShaderDisassembler::JUMP(CF_WORD0 w0, CF_WORD1 w1) {
	Emit("JUMP");
	if (w1.popCount)
		Emit("POP_CNT(%i)", w1.popCount);
	Emit("ADDR(%i)", w0.addr);
	Endl();
}
void GX2ShaderDisassembler::ELSE(CF_WORD0 w0, CF_WORD1 w1) {
	Emit("ELSE");
	if (w1.popCount)
		Emit("POP_CNT(%i)", w1.popCount);
	Emit("ADDR(%i)", w0.addr);
	Endl();
}
void GX2ShaderDisassembler::CALL_FS(CF_WORD0 w0, CF_WORD1 w1) {
	Emit("CALL_FS");
	if (!w1.barrier)
		Emit("NO_BARRIER");
	Endl();
}
void GX2ShaderDisassembler::EMIT_VERTEX(CF_WORD0 w0, CF_WORD1 w1) {
	Emit("EMIT_VERTEX");
	Endl();
}

void GX2ShaderDisassembler::ALU_SRC_LITERAL(u32 chan) {
	const u32 *src = slotPtr_ - 2;
	while (!((ALU_WORD0 *)src)->last)
		src += 2;
	src += 2;
	Emit("(0x%08X, %.0f)", src[chan], *(float *)&src[chan]);
	aluLiterals_ = (chan >> 1) + 1;
}
void GX2ShaderDisassembler::ALU_SRC(u32 sel, u32 chan, bool neg) {
	switch (sel) {
	case 0xF4: Emit("1.0_L"); return;
	case 0xF5: Emit("1.0_M"); return;
	case 0xF6: Emit("0.5_L"); return;
	case 0xF7: Emit("0.5_M"); return;
	case 0xF8: Emit("0.0f"); return;
	case 0xF9: Emit("1.0f"); return;
	case 0xFA: Emit("1"); return;
	case 0xFB: Emit("-1"); return;
	case 0xFC: Emit("0.5f"); return;
	case 0xFD: ALU_SRC_LITERAL(chan); return;
	case 0xFE: Emit("PV%i", groupIndex_ - 2); break;
	case 0xFF: Emit("PS%i", groupIndex_ - 2); return;
	default:
		if (neg)
			EmitPrefix("-");
		if (sel < 0x80)
			Emit("R%i", sel);
		else if (sel < 0xA0)
			Emit("KC0[%i]", sel - 0x80);
		else if (sel < 0xC0)
			Emit("KC1[%i]", sel - 0xA0);
		else if (sel < 0x100)
			Emit("UNKNOWN[%i]", sel - 0xC0);
		else
			Emit("C[%i]", sel - 0x100);
	}

	EmitSuffix(".%c", channels[chan]);
}
void GX2ShaderDisassembler::ALU_OP2(ALU_WORD0 w0, ALU_WORD1_OP2 w1) {
	Emit("%-15s", GetInstName(w1.inst));

	switch (w1.omod) {
	case ALU_OMOD::M2: EmitSuffix("x2"); break;
	case ALU_OMOD::M4: EmitSuffix("x4"); break;
	case ALU_OMOD::D2: EmitSuffix("/2"); break;
	default: break;
	}

	if (w1.writeMask)
		Emit("R%i.%c", w1.dstGPR, channels[w1.dstElem]);
	else
		Emit("____", w1.dstGPR, channels[w1.dstElem]);

	int opCnt = GetOperandsCount(w1.inst);
	if (opCnt > 0) {
		EmitSuffix(",");
		ALU_SRC(w0.src0Sel, w0.src0Chan, w0.src0Neg);
	}

	if (opCnt > 1) {
		EmitSuffix(",");
		ALU_SRC(w0.src1Sel, w0.src1Chan, w0.src1Neg);
	}

	if (w1.updateExecuteMask) {
		EmitPrefix("UPDATE_EXEC_MASK");
		switch ((EXECUTE_MASK_OP)w1.omod) {
		default:
		case EXECUTE_MASK_OP::DEACTIVATE: Emit("(DEACTIVATE)"); break;
		case EXECUTE_MASK_OP::BREAK: Emit("(BREAK)"); break;
		case EXECUTE_MASK_OP::CONTINUE: Emit("(CONTINUE)"); break;
		case EXECUTE_MASK_OP::KILL: Emit("(KILL)"); break;
		}
	}
	if (w1.updatePred)
		Emit("UPDATE_PRED");
}
void GX2ShaderDisassembler::ALU_OP3(ALU_WORD0 w0, ALU_WORD1_OP3 w1) {
	Emit("%-15s", GetInstName(w1.inst));
	Emit("R%i.%c,", w1.dstGPR, channels[w1.dstElem]);

	ALU_SRC(w0.src0Sel, w0.src0Chan, w0.src0Neg);
	EmitSuffix(",");
	ALU_SRC(w0.src1Sel, w0.src1Chan, w0.src0Neg);
	EmitSuffix(",");
	ALU_SRC(w1.src2Sel, w1.src2Chan, w0.src0Neg);
}

void GX2ShaderDisassembler::ALU(CF_ALU_WORD0 w0, CF_ALU_WORD1 w1) {
	Emit("%s:", GetInstName(w1.cfInst));
	Emit("ADDR(%i)", w0.addr);
	Emit("CNT(%i)", w1.count + 1);
	if (w0.kcacheMode0)
		Emit("KCACHE0(CB%i:%i-%i)", w0.kcacheBank0, w1.kcacheAddr0 * 16, (w1.kcacheAddr0 + w0.kcacheMode0) * 16 - 1);
	if (w1.kcacheMode1)
		Emit("KCACHE1(CB%i:%i-%i)", w0.kcacheBank1, w1.kcacheAddr1 * 16, (w1.kcacheAddr1 + w1.kcacheMode1) * 16 - 1);
	Endl();
	indent_++;
	slotPtr_ = (u32 *)&index_[w0.addr];
	bool aluFirst = true;
	aluLiterals_ = 0;
	bool units[4] = {};
	for (int i = 0; i < w1.count + 1; i++) {
		ALU_WORD0 alu0 = *(ALU_WORD0 *)slotPtr_++;
		u32 alu1 = *slotPtr_++;
		ALU_WORD1_OP2 alu1op2 = *(ALU_WORD1_OP2 *)&alu1;
		ALU_WORD1_OP3 alu1op3 = *(ALU_WORD1_OP3 *)&alu1;

		if (aluLiterals_) {
			aluLiterals_--;
			continue;
		}

		if (aluFirst) {
			if (i > 0)
				Endl();
			Emit("%2i", groupIndex_++);
			aluFirst = false;
			memset(units, 0, sizeof(units));
		} else
			Emit("  ");

		bool isTrans = (alu1op2.is_op3 ? ALU_OP_IS_TRANS(alu1op3.inst) : ALU_OP_IS_TRANS(alu1op2.inst));

		Emit("%c:", (isTrans || units[alu1op2.dstElem]) ? 't' : channels[alu1op2.dstElem]);
		units[alu1op2.dstElem] = true;

		if (alu1op2.is_op3)
			ALU_OP3(alu0, alu1op3);
		else
			ALU_OP2(alu0, alu1op2);

		if (isTrans) {
			switch (alu1op2.bankSwizzle) {
			default:
			case BankSwizzle::ALU_SCL_210: Emit("SCL_210"); break;
			case BankSwizzle::ALU_SCL_122: Emit("SCL_122"); break;
			case BankSwizzle::ALU_SCL_212: Emit("SCL_212"); break;
			case BankSwizzle::ALU_SCL_221: Emit("SCL_221"); break;
			}

		} else {
			switch (alu1op2.bankSwizzle) {
			default:
			case BankSwizzle::ALU_VEC_012: break;
			case BankSwizzle::ALU_VEC_021: Emit("ALU_VEC_021"); break;
			case BankSwizzle::ALU_VEC_120: Emit("ALU_VEC_120"); break;
			case BankSwizzle::ALU_VEC_102: Emit("ALU_VEC_102"); break;
			case BankSwizzle::ALU_VEC_201: Emit("ALU_VEC_201"); break;
			case BankSwizzle::ALU_VEC_210: Emit("ALU_VEC_210"); break;
			}
		}

		Endl();
		aluFirst = alu0.last;
	}
	slotPtr_ = nullptr;
	indent_--;
}

void GX2ShaderDisassembler::CF_EXPORT(CF_ALLOC_EXPORT_WORD0 w0, CF_ALLOC_EXPORT_WORD1 w1) {
	Emit("%s:", GetInstName(w1.cfInst));
	switch (w0.type) {
	case 0: Emit("PIX%i,", w0.arrayBase); break;
	case 1: Emit("POS%i,", w0.arrayBase - 0x3C); break;
	default: Emit("PARAM%i,", w0.arrayBase); break;
	}

	Emit("R%i.%c%c%c%c", w0.RWGPR, channels[w1.selX], channels[w1.selY], channels[w1.selZ], channels[w1.selW]);
	if (!w1.barrier)
		Emit("NO_BARRIER");
	Endl();
}

bool GX2ShaderDisassembler::CF() {
	CF_WORD0 w0 = *(CF_WORD0 *)src_++;
	CF_WORD1 w1 = *(CF_WORD1 *)src_++;
	Emit("%02u", cf_cnt_++);

	if (w1.instIsAlu)
		ALU(*(CF_ALU_WORD0 *)&w0, *(CF_ALU_WORD1 *)&w1);
	else if (w1.instIsExport)
		CF_EXPORT(*(CF_ALLOC_EXPORT_WORD0 *)&w0, *(CF_ALLOC_EXPORT_WORD1 *)&w1);
	else {
		switch (w1.cfInst) {
		case CF_INST::TEX: TEX(w0, w1); break;
		case CF_INST::VTX: VTX(w0, w1); break;
		case CF_INST::JUMP: JUMP(w0, w1); break;
		case CF_INST::ELSE: ELSE(w0, w1); break;
		case CF_INST::CALL_FS: CALL_FS(w0, w1); break;
		case CF_INST::EMIT_VERTEX: EMIT_VERTEX(w0, w1); break;
		default:
			Emit("unkown CF inst");
			Endl();
			break;
		}
	}
	if (!w1.instIsAlu && w1.endOfProgram) {
		Emit("END_OF_PROGRAM");
		Endl();
	}

	return !w1.instIsAlu && w1.endOfProgram;
}

void GX2ShaderDisassembler::disassemble(const void *data, u32 size, char *buffer) {
#ifdef __BIG_ENDIAN__
	u32 *copy = (u32 *)malloc(size);
	for (int i = 0; i < size >> 2; i++)
		copy[i] = __builtin_bswap32(((const u32 *)data)[i]);
	index_ = (const u64 *)copy;
#else
	index_ = (const u64 *)data;
#endif
	src_ = (const u32 *)index_;
	slotPtr_ = nullptr;
	setOutputBuffer(buffer);
	indent_ = 0;

	cf_cnt_ = 0;
	groupIndex_ = 0;
	while (!CF() && ((u8 *)src_ - (u8 *)index_) < size)
		;
#ifdef __BIG_ENDIAN__
	free(copy);
#endif
}

void DisassembleGX2Shader(const void *data, u32 size, char *buffer) {
	GX2ShaderDisassembler disasm;
	disasm.disassemble(data, size, buffer);
}
