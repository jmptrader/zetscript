#pragma once


#define MAX_EXPRESSION_LENGTH 8192

enum GROUP_TYPE{
	GROUP_0=0, // +,-,||
	GROUP_1, // *,/,==,>,<,<=,>=
	GROUP_2, // &&
	GROUP_3, // !
	MAX_GROUPS
};


enum NODE_TYPE{
	UNKNOWN_NODE=0,
	MAIN_NODE=1,
	PUNCTUATOR_NODE,
	EXPRESSION_NODE,
	KEYWORD_NODE,
	ARGS_DECL_NODE,
	ARGS_PASS_NODE,
	ARRAY_ACCESS_NODE,
	ARRAY_INDEX_NODE,
	ARRAY_OBJECT_NODE,
	FUNCTION_OBJECT_NODE,
	SYMBOL_NODE,
	BODY_NODE,
	GROUP_CASES_NODE,
	CONDITIONAL_NODE,
	PRE_FOR_NODE,
	POST_FOR_NODE,
	CLASS_VAR_COLLECTION_NODE,
	CLASS_FUNCTION_COLLECTION_NODE,
	BASE_CLASS_NODE,
	CALLING_OBJECT_NODE,
	ARRAY_REF_NODE,
	FUNCTION_REF_NODE,
	MAX_NODE_TYPE
};


enum KEYWORD_TYPE{
	UNKNOWN_KEYWORD=0,
	IF_KEYWORD,
	ELSE_KEYWORD,
	FOR_KEYWORD,
	WHILE_KEYWORD,
	VAR_KEYWORD,
	SWITCH_KEYWORD,
	CASE_KEYWORD,
	DEFAULT_KEYWORD,
	BREAK_KEYWORD,
	RETURN_KEYWORD,
	FUNCTION_KEYWORD,
	CLASS_KEYWORD,
	THIS_KEYWORD,
	NEW_KEYWORD,
	DELETE_KEYWORD,
	MAX_KEYWORD
};

enum PUNCTUATOR_TYPE{

	UNKNOWN_PUNCTUATOR=0,

	//--------------------------------
	// First OPERATORS 2 char size


	SHIFT_LEFT_PUNCTUATOR=1, // <<
	SHIFT_RIGHT_PUNCTUATOR, // >>


	LOGIC_AND_PUNCTUATOR, // &&
	LOGIC_OR_PUNCTUATOR, // ||
	LOGIC_EQUAL_PUNCTUATOR, // =
	LOGIC_NOT_EQUAL_PUNCTUATOR, // !=
	LOGIC_GTE_PUNCTUATOR, // >=
	LOGIC_LTE_PUNCTUATOR, // <=

	PRE_INC_PUNCTUATOR, // ++
	PRE_DEC_PUNCTUATOR, // --

	POST_INC_PUNCTUATOR, // ++
	POST_DEC_PUNCTUATOR, // --

	// Then OPERATORS 1 char size
	ADD_PUNCTUATOR, // +
	SUB_PUNCTUATOR, // -
	MUL_PUNCTUATOR, // *
	DIV_PUNCTUATOR, // /
	MOD_PUNCTUATOR, // %

	FIELD_PUNCTUATOR, // .

	ASSIGN_PUNCTUATOR, // =

	BINARY_XOR_PUNCTUATOR, // ^
	BINARY_AND_PUNCTUATOR, // &
	BINARY_OR_PUNCTUATOR, // |

	LOGIC_GT_PUNCTUATOR, // >
	LOGIC_LT_PUNCTUATOR, // <
	LOGIC_NOT_PUNCTUATOR, // !

	INLINE_IF_PUNCTUATOR, // ?
	INLINE_ELSE_PUNCTUATOR, // :


	MAX_OPERATOR_PUNCTUATORS,


	//---------------------------
	// SPECIAL CHARACTERS

	COMA_PUNCTUATOR=1,
	SEMICOLON_PUNCTUATOR,

	OPEN_PARENTHESIS_PUNCTUATOR,
	CLOSE_PARENTHESIS_PUNCTUATOR,

	OPEN_BRAKET_PUNCTUATOR,
	CLOSE_BRAKET_PUNCTUATOR,

	OPEN_SQUARE_BRAKET_PUNCTUATOR,
	CLOSE_SQUARE_BRAKET_PUNCTUATOR,

	MAX_SPECIAL_PUNCTUATORS


};





class tASTNode;
class CScope;
typedef tASTNode *PASTNode;

typedef struct{
	KEYWORD_TYPE id;
	const char *str;
	char * (* parse_fun )(const char *,int & ,  CScriptFunction *, PASTNode *);
}tInfoKeyword;

typedef struct{
	PUNCTUATOR_TYPE id;
	const char *str;
	bool (* parse_fun )(const char *);
}tInfoPunctuator;


enum{
	LEFT_NODE=0,
	RIGHT_NODE
};



class tASTNode{

	void destroyChildren_Recursive(PASTNode _node){

		if(_node != NULL){

			for(unsigned i = 0; i < _node->children.size(); i++){
				if(_node->children[i]!= NULL){
					destroyChildren_Recursive(_node->children[i]);
				}
			}

			/*if(_node->keyword_info!=NULL){
				print_info_cr("deallocating %s ",_node->keyword_info->str);
			}else if(_node->operator_info!=NULL){
				print_info_cr("deallocating %s ",_node->operator_info->str);
			}
			else{
				print_info_cr("deallocating %s ",_node->value_symbol.c_str());
			}*/

			_node->children.clear();
			delete _node;
			_node = NULL;
		}

	}

	void destroyChildren(){
		for(unsigned i = 0; i < children.size(); i++){
			destroyChildren_Recursive(children[i]);
		}
		children.clear();
	}

public:

	NODE_TYPE node_type;
	tInfoKeyword *keyword_info;
	tInfoPunctuator *operator_info,*pre_post_operator_info;
	string 	value_symbol;
	string type_ptr;
	CScope *scope_ptr; // saves scope info ptr (only for global vars).
	string type_class;
	int definedValueline;
	PASTNode parent;
	vector<PASTNode> children; //left,right;
	void *aux_value;

	tASTNode(int preallocate_num_nodes=0){
		node_type = UNKNOWN_NODE;
		keyword_info = NULL;
		pre_post_operator_info = NULL;
		definedValueline=-1;
		operator_info=NULL;
		value_symbol="";
		parent=NULL;
		aux_value = NULL;
		scope_ptr = NULL;

		if(preallocate_num_nodes > 0){
			for(int i = 0; i < preallocate_num_nodes; i++){
				children.push_back(NULL);
			}
		}
	}

	~tASTNode(){
		destroyChildren();
	}

};

class CAst{
public:

	static tInfoKeyword defined_keyword[MAX_KEYWORD];
	static tInfoPunctuator defined_operator_punctuator[MAX_OPERATOR_PUNCTUATORS];
	static tInfoPunctuator defined_special_punctuator[MAX_SPECIAL_PUNCTUATORS];

	static void createSingletons();
	static void destroySingletons();

	static tInfoKeyword * isKeyword(const char *c);


	/**
	 * Given starting char, try find an expression ended with ; and return new char pointer.
	 * @s: current char
	 * @m_line: current line
	 * @node: nodes
	 */
	static bool generateAST(const char *s, CScriptFunction *sf, PASTNode *ast_node_to_be_evaluated);



private:


	// string generic utils...
	static char *getSymbolName(const char *s,int & m_startLine);
	static char * getEndWord(const char *s, int m_line);


	//static PASTNode preNodePunctuator(tInfoPunctuator * punctuator,PASTNode affected_op);
	//static PASTNode postOperator(tInfoPunctuator * punctuator,PASTNode affected_op);

	static bool printErrorUnexpectedKeywordOrPunctuator(const char *current_string_ptr, int m_line);

	// Punctuators...
	static bool parsePlusPunctuator(const char *s);
	static bool parseMinusPunctuator(const char *s);
	static bool parseMulPunctuator(const char *s);
	static bool parseDivPunctuator(const char *s);
	static bool parseModPunctuator(const char *s);

	static bool parseFieldPunctuator(const char *s);
	static bool parseInlineIfPunctuator(const char *s);
	static bool parseInlineElsePunctuator(const char *s);

	static bool parseAssignPunctuator(const char *s);

	static bool parseBinaryXorPunctuator(const char *s);
	static bool parseBinaryAndPunctuator(const char *s);
	static bool parseBinaryOrPunctuator(const char *s);
	static bool parseShiftLeftPunctuator(const char *s);
	static bool parseShiftRightPunctuator(const char *s);

	static bool parseLogicAndPunctuator(const char *s);
	static bool parseLogicOrPunctuator(const char *s);
	static bool parseLogicEqualPunctuator(const char *s);
	static bool parseLogicNotEqualPunctuator(const char *s);
	static bool parseLogicGreatherThanPunctuator(const char *s);
	static bool parseLogicLessThanPunctuator(const char *s);
	static bool parseLogicGreatherEqualThanPunctuator(const char *s);
	static bool parseLessEqualThanPunctuator(const char *s);
	static bool parseNotPunctuator(const char *s);


	static tInfoPunctuator *checkPreOperatorPunctuator(const char *s);
	static tInfoPunctuator *checkPostOperatorPunctuator(const char *s);

	static bool parseIncPunctuator(const char *s);
	static bool parseDecPunctuator(const char *s);



	static tInfoPunctuator * parsePunctuatorGroup0(const char *s);
	static tInfoPunctuator * parsePunctuatorGroup1(const char *s);
	static tInfoPunctuator * parsePunctuatorGroup2(const char *s);
	static tInfoPunctuator * parsePunctuatorGroup3(const char *s);

	static tInfoPunctuator *  isOperatorPunctuator(const char *s);
	static tInfoPunctuator *  isSpecialPunctuator(const char *s);
	static tInfoPunctuator * isPunctuator(const char *s);


	// AST core functions ...
	static char * generateAST_Recursive(const char *s, int & m_line, CScriptFunction *sf, bool & error, PASTNode *node_to_be_evaluated=NULL, bool allow_breaks = false);
	static char * parseExpression(const char *s, int & m_line, CScriptFunction *sf, PASTNode * ast_node_to_be_evaluated=NULL);
	static char * parseExpression_Recursive(const char *s, int & m_line, CScriptFunction *sf, PASTNode *ast_node_to_be_evaluated=NULL,GROUP_TYPE type_group=GROUP_TYPE::GROUP_0,PASTNode parent=NULL);

	/**
	 * this functions tries to evaluate expression that was get from getSymbolValue and didn't know as trivial expression like (), function(), etc.
	 * Must be evaluated later with this function.
	 */
	static char *   deduceExpression(const char *str, int & m_line, CScriptFunction *sf, PASTNode *ast_node_to_be_evaluated=NULL, PASTNode parent=NULL);


	// parse block { }
	static char * parseBlock(const char *s,int & m_line,  CScriptFunction *sf, bool & error, PASTNode *ast_node_to_be_evaluated=NULL, bool push_scope=true);


	// keyword...

	static char * parseKeyWord(const char *s, int & m_start_line, CScriptFunction *sf, bool & error, PASTNode *ast_node_to_be_evaluated=NULL);

	static char * parseIf(const char *s,int & m_line,  CScriptFunction *sf, PASTNode *ast_node_to_be_evaluated=NULL);
	static char * parseFor(const char *s,int & m_line,  CScriptFunction *sf, PASTNode *ast_node_to_be_evaluated=NULL);
	static char * parseWhile(const char *s,int & m_line,  CScriptFunction *sf, PASTNode *ast_node_to_be_evaluated=NULL);
	static char * parseSwitch(const char *s,int & m_line,  CScriptFunction *sf, PASTNode *ast_node_to_be_evaluated=NULL);
	static char * parseVar(const char *s,int & m_line,  CScriptFunction *sf, PASTNode *ast_node_to_be_evaluated=NULL);
	static char * parseReturn(const char *s,int & m_line,  CScriptFunction *sf, PASTNode *ast_node_to_be_evaluated=NULL);
	static char * parseFunction(const char *s,int & m_line,  CScriptFunction *sf, PASTNode *ast_node_to_be_evaluated=NULL);
	static char * parseNew(const char *s,int & m_line,  CScriptFunction *sf, PASTNode *ast_node_to_be_evaluated=NULL);
	static char * parseDelete(const char *s,int & m_line,  CScriptFunction *sf, PASTNode *ast_node_to_be_evaluated=NULL);

	static char * parseClass(const char *s,int & m_line,  CScriptFunction *sf, PASTNode *ast_node_to_be_evaluated=NULL);


	static char * parseArgs(char c1, char c2,const char *s,int & m_line,  CScriptFunction *sf, PASTNode *ast_node_to_be_evaluated=NULL);
	static bool isMarkEndExpression(char c);

	/**
	 * Try to get symbol. It can be trivial (i.e values or names) or not trivial ( inline functions, if-else, etc). At the end, the parse will perform to
	 * parse non-trivial symbols with a special function.
	 */
	static char * getSymbolValue(
			const char *current_string_ptr,
			int & m_line,
			CScriptFunction *sf,
			string & symbol_name,
			int & m_definedSymbolLine,
			tInfoPunctuator *pre_operator,
			tInfoPunctuator **post_operator,
			bool & is_symbol_trivial
	);

};

