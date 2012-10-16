#
# tools\gawk -f scripts\macroopcode.awk macroopcode.hpp
#
BEGIN{
	const["INTERNAL_MACRO_BASE"]      = 0x00080000;
	const["KEY_MACRO_BASE"]           =const["INTERNAL_MACRO_BASE"];
	const["KEY_MACRO_OP_BASE"]        =const["INTERNAL_MACRO_BASE"]+0x0000;     # opcode             0x00080000 - 0x000803FF
	const["KEY_MACRO_C_BASE"]         =const["INTERNAL_MACRO_BASE"]+0x0400;     # булевые условия    0x00080400 - 0x000807FF
	const["KEY_MACRO_V_BASE"]         =const["INTERNAL_MACRO_BASE"]+0x0800;     # разные переменные  0x00080800 - 0x00080BFF
	const["KEY_MACRO_F_BASE"]         =const["INTERNAL_MACRO_BASE"]+0x0C00;     # функции            0x00080C00 -
	const["KEY_MACRO_U_BASE"]         =const["INTERNAL_MACRO_BASE"]+0x8000;     # внешние функции    0x00088000 -
	const["KEY_MACRO_ENDBASE"]        =0x000FFFFF;

	cur_op=0;
}

/\tMCODE_/{
	eq=index($1,"=");
	cm=index($1,",");
	sl=index($0,"/");

	comment="";
	if(sl)
		comment="   " substr($0,sl)

	if (eq > 0)
	{
		cur_op=const[substr($1,eq+1,cm-eq-1)];
		name_op=substr($1,1,eq-1);
	}
	else if(cm > 0)
	{
		cur_op++;
		name_op=substr($1,1,cm-1);
	}

	printf "%-39s = 0x%04X%s\n",name_op,cur_op,comment;
}
