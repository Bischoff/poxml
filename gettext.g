
header "pre_include_hpp" {
#include "parser.h"
}

options {
	language="Cpp";
}

{
#include <iostream>
#include "GettextLexer.hpp"
#include "GettextParser.hpp"
#include "antlr/AST.hpp"
#include "antlr/CommonAST.hpp"

/* 
int main()
{
	ANTLR_USING_NAMESPACE(std)
	ANTLR_USING_NAMESPACE(antlr)
	try {
		GettextLexer lexer(cin);
		GettextParser parser(lexer);
		parser.file();

	} catch(exception& e) {
		cerr << "exception: " << e.what() << endl;
	}
}
*/
}

class GettextParser extends Parser;

options {
	codeGenMakeSwitchThreshold = 3;
	codeGenBitsetTestThreshold = 4;
}

file returns [ MsgList ml ]
{
string c, mi, ms;
MsgBlock mb;
MsgList ml2;
}
 : (comment T_MSGID) => (mb=file_block ml2=file { ml = ml2; ml.append(mb); } )
 | (comment EOF) => c=comment { (void)c; }
 ;

file_block returns [ MsgBlock mb ]
{
string c, mi, ms;
}
  : c=comment mi=msgid ms=msgstr {
	(void)c;
	mb.msgid = QString::fromUtf8(mi.c_str());
	mb.msgstr = QString::fromUtf8(ms.c_str());   
  }
  ;

comment returns [string s]
{
string r;
}
 : (c:T_COMMENT r=comment { s = c->getText() + r; } )
 | /* nothing */
 ;

msgid returns [string s]
 : T_MSGID t:T_STRING { s = t->getText(); }
 ;

msgstr returns [string s]
 : T_MSGSTR t:T_STRING { s = t->getText(); }
 ;

class GettextLexer extends Lexer;
options {
        charVocabulary = '\u0000'..'\u00FF';
        testLiterals=false;    // don't automatically test for literals
}

WS 
  : (' ' | '\t' 
  | ('\n' | "\r\n") { newline(); }
  ) { $setType(ANTLR_USE_NAMESPACE(antlr)Token::SKIP); }
  ;

T_COMMENT : '#' (~'\n')*  
          ;

MSG_TAG : "msg" ( "id" { $setType(T_MSGID); }
                | "str" { $setType(T_MSGSTR); }
                )
        ;

T_STRING 
	:	('"'! (ESC|~'"')* ('"'! (' ' | 't')*! '\n'! { newline(); } ))+
	;

// copied from example
protected
ESC	:	'\\'
		(	'n'
		|	'r'
		|	't'
		|	'b'
		|	'f'
		|	'"'
		|	'\''
		|	'\\'
		|	('0'..'3')
			(
				options {
					warnWhenFollowAmbig = false;
				}
			:	('0'..'9')
				(	
					options {
						warnWhenFollowAmbig = false;
					}
				:	'0'..'9'
				)?
			)?
		|	('4'..'7')
			(
				options {
					warnWhenFollowAmbig = false;
				}
			:	('0'..'9')
			)?
		)
	;