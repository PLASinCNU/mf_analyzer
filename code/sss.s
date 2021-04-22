	.section	__TEXT,__text,regular,pure_instructions
	.macosx_version_min 10, 13
	.globl	_f                      ## -- Begin function f
	.p2align	4, 0x90
_f:                                     ## @f
Lfunc_begin0:
	.file	1 "/Users/plas/rete/code/secVul.c"
	.loc	1 3 0                   ## secVul.c:3:0
	.cfi_startproc
## %bb.0:                               ## %entry
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register %rbp
	subq	$16, %rsp
	movl	%edi, -8(%rbp)
Ltmp0:
	.loc	1 4 6 prologue_end      ## secVul.c:4:6
	movl	$2, -16(%rbp)
	.loc	1 5 13                  ## secVul.c:5:13
	movl	-8(%rbp), %eax
	.loc	1 5 9 is_stmt 0         ## secVul.c:5:9
	movl	%eax, -4(%rbp)
	.loc	1 6 10 is_stmt 1        ## secVul.c:6:10
	movl	-8(%rbp), %eax
	.loc	1 6 12 is_stmt 0        ## secVul.c:6:12
	addl	$6, %eax
	.loc	1 6 6                   ## secVul.c:6:6
	movl	%eax, -12(%rbp)
Ltmp1:
	.loc	1 9 9 is_stmt 1         ## secVul.c:9:9
	movl	-4(%rbp), %eax
	.loc	1 9 10 is_stmt 0        ## secVul.c:9:10
	addl	-12(%rbp), %eax
	.loc	1 9 13                  ## secVul.c:9:13
	cmpl	$10, %eax
Ltmp2:
	.loc	1 9 9                   ## secVul.c:9:9
	jle	LBB0_2
## %bb.1:                               ## %if.then
Ltmp3:
	.loc	1 10 11 is_stmt 1       ## secVul.c:10:11
	movl	$1, -16(%rbp)
	.loc	1 11 43                 ## secVul.c:11:43
	movl	-4(%rbp), %esi
	.loc	1 11 9 is_stmt 0        ## secVul.c:11:9
	leaq	L_.str(%rip), %rdi
	movb	$0, %al
	callq	_printf
Ltmp4:
LBB0_2:                                 ## %if.end
	.loc	1 19 2 is_stmt 1        ## secVul.c:19:2
	xorl	%eax, %eax
	addq	$16, %rsp
	popq	%rbp
	retq
Ltmp5:
Lfunc_end0:
	.cfi_endproc
                                        ## -- End function
	.section	__TEXT,__cstring,cstring_literals
L_.str:                                 ## @.str
	.asciz	"Hello User. k is : %d\n"

	.section	__DWARF,__debug_str,regular,debug
Linfo_string:
	.asciz	"clang version 8.0.0 (trunk 342118)" ## string offset=0
	.asciz	"secVul.c"              ## string offset=35
	.asciz	"/Users/plas/rete/code" ## string offset=44
	.asciz	"f"                     ## string offset=66
	.asciz	"int"                   ## string offset=68
	.asciz	"p"                     ## string offset=72
	.asciz	"i"                     ## string offset=74
	.asciz	"k"                     ## string offset=76
	.asciz	"j"                     ## string offset=78
	.section	__DWARF,__debug_abbrev,regular,debug
Lsection_abbrev:
	.byte	1                       ## Abbreviation Code
	.byte	17                      ## DW_TAG_compile_unit
	.byte	1                       ## DW_CHILDREN_yes
	.byte	37                      ## DW_AT_producer
	.byte	14                      ## DW_FORM_strp
	.byte	19                      ## DW_AT_language
	.byte	5                       ## DW_FORM_data2
	.byte	3                       ## DW_AT_name
	.byte	14                      ## DW_FORM_strp
	.byte	16                      ## DW_AT_stmt_list
	.byte	23                      ## DW_FORM_sec_offset
	.byte	27                      ## DW_AT_comp_dir
	.byte	14                      ## DW_FORM_strp
	.ascii	"\264B"                 ## DW_AT_GNU_pubnames
	.byte	25                      ## DW_FORM_flag_present
	.byte	17                      ## DW_AT_low_pc
	.byte	1                       ## DW_FORM_addr
	.byte	18                      ## DW_AT_high_pc
	.byte	6                       ## DW_FORM_data4
	.byte	0                       ## EOM(1)
	.byte	0                       ## EOM(2)
	.byte	2                       ## Abbreviation Code
	.byte	46                      ## DW_TAG_subprogram
	.byte	1                       ## DW_CHILDREN_yes
	.byte	17                      ## DW_AT_low_pc
	.byte	1                       ## DW_FORM_addr
	.byte	18                      ## DW_AT_high_pc
	.byte	6                       ## DW_FORM_data4
	.byte	64                      ## DW_AT_frame_base
	.byte	24                      ## DW_FORM_exprloc
	.byte	3                       ## DW_AT_name
	.byte	14                      ## DW_FORM_strp
	.byte	58                      ## DW_AT_decl_file
	.byte	11                      ## DW_FORM_data1
	.byte	59                      ## DW_AT_decl_line
	.byte	11                      ## DW_FORM_data1
	.byte	39                      ## DW_AT_prototyped
	.byte	25                      ## DW_FORM_flag_present
	.byte	73                      ## DW_AT_type
	.byte	19                      ## DW_FORM_ref4
	.byte	63                      ## DW_AT_external
	.byte	25                      ## DW_FORM_flag_present
	.byte	0                       ## EOM(1)
	.byte	0                       ## EOM(2)
	.byte	3                       ## Abbreviation Code
	.byte	5                       ## DW_TAG_formal_parameter
	.byte	0                       ## DW_CHILDREN_no
	.byte	2                       ## DW_AT_location
	.byte	24                      ## DW_FORM_exprloc
	.byte	3                       ## DW_AT_name
	.byte	14                      ## DW_FORM_strp
	.byte	58                      ## DW_AT_decl_file
	.byte	11                      ## DW_FORM_data1
	.byte	59                      ## DW_AT_decl_line
	.byte	11                      ## DW_FORM_data1
	.byte	73                      ## DW_AT_type
	.byte	19                      ## DW_FORM_ref4
	.byte	0                       ## EOM(1)
	.byte	0                       ## EOM(2)
	.byte	4                       ## Abbreviation Code
	.byte	52                      ## DW_TAG_variable
	.byte	0                       ## DW_CHILDREN_no
	.byte	2                       ## DW_AT_location
	.byte	24                      ## DW_FORM_exprloc
	.byte	3                       ## DW_AT_name
	.byte	14                      ## DW_FORM_strp
	.byte	58                      ## DW_AT_decl_file
	.byte	11                      ## DW_FORM_data1
	.byte	59                      ## DW_AT_decl_line
	.byte	11                      ## DW_FORM_data1
	.byte	73                      ## DW_AT_type
	.byte	19                      ## DW_FORM_ref4
	.byte	0                       ## EOM(1)
	.byte	0                       ## EOM(2)
	.byte	5                       ## Abbreviation Code
	.byte	36                      ## DW_TAG_base_type
	.byte	0                       ## DW_CHILDREN_no
	.byte	3                       ## DW_AT_name
	.byte	14                      ## DW_FORM_strp
	.byte	62                      ## DW_AT_encoding
	.byte	11                      ## DW_FORM_data1
	.byte	11                      ## DW_AT_byte_size
	.byte	11                      ## DW_FORM_data1
	.byte	0                       ## EOM(1)
	.byte	0                       ## EOM(2)
	.byte	0                       ## EOM(3)
	.section	__DWARF,__debug_info,regular,debug
Lsection_info:
Lcu_begin0:
	.long	128                     ## Length of Unit
	.short	4                       ## DWARF version number
.set Lset0, Lsection_abbrev-Lsection_abbrev ## Offset Into Abbrev. Section
	.long	Lset0
	.byte	8                       ## Address Size (in bytes)
	.byte	1                       ## Abbrev [1] 0xb:0x79 DW_TAG_compile_unit
	.long	0                       ## DW_AT_producer
	.short	12                      ## DW_AT_language
	.long	35                      ## DW_AT_name
.set Lset1, Lline_table_start0-Lsection_line ## DW_AT_stmt_list
	.long	Lset1
	.long	44                      ## DW_AT_comp_dir
                                        ## DW_AT_GNU_pubnames
	.quad	Lfunc_begin0            ## DW_AT_low_pc
.set Lset2, Lfunc_end0-Lfunc_begin0     ## DW_AT_high_pc
	.long	Lset2
	.byte	2                       ## Abbrev [2] 0x2a:0x52 DW_TAG_subprogram
	.quad	Lfunc_begin0            ## DW_AT_low_pc
.set Lset3, Lfunc_end0-Lfunc_begin0     ## DW_AT_high_pc
	.long	Lset3
	.byte	1                       ## DW_AT_frame_base
	.byte	86
	.long	66                      ## DW_AT_name
	.byte	1                       ## DW_AT_decl_file
	.byte	3                       ## DW_AT_decl_line
                                        ## DW_AT_prototyped
	.long	124                     ## DW_AT_type
                                        ## DW_AT_external
	.byte	3                       ## Abbrev [3] 0x43:0xe DW_TAG_formal_parameter
	.byte	2                       ## DW_AT_location
	.byte	145
	.byte	120
	.long	72                      ## DW_AT_name
	.byte	1                       ## DW_AT_decl_file
	.byte	3                       ## DW_AT_decl_line
	.long	124                     ## DW_AT_type
	.byte	4                       ## Abbrev [4] 0x51:0xe DW_TAG_variable
	.byte	2                       ## DW_AT_location
	.byte	145
	.byte	112
	.long	74                      ## DW_AT_name
	.byte	1                       ## DW_AT_decl_file
	.byte	4                       ## DW_AT_decl_line
	.long	124                     ## DW_AT_type
	.byte	4                       ## Abbrev [4] 0x5f:0xe DW_TAG_variable
	.byte	2                       ## DW_AT_location
	.byte	145
	.byte	124
	.long	76                      ## DW_AT_name
	.byte	1                       ## DW_AT_decl_file
	.byte	5                       ## DW_AT_decl_line
	.long	124                     ## DW_AT_type
	.byte	4                       ## Abbrev [4] 0x6d:0xe DW_TAG_variable
	.byte	2                       ## DW_AT_location
	.byte	145
	.byte	116
	.long	78                      ## DW_AT_name
	.byte	1                       ## DW_AT_decl_file
	.byte	6                       ## DW_AT_decl_line
	.long	124                     ## DW_AT_type
	.byte	0                       ## End Of Children Mark
	.byte	5                       ## Abbrev [5] 0x7c:0x7 DW_TAG_base_type
	.long	68                      ## DW_AT_name
	.byte	5                       ## DW_AT_encoding
	.byte	4                       ## DW_AT_byte_size
	.byte	0                       ## End Of Children Mark
	.section	__DWARF,__debug_macinfo,regular,debug
Ldebug_macinfo:
	.byte	0                       ## End Of Macro List Mark
	.section	__DWARF,__apple_names,regular,debug
Lnames_begin:
	.long	1212240712              ## Header Magic
	.short	1                       ## Header Version
	.short	0                       ## Header Hash Function
	.long	1                       ## Header Bucket Count
	.long	1                       ## Header Hash Count
	.long	12                      ## Header Data Length
	.long	0                       ## HeaderData Die Offset Base
	.long	1                       ## HeaderData Atom Count
	.short	1                       ## DW_ATOM_die_offset
	.short	6                       ## DW_FORM_data4
	.long	0                       ## Bucket 0
	.long	177675                  ## Hash in Bucket 0
.set Lset4, LNames0-Lnames_begin        ## Offset in Bucket 0
	.long	Lset4
LNames0:
	.long	66                      ## f
	.long	1                       ## Num DIEs
	.long	42
	.long	0
	.section	__DWARF,__apple_objc,regular,debug
Lobjc_begin:
	.long	1212240712              ## Header Magic
	.short	1                       ## Header Version
	.short	0                       ## Header Hash Function
	.long	1                       ## Header Bucket Count
	.long	0                       ## Header Hash Count
	.long	12                      ## Header Data Length
	.long	0                       ## HeaderData Die Offset Base
	.long	1                       ## HeaderData Atom Count
	.short	1                       ## DW_ATOM_die_offset
	.short	6                       ## DW_FORM_data4
	.long	-1                      ## Bucket 0
	.section	__DWARF,__apple_namespac,regular,debug
Lnamespac_begin:
	.long	1212240712              ## Header Magic
	.short	1                       ## Header Version
	.short	0                       ## Header Hash Function
	.long	1                       ## Header Bucket Count
	.long	0                       ## Header Hash Count
	.long	12                      ## Header Data Length
	.long	0                       ## HeaderData Die Offset Base
	.long	1                       ## HeaderData Atom Count
	.short	1                       ## DW_ATOM_die_offset
	.short	6                       ## DW_FORM_data4
	.long	-1                      ## Bucket 0
	.section	__DWARF,__apple_types,regular,debug
Ltypes_begin:
	.long	1212240712              ## Header Magic
	.short	1                       ## Header Version
	.short	0                       ## Header Hash Function
	.long	1                       ## Header Bucket Count
	.long	1                       ## Header Hash Count
	.long	20                      ## Header Data Length
	.long	0                       ## HeaderData Die Offset Base
	.long	3                       ## HeaderData Atom Count
	.short	1                       ## DW_ATOM_die_offset
	.short	6                       ## DW_FORM_data4
	.short	3                       ## DW_ATOM_die_tag
	.short	5                       ## DW_FORM_data2
	.short	4                       ## DW_ATOM_type_flags
	.short	11                      ## DW_FORM_data1
	.long	0                       ## Bucket 0
	.long	193495088               ## Hash in Bucket 0
.set Lset5, Ltypes0-Ltypes_begin        ## Offset in Bucket 0
	.long	Lset5
Ltypes0:
	.long	68                      ## int
	.long	1                       ## Num DIEs
	.long	124
	.short	36
	.byte	0
	.long	0
	.section	__DWARF,__debug_gnu_pubn,regular,debug
.set Lset6, LpubNames_end0-LpubNames_begin0 ## Length of Public Names Info
	.long	Lset6
LpubNames_begin0:
	.short	2                       ## DWARF Version
.set Lset7, Lcu_begin0-Lsection_info    ## Offset of Compilation Unit Info
	.long	Lset7
	.long	132                     ## Compilation Unit Length
	.long	42                      ## DIE offset
	.byte	48                      ## Kind: FUNCTION, EXTERNAL
	.asciz	"f"                     ## External Name
	.long	0                       ## End Mark
LpubNames_end0:
	.section	__DWARF,__debug_gnu_pubt,regular,debug
.set Lset8, LpubTypes_end0-LpubTypes_begin0 ## Length of Public Types Info
	.long	Lset8
LpubTypes_begin0:
	.short	2                       ## DWARF Version
.set Lset9, Lcu_begin0-Lsection_info    ## Offset of Compilation Unit Info
	.long	Lset9
	.long	132                     ## Compilation Unit Length
	.long	124                     ## DIE offset
	.byte	144                     ## Kind: TYPE, STATIC
	.asciz	"int"                   ## External Name
	.long	0                       ## End Mark
LpubTypes_end0:

.subsections_via_symbols
	.section	__DWARF,__debug_line,regular,debug
Lsection_line:
Lline_table_start0:
