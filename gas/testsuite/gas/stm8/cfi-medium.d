#name: STM8 medium call frame information
#readelf: --debug-dump=frames
#...
  Return address column: 11
#...
  DW_CFA_def_cfa: r5 ofs 2
#...
.*FDE cie=.* pc=00000000..00012520
  DW_CFA_val_expression: r11 \(DW_OP_consts: -1; DW_OP_plus; DW_OP_deref_size: 2\)
  DW_CFA_advance_loc4: 75040 to 00012520
  DW_CFA_def_cfa_offset: 66
#pass
