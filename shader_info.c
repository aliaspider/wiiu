
#include <stdio.h>
#include <wiiu/gx2/shader_info.h>

int GX2VertexShaderInfo(const GX2VertexShader *vs, char *buffer) {
	char *ptr = buffer;
	ptr += sprintf(ptr, "num gprs: %i\n", vs->regs.sq_pgm_resources_vs.num_gprs);
	ptr += sprintf(ptr, "stack size: %i\n", vs->regs.sq_pgm_resources_vs.stack_size);
	ptr += sprintf(ptr, "export count: %i\n", vs->regs.spi_vs_out_config.vs_export_count);
	ptr += sprintf(ptr, "num spi vs out id: %i\n", vs->regs.num_spi_vs_out_id);
	int num_spi_vs_out_id = 0;
	for (int i = 0; i < countof(vs->regs.spi_vs_out_id); i++) {
		if (*(u32 *)&vs->regs.spi_vs_out_id[i] != 0xFFFFFFFF)
			num_spi_vs_out_id = i + 1;
	}
	for (int i = 0; i < num_spi_vs_out_id; i++) {
		ptr += sprintf(ptr, "spi vs out id[%i] : 0x%02X, 0x%02X, 0x%02X, 0x%02X\n", i,
							vs->regs.spi_vs_out_id[i].semantic_0, vs->regs.spi_vs_out_id[i].semantic_1,
							vs->regs.spi_vs_out_id[i].semantic_2, vs->regs.spi_vs_out_id[i].semantic_3);
	}
	ptr += sprintf(ptr, "sq vtx semantic clear : 0x%08X (~0x%08X)\n", vs->regs.sq_vtx_semantic_clear,
						~vs->regs.sq_vtx_semantic_clear);
	ptr += sprintf(ptr, "num sq vtx semantic : %i\n", vs->regs.num_sq_vtx_semantic);
	int num_sq_vtx_semantic = 0;
	for (int i = 0; i < countof(vs->regs.sq_vtx_semantic); i++) {
		if (vs->regs.sq_vtx_semantic[i] != 0xFF)
			num_sq_vtx_semantic = i + 1;
	}
	ptr += sprintf(ptr, "sq vtx semantic : ");
	for (int i = 0; i < num_sq_vtx_semantic; i++) {
		ptr += sprintf(ptr, "0x%02X, ", vs->regs.sq_vtx_semantic[i]);
	}
	ptr += sprintf(ptr, "\n");
	ptr += sprintf(ptr, "vgt vtx reuse_depth : 0x%X\n", vs->regs.vgt_vertex_reuse_block_cntl.vtx_reuse_depth);
	ptr += sprintf(ptr, "vgt hos reuse_depth : 0x%X\n", vs->regs.vgt_hos_reuse_depth.reuse_depth);
	return ptr - buffer;
}

int GX2PixelShaderInfo(const GX2PixelShader *ps, char *buffer) {
	char *ptr = buffer;
	ptr += sprintf(ptr, "num gprs: %i\n", ps->regs.sq_pgm_resources_ps.num_gprs);
	ptr += sprintf(ptr, "stack size: %i\n", ps->regs.sq_pgm_resources_ps.stack_size);

	ptr += sprintf(ptr, "export_mode: %i\n", ps->regs.sq_pgm_exports_ps.export_mode);
	ptr += sprintf(ptr, "num_interp: %i\n", ps->regs.spi_ps_in_control_0.num_interp);
	ptr += sprintf(ptr, "persp_gradient_ena: %s\n", ps->regs.spi_ps_in_control_0.persp_gradient_ena ? "TRUE" : "FALSE");
	ptr += sprintf(ptr, "baryc_sample_cntl: %s\n",
						(ps->regs.spi_ps_in_control_0.baryc_sample_cntl == spi_baryc_cntl_centroids_only)
							? "spi_baryc_cntl_centroids_only"
						: (ps->regs.spi_ps_in_control_0.baryc_sample_cntl == spi_baryc_cntl_centers_only)
							? "spi_baryc_cntl_centers_only"
						: (ps->regs.spi_ps_in_control_0.baryc_sample_cntl == spi_baryc_cntl_centroids_and_centers)
							? "spi_baryc_cntl_centroids_and_centers"
							: "unknown");
	ptr += sprintf(ptr, "num_spi_ps_input_cntl: %i\n", ps->regs.num_spi_ps_input_cntl);
	for (int i = 0; i < ps->regs.num_spi_ps_input_cntl; i++) {
		ptr += sprintf(ptr, "spi_ps_input_cntls[%i].semantic: %i\n", i, ps->regs.spi_ps_input_cntls[i].semantic);
		ptr += sprintf(ptr, "spi_ps_input_cntls[%i].default_val: %i\n", i, ps->regs.spi_ps_input_cntls[i].default_val);
	}
	ptr += sprintf(ptr, "output_enable: %X %X %X %X %X %X %X %X\n",
						ps->regs.cb_shader_mask.output0_enable, ps->regs.cb_shader_mask.output1_enable,
						ps->regs.cb_shader_mask.output2_enable, ps->regs.cb_shader_mask.output3_enable,
						ps->regs.cb_shader_mask.output4_enable, ps->regs.cb_shader_mask.output5_enable,
						ps->regs.cb_shader_mask.output6_enable, ps->regs.cb_shader_mask.output7_enable);
	ptr += sprintf(ptr, "rt_enable:     %i %i %i %i %i %i %i %i\n", ps->regs.cb_shader_control.rt0_enable,
						ps->regs.cb_shader_control.rt1_enable, ps->regs.cb_shader_control.rt2_enable,
						ps->regs.cb_shader_control.rt3_enable, ps->regs.cb_shader_control.rt4_enable,
						ps->regs.cb_shader_control.rt5_enable, ps->regs.cb_shader_control.rt6_enable,
						ps->regs.cb_shader_control.rt7_enable);
	ptr += sprintf(
		ptr, "z_order: %s\n",
		(ps->regs.db_shader_control.z_order == db_z_order_late_z)                ? "db_z_order_late_z"
		: (ps->regs.db_shader_control.z_order == db_z_order_early_z_then_late_z) ? "db_z_order_early_z_then_late_z"
		: (ps->regs.db_shader_control.z_order == db_z_order_re_z)                ? "db_z_order_re_z"
		: (ps->regs.db_shader_control.z_order == db_z_order_early_z_then_re_z)   ? "db_z_order_early_z_then_re_z"
																										 : "unknown");
	ptr += sprintf(ptr, "kill_enable: %s\n", ps->regs.db_shader_control.kill_enable? "TRUE": "FALSE");

	return ptr - buffer;
}
