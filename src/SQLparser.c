/*	PANDA -- a simple transaction monitor

Copyright (C) 2000-2003 Ogochan & JMA (Japan Medical Association).

This module is part of PANDA.

	PANDA is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY.  No author or distributor accepts responsibility
to anyone for the consequences of using it or for whether it serves
any particular purpose or works at all, unless he says so in writing.
Refer to the GNU General Public License for full details. 

	Everyone is granted permission to copy, modify and redistribute
PANDA, but only under the conditions described in the GNU General
Public License.  A copy of this license is supposed to have been given
to you along with PANDA so you can know your rights and
responsibilities.  It should be in a file named COPYING.  Among other
things, the copyright notice and this notice must be preserved on all
copies. 
*/

/*
#define	DEBUG
#define	TRACE
#define	MAIN
*/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<glib.h>
#define		_SQL_PARSER
#include	"types.h"
#include	"value.h"
#include	"misc.h"
#include	"DDlex.h"
#include	"DDparser.h"
#include	"SQLlex.h"
#include	"SQLparser.h"
#include	"debug.h"

#define	MarkCode(lbs)		((lbs)->ptr)
#define	SeekCode(lbs,pos)	((lbs)->ptr = (pos))

static	void
_Error(
	char	*msg,
	char	*fn,
	int		line)
{
	printf("%s:%d:%s\n",fn,line,msg);
	exit(1);
}
#undef	Error
#define	Error(msg)	{fDD_Error=TRUE;_Error((msg),DD_FileName,DD_cLine);}

#define	GetSymbol	(DD_Token = SQL_Lex())

extern	LargeByteString	*
ParSQL(
	RecordStruct	*rec)
{
	LargeByteString	*sql;
	ValueStruct		*val;
	Bool	fInto
	,		fAster;
	size_t	mark
	,		current;
	int		into;
	int		n;
	Bool	fInsert;

dbgmsg(">ParSQL");
	sql = NewLBS();
	GetSymbol;
	fInto = FALSE;
	mark = 0;
	into = 0;
	fAster = FALSE;
	fInsert = FALSE;
	while	( DD_Token != '}' ) {
		switch	(DD_Token) {
		  case	T_SYMBOL:
			if		(  ( val = GetRecordItem(rec->rec,DD_ComSymbol) )  !=  NULL  ) {
				do {
					LBS_EmitString(sql,DD_ComSymbol);
					if		(  GetSymbol  ==  '.'  ) {
						LBS_EmitChar(sql,'_');
						GetSymbol;
					}
				}	while	(  DD_Token  ==  T_SYMBOL  );
			} else {
				LBS_EmitString(sql,DD_ComSymbol);
				GetSymbol;
			}
			LBS_EmitSpace(sql);
			break;
		  case	T_SCONST:
			LBS_EmitChar(sql,'\'');
			LBS_EmitString(sql,DD_ComSymbol);
			LBS_EmitChar(sql,'\'');
			LBS_EmitSpace(sql);
			GetSymbol;
			break;
		  case	T_SQL:
			LBS_EmitString(sql,DD_ComSymbol);
			LBS_EmitSpace(sql);
			GetSymbol;
			break;
		  case	T_LIKE:
		  case	T_ILIKE:
			LBS_EmitString(sql,DD_ComSymbol);
			LBS_EmitSpace(sql);
			LBS_Emit(sql,SQL_OP_VCHAR);
			GetSymbol;
			break;
		  case	T_INSERT:
			LBS_EmitString(sql,DD_ComSymbol);
			LBS_EmitSpace(sql);
			fInsert = TRUE;
			GetSymbol;
			break;
		  case	T_INTO:
			if		(  !fInsert  ) {
				LBS_Emit(sql,SQL_OP_INTO);
				mark = MarkCode(sql);
				into = 0;
				LBS_EmitInt(sql,0);
				fInto = TRUE;
			} else {
				LBS_EmitString(sql,"INTO ");
			}
			GetSymbol;
			break;
		  case	':':
			if		(  fInto  ) {
				LBS_Emit(sql,SQL_OP_STO);
				into ++;
			} else {
				LBS_Emit(sql,SQL_OP_REF);
			}
			if		(  GetSymbol  ==  T_SYMBOL  ) {
				val = rec->rec;
				do {
					val = GetRecordItem(val,DD_ComSymbol);
					if		(  val  ==  NULL  ) {
						Error("item name missing");
					}
					switch	(GetSymbol) {
					  case	'.':
						GetSymbol;
						break;
					  case	'[':
						if		(  val->type  ==  GL_TYPE_ARRAY  ) {
							if		(  GetSymbol  ==  T_SYMBOL  ) {
								n = atoi(DD_ComSymbol) - 1;
								val = GetArrayItem(val,n);
								if		(  GetSymbol  !=  ']'  ) {
									Error("] missing");
								}
							}
						} else {
							Error("not array");
						}
						GetSymbol;
						break;
					  default:
						break;
					}
				}	while	(  DD_Token  ==  T_SYMBOL  );
				LBS_EmitPointer(sql,(void *)val);
				if		(  DD_Token  ==  ','  ) {
					if		(  !fInto  ) {
						LBS_EmitChar(sql,',');
					}
					GetSymbol;
				}
				LBS_EmitSpace(sql);
				fAster = FALSE;
			} else
			if		(  DD_Token  ==  '*'  ) {
				fAster = TRUE;
				GetSymbol;
			}
			break;
		  case	';':
			if		(  fInto  ) {
				current = MarkCode(sql);
				SeekCode(sql,mark);
				if		(  fAster  ) {
					LBS_EmitInt(sql,-1);
				} else {
					LBS_EmitInt(sql,into);
				}
				SeekCode(sql,current);
			}
			LBS_Emit(sql,SQL_OP_EOL);
			GetSymbol;
			fInto = FALSE;
			fInsert = FALSE;
			break;
		  case	'*':
			LBS_EmitChar(sql,'*');
			LBS_EmitSpace(sql);
			GetSymbol;
			break;
		  case	'~':
			LBS_EmitChar(sql,'~');
			LBS_EmitSpace(sql);
			LBS_Emit(sql,SQL_OP_VCHAR);
			GetSymbol;
			break;
		  default:
			LBS_EmitChar(sql,(char)DD_Token);
			GetSymbol;
			break;
		}
	}
	LBS_EmitFix(sql);
dbgmsg("<ParSQL");
	return	(sql);
}

extern	void
SQL_ParserInit(void)
{
	SQL_LexInit();
}

#ifdef	MAIN
#include	"dirs.h"
static	void
DumpPKey(
	KeyStruct	*pkey)
{
	char	***pka
	,		**pk;

	printf("*** dump pkey ***\n");
	pka = pkey->item;
	while	(  *pka  !=  NULL  ) {
		pk = *pka;
		while	(  *pk  !=  NULL  ) {
			printf("[%s]",*pk);
			pk ++;
		}
		printf("\n");
		pka ++;
	}
	
}

static	void
DumpSQL(
	char	*name,
	int		ix,
	PathStruct	*path)
{
	int		c;
	ValueStruct	*val;
	LargeByteString	*sql;

	printf("*** dump SQL ***\n");
	printf("command name = [%s]\n",name);
	sql = path->ops[ix-1];
	if		(  sql  !=  NULL  ) {
		while	(  ( c = FetchByte(sql) )  >=  0  ) {
			if		(  c  < 0x7F  ) {
				printf("%c",c);
			} else {
				switch	(c) {
				  case	SQL_OP_MSB:
					printf("%c",c | 0x80);
					break;
				  case	SQL_OP_INTO:
					printf("$into\n");
					val = (ValueStruct *)FetchPointer(sql);
					DumpValueStruct(val);
					break;
				  case	SQL_OP_REF:
					printf("$ref\n");
					val = (ValueStruct *)FetchPointer(sql);
					DumpValueStruct(val);
					break;
				  case	SQL_OP_EOL:
					printf(";\n");
					break;
				  default:
					break;
				}
			}
		}
	}
}

static	void
DumpPath(
	PathStruct	*path)
{
	printf("*** dump path ***\n");
	printf("path name = [%s]\n",path->name);
	g_hash_table_foreach(path->opHash,(GHFunc)DumpSQL,path);
}

static	void
DumpDB_Struct(
	DB_Struct	*db)
{
	int		i;

	DumpPKey(db->pkey);
	for	( i = 0 ; i < db->pcount ; i ++ ) {
		DumpPath(db->path[i]);
	}
}

extern	int
main(
	int		argc,
	char	**argv)
{
	RecordStruct	*ret;

	DD_ParserInit();
	ret = DD_ParserDataDefines(argv[1]);

	printf("*** dump ***\n");
	DumpValueStruct(ret->rec);
	DumpDB_Struct(ret->opt.db);

	return	(0);
}
#endif