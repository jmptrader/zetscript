/*
 *  This file is distributed under the MIT License.
 *  See LICENSE file for details.
 */

#include "CZetScript.h"


#define MAX_EXPRESSION_LENGHT 2096

#ifdef  __ZETSCRIPT_VERBOSE_MESSAGE__

#define print_ast_cr zs_print_debug_cr
#else
#define print_ast_cr(s,...)
#endif

namespace zetscript{


	void  		writeErrorMsg(const char *filename, int line, const  char  *string_text, ...);

	tKeywordInfo_Old CASTNode::defined_keyword[MAX_KEYWORD];
	tDirectiveInfo CASTNode::defined_directive[MAX_DIRECTIVES];
	tPunctuatorInfo CASTNode::defined_operator_punctuator[MAX_PUNCTUATORS];
	int CASTNode::DUMMY_LINE=0;
	const char * CASTNode::current_parsing_filename=DEFAULT_NO_FILENAME;
	int CASTNode::current_idx_parsing_filename=-1;

	vector<tInfoAstNodeToCompile> * CASTNode::astNodeToCompile=NULL;

	bool IS_SINGLE_COMMENT(char *str);
	bool IS_START_COMMENT(char *str);
	bool IS_END_COMMENT(char *str);
	char *ADVANCE_TO_END_COMMENT(char *aux_p, int &m_line);
	char *IGNORE_BLANKS(const char *str, int &m_line);


	vector<CASTNode *> 			* CASTNode::vec_ast_node=NULL;

	void CASTNode::setVectorASTNode(vector<CASTNode *> 	* set_vec_ast_node){
		vec_ast_node = set_vec_ast_node;
	}

	vector<CASTNode *> 		*		CASTNode::getVectorASTNode(){
		return vec_ast_node;
	}

	CASTNode	*CASTNode::newASTNode(){

		if(vec_ast_node->size() >= MAX_AST_NODES){
			THROW_RUNTIME_ERROR("Max AST Nodes reached (%i)",CZetScriptUtils::intToString(MAX_AST_NODES));
			return NULL;
		}

		CASTNode	*ast_node = new CASTNode();
		vec_ast_node->push_back(ast_node);
		ast_node->idxAstNode = (short)(vec_ast_node->size()-1);
		ast_node->idxFilename =current_idx_parsing_filename;
		return ast_node;
	}

	/**
	 * Get CASTNode Node by its idx, regarding current state.
	 */
	CASTNode 		* CASTNode::getAstNode(short idx){
		if(idx==ZS_UNDEFINED_IDX){
			return NULL;
		}

		if(idx < 0 || (unsigned)idx >= vec_ast_node->size()){
			THROW_RUNTIME_ERROR("Ast node out of bound");
			return NULL;
		}
		 return vec_ast_node->at(idx);
	}

	int 	CASTNode::getScopeIdx(short idx){
		if(idx==ZS_UNDEFINED_IDX){
			return ZS_UNDEFINED_IDX;
		}

		if(idx < 0 || (unsigned)idx >= vec_ast_node->size()){
			THROW_RUNTIME_ERROR("Ast node out of bound");
			return ZS_UNDEFINED_IDX;
		}

		return vec_ast_node->at(idx)->idxScope;
	}

	CScope * 	CASTNode::getScope(short idx){
		if(idx==ZS_UNDEFINED_IDX){
			return NULL;
		}

		if(idx < 0 || (unsigned)idx >= vec_ast_node->size()){
			THROW_RUNTIME_ERROR("Ast node out of bound");
			return NULL;
		}

		return SCOPE_NODE(vec_ast_node->at(idx)->idxScope);
	}

	int CASTNode::findConstructorIdxNode(short idxAstNode){

		PASTNode _node=AST_NODE(idxAstNode);

		if(_node->node_type!=NODE_TYPE::ARGS_PASS_NODE) {THROW_RUNTIME_ERROR("children[0] is not args_pass_node");return ZS_UNDEFINED_IDX;}
		for(unsigned i = 0; i < _node->children.size(); i++){
			PASTNode child_node = AST_NODE(_node->children[i]);
			if(child_node->node_type == NODE_TYPE::KEYWORD_NODE){

				if(child_node->keyword_info==KEYWORD_TYPE::FUNCTION_KEYWORD){
					if(child_node->symbol_value == _node->symbol_value){
						return i;
					}
				}
			}
		}
		return ZS_UNDEFINED_IDX;
	}

	PASTNode CASTNode::itHasReturnSymbol(PASTNode _node){

		PASTNode _ret;
		if(_node == NULL) return NULL;
		if(_node->keyword_info == RETURN_KEYWORD) return _node;

		for(unsigned i = 0; i < _node->children.size(); i++){
			if((_ret = itHasReturnSymbol(AST_NODE(_node->children[i]))) != NULL){
				return _ret;
			}
		}
		return NULL;//itHasReturnSymbol(PASTNode _node);
	}

	void CASTNode::destroySingletons(){
		if(astNodeToCompile != NULL){
			delete astNodeToCompile;
			astNodeToCompile=NULL;
		}
	}

	int 		CASTNode::getAstLine(short idx){
		if(idx==ZS_UNDEFINED_IDX){
			return -1;
		}

		if(idx < 0 || (unsigned)idx >= vec_ast_node->size()){
			THROW_RUNTIME_ERROR("Ast node out of bound");
			return -1;
		}
		return vec_ast_node->at(idx)->line_value;
	}

	const char *	CASTNode::getAstFilename(short idx){
		if(idx==ZS_UNDEFINED_IDX){
			return DEFAULT_NO_FILENAME;
		}

		if(idx < 0 || (unsigned)idx >= vec_ast_node->size()){
			THROW_RUNTIME_ERROR(DEFAULT_NO_FILENAME);
			return "";
		}

		if(vec_ast_node->at(idx)->idxFilename != -1){
			return CZetScript::getInstance()->getParsedFilenameFromIdx(vec_ast_node->at(idx)->idxFilename);
		}

		return DEFAULT_NO_FILENAME;
	}

	const char * CASTNode::getAstSymbolName(short idx){
		if(idx==ZS_UNDEFINED_IDX){
			return "undefined symbol";
		}

		if(idx < 0 || (unsigned)idx >= vec_ast_node->size()){
			THROW_RUNTIME_ERROR("Ast node out of bound");
			return "";
		}
		return vec_ast_node->at(idx)->symbol_value.c_str();
	}

	const char 		* CASTNode::getAstSymbolNameConstChar(short idx){
		if(idx==ZS_UNDEFINED_IDX){
			return "undefined symbol";
		}

		if(idx < 0 || (unsigned)idx >= vec_ast_node->size()){
			THROW_RUNTIME_ERROR("Ast node out of bound");
			return NULL;
		}

		return vec_ast_node->at(idx)->symbol_value.c_str();
	}

	bool CASTNode::parsePlusPunctuator(const char *s){
		if(*s=='+')
			return ((*(s+1) != '+') && (*(s+1) != '='));
		return false;
	}

	bool CASTNode::parseMinusPunctuator(const char *s){
		if(*s=='-')
			return ((*(s+1) != '-') && (*(s+1) != '='));
		return false;
	}

	bool CASTNode::parseMulPunctuator(const char *s){
		if(*s == '*')
			return ((*(s+1) != '='));
		return false;
	}

	bool CASTNode::parseDivPunctuator(const char *s){
		if(*s == '/')
			return ((*(s+1) != '='));
		return false;
	}

	bool CASTNode::parseModPunctuator(const char *s){
		if(*s == '%')
			return ((*(s+1) != '='));
		return false;
	}

	bool CASTNode::parseFieldPunctuator(const char *s){
		return *s == '.';
	}

	bool CASTNode::parseInlineIfPunctuator(const char *s){
		return *s == '?';
	}

	bool CASTNode::parseInlineElsePunctuator(const char *s){
		return *s == ':';
	}

	bool CASTNode::parseAssignPunctuator(const char *s){
		if(*s=='=')
			return (*(s+1) != '=');
		return false;
	}

	bool CASTNode::parseAddAssignPunctuator(const char *s){
		if(*s=='+')
			return (*(s+1) == '=');
		return false;
	}

	bool CASTNode::parseSubAssignPunctuator(const char *s){
		if(*s=='-')
			return (*(s+1) == '=');
		return false;
	}

	bool CASTNode::parseMulAssignPunctuator(const char *s){
		if(*s=='*')
			return (*(s+1) == '=');
		return false;
	}

	bool CASTNode::parseDivAssignPunctuator(const char *s){
		if(*s=='/')
			return (*(s+1) == '=');
		return false;
	}

	bool CASTNode::parseModAssignPunctuator(const char *s){
		if(*s=='%')
			return (*(s+1) == '=');
		return false;
	}


	bool CASTNode::parseBinaryXorPunctuator(const char *s){
		if(*s == '^')
			return ((*(s+1) != '='));
		return false;
	}

	bool CASTNode::parseBinaryAndPunctuator(const char *s){
		if(*s=='&')
			return ((*(s+1) != '&')  && (*(s+1) != '='));
		return false;
	}

	bool CASTNode::parseBinaryOrPunctuator(const char *s){
		if(*s=='|')
			return ((*(s+1) != '|') &&  (*(s+1) != '='));
		return false;
	}

	bool CASTNode::parseShiftLeftPunctuator(const char *s){
		if(*s=='<')
			return (*(s+1) == '<');
		return false;
	}

	bool CASTNode::parseShiftRightPunctuator(const char *s){
		if(*s=='>')
			return (*(s+1) == '>');
		return false;
	}

	bool CASTNode::parseLogicAndPunctuator(const char *s){
		if(*s=='&')
			return (*(s+1) == '&');
		return false;
	}

	bool CASTNode::parseLogicOrPunctuator(const char *s){
		if(*s=='|')
			return (*(s+1) == '|');
		return false;
	}

	bool CASTNode::parseLogicEqualPunctuator(const char *s){
		if(*s=='=')
			return (*(s+1) == '=');
		return false;
	}

	bool CASTNode::parseInstanceOfPunctuator(const char *s){
		return strncmp("instanceof",s,10) == 0;
	}

	bool CASTNode::parseLogicNotEqualPunctuator(const char *s){
		if(*s=='!')
			return (*(s+1) == '=');
		return false;
	}

	bool CASTNode::parseLogicGreatherThanPunctuator(const char *s){
		if( *s == '>')
			return (*(s+1) != '>');
		return false;
	}

	bool CASTNode::parseLogicLessThanPunctuator(const char *s){
		if(*s == '<')
			return (*(s+1) != '<');

		return false;
	}

	bool CASTNode::parseLogicGreatherEqualThanPunctuator(const char *s){
		if(*s=='>')
			return (*(s+1) == '=');
		return false;
	}

	bool CASTNode::parseLessEqualThanPunctuator(const char *s){
		if(*s=='<')
			return (*(s+1) == '=');
		return false;
	}

	bool CASTNode::parseNotPunctuator(const char *s){
		if(*s=='!')
			return (*(s+1) != '=');
		return false;
	}


	bool CASTNode::parseIncPunctuator(const char *s){
		if(*s=='+')
			return (*(s+1) == '+');
		return false;
	}

	bool CASTNode::parseDecPunctuator(const char *s){
		if(*s=='-')
			return (*(s+1) == '-');
		return false;
	}


	__PUNCTUATOR_TYPE_OLD__  CASTNode::parsePunctuatorGroup0(const char *s){

		__PUNCTUATOR_TYPE_OLD__ index_to_evaluate[]={
				ASSIGN_PUNCTUATOR,
				ADD_ASSIGN_PUNCTUATOR,
				SUB_ASSIGN_PUNCTUATOR,
				MUL_ASSIGN_PUNCTUATOR,
				DIV_ASSIGN_PUNCTUATOR,
				MOD_ASSIGN_PUNCTUATOR
		};

		for(unsigned char  i = 0; i < ARRAY_LENGTH(index_to_evaluate); i++){

			if(defined_operator_punctuator[index_to_evaluate[i]].eval_fun == NULL){
				THROW_RUNTIME_ERROR("internal: %s not have parse function",defined_operator_punctuator[index_to_evaluate[i]].str);
				return __PUNCTUATOR_TYPE_OLD__::UNKNOWN_PUNCTUATOR;
			}

			if(defined_operator_punctuator[index_to_evaluate[i]].eval_fun(s)){
				return defined_operator_punctuator[index_to_evaluate[i]].id;
			}
		}

		return __PUNCTUATOR_TYPE_OLD__::UNKNOWN_PUNCTUATOR;
	}

	__PUNCTUATOR_TYPE_OLD__  CASTNode::parsePunctuatorGroup1(const char *s){

		__PUNCTUATOR_TYPE_OLD__ index_to_evaluate[]={
				TERNARY_IF_PUNCTUATOR,
				TERNARY_ELSE_PUNCTUATOR
		};

		for(unsigned char  i = 0; i < ARRAY_LENGTH(index_to_evaluate); i++){

			if(defined_operator_punctuator[index_to_evaluate[i]].eval_fun == NULL){
				THROW_RUNTIME_ERROR("internal: %s not have parse function",defined_operator_punctuator[index_to_evaluate[i]].str);
				return __PUNCTUATOR_TYPE_OLD__::UNKNOWN_PUNCTUATOR;
			}

			if(defined_operator_punctuator[index_to_evaluate[i]].eval_fun(s)){
				return defined_operator_punctuator[index_to_evaluate[i]].id;
			}
		}

		return __PUNCTUATOR_TYPE_OLD__::UNKNOWN_PUNCTUATOR;
	}

	__PUNCTUATOR_TYPE_OLD__  CASTNode::parsePunctuatorGroup2(const char *s){

		__PUNCTUATOR_TYPE_OLD__ index_to_evaluate[]={

				LOGIC_AND_PUNCTUATOR,
				LOGIC_OR_PUNCTUATOR
		};

		for(unsigned char  i = 0; i < ARRAY_LENGTH(index_to_evaluate); i++){

			if(defined_operator_punctuator[index_to_evaluate[i]].eval_fun == NULL){
				THROW_RUNTIME_ERROR("internal: %s not have parse function",defined_operator_punctuator[index_to_evaluate[i]].str);
				return __PUNCTUATOR_TYPE_OLD__::UNKNOWN_PUNCTUATOR;
			}

			if(defined_operator_punctuator[index_to_evaluate[i]].eval_fun(s)){
				return defined_operator_punctuator[index_to_evaluate[i]].id;
			}
		}
		return __PUNCTUATOR_TYPE_OLD__::UNKNOWN_PUNCTUATOR;
	}

	__PUNCTUATOR_TYPE_OLD__  CASTNode::parsePunctuatorGroup3(const char *s){

		__PUNCTUATOR_TYPE_OLD__ index_to_evaluate[]={

				LOGIC_EQUAL_PUNCTUATOR,
				LOGIC_LTE_PUNCTUATOR,
				LOGIC_GTE_PUNCTUATOR,
				LOGIC_GT_PUNCTUATOR,
				LOGIC_LT_PUNCTUATOR,
				INSTANCEOF_PUNCTUATOR
		};

		for(unsigned char  i = 0; i < ARRAY_LENGTH(index_to_evaluate); i++){

			if(defined_operator_punctuator[index_to_evaluate[i]].eval_fun == NULL){
				THROW_RUNTIME_ERROR("internal: %s not have parse function",defined_operator_punctuator[index_to_evaluate[i]].str);
				return __PUNCTUATOR_TYPE_OLD__::UNKNOWN_PUNCTUATOR;
			}

			if(defined_operator_punctuator[index_to_evaluate[i]].eval_fun(s)){
				return defined_operator_punctuator[index_to_evaluate[i]].id;
			}
		}
		return __PUNCTUATOR_TYPE_OLD__::UNKNOWN_PUNCTUATOR;
	}

	__PUNCTUATOR_TYPE_OLD__  CASTNode::parsePunctuatorGroup4(const char *s){

		__PUNCTUATOR_TYPE_OLD__ index_to_evaluate[]={
				ADD_PUNCTUATOR,
				SUB_PUNCTUATOR,
				BINARY_XOR_PUNCTUATOR,
				BINARY_AND_PUNCTUATOR,
				BINARY_OR_PUNCTUATOR,
				SHIFT_LEFT_PUNCTUATOR,
				SHIFT_RIGHT_PUNCTUATOR
		};

		for(unsigned char  i = 0; i < ARRAY_LENGTH(index_to_evaluate); i++){

			if(defined_operator_punctuator[index_to_evaluate[i]].eval_fun == NULL){
				THROW_RUNTIME_ERROR("internal: %s not have parse function",defined_operator_punctuator[index_to_evaluate[i]].str);
				return __PUNCTUATOR_TYPE_OLD__::UNKNOWN_PUNCTUATOR;
			}

			if(defined_operator_punctuator[index_to_evaluate[i]].eval_fun(s)){
				return defined_operator_punctuator[index_to_evaluate[i]].id;
			}
		}

		return __PUNCTUATOR_TYPE_OLD__::UNKNOWN_PUNCTUATOR;
	}

	__PUNCTUATOR_TYPE_OLD__  CASTNode::parsePunctuatorGroup5(const char *s){

		__PUNCTUATOR_TYPE_OLD__ index_to_evaluate[]={
				LOGIC_NOT_EQUAL_PUNCTUATOR
		};

		for(unsigned char  i = 0; i < ARRAY_LENGTH(index_to_evaluate); i++){

			if(defined_operator_punctuator[index_to_evaluate[i]].eval_fun == NULL){
				THROW_RUNTIME_ERROR("internal: %s not have parse function",defined_operator_punctuator[index_to_evaluate[i]].str);
				return __PUNCTUATOR_TYPE_OLD__::UNKNOWN_PUNCTUATOR;
			}

			if(defined_operator_punctuator[index_to_evaluate[i]].eval_fun(s)){
				return defined_operator_punctuator[index_to_evaluate[i]].id;
			}
		}

		return __PUNCTUATOR_TYPE_OLD__::UNKNOWN_PUNCTUATOR;
	}

	__PUNCTUATOR_TYPE_OLD__ CASTNode::parsePunctuatorGroup6(const char *s){

		__PUNCTUATOR_TYPE_OLD__ index_to_evaluate[]={

				MUL_PUNCTUATOR,
				DIV_PUNCTUATOR,
				MOD_PUNCTUATOR
		};

		for(unsigned char  i = 0; i < ARRAY_LENGTH(index_to_evaluate); i++){

			if(defined_operator_punctuator[index_to_evaluate[i]].eval_fun == NULL){
				THROW_RUNTIME_ERROR("internal: %s not have parse function",defined_operator_punctuator[index_to_evaluate[i]].str);
				return __PUNCTUATOR_TYPE_OLD__::UNKNOWN_PUNCTUATOR;
			}

			if(defined_operator_punctuator[index_to_evaluate[i]].eval_fun(s)){
				return defined_operator_punctuator[index_to_evaluate[i]].id;
			}
		}
		return __PUNCTUATOR_TYPE_OLD__::UNKNOWN_PUNCTUATOR;
	}

	__PUNCTUATOR_TYPE_OLD__  CASTNode::parsePunctuatorGroup7(const char *s){

		__PUNCTUATOR_TYPE_OLD__ index_to_evaluate[]={
				LOGIC_NOT_PUNCTUATOR

		};

		for(unsigned char  i = 0; i < ARRAY_LENGTH(index_to_evaluate); i++){
			if(defined_operator_punctuator[index_to_evaluate[i]].eval_fun == NULL){
				THROW_RUNTIME_ERROR("internal: %s not have parse function",defined_operator_punctuator[index_to_evaluate[i]].str);
				return __PUNCTUATOR_TYPE_OLD__::UNKNOWN_PUNCTUATOR;
			}

			if(defined_operator_punctuator[index_to_evaluate[i]].eval_fun(s)){
				return defined_operator_punctuator[index_to_evaluate[i]].id;
			}
		}
		return __PUNCTUATOR_TYPE_OLD__::UNKNOWN_PUNCTUATOR;
	}

	__PUNCTUATOR_TYPE_OLD__  CASTNode::parsePunctuatorGroup8(const char *s){

		__PUNCTUATOR_TYPE_OLD__ index_to_evaluate[]={
				FIELD_PUNCTUATOR

		};

		for(unsigned char  i = 0; i < ARRAY_LENGTH(index_to_evaluate); i++){
			if(defined_operator_punctuator[index_to_evaluate[i]].eval_fun == NULL){
				THROW_RUNTIME_ERROR("internal: %s not have parse function",defined_operator_punctuator[index_to_evaluate[i]].str);
				return __PUNCTUATOR_TYPE_OLD__::UNKNOWN_PUNCTUATOR;
			}
			if(defined_operator_punctuator[index_to_evaluate[i]].eval_fun(s)){
				return defined_operator_punctuator[index_to_evaluate[i]].id;
			}
		}

		return __PUNCTUATOR_TYPE_OLD__::UNKNOWN_PUNCTUATOR;
	}

	__PUNCTUATOR_TYPE_OLD__   CASTNode::isOperatorPunctuator(const char *s){

		for(unsigned char  i = 0; i < MAX_OPERATOR_PUNCTUATORS; i++){
			if(defined_operator_punctuator[i].eval_fun != NULL){
				if(defined_operator_punctuator[i].eval_fun(s)){
					return defined_operator_punctuator[i].id;
				}
			}
		}
		return __PUNCTUATOR_TYPE_OLD__::UNKNOWN_PUNCTUATOR;
	}

	__PUNCTUATOR_TYPE_OLD__   CASTNode::isSpecialPunctuator(const char *s){

		for(unsigned char i = START_SPECIAL_PUNCTUATORS; i < MAX_SPECIAL_PUNCTUATORS; i++){

			if(*defined_operator_punctuator[i].str == *s){
				return defined_operator_punctuator[i].id;
			}
		}

		return __PUNCTUATOR_TYPE_OLD__::UNKNOWN_PUNCTUATOR;
	}

	__PUNCTUATOR_TYPE_OLD__   CASTNode::parseArithmeticPunctuator(const char *s){

		__PUNCTUATOR_TYPE_OLD__ index_to_evaluate[]={
				SHIFT_LEFT_PUNCTUATOR, // <<
				SHIFT_RIGHT_PUNCTUATOR, // >>

				LOGIC_AND_PUNCTUATOR, // &&
				LOGIC_OR_PUNCTUATOR, // ||
				LOGIC_EQUAL_PUNCTUATOR, // =
				LOGIC_NOT_EQUAL_PUNCTUATOR, // !=
				LOGIC_GTE_PUNCTUATOR, // >=
				LOGIC_LTE_PUNCTUATOR, // <=

				INSTANCEOF_PUNCTUATOR, // instanceof

				// Then OPERATORS 1 char size
				ADD_PUNCTUATOR, // +
				SUB_PUNCTUATOR, // -
				MUL_PUNCTUATOR, // *
				DIV_PUNCTUATOR, // /
				MOD_PUNCTUATOR, // %

				ASSIGN_PUNCTUATOR, // =
				ADD_ASSIGN_PUNCTUATOR, // +=
				SUB_ASSIGN_PUNCTUATOR, // -=
				MUL_ASSIGN_PUNCTUATOR, // *=
				DIV_ASSIGN_PUNCTUATOR, // /=
				MOD_ASSIGN_PUNCTUATOR, // %=

				BINARY_XOR_PUNCTUATOR, // ^
				BINARY_AND_PUNCTUATOR, // &
				BINARY_OR_PUNCTUATOR, // |

				LOGIC_GT_PUNCTUATOR, // >
				LOGIC_LT_PUNCTUATOR, // <
				LOGIC_NOT_PUNCTUATOR // !
		};

		for(unsigned i = 0; i < ARRAY_LENGTH(index_to_evaluate); i++){

			if(defined_operator_punctuator[index_to_evaluate[i]].eval_fun == NULL){
				THROW_RUNTIME_ERROR("internal: %s not have parse function",defined_operator_punctuator[index_to_evaluate[i]].str);
				return __PUNCTUATOR_TYPE_OLD__::UNKNOWN_PUNCTUATOR;
			}

			if(defined_operator_punctuator[index_to_evaluate[i]].eval_fun(s)){
				return defined_operator_punctuator[index_to_evaluate[i]].id;
			}
		}
		return __PUNCTUATOR_TYPE_OLD__::UNKNOWN_PUNCTUATOR;;
	}

	__PUNCTUATOR_TYPE_OLD__   CASTNode::isPunctuator(const char *s){

		__PUNCTUATOR_TYPE_OLD__ ip = isOperatorPunctuator(s);

		if(ip!=__PUNCTUATOR_TYPE_OLD__::UNKNOWN_PUNCTUATOR){
			return ip;
		}

		return isSpecialPunctuator(s);
	}

	// to string utils ...
	char * CASTNode::getEndWord(const char *s, int m_line){

		char *start_str=(char *)s;
		char *aux=(char *)s;
		__PUNCTUATOR_TYPE_OLD__ sp;
		KEYWORD_TYPE key_w;
		 bool is_possible_number=false;
		 int i=0;
		 bool start_digit = false;

		if(*aux == '\"'){
			aux++;
			while((*aux)!=0 && !((*aux)=='\n') && !((*aux)=='\"' && *(aux-1) !='\\')) {
				aux++;
			}

			if(*aux != '\"') {
				writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Error \" not closed");
				return NULL;
			}
			aux++;

		}else{

			if((key_w = isKeyword(s))!= KEYWORD_TYPE::UNKNOWN_KEYWORD){
				if( key_w != KEYWORD_TYPE::THIS_KEYWORD){
				 //&& key_w->id != KEYWORD_TYPE::SUPER_KEYWORD ){ // unexpected token ?
					writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Unexpected keyword \"%s\". Forgot \";\" ?",defined_keyword[key_w].str);
					return NULL;
				}
			}

			while((*aux)!=0 && !(
					(*aux)==' ' ||
					(*aux)=='\t' ||
					(*aux)=='\n' ||
					(*aux)=='\r'
							) &&
					(isSpecialPunctuator(aux)==__PUNCTUATOR_TYPE_OLD__::UNKNOWN_PUNCTUATOR)

			) {
				// check for special punctuator ( the field '.' separator is processed within the word )
				if(i==0 && !start_digit){ // possible digit ...

					is_possible_number = CZetScriptUtils::isDigit(*aux);
					start_digit = true;
				}

				if((sp = isOperatorPunctuator(aux))!=__PUNCTUATOR_TYPE_OLD__::UNKNOWN_PUNCTUATOR){
					if(sp == FIELD_PUNCTUATOR  || ((*aux=='-' ||  *aux=='+') && ((i>0 && (*(aux-1)=='e'))))){
						if(!is_possible_number){
							return aux;
						}
					}
					else{
						return aux;
					}
				}

				aux++;
				i++;
			}

			if(is_possible_number){
				string num = CZetScriptUtils::copyStringFromInterval(start_str,aux);

				if(!CZetScriptUtils::isBinary(num)){

					if(!CZetScriptUtils::isNumber(num)){
						writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"%s is not a valid number",num.c_str());
						return NULL;
					}
				}
			}
		}
		return aux;
	}

	char *CASTNode::getSymbolName(const char *s,int & m_line){

		char *aux_p=(char *)s;
		__PUNCTUATOR_TYPE_OLD__ end_punctuator=isPunctuator(s);


		if(end_punctuator != __PUNCTUATOR_TYPE_OLD__::UNKNOWN_PUNCTUATOR){
			writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Unexpected '%s'",defined_operator_punctuator[end_punctuator].str);
			return NULL;
		}

		if(*aux_p!=0 && (
		   ('a' <= *aux_p && *aux_p <='z') ||
		   ('A' <= *aux_p && *aux_p <='Z') ||
		   *aux_p == '_'
		 )
		){ // let's see it has right chars...

			aux_p++;
			while((*aux_p!=0) && (('a' <= *aux_p && *aux_p <='z') ||
				  ('0' <= *aux_p && *aux_p <='9') ||
				  (*aux_p=='_') ||
				  ('A' <= *aux_p && *aux_p <='Z'))){
				aux_p++;
			}
		}else{
			writeErrorMsg(CURRENT_PARSING_FILENAME,m_line," Symbol name cannot begin with %c", *aux_p);
			return NULL;
		}

		return aux_p;
	}

	KEYWORD_TYPE CASTNode::isKeyword(const char *c){
		int m_line=0;
		char *str=IGNORE_BLANKS(c,m_line);

		for(int i = 0; i < MAX_KEYWORD; i++){
			int size = strlen(defined_keyword[i].str);
			char *aux = str+size;
			if((strncmp(str,defined_keyword[i].str,size)==0) && (
					*aux == 0  || // carry end
					*aux == ' '  || // space

					*aux == '\t'  || // tab
					isOperatorPunctuator(aux)!=__PUNCTUATOR_TYPE_OLD__::UNKNOWN_PUNCTUATOR ||
					isSpecialPunctuator(aux)!=__PUNCTUATOR_TYPE_OLD__::UNKNOWN_PUNCTUATOR ||
					*aux == '\n' || // carry return

				   (*aux == '/' && *(aux+1) == '*')) //start block comment
				   ){
				return defined_keyword[i].id;
			}
		}
		return KEYWORD_TYPE::UNKNOWN_KEYWORD;
	}

	DIRECTIVE_TYPE CASTNode::isDirective(const char *c){
		int m_line=0;
		char *str=IGNORE_BLANKS(c,m_line);

		for(int i = 0; i < MAX_DIRECTIVES; i++){
			if(defined_directive[i].str){
				int size = strlen(defined_directive[i].str);

				if(strncmp(str,defined_directive[i].str,size)==0)
				{
					return defined_directive[i].id;
				}
			}
		}
		return DIRECTIVE_TYPE::UNKNOWN_DIRECTIVE;
	}
	//------------------------------------------------------------------------------------------------------------
	__PUNCTUATOR_TYPE_OLD__ CASTNode::checkPreOperatorPunctuator(const char *s){

		if(parseIncPunctuator(s)) 	return PRE_INC_PUNCTUATOR;
		if(parseDecPunctuator(s))	return PRE_DEC_PUNCTUATOR;
		if(parsePlusPunctuator(s)) 	return ADD_PUNCTUATOR;
		if(parseMinusPunctuator(s)) return SUB_PUNCTUATOR;
		if(parseNotPunctuator(s))   return LOGIC_NOT_PUNCTUATOR;

		return UNKNOWN_PUNCTUATOR;
	}

	__PUNCTUATOR_TYPE_OLD__ CASTNode::checkPostOperatorPunctuator(const char *s){

		__PUNCTUATOR_TYPE_OLD__ op=UNKNOWN_PUNCTUATOR;

		if(parseIncPunctuator(s)){
			op=POST_INC_PUNCTUATOR;
		}

		if(parseDecPunctuator(s)){
			op = POST_DEC_PUNCTUATOR;
		}

		if(op != UNKNOWN_PUNCTUATOR){ // let's check some situations whether is not allowed having post operator
			__PUNCTUATOR_TYPE_OLD__ pt=UNKNOWN_PUNCTUATOR;
			int line=0;

			char *aux=(char *)(s+strlen(defined_operator_punctuator[op].str));

			aux=IGNORE_BLANKS(aux,line); // advance to next char...

			if(*aux == 0){
				return op;
			}

			// if is an operator ... ok!
			pt=isPunctuator(aux);

			if(pt != UNKNOWN_PUNCTUATOR){
				if(pt < MAX_OPERATOR_PUNCTUATORS){ // ok...
					return op;
				}

				if(   pt == COMA_PUNCTUATOR  // ok
				  ||  pt == SEMICOLON_PUNCTUATOR
				  ||  pt == CLOSE_PARENTHESIS_PUNCTUATOR
				  ||  pt == CLOSE_SQUARE_BRAKET_PUNCTUATOR
				  ){
					return op;
				}
			}
		}
		return UNKNOWN_PUNCTUATOR;
	}

	char *CASTNode::functionArrayAccess_Recursive(const char *str, int & m_line, CScope *scope_info, PASTNode *ast_node_to_be_evaluated, PASTNode parent){
		// PRE: **ast_node_to_be_evaluated must be created and is i/o ast pointer variable where to write changes.
		char *aux = (char *)str;

		aux = IGNORE_BLANKS(aux, m_line);

		if((*aux == '(' || *aux == '[')){

			 if(ast_node_to_be_evaluated != NULL) {// save node
				 if((*ast_node_to_be_evaluated = CASTNode::newASTNode())==NULL) return NULL;
				 //(*ast_node_to_be_evaluated)->symbol_value = symbol_value; // is array or function ...
				 (*ast_node_to_be_evaluated)->line_value = m_line;
				 (*ast_node_to_be_evaluated)->node_type  = ARRAY_REF_NODE;
				 (*ast_node_to_be_evaluated)->idxScope =scope_info->idxScope;
			 }

			 if(*aux == '('){
				 if(ast_node_to_be_evaluated != NULL){
					 (*ast_node_to_be_evaluated)->node_type  = FUNCTION_REF_NODE;
					 (*ast_node_to_be_evaluated)->line_value = m_line;
					 (*ast_node_to_be_evaluated)->idxScope =scope_info->idxScope;
				 }
			 }

			PASTNode args_obj=NULL;

			if(*aux == '('){ // function access

				if( ast_node_to_be_evaluated != NULL){
					if((*ast_node_to_be_evaluated)->node_type != FUNCTION_REF_NODE && (*ast_node_to_be_evaluated)->node_type != FUNCTION_OBJECT_NODE){
						writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Expected function object before '('");
						return NULL;
					}
				}

				if((aux=parseArgs('(', ')',aux,m_line,scope_info,ast_node_to_be_evaluated != NULL? &args_obj : NULL)) == NULL){
					return NULL;
				}

				if(ast_node_to_be_evaluated != NULL){
					args_obj->node_type = NODE_TYPE::ARGS_PASS_NODE;
					//vector_args_node.push_back(args_node);//[0]->node_type = NODE_TYPE::ARGS_PASS_NODE;
				}
			}else if (*aux == '['){ // array acces multi dimensional like this : array[i][j][z] ...

				int i = 0;
				bool end = false;

				if( ast_node_to_be_evaluated != NULL){
					if((*ast_node_to_be_evaluated)->node_type != ARRAY_REF_NODE){
						writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Expected array object before '['");
						return NULL;
					}
				}

				if( ast_node_to_be_evaluated != NULL){
					if((args_obj = CASTNode::newASTNode())==NULL){return NULL;}
					args_obj->node_type = ARRAY_ACCESS_NODE;
				}

				do{
					PASTNode args_node=NULL;
					char *aux_ptr;
					int ini_line_access=m_line;

					if((aux_ptr=parseArgs('[', ']',aux,m_line,scope_info,ast_node_to_be_evaluated != NULL ? &args_node: NULL)) != NULL){
						if( ast_node_to_be_evaluated != NULL){
							if(args_node->children.size() != 1){
								writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Invalid array access");
								return NULL;
							}
							args_obj->children.push_back(args_node->idxAstNode);
							args_node->node_type = NODE_TYPE::ARRAY_INDEX_NODE;
							args_node->line_value =ini_line_access;
						}
						aux =  IGNORE_BLANKS(aux_ptr,m_line);
					}else{
						if(i == 0){
							return NULL;
						}else{
							end=true;
						}
					}
					i++;

				}while(!end);
			}

			if(ast_node_to_be_evaluated != NULL){
				// the deepth node is the object and the parent will the access node (function or array) ...
				if(args_obj != NULL){ // there's arguments to be pushed ...
					PASTNode obj =  *ast_node_to_be_evaluated;
					PASTNode calling_object;
					if((calling_object= CASTNode::newASTNode()) == NULL) return NULL;
					//tPunctuatorInfo *ip=NULL;

					calling_object->node_type = CALLING_OBJECT_NODE;

					obj->idxAstParent = calling_object->idxAstNode;
					obj->symbol_value = "--";
					args_obj->idxAstParent = calling_object->idxAstNode;

					calling_object->children.push_back(obj->idxAstNode); // the object itself...
					calling_object->children.push_back(args_obj->idxAstNode); // the args itself...
					calling_object->idxAstParent=-1;
					if(parent!=NULL){
						calling_object->idxAstParent=parent->idxAstNode;
					}

					// finally save ast node...

					*ast_node_to_be_evaluated = calling_object;
				}
			}

			PASTNode another_access=NULL;
			aux = functionArrayAccess_Recursive(aux,m_line,scope_info,ast_node_to_be_evaluated!=NULL?&another_access:NULL,ast_node_to_be_evaluated!=NULL?*ast_node_to_be_evaluated:NULL);

			if(another_access != NULL){
				(*ast_node_to_be_evaluated)->children.push_back(another_access->idxAstNode);
			}
		}
		return aux;
	}

	char *CASTNode::functionArrayAccess(const char *str, int & m_line, CScope *scope_info, PASTNode *ast_node_to_be_evaluated, PASTNode parent){
		// PRE: **ast_node_to_be_evaluated must be created and is i/o ast pointer variable where to write changes.
		return functionArrayAccess_Recursive(str,m_line,scope_info,ast_node_to_be_evaluated,parent);
	}

	char * CASTNode::deduceExpression(const char *str, int & m_line, CScope *scope_info, PASTNode *ast_node_to_be_evaluated, PASTNode parent){
		// PRE: **ast_node_to_be_evaluated must be created and is i/o ast pointer variable where to write changes.
		char *aux = (char *)str;
		char *end_expression;
		int m_startLine = m_line;
		KEYWORD_TYPE key_w  = KEYWORD_TYPE::UNKNOWN_KEYWORD;
		vector<PASTNode> vector_args_node;
		bool try_array_or_function_access = false;
		bool should_be_access=false;

		string symbol_value="";

		if(ast_node_to_be_evaluated != NULL){
			*ast_node_to_be_evaluated = NULL;
		}

		aux = IGNORE_BLANKS(aux, m_startLine);

		key_w = isKeyword(aux);

		if(key_w != KEYWORD_TYPE::UNKNOWN_KEYWORD){ // can be new,delete or function...

			aux = IGNORE_BLANKS(aux+strlen(defined_keyword[key_w].str), m_startLine);

			switch(key_w){
			// sould be function object ...
			case KEYWORD_TYPE::FUNCTION_KEYWORD:
				// function objects are stored in MainClass or global scope.
				if((aux=parseFunction(str,m_startLine,scope_info,ast_node_to_be_evaluated!=NULL?ast_node_to_be_evaluated:NULL)) == NULL){
					return NULL;
				}
				try_array_or_function_access = true;
				break;
			case KEYWORD_TYPE::NEW_KEYWORD:
				if((aux = parseNew(str,m_startLine,scope_info,ast_node_to_be_evaluated!=NULL?ast_node_to_be_evaluated:NULL)) == NULL){
					return NULL;
				}
				break;
			case KEYWORD_TYPE::DELETE_KEYWORD:
				THROW_RUNTIME_ERROR("internal error! delete bad ast processing!");
				return NULL;
				/*if((aux = parseDelete(str,m_startLine,scope_info,ast_node_to_be_evaluated!=NULL?ast_node_to_be_evaluated:NULL)) == NULL){
					return NULL;
				}*/
				break;

			case KEYWORD_TYPE::THIS_KEYWORD:
				return parseExpression_Recursive(str,  m_line, scope_info, ast_node_to_be_evaluated,GROUP_TYPE::GROUP_0,parent);
				break;
			default:
				writeErrorMsg(CURRENT_PARSING_FILENAME,m_startLine,"Invalid using of \"%s\"",defined_keyword[key_w].str);
				break;
			}
		}else if(*aux == '[') { // is an array object ...

			if((aux = parseArgs('[', ']',aux,m_startLine,scope_info,ast_node_to_be_evaluated!=NULL?ast_node_to_be_evaluated:NULL))== NULL){
				return NULL;
			}

			aux = IGNORE_BLANKS(aux, m_startLine);
			try_array_or_function_access = true;

			if(ast_node_to_be_evaluated != NULL){
				(*ast_node_to_be_evaluated)->node_type = ARRAY_OBJECT_NODE;
			}
		}
		else{ // symbol or expression...
			char *word_str=NULL;
			// try to get function/array object ...
			end_expression = getEndWord(aux, m_startLine);
			//bool function_or_array = false;

			 if(!(end_expression == NULL || end_expression == aux)){ // is valid word...

				 symbol_value = CZetScriptUtils::copyStringFromInterval(aux,end_expression);
				 word_str = IGNORE_BLANKS(end_expression, m_startLine);

				 try_array_or_function_access = true;

				 should_be_access = (*word_str == '(' || *word_str == '[');

				 if(!should_be_access){
					 return parseExpression_Recursive(aux,  m_line, scope_info, ast_node_to_be_evaluated,GROUP_TYPE::GROUP_0,parent);
				 }
				 else{
					 aux = word_str;
				 }
			 }else{ // try parse expression...
				 return parseExpression_Recursive(aux,  m_line, scope_info, ast_node_to_be_evaluated,GROUP_TYPE::GROUP_0,parent);
			 }
		}

		if(try_array_or_function_access){// try array/function access
			if((aux = functionArrayAccess(aux, m_line,scope_info,ast_node_to_be_evaluated,parent)) == NULL){
				return NULL;
			}

			if(ast_node_to_be_evaluated != NULL){
				if(*ast_node_to_be_evaluated == NULL && should_be_access){
					writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Cannot parse 3");
					return NULL;
				}

				if((*ast_node_to_be_evaluated)->children.size() > 0){
					vec_ast_node->at((*ast_node_to_be_evaluated)->children[0])->symbol_value = symbol_value;
				}
			}
		}
		return aux;
	}

	bool CASTNode::printErrorUnexpectedKeywordOrPunctuator(const char *current_string_ptr, int m_line){
		__PUNCTUATOR_TYPE_OLD__ ip=CASTNode::isPunctuator(current_string_ptr);
		KEYWORD_TYPE kw=CASTNode::isKeyword(current_string_ptr);

		if(kw!=KEYWORD_TYPE::UNKNOWN_KEYWORD){
			writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Unexpected %s",defined_keyword[kw].str);
			return true;
		}
		else if(ip!=__PUNCTUATOR_TYPE_OLD__::UNKNOWN_PUNCTUATOR){
			writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Unexpected %s",defined_operator_punctuator[ip].str);
			return true;
		}
		return false;
	}

	char *CASTNode::getSymbolValue(
			const char *current_string_ptr,
			int & m_line,
			CScope *scope_info,

			string & symbol_name,
			int & m_definedSymbolLine,
			__PUNCTUATOR_TYPE_OLD__ pre_operator,
			__PUNCTUATOR_TYPE_OLD__ & post_operator,
			bool & is_symbol_trivial
			){

		char *aux = (char *)current_string_ptr;
		char *end_expression = aux;
		is_symbol_trivial = false;
		m_definedSymbolLine = m_line;

		if(symbol_name == "super"){
			writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Invalid using \"super\" keyword");
			return NULL;
		}

		//----------------------------------------------------------
		// GETTING TRIVIAL SYMBOLS
		if(*aux=='('){ // packed symbol...

			if(pre_operator != __PUNCTUATOR_TYPE_OLD__::UNKNOWN_PUNCTUATOR){
				if(pre_operator == __PUNCTUATOR_TYPE_OLD__::PRE_INC_PUNCTUATOR ||
				   pre_operator == __PUNCTUATOR_TYPE_OLD__::PRE_DEC_PUNCTUATOR){
					writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Unexpected '%s' before (",defined_operator_punctuator[pre_operator].str);
					return NULL;
				}
			}

			// update '('
			aux=aux+1;

			// only parse no evaluate (don't save ast node)
			end_expression = parseExpression_Recursive(aux, m_line, scope_info,NULL);//, ast_node_to_be_evaluated, type_group,parent);

			if(end_expression == NULL){
				return NULL;
			}

			if(*end_expression != ')'){
				writeErrorMsg(CURRENT_PARSING_FILENAME,m_definedSymbolLine,"Not closed parenthesis starting");
				return NULL;
			}

			 if(end_expression == NULL || end_expression == aux){
				 writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Expected expression");
				 return NULL;
			 }

			 symbol_name=CZetScriptUtils::copyStringFromInterval(aux,end_expression);
			end_expression=end_expression+1;
			//end_expression = aux+1;
		}else{ // check for symbols (must have a symbol at least)

			KEYWORD_TYPE key_w;
			bool is_function_or_operator=false;
			bool is_new=false;
			bool is_delete=false;
			// usually we have to take care about special op symbols

			if((key_w =isKeyword(aux)) != KEYWORD_TYPE::UNKNOWN_KEYWORD){
				is_function_or_operator = key_w == KEYWORD_TYPE::FUNCTION_KEYWORD;
				is_new=key_w == KEYWORD_TYPE::NEW_KEYWORD;
				is_delete=key_w == KEYWORD_TYPE::DELETE_KEYWORD;
			}

			if(is_function_or_operator){ // function object ...

				if(pre_operator!=__PUNCTUATOR_TYPE_OLD__::UNKNOWN_PUNCTUATOR){
					if(pre_operator == __PUNCTUATOR_TYPE_OLD__::PRE_INC_PUNCTUATOR ||
						pre_operator == __PUNCTUATOR_TYPE_OLD__::PRE_DEC_PUNCTUATOR){
						writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Unexpected '%s' before ( ",defined_operator_punctuator[pre_operator].str);
						return NULL;
					}
				}
				// parse function but do not create ast node (we will create in trivial case value
				end_expression = parseFunction(aux,m_line,scope_info,NULL);

				//symbol_node->symbol_value="anonymous_function";
				if(end_expression == NULL){
					return NULL;
				}
				symbol_name = CZetScriptUtils::copyStringFromInterval(aux, end_expression);
			}
			else // array object ...
			{
				if(*aux == '['){ // vector object ...
					// parse function but do not create ast node (we will create in trivial case value
					end_expression = parseArgs('[', ']',aux,m_line,scope_info,NULL);

					if(end_expression == NULL){
						return NULL;
					}
					symbol_name = CZetScriptUtils::copyStringFromInterval(aux, end_expression);
				}
				else{
					char *start_expression = aux;
					is_symbol_trivial = true;

					// treat as symbol...
					if(is_new || is_delete) {
						aux = IGNORE_BLANKS(aux + strlen(defined_keyword[key_w].str), m_line);
						is_symbol_trivial = false;
					}

					end_expression = getEndWord(aux, m_line);

					 if(end_expression == NULL || end_expression == aux){

						 // check if punctuator or keyword..

						 if(end_expression != NULL){
							 if(!printErrorUnexpectedKeywordOrPunctuator(aux, m_line)){
								 writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Expected symbol");
							 }
						 }
						 return NULL;
					 }

					 symbol_name=CZetScriptUtils::copyStringFromInterval(start_expression,end_expression);

					 // check for post opertator...
					 end_expression = IGNORE_BLANKS(end_expression, m_line);
					 if((post_operator = checkPostOperatorPunctuator(end_expression)) != __PUNCTUATOR_TYPE_OLD__::UNKNOWN_PUNCTUATOR){
						 end_expression+=strlen(defined_operator_punctuator[post_operator].str);
					 }

					 aux = start_expression;
				}
			}

			// if there's function or array access after symbol or object created ...
			end_expression = IGNORE_BLANKS(end_expression, m_line);

			if(*end_expression == '[' || *end_expression == '('){ // function or array access --> process its ast but not save ...
				is_symbol_trivial = false;
				end_expression = functionArrayAccess(end_expression,m_line,scope_info,NULL);

				if(end_expression == NULL){
					return NULL;
				}

				end_expression = IGNORE_BLANKS(end_expression, m_line);
				// check for post opertator...
				if(((post_operator) = checkPostOperatorPunctuator(end_expression)) != __PUNCTUATOR_TYPE_OLD__::UNKNOWN_PUNCTUATOR){
				 end_expression+=strlen(defined_operator_punctuator[post_operator].str);
				 end_expression = IGNORE_BLANKS(end_expression, m_line);
				}

				symbol_name = CZetScriptUtils::copyStringFromInterval(aux, end_expression);
			}
		}
		// GETTING TRIVIAL SYMBOLS
		//----------------------------------------------------------
		return end_expression;
	}

	bool CASTNode::isMarkEndExpression(char c){
		return (c==0 || c==';' || c==',' ||  c==')'  || c==']' || c=='}');//|| c==':');
	}
	//-----------------------------------------------------------------------------------------------------------
/*	char * CASTNode::parseExpression_Recursive_old(const char *s, int & m_line,CScope *scope_info, PASTNode *ast_node_to_be_evaluated, GROUP_TYPE type_group,PASTNode parent ){
		// PRE: **ast_node_to_be_evaluated must be created and is i/o ast pointer variable where to write changes.
		char *aux=(char *)s;
		char *s_effective_start=(char *)s;
		char *expr_start_op=NULL;
		int start_line = m_line; // set another start line because left node or reparse to try another group was already parsed before.
		int m_lineOperator=-2;
		char *end_expression=(char *)s ; // by default end expression isequal to

		bool is_symbol_trivial_value=false;
		string symbol_value;
		string operator_str="";
		__PUNCTUATOR_TYPE_OLD__ pre_operator=__PUNCTUATOR_TYPE_OLD__::UNKNOWN_PUNCTUATOR,
						post_operator=__PUNCTUATOR_TYPE_OLD__::UNKNOWN_PUNCTUATOR,
						pre_operator_packed_node=__PUNCTUATOR_TYPE_OLD__::UNKNOWN_PUNCTUATOR,
						operator_group=__PUNCTUATOR_TYPE_OLD__::UNKNOWN_PUNCTUATOR;
		bool theres_some_operator=false;
		int m_definedSymbolLine;
		bool special_pre_post_cond = false; // in case of particular pre/post...
		bool is_packed_node = false;

		aux=IGNORE_BLANKS(aux, m_line);

		if(isMarkEndExpression(*aux)){ // returning because is trivial!
			return aux;
		}

		if(type_group>=MAX_GROUPS) {
			THROW_RUNTIME_ERROR("Internal:Cannot find ast tree operator");
			return NULL;
		}

		print_ast_cr("new expression eval:\"%.80s ...\" group:%i at line %i",aux,type_group, m_line);

		// searching for operator!
		if(*aux == '{'){ //json expression...
			print_ast_cr("detected json expression");
			return parseStruct(aux,m_line,scope_info,ast_node_to_be_evaluated);
		}

		print_ast_cr("searching for operator type %i...",type_group);

		while(!isMarkEndExpression(*aux) && (operator_group==0)){
			special_pre_post_cond = false;
			print_ast_cr("checkpoint1:%c\n",*aux);
			// 1. ignore spaces...
			aux=IGNORE_BLANKS(aux, m_line);

			if((pre_operator=checkPreOperatorPunctuator(aux))!=__PUNCTUATOR_TYPE_OLD__::UNKNOWN_PUNCTUATOR){

				aux+=strlen(defined_operator_punctuator[pre_operator].str);
				aux=IGNORE_BLANKS(aux, m_line);
			}

			if(*aux=='('){ // packed node let's said that is a packed node...

				pre_operator_packed_node=pre_operator;
				pre_operator=__PUNCTUATOR_TYPE_OLD__::UNKNOWN_PUNCTUATOR;

				if(ast_node_to_be_evaluated != NULL){
					is_packed_node = true;
				}
			}

			int start_get_symbol_line=m_line;
			// try get symbol string
			if((aux=getSymbolValue(aux, m_line, scope_info,symbol_value, m_definedSymbolLine,pre_operator,post_operator,is_symbol_trivial_value)) == NULL){
				return NULL;
			}

			print_ast_cr("checkpoint3:%c\n",*aux);

			aux=IGNORE_BLANKS(aux, m_line);

			if(!isMarkEndExpression(*aux)){ // is not end expression

				if((operator_group=isOperatorPunctuator(aux))!=0){

					// particular cases... since the getSymbol alrady checks the pre operator it cannot be possible to get a pre inc or pre dec...
					if(operator_group ==  PRE_INC_PUNCTUATOR){ // ++ really is a + PUNCTUATOR...
						operator_group = ADD_PUNCTUATOR;
						special_pre_post_cond=true;
					}
					else if(operator_group ==  PRE_DEC_PUNCTUATOR){ // -- really is a + PUNCTUATOR...
						operator_group = SUB_PUNCTUATOR;
						pre_operator = SUB_PUNCTUATOR;
					}

					theres_some_operator |= true;
					expr_start_op=aux;
					m_lineOperator = m_line;

					if(operator_group != SUB_PUNCTUATOR) // advance ...
						aux+=strlen(defined_operator_punctuator[operator_group].str);

					if(!special_pre_post_cond && (operator_group != SUB_PUNCTUATOR)){ // not check because special case pre/post op...

						switch(type_group){
						case GROUP_0:	operator_group = parsePunctuatorGroup0(expr_start_op);break;
						case GROUP_1:	operator_group = parsePunctuatorGroup1(expr_start_op);break;
						case GROUP_2:	operator_group = parsePunctuatorGroup2(expr_start_op);break;
						case GROUP_3:	operator_group = parsePunctuatorGroup3(expr_start_op);break;
						case GROUP_4:	operator_group = parsePunctuatorGroup4(expr_start_op);break;
						case GROUP_5:	operator_group = parsePunctuatorGroup5(expr_start_op);break;
						case GROUP_6:	operator_group = parsePunctuatorGroup6(expr_start_op);break;
						case GROUP_7:	operator_group = parsePunctuatorGroup7(expr_start_op);break;
						case GROUP_8:	operator_group = parsePunctuatorGroup8(expr_start_op);break;
						default: break;
						}
					}
				}else{
					writeErrorMsg(CURRENT_PARSING_FILENAME,start_get_symbol_line,"expected ';' or operator or punctuator after \"%s\"",symbol_value.c_str());
					return NULL;
				}
			}
		}

		if(operator_group==__PUNCTUATOR_TYPE_OLD__::UNKNOWN_PUNCTUATOR) {// there's no any operators \"type_group\"...
			if(!theres_some_operator){ // only we have a value (trivial)

				if(ast_node_to_be_evaluated != NULL){

					if(is_symbol_trivial_value){


						if(SCOPE_NODE(scope_info->idxScope)->getIdxBaseScope()==IDX_GLOBAL_SCOPE){
							if(symbol_value == "this"){
								writeErrorMsg(CURRENT_PARSING_FILENAME,m_definedSymbolLine,"\"this\" keyword is allowed only in member classes");
								return NULL;
							}

							if(symbol_value == "super"){
								writeErrorMsg(CURRENT_PARSING_FILENAME,m_definedSymbolLine,"\"super\" keyword is allowed only in member classes");
								return NULL;
							}
						}

						if(((*ast_node_to_be_evaluated)=CASTNode::newASTNode()) == NULL) return NULL;
						(*ast_node_to_be_evaluated)->node_type = SYMBOL_NODE;
						(*ast_node_to_be_evaluated)->symbol_value=symbol_value; // assign its value ...
						(*ast_node_to_be_evaluated)->idxScope = scope_info->idxScope;

					}else{
						if(deduceExpression(symbol_value.c_str(),m_definedSymbolLine,scope_info, ast_node_to_be_evaluated, parent) == NULL){
							return NULL;
						}

						if(is_packed_node){ // packed node let's say that is a packed node...
							if(ast_node_to_be_evaluated != NULL){
								(*ast_node_to_be_evaluated)->is_packed_node = true;

								if(pre_operator_packed_node != UNKNOWN_PUNCTUATOR){ // we must create op...
									// 2. create neg node.

									//(*ast_node_to_be_evaluated)->pre_post_operator_info = UNKNOWN_PUNCTUATOR;
									PASTNode ast_neg_node=NULL;
									if((ast_neg_node = CASTNode::newASTNode())==NULL) return NULL;
									ast_neg_node->node_type = NODE_TYPE::PUNCTUATOR_NODE;
									ast_neg_node->operator_info = pre_operator_packed_node;

									ast_neg_node->idxAstParent =(* ast_node_to_be_evaluated)->idxAstParent;
									(*ast_node_to_be_evaluated)->idxAstParent = ast_neg_node->idxAstNode;
									ast_neg_node->children.push_back((*ast_node_to_be_evaluated)->idxAstNode);

									(*ast_node_to_be_evaluated)=ast_neg_node;
								}
							}
						}
					}

					print_ast_cr("---------------------");
					print_ast_cr("%s value \"%s\" at line %i",(is_symbol_trivial_value?"trivial":"NOT trivial"),symbol_value.c_str(), m_definedSymbolLine);
					print_ast_cr("---------------------");

					// put values by default ...
					(*ast_node_to_be_evaluated)->idxAstParent=ZS_UNDEFINED_IDX;
					if(parent!=NULL){
						(*ast_node_to_be_evaluated)->idxAstParent=parent->idxAstNode;
					}
					(*ast_node_to_be_evaluated)->line_value=m_definedSymbolLine;


					if(pre_operator!= __PUNCTUATOR_TYPE_OLD__::UNKNOWN_PUNCTUATOR || post_operator != __PUNCTUATOR_TYPE_OLD__::UNKNOWN_PUNCTUATOR){ // create pre operator node ...

						if(post_operator!=__PUNCTUATOR_TYPE_OLD__::UNKNOWN_PUNCTUATOR)
							(*ast_node_to_be_evaluated)->pre_post_operator_info = post_operator; // preNodePunctuator(post_operator,*ast_node_to_be_evaluated);

						if(pre_operator!=__PUNCTUATOR_TYPE_OLD__::UNKNOWN_PUNCTUATOR){
							if(post_operator!=__PUNCTUATOR_TYPE_OLD__::UNKNOWN_PUNCTUATOR){
								if(     (pre_operator == PRE_INC_PUNCTUATOR  || pre_operator  == PRE_DEC_PUNCTUATOR) &&
										(post_operator== POST_INC_PUNCTUATOR || post_operator == POST_DEC_PUNCTUATOR)){
									writeErrorMsg(CURRENT_PARSING_FILENAME,m_definedSymbolLine,"object \"%s\" has left \"%s\" and right \"%s\" is ambiguous",(*ast_node_to_be_evaluated)->symbol_value.c_str(),defined_operator_punctuator[pre_operator].str, defined_operator_punctuator[post_operator].str);
									return NULL;
								}
							}
							(*ast_node_to_be_evaluated)->pre_post_operator_info = pre_operator; //preNodePunctuator(pre_operator,*ast_node_to_be_evaluated);
						}
					}
				}
			}
			else{

				if(end_expression!= NULL){
				// there's a Punctuator, so let's perform generate its AST
					// reset prePunctuator...
					pre_operator=__PUNCTUATOR_TYPE_OLD__::UNKNOWN_PUNCTUATOR;
					print_ast_cr("try to generate group1 expression: %.40s ...\n",s_effective_start);
					return parseExpression_Recursive(
							s,
							start_line, // start line because was reparsed before (i.e group -1)
							scope_info,
							ast_node_to_be_evaluated,
							(GROUP_TYPE)(((int)type_group)+1),parent);
				}
			}
		}else{ // we found the operator respect of GROUPX so let's put the AST to the left the resulting expression...

			PASTNode left_node = NULL;
			PASTNode right_node = NULL;

			if(ast_node_to_be_evaluated != NULL){
				if((*ast_node_to_be_evaluated=CASTNode::newASTNode())==NULL) return NULL; // always preallocate 2 nodes (left and right)
				(*ast_node_to_be_evaluated)->idxAstParent = ZS_UNDEFINED_IDX;
				if(parent!=NULL){
					(*ast_node_to_be_evaluated)->idxAstParent=parent->idxAstNode;
				}
				(*ast_node_to_be_evaluated)->node_type = EXPRESSION_NODE;
			}
			char * expr_op_end = expr_start_op;


			if((operator_group != SUB_PUNCTUATOR)){ // no advance ... we'll evaluate with preoperator
				expr_op_end+=strlen(defined_operator_punctuator[operator_group].str);
			}

			print_ast_cr("operator \"%s\" found we can evaluate left and right branches!!\n",CASTNode::defined_operator_punctuator[operator_group].str);
			char eval_left[MAX_EXPRESSION_LENGTH]={0};

			// LEFT BRANCH
			strncpy(eval_left,s_effective_start,expr_start_op-s_effective_start); // copy its left side...
			if(parseExpression_Recursive(eval_left,
										start_line, // start line because was reparsed before...
										 scope_info,
										 ast_node_to_be_evaluated != NULL ? &left_node: NULL,
										 type_group,
										 ast_node_to_be_evaluated != NULL? *ast_node_to_be_evaluated : NULL)==NULL){
				return NULL;
			}



			if(ast_node_to_be_evaluated != NULL){
				if(left_node != NULL){
					(*ast_node_to_be_evaluated)->children.push_back(left_node->idxAstNode);

					if(operator_group == __PUNCTUATOR_TYPE_OLD__::FIELD_PUNCTUATOR){
						(*ast_node_to_be_evaluated)->pre_post_operator_info=left_node->pre_post_operator_info;
						left_node->pre_post_operator_info=__PUNCTUATOR_TYPE_OLD__::UNKNOWN_PUNCTUATOR;
					}
				}
			}

			// RIGHT BRANCH
			if((aux=parseExpression_Recursive(
									expr_op_end,
									m_line,
									scope_info,
									ast_node_to_be_evaluated != NULL ? &right_node : NULL,
									type_group,
									ast_node_to_be_evaluated != NULL ? (*ast_node_to_be_evaluated) : NULL)) == NULL){
				return NULL;
			}

			if(ast_node_to_be_evaluated != NULL){
				if(right_node != NULL){
					(*ast_node_to_be_evaluated)->children.push_back(right_node->idxAstNode);
				}
			}

			if(ast_node_to_be_evaluated != NULL){
				// minus operators has special management because two negatives can be + but sums of negatives works
				if(pre_operator == SUB_PUNCTUATOR) { // check -(expr)
					// 1. change - by +
					//operator_group=ADD_PUNCTUATOR;
					CASTNode *rn =AST_NODE((*ast_node_to_be_evaluated)->children[RIGHT_NODE]);



					// 3. insert node between rigth node and ast_node

					if( rn->is_packed_node){ // end symbol node... let's take the right one...

						// 2. create neg node.
						PASTNode ast_neg_node=NULL;
						if((ast_neg_node = CASTNode::newASTNode())==NULL) return NULL;
						ast_neg_node->node_type = NODE_TYPE::PUNCTUATOR_NODE;
						ast_neg_node->operator_info = SUB_PUNCTUATOR;


						ast_neg_node->idxAstParent = (*ast_node_to_be_evaluated)->idxAstNode;
						ast_neg_node->children.push_back((*ast_node_to_be_evaluated)->children[RIGHT_NODE]);
						(*ast_node_to_be_evaluated)->children[RIGHT_NODE]=ast_neg_node->idxAstNode;
					}
				}

				if(operator_group == SUB_PUNCTUATOR){
					operator_group=ADD_PUNCTUATOR;
				}

				(*ast_node_to_be_evaluated)->node_type = PUNCTUATOR_NODE;
				(*ast_node_to_be_evaluated)->operator_info = operator_group;
				(*ast_node_to_be_evaluated)->idxScope = ZS_UNDEFINED_IDX;
				if(scope_info != NULL){
					(*ast_node_to_be_evaluated)->idxScope = scope_info->idxScope;
				}
				(*ast_node_to_be_evaluated)->line_value = m_lineOperator;
			}
		}
		return aux;
	}

	char * CASTNode::parseExpression_old(const char *s, int & m_line, CScope *scope_info, PASTNode * ast_node_to_be_evaluated ){

		// PRE: s is current string to parse. This function tries to parse an expression like i+1; and generates binary ast.
		// If this functions finds ';' then the function will generate ast.
		if(*s==0) {
			writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"End string");
			return NULL;
		}

		// last character is in charge of who is calling parseExpression because there's many ending cases ): [ ';' ',' ')' , ']' ]
		char *aux_p = parseExpression_Recursive(s,m_line,scope_info,ast_node_to_be_evaluated);
		//char *aux = parseExpression_Recursive(s, m_line, scope_info, ast_node_to_be_evaluated, NULL);

		if(aux_p != NULL && ast_node_to_be_evaluated != NULL && *ast_node_to_be_evaluated!=NULL){ // can save the node and tells that is an starting of expression node...

			PASTNode ast_node= NULL;
			if((ast_node=CASTNode::newASTNode())==NULL) return NULL;
			ast_node->node_type = EXPRESSION_NODE;
			ast_node->children.push_back((*ast_node_to_be_evaluated)->idxAstNode);
			(*ast_node_to_be_evaluated)->idxAstParent = ast_node->idxAstNode; // save parent ..
			*ast_node_to_be_evaluated=ast_node;
		}
		return aux_p;
	}*/

	char * CASTNode::parseExpression_Recursive(const char *s, int & m_line,CScope *scope_info, PASTNode *ast_node_to_be_evaluated, GROUP_TYPE type_group,PASTNode parent ){
		// PRE: **ast_node_to_be_evaluated must be created and is i/o ast pointer variable where to write changes.
		char *aux=(char *)s;
		char *s_effective_start=(char *)s;
		char *expr_start_op=NULL;
		int start_line = m_line; // set another start line because left node or reparse to try another group was already parsed before.
		int m_lineOperator=-2;
		char *end_expression=(char *)s ; // by default end expression isequal to

		bool is_symbol_trivial_value=false;
		string symbol_value;
		string operator_str="";
		__PUNCTUATOR_TYPE_OLD__ pre_operator=__PUNCTUATOR_TYPE_OLD__::UNKNOWN_PUNCTUATOR,
						post_operator=__PUNCTUATOR_TYPE_OLD__::UNKNOWN_PUNCTUATOR,
						pre_operator_packed_node=__PUNCTUATOR_TYPE_OLD__::UNKNOWN_PUNCTUATOR,
						operator_group=__PUNCTUATOR_TYPE_OLD__::UNKNOWN_PUNCTUATOR;
		bool theres_some_operator=false;
		int m_definedSymbolLine;
		bool special_pre_post_cond = false; // in case of particular pre/post...
		bool is_packed_node = false;

		aux=IGNORE_BLANKS(aux, m_line);

		if(isMarkEndExpression(*aux)){ // returning because is trivial!
			return aux;
		}

		if(type_group>=MAX_GROUPS) {
			THROW_RUNTIME_ERROR("Internal:Cannot find ast tree operator");
			return NULL;
		}

		print_ast_cr("new expression eval:\"%.80s ...\" group:%i at line %i",aux,type_group, m_line);

		// searching for operator!
		if(*aux == '{'){ //json expression...
			print_ast_cr("detected json expression");
			return parseStruct(aux,m_line,scope_info,ast_node_to_be_evaluated);
		}

		print_ast_cr("searching for operator type %i...",type_group);

		while(!isMarkEndExpression(*aux) && (operator_group==0)){
			special_pre_post_cond = false;
			print_ast_cr("checkpoint1:%c\n",*aux);
			// 1. ignore spaces...
			aux=IGNORE_BLANKS(aux, m_line);

			if((pre_operator=checkPreOperatorPunctuator(aux))!=__PUNCTUATOR_TYPE_OLD__::UNKNOWN_PUNCTUATOR){

				aux+=strlen(defined_operator_punctuator[pre_operator].str);
				aux=IGNORE_BLANKS(aux, m_line);
			}

			if(*aux=='('){ // packed node let's said that is a packed node...

				// TODO: Recursive ...
			}

			int start_get_symbol_line=m_line;
			// try get symbol string
			if((aux=getSymbolValue(aux, m_line, scope_info,symbol_value, m_definedSymbolLine,pre_operator,post_operator,is_symbol_trivial_value)) == NULL){
				return NULL;
			}

			print_ast_cr("checkpoint3:%c\n",*aux);

			aux=IGNORE_BLANKS(aux, m_line);

			if(!isMarkEndExpression(*aux)){ // is not end expression

				if((operator_group=isOperatorPunctuator(aux))!=0){

					// particular cases... since the getSymbol alrady checks the pre operator it cannot be possible to get a pre inc or pre dec...
					if(operator_group ==  PRE_INC_PUNCTUATOR){ // ++ really is a + PUNCTUATOR...
						operator_group = ADD_PUNCTUATOR;
						special_pre_post_cond=true;
					}
					else if(operator_group ==  PRE_DEC_PUNCTUATOR){ // -- really is a + PUNCTUATOR...
						operator_group = SUB_PUNCTUATOR;
						pre_operator = SUB_PUNCTUATOR;
					}

					theres_some_operator |= true;
					expr_start_op=aux;
					m_lineOperator = m_line;

					if(operator_group != SUB_PUNCTUATOR) // advance ...
						aux+=strlen(defined_operator_punctuator[operator_group].str);

					if(!special_pre_post_cond && (operator_group != SUB_PUNCTUATOR)){ // not check because special case pre/post op...

						switch(type_group){
						case GROUP_0:	operator_group = parsePunctuatorGroup0(expr_start_op);break;
						case GROUP_1:	operator_group = parsePunctuatorGroup1(expr_start_op);break;
						case GROUP_2:	operator_group = parsePunctuatorGroup2(expr_start_op);break;
						case GROUP_3:	operator_group = parsePunctuatorGroup3(expr_start_op);break;
						case GROUP_4:	operator_group = parsePunctuatorGroup4(expr_start_op);break;
						case GROUP_5:	operator_group = parsePunctuatorGroup5(expr_start_op);break;
						case GROUP_6:	operator_group = parsePunctuatorGroup6(expr_start_op);break;
						case GROUP_7:	operator_group = parsePunctuatorGroup7(expr_start_op);break;
						case GROUP_8:	operator_group = parsePunctuatorGroup8(expr_start_op);break;
						default: break;
						}
					}
				}else{
					writeErrorMsg(CURRENT_PARSING_FILENAME,start_get_symbol_line,"expected ';' or operator or punctuator after \"%s\"",symbol_value.c_str());
					return NULL;
				}
			}
		}

		if(operator_group==__PUNCTUATOR_TYPE_OLD__::UNKNOWN_PUNCTUATOR) {// there's no any operators \"type_group\"...
			if(!theres_some_operator){ // only we have a value (trivial)

				if(ast_node_to_be_evaluated != NULL){

					if(is_symbol_trivial_value){


						if(SCOPE_NODE(scope_info->idxScope)->getIdxBaseScope()==IDX_GLOBAL_SCOPE){
							if(symbol_value == "this"){
								writeErrorMsg(CURRENT_PARSING_FILENAME,m_definedSymbolLine,"\"this\" keyword is allowed only in member classes");
								return NULL;
							}

							if(symbol_value == "super"){
								writeErrorMsg(CURRENT_PARSING_FILENAME,m_definedSymbolLine,"\"super\" keyword is allowed only in member classes");
								return NULL;
							}
						}

						if(((*ast_node_to_be_evaluated)=CASTNode::newASTNode()) == NULL) return NULL;
						(*ast_node_to_be_evaluated)->node_type = SYMBOL_NODE;
						(*ast_node_to_be_evaluated)->symbol_value=symbol_value; // assign its value ...
						(*ast_node_to_be_evaluated)->idxScope = scope_info->idxScope;

					}else{
						if(deduceExpression(symbol_value.c_str(),m_definedSymbolLine,scope_info, ast_node_to_be_evaluated, parent) == NULL){
							return NULL;
						}

						if(is_packed_node){ // packed node let's say that is a packed node...
							if(ast_node_to_be_evaluated != NULL){
								(*ast_node_to_be_evaluated)->is_packed_node = true;

								if(pre_operator_packed_node != UNKNOWN_PUNCTUATOR){ // we must create op...
									// 2. create neg node.

									//(*ast_node_to_be_evaluated)->pre_post_operator_info = UNKNOWN_PUNCTUATOR;
									PASTNode ast_neg_node=NULL;
									if((ast_neg_node = CASTNode::newASTNode())==NULL) return NULL;
									ast_neg_node->node_type = NODE_TYPE::PUNCTUATOR_NODE;
									ast_neg_node->operator_info = pre_operator_packed_node;

									ast_neg_node->idxAstParent =(* ast_node_to_be_evaluated)->idxAstParent;
									(*ast_node_to_be_evaluated)->idxAstParent = ast_neg_node->idxAstNode;
									ast_neg_node->children.push_back((*ast_node_to_be_evaluated)->idxAstNode);

									(*ast_node_to_be_evaluated)=ast_neg_node;
								}
							}
						}
					}

					print_ast_cr("---------------------");
					print_ast_cr("%s value \"%s\" at line %i",(is_symbol_trivial_value?"trivial":"NOT trivial"),symbol_value.c_str(), m_definedSymbolLine);
					print_ast_cr("---------------------");

					// put values by default ...
					(*ast_node_to_be_evaluated)->idxAstParent=ZS_UNDEFINED_IDX;
					if(parent!=NULL){
						(*ast_node_to_be_evaluated)->idxAstParent=parent->idxAstNode;
					}
					(*ast_node_to_be_evaluated)->line_value=m_definedSymbolLine;


					if(pre_operator!= __PUNCTUATOR_TYPE_OLD__::UNKNOWN_PUNCTUATOR || post_operator != __PUNCTUATOR_TYPE_OLD__::UNKNOWN_PUNCTUATOR){ // create pre operator node ...

						if(post_operator!=__PUNCTUATOR_TYPE_OLD__::UNKNOWN_PUNCTUATOR)
							(*ast_node_to_be_evaluated)->pre_post_operator_info = post_operator; // preNodePunctuator(post_operator,*ast_node_to_be_evaluated);

						if(pre_operator!=__PUNCTUATOR_TYPE_OLD__::UNKNOWN_PUNCTUATOR){
							if(post_operator!=__PUNCTUATOR_TYPE_OLD__::UNKNOWN_PUNCTUATOR){
								if(     (pre_operator == PRE_INC_PUNCTUATOR  || pre_operator  == PRE_DEC_PUNCTUATOR) &&
										(post_operator== POST_INC_PUNCTUATOR || post_operator == POST_DEC_PUNCTUATOR)){
									writeErrorMsg(CURRENT_PARSING_FILENAME,m_definedSymbolLine,"object \"%s\" has left \"%s\" and right \"%s\" is ambiguous",(*ast_node_to_be_evaluated)->symbol_value.c_str(),defined_operator_punctuator[pre_operator].str, defined_operator_punctuator[post_operator].str);
									return NULL;
								}
							}
							(*ast_node_to_be_evaluated)->pre_post_operator_info = pre_operator; //preNodePunctuator(pre_operator,*ast_node_to_be_evaluated);
						}
					}
				}
			}
			else{

				if(end_expression!= NULL){
				// there's a Punctuator, so let's perform generate its AST
					// reset prePunctuator...
					pre_operator=__PUNCTUATOR_TYPE_OLD__::UNKNOWN_PUNCTUATOR;
					print_ast_cr("try to generate group1 expression: %.40s ...\n",s_effective_start);
					return parseExpression_Recursive(
							s,
							start_line, // start line because was reparsed before (i.e group -1)
							scope_info,
							ast_node_to_be_evaluated,
							(GROUP_TYPE)(((int)type_group)+1),parent);
				}
			}
		}else{ // we found the operator respect of GROUPX so let's put the AST to the left the resulting expression...

			PASTNode left_node = NULL;
			PASTNode right_node = NULL;

			if(ast_node_to_be_evaluated != NULL){
				if((*ast_node_to_be_evaluated=CASTNode::newASTNode())==NULL) return NULL; // always preallocate 2 nodes (left and right)
				(*ast_node_to_be_evaluated)->idxAstParent = ZS_UNDEFINED_IDX;
				if(parent!=NULL){
					(*ast_node_to_be_evaluated)->idxAstParent=parent->idxAstNode;
				}
				(*ast_node_to_be_evaluated)->node_type = EXPRESSION_NODE;
			}
			char * expr_op_end = expr_start_op;


			if((operator_group != SUB_PUNCTUATOR)){ // no advance ... we'll evaluate with preoperator
				expr_op_end+=strlen(defined_operator_punctuator[operator_group].str);
			}

			print_ast_cr("operator \"%s\" found we can evaluate left and right branches!!\n",CASTNode::defined_operator_punctuator[operator_group].str);
			char eval_left[MAX_EXPRESSION_LENGTH]={0};

			// LEFT BRANCH
			strncpy(eval_left,s_effective_start,expr_start_op-s_effective_start); // copy its left side...
			if(parseExpression_Recursive(eval_left,
										start_line, // start line because was reparsed before...
										 scope_info,
										 ast_node_to_be_evaluated != NULL ? &left_node: NULL,
										 type_group,
										 ast_node_to_be_evaluated != NULL? *ast_node_to_be_evaluated : NULL)==NULL){
				return NULL;
			}



			if(ast_node_to_be_evaluated != NULL){
				if(left_node != NULL){
					(*ast_node_to_be_evaluated)->children.push_back(left_node->idxAstNode);

					if(operator_group == __PUNCTUATOR_TYPE_OLD__::FIELD_PUNCTUATOR){
						(*ast_node_to_be_evaluated)->pre_post_operator_info=left_node->pre_post_operator_info;
						left_node->pre_post_operator_info=__PUNCTUATOR_TYPE_OLD__::UNKNOWN_PUNCTUATOR;
					}
				}
			}

			// RIGHT BRANCH
			if((aux=parseExpression_Recursive(
									expr_op_end,
									m_line,
									scope_info,
									ast_node_to_be_evaluated != NULL ? &right_node : NULL,
									type_group,
									ast_node_to_be_evaluated != NULL ? (*ast_node_to_be_evaluated) : NULL)) == NULL){
				return NULL;
			}

			if(ast_node_to_be_evaluated != NULL){
				if(right_node != NULL){
					(*ast_node_to_be_evaluated)->children.push_back(right_node->idxAstNode);
				}
			}

			if(ast_node_to_be_evaluated != NULL){
				// minus operators has special management because two negatives can be + but sums of negatives works
				if(pre_operator == SUB_PUNCTUATOR) { // check -(expr)
					// 1. change - by +
					//operator_group=ADD_PUNCTUATOR;
					CASTNode *rn =AST_NODE((*ast_node_to_be_evaluated)->children[RIGHT_NODE]);



					// 3. insert node between rigth node and ast_node

					if(/*rn->node_type == NODE_TYPE::SYMBOL_NODE ||*/ rn->is_packed_node){ // end symbol node... let's take the right one...

						// 2. create neg node.
						PASTNode ast_neg_node=NULL;
						if((ast_neg_node = CASTNode::newASTNode())==NULL) return NULL;
						ast_neg_node->node_type = NODE_TYPE::PUNCTUATOR_NODE;
						ast_neg_node->operator_info = SUB_PUNCTUATOR;


						ast_neg_node->idxAstParent = (*ast_node_to_be_evaluated)->idxAstNode;
						ast_neg_node->children.push_back((*ast_node_to_be_evaluated)->children[RIGHT_NODE]);
						(*ast_node_to_be_evaluated)->children[RIGHT_NODE]=ast_neg_node->idxAstNode;
					}
				}

				if(operator_group == SUB_PUNCTUATOR){
					operator_group=ADD_PUNCTUATOR;
				}

				(*ast_node_to_be_evaluated)->node_type = PUNCTUATOR_NODE;
				(*ast_node_to_be_evaluated)->operator_info = operator_group;
				(*ast_node_to_be_evaluated)->idxScope = ZS_UNDEFINED_IDX;
				if(scope_info != NULL){
					(*ast_node_to_be_evaluated)->idxScope = scope_info->idxScope;
				}
				(*ast_node_to_be_evaluated)->line_value = m_lineOperator;
			}
		}
		return aux;
	}

	char * CASTNode::parseExpression(const char *s, int & m_line, CScope *scope_info, PASTNode * ast_node_to_be_evaluated ){


		vector<CASTNode> vt;

		// PRE: s is current string to parse. This function tries to parse an expression like i+1; and generates binary ast.
		// If this functions finds ';' then the function will generate ast.
		if(*s==0) {
			writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"End string");
			return NULL;
		}

		// last character is in charge of who is calling parseExpression because there's many ending cases ): [ ';' ',' ')' , ']' ]
		char *aux_p = parseExpression_Recursive(s,m_line,scope_info,ast_node_to_be_evaluated);
		//char *aux = parseExpression_Recursive(s, m_line, scope_info, ast_node_to_be_evaluated, NULL);

		if(aux_p != NULL && ast_node_to_be_evaluated != NULL && *ast_node_to_be_evaluated!=NULL){ // can save the node and tells that is an starting of expression node...

			PASTNode ast_node= NULL;
			if((ast_node=CASTNode::newASTNode())==NULL) return NULL;
			ast_node->node_type = EXPRESSION_NODE;
			ast_node->children.push_back((*ast_node_to_be_evaluated)->idxAstNode);
			(*ast_node_to_be_evaluated)->idxAstParent = ast_node->idxAstNode; // save parent ..
			*ast_node_to_be_evaluated=ast_node;
		}
		return aux_p;
	}

	//---------------------------------------------------------------------------------------------------------------
	// PARSE OBJECT FUNCTIONS

	char * CASTNode::parseStruct_Recursive(const char *s,int & m_line,  CScope *scope_info, PASTNode *ast_node_to_be_evaluated){
		// PRE: **ast_node_to_be_evaluated must be created and is i/o ast pointer variable where to write changes.
		char *aux_p = (char *)s;
		string symbol_value;
		char *end_p;
		PASTNode attr_node = NULL;
		int m_lineSymbol;

		if(*aux_p == '{'){ // go for final ...

			if(ast_node_to_be_evaluated!=NULL){
				if(((*ast_node_to_be_evaluated)=CASTNode::newASTNode())==NULL) return NULL;
				(*ast_node_to_be_evaluated)->line_value = m_line;
				(*ast_node_to_be_evaluated)->node_type = STRUCT_NODE;
				(*ast_node_to_be_evaluated)->idxScope = ZS_UNDEFINED_IDX;
				if(scope_info != NULL){
					(*ast_node_to_be_evaluated)->idxScope = scope_info->idxScope;
				}
			}

			// this solve problem void structs...
			aux_p=IGNORE_BLANKS(aux_p+1,m_line);

			while (*aux_p != '}' && *aux_p != 0){

				m_lineSymbol = m_line;
				//aux_p=IGNORE_BLANKS(aux_p+1,m_line);

				// expect word...
				end_p = getEndWord(aux_p, m_line);

				 if(end_p == NULL || end_p == aux_p){
					 writeErrorMsg(CURRENT_PARSING_FILENAME,m_lineSymbol ,"Expected symbol after ','");
					 return NULL;
				 }


				 symbol_value = CZetScriptUtils::copyStringFromInterval(aux_p,end_p);
				 aux_p=IGNORE_BLANKS(end_p,m_line);

				 if(*aux_p != ':'){ // expected : ...
					 writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Expected ':'");
					 return NULL;
				 }

				 aux_p++;

				 // go to variable...
				 if((aux_p=parseExpression(aux_p,m_line,scope_info,ast_node_to_be_evaluated!=NULL?&attr_node:NULL)) == NULL){  //IGNORE_BLANKS(aux_p+1,m_line);
					 return NULL;
				 }

				 // for each attribute we stack to items SYMBOL_NODE and EXPRESSION_NODE ...
				 if(ast_node_to_be_evaluated!=NULL){
					 attr_node->symbol_value = symbol_value;
					 attr_node->line_value = m_lineSymbol;
					(*ast_node_to_be_evaluated)->children.push_back(attr_node->idxAstNode);
					attr_node->idxAstParent =(*ast_node_to_be_evaluated)->idxAstNode;
				 }

				 aux_p=IGNORE_BLANKS(aux_p,m_line);

				 if(*aux_p == ','){
					 aux_p=IGNORE_BLANKS(aux_p+1,m_line);
				 }
				 else if(*aux_p != '}' ){
					 writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"expected '}' or ','");
					 return NULL;
				 }
			}
		}
		else{
			writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Expected '{'");
		}
		return aux_p;
	}

	char * CASTNode::parseStruct(const char *s,int & m_line,  CScope *scope_info, PASTNode *ast_node_to_be_evaluated){
		// PRE: **ast_node_to_be_evaluated must be created and is i/o ast pointer variable where to write changes.
		char *end=parseStruct_Recursive(s,m_line,  scope_info, ast_node_to_be_evaluated);

		if(end == NULL){
			return NULL;
		}

		if(*end != '}'){
			writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Expected '{'");
			return NULL;
		}

		end=IGNORE_BLANKS(end+1,m_line);

		return (end);
	}
	//---------------------------------------------------------------------------------------------------------------
	// PARSE KEYWORDS

	char * CASTNode::parseNew(const char *s,int & m_line,  CScope *scope_info, PASTNode *ast_node_to_be_evaluated){
		// PRE: **ast_node_to_be_evaluated must be created and is i/o ast pointer variable where to write changes.
		char *aux_p = (char *)s;
		char *end_p;
		string symbol_value;

		KEYWORD_TYPE key_w;
		PASTNode args_node=NULL;

		aux_p=IGNORE_BLANKS(aux_p,m_line);

		// check for keyword ...
		key_w = isKeyword(aux_p);

		if(key_w != KEYWORD_TYPE::UNKNOWN_KEYWORD){

			if(key_w == KEYWORD_TYPE::NEW_KEYWORD){
				aux_p=IGNORE_BLANKS(aux_p+strlen(defined_keyword[key_w].str),m_line);
				// try get symbol ...

				end_p = getEndWord(aux_p, m_line);

				 if(end_p == NULL || end_p == aux_p){
					 writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Expected symbol");
					 return NULL;
				 }
				 symbol_value = CZetScriptUtils::copyStringFromInterval(aux_p,end_p);
				 aux_p=IGNORE_BLANKS(end_p,m_line);

				 if(*aux_p != '('){
					 writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Expected '(' after \'%s\'",defined_keyword[key_w].str);
					 return NULL;
				 }

				 aux_p = parseArgs('(', ')',aux_p,m_line,scope_info,&args_node);
				 if(aux_p == NULL){
					 return NULL;
				 }

				args_node->node_type = ARGS_PASS_NODE;
				 // it seems everything is allright... let's create the node...

				if(((*ast_node_to_be_evaluated) = CASTNode::newASTNode())==NULL) return NULL;
				(*ast_node_to_be_evaluated)->node_type = NEW_OBJECT_NODE;
				(*ast_node_to_be_evaluated)->keyword_info = KEYWORD_TYPE::UNKNOWN_KEYWORD;
				(*ast_node_to_be_evaluated)->symbol_value = symbol_value;
				(*ast_node_to_be_evaluated)->children.push_back(args_node->idxAstNode);

				return aux_p;
			}
		}
		return NULL;
	}

	char * CASTNode::parseDelete(const char *s,int & m_line,  CScope *scope_info, PASTNode *ast_node_to_be_evaluated){
		// PRE: **ast_node_to_be_evaluated must be created and is i/o ast pointer variable where to write changes.
		char *aux_p = (char *)s;
		char *end_p;
		string symbol_value;
		KEYWORD_TYPE key_w;
		PASTNode   value;

		aux_p=IGNORE_BLANKS(aux_p,m_line);

		// check for keyword ...
		key_w = isKeyword(aux_p);

		if(key_w != KEYWORD_TYPE::UNKNOWN_KEYWORD){
			if(key_w == KEYWORD_TYPE::DELETE_KEYWORD){

				aux_p=IGNORE_BLANKS(aux_p+strlen(defined_keyword[key_w].str),m_line);

				end_p = getEndWord(aux_p, m_line);

				 if(end_p == NULL || end_p == aux_p){
					 writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Expected symbol");
					 return NULL;
				 }

				 symbol_value = CZetScriptUtils::copyStringFromInterval(aux_p,end_p);

				if(ast_node_to_be_evaluated != NULL){
					if(((*ast_node_to_be_evaluated) = CASTNode::newASTNode()) == NULL) return NULL;
					(*ast_node_to_be_evaluated)->node_type = DELETE_OBJECT_NODE;
					(*ast_node_to_be_evaluated)->keyword_info = KEYWORD_TYPE::UNKNOWN_KEYWORD;
					(*ast_node_to_be_evaluated)->line_value=m_line;
					(*ast_node_to_be_evaluated)->symbol_value = symbol_value;
					(*ast_node_to_be_evaluated)->idxScope = scope_info->idxScope;
				}

				//parse expression ...
				if((aux_p = parseExpression(aux_p,m_line,scope_info,(ast_node_to_be_evaluated != NULL ? &value : NULL)))==NULL){
					return NULL;
				}

				if(ast_node_to_be_evaluated != NULL){
					(*ast_node_to_be_evaluated)->children.push_back(value->idxAstNode);
				}

				return aux_p;
			}
		}
		return NULL;
	}

	char * CASTNode::parseClass(const char *s,int & m_line, CScope *scope_info, PASTNode *ast_node_to_be_evaluated){
		// PRE: **ast_node_to_be_evaluated must be created and is i/o ast pointer variable where to write changes.
		char *aux_p = (char *)s;
		char *end_p;

		CScope *class_scope_info=NULL;
		//CScriptClass *class_info=NULL;
		int class_line;
		string class_name;
		//CScriptFunctionObject * class_object=NULL;

		KEYWORD_TYPE key_w;
		PASTNode function_collection_node=NULL,vars_collection_node=NULL,child_node=NULL, base_class_node = NULL;

		aux_p=IGNORE_BLANKS(aux_p,m_line);

		// check for keyword ...
		key_w = isKeyword(aux_p);

		if(key_w != KEYWORD_TYPE::UNKNOWN_KEYWORD){

			if(key_w == KEYWORD_TYPE::CLASS_KEYWORD){

				if(scope_info->getIdxParent()!=ZS_UNDEFINED_IDX){
					writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"class keyword is not allowed");
					return NULL;
				}

				aux_p=IGNORE_BLANKS(aux_p+strlen(defined_keyword[key_w].str),m_line);

				// check for symbol's name
				end_p = getEndWord(aux_p, m_line);
				if(end_p == NULL || end_p == aux_p){
					writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Expected name class");
					return NULL;
				}
				// try to register class...
				class_line = m_line;
				class_name = CZetScriptUtils::copyStringFromInterval(aux_p, end_p);

				zs_print_debug_cr("registered class \"%s\" line %i ",class_name.c_str(), class_line);

				aux_p=IGNORE_BLANKS(end_p,m_line);

				if(*aux_p == ':' ){
					string ext_name;
					aux_p=IGNORE_BLANKS(aux_p+1,m_line);

					end_p = getEndWord(aux_p, m_line);

					if(end_p == NULL || end_p == aux_p){
						writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Expected class name");
						return NULL;
					}

					ext_name=CZetScriptUtils::copyStringFromInterval(aux_p, end_p);

					if(ast_node_to_be_evaluated != NULL){
						if((base_class_node = CASTNode::newASTNode()) == NULL) return NULL;
						base_class_node->symbol_value = CZetScriptUtils::copyStringFromInterval(aux_p, end_p);
						base_class_node->node_type = BASE_CLASS_NODE;
					}

					aux_p=IGNORE_BLANKS(end_p, m_line);
				}
				if(*aux_p == '{' ){

					aux_p=IGNORE_BLANKS(aux_p+1,m_line);

					// it seem's we have a good built class...
					//if(ast_node_to_be_evaluated != NULL){
					if((*ast_node_to_be_evaluated = CASTNode::newASTNode())== NULL) return NULL;
					(*ast_node_to_be_evaluated)->node_type = KEYWORD_NODE;
					(*ast_node_to_be_evaluated)->keyword_info = key_w;
					(*ast_node_to_be_evaluated)->symbol_value = class_name;
					(*ast_node_to_be_evaluated)->line_value = m_line;
					(*ast_node_to_be_evaluated)->idxFilename = CURRENT_IDX_PARSING_FILENAME;

					CScope *scp = CScope::newScope((*ast_node_to_be_evaluated));
					(*ast_node_to_be_evaluated)->idxScope =scp->idxScope;
					class_scope_info =scp;

					// create var & functions collection...
					if((vars_collection_node = CASTNode::newASTNode())==NULL) return NULL;
					if((function_collection_node = CASTNode::newASTNode())==NULL) return NULL;
					(*ast_node_to_be_evaluated)->children.push_back(vars_collection_node->idxAstNode);     // children[0] --> vars
					(*ast_node_to_be_evaluated)->children.push_back(function_collection_node->idxAstNode); // children[1] --> functions

					if(base_class_node != NULL) {
						(*ast_node_to_be_evaluated)->children.push_back(	base_class_node ->idxAstNode);
					}
					// register info class ...
					// check for named functions or vars...
					while(*aux_p != '}' && *aux_p != 0){

						// 1st. check whether parse a keyword...
						key_w = isKeyword(aux_p);
						if(key_w != KEYWORD_TYPE::UNKNOWN_KEYWORD){
							switch(key_w){
							default:
								writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Expected \"var\" or \"function\" keyword");
								return NULL;
								break;
							case KEYWORD_TYPE::FUNCTION_KEYWORD:
								if((aux_p = parseFunction(aux_p, m_line,class_scope_info, &child_node)) != NULL){
									if(child_node->symbol_value != ""){
										if(ast_node_to_be_evaluated != NULL){
											function_collection_node->children.push_back(child_node->idxAstNode);
										}
									}
									else {
										writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Expected symbol after function");
										return NULL;
									}
								} else{
									return NULL;
								}
								break;
							case KEYWORD_TYPE::VAR_KEYWORD:
								if((aux_p = parseVar(aux_p, m_line,class_scope_info, &child_node)) != NULL){

									if(ast_node_to_be_evaluated != NULL){
										vars_collection_node->children.push_back(child_node->idxAstNode);
									}

								} else{
									return NULL;
								}
								break;
							}
						}else{
							writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Expected \"var\" or \"function\" keyword");
							return NULL;
						}
						aux_p=IGNORE_BLANKS(aux_p,m_line);
					}

					if(*aux_p != '}'){
						writeErrorMsg(CURRENT_PARSING_FILENAME,class_line ,"class \"%s\" declared is not closed ",class_name.c_str());
						return NULL;
					}

					aux_p=IGNORE_BLANKS(aux_p+1,m_line);

					if(*aux_p != ';'){
						writeErrorMsg(CURRENT_PARSING_FILENAME,class_line ,"class \"%s\" not end with ;",class_name.c_str());
						return NULL;
					}

					return aux_p+1;

				}else{
					writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Expected '{'");
					return NULL;
				}
			}
		}
		return NULL;
	}

	char * CASTNode::parseArgs(char c1,char c2,const char *s,int & m_line,  CScope *scope_info, PASTNode *ast_node_to_be_evaluated){
		// PRE: **ast_node_to_be_evaluated must be created and is i/o ast pointer variable where to write changes.
		char *aux_p = (char *)s;
		PASTNode   node_arg_expression=NULL;

		aux_p=IGNORE_BLANKS(aux_p,m_line);

		// check for keyword ...
		if(*aux_p == c1){
			aux_p++;
			aux_p=IGNORE_BLANKS(aux_p,m_line);

			if(ast_node_to_be_evaluated!=NULL){
				if(((*ast_node_to_be_evaluated) = CASTNode::newASTNode()) == NULL) return NULL;
			}

			if(*aux_p != c2 ){
				if(*aux_p == ',' ){
					writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Unexpected %c",c2);
					return NULL;
				}

				do{
					if((aux_p = parseExpression(aux_p,m_line,scope_info,(ast_node_to_be_evaluated != NULL ? &node_arg_expression : NULL)))==NULL){
						return NULL;
					}

					// push arg into node...
					if(ast_node_to_be_evaluated!=NULL){
						(*ast_node_to_be_evaluated)->children.push_back(node_arg_expression->idxAstNode);
					}

					if(*aux_p == ',' ){
						aux_p++;
						aux_p=IGNORE_BLANKS(aux_p,m_line);
					}else{
						if(*aux_p != c2 ){
							writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Expected %c",c2);
							return NULL;
						}
					}

				}while(*aux_p != c2 && *aux_p != 0);
			}

			return aux_p+1;
		}
		return NULL;
	}

	PASTNode  CASTNode::findAst(const string & _symbol_name_to_find, NODE_TYPE _node_type, KEYWORD_TYPE _keyword_type){
		return findAstRecursive(_symbol_name_to_find, _node_type, _keyword_type, MAIN_AST_NODE);
	}

	PASTNode  CASTNode::findAstRecursive(const string & _symbol_name_to_find, NODE_TYPE _node_type,KEYWORD_TYPE _keyword_type, PASTNode _node){
		if(_node->symbol_value == _symbol_name_to_find && _node->node_type == _node_type){
			if(_node_type==NODE_TYPE::KEYWORD_NODE){
				if(_keyword_type== _node->keyword_info){
					return _node;
				}
			}
			else{
				return _node;
			}
		}

		for(unsigned i = 0; i < _node->children.size(); i++){
			PASTNode ch;
			if((ch=findAstRecursive(_symbol_name_to_find,_node_type,_keyword_type,AST_NODE(_node->children[i])))!=NULL){
				return ch;
			}
		}
		return NULL;
	}

	char * CASTNode::parseFunction(const char *s,int & m_line,  CScope *scope_info, PASTNode *ast_node_to_be_evaluated){

		// PRE: **ast_node_to_be_evaluated must be created and is i/o ast pointer variable where to write changes.
		char *aux_p = (char *)s;
		char *symbol_value,*end_var;
		KEYWORD_TYPE key_w;

		PASTNode args_node=NULL, body_node=NULL, arg_node=NULL;
		string conditional_str;
		bool error=false;

		tScopeVar * irv=NULL;
		string str_name;
		string class_member,class_name, function_name="";
		PASTNode class_node=NULL;
		PASTNode function_collection_node=NULL;
		int idxScope=ZS_UNDEFINED_IDX;
		CScope *body_scope=NULL;


		if(scope_info != NULL){
			idxScope=scope_info->idxScope;
		}

		aux_p=IGNORE_BLANKS(aux_p,m_line);

		// check for keyword ...
		key_w = isKeyword(aux_p);

		if(key_w != KEYWORD_TYPE::UNKNOWN_KEYWORD){

			if(key_w == KEYWORD_TYPE::FUNCTION_KEYWORD){

				// advance keyword...
				aux_p += strlen(defined_keyword[key_w].str);
				aux_p=IGNORE_BLANKS(aux_p,m_line);

				if(ast_node_to_be_evaluated!=NULL){ // we save node...

					if((*ast_node_to_be_evaluated = CASTNode::newASTNode())==NULL) return NULL;
					(*ast_node_to_be_evaluated)->node_type = KEYWORD_NODE;
					(*ast_node_to_be_evaluated)->keyword_info = key_w;

					if((args_node = CASTNode::newASTNode()) == NULL) return NULL;
					(*ast_node_to_be_evaluated)->children.push_back(args_node->idxAstNode);

					args_node->node_type = NODE_TYPE::ARGS_DECL_NODE;

					// start line ...
					(*ast_node_to_be_evaluated)->line_value = m_line;
				}

				bool named_function = *aux_p!='(';

				if(named_function){ // is named function..

					if((end_var=isClassMember(aux_p,m_line,class_name,class_member,class_node, error,key_w))!=NULL){ // check if particular case extension attribute class
						idxScope = class_node->idxScope; // override scope info
						symbol_value = (char *)class_member.c_str();
						function_name = symbol_value;
						function_collection_node=AST_NODE(class_node->children[1]);
					}
					else{
						if(error){
							return NULL;
						}
						else{ // get normal name...

							// check whwther the function is anonymous with a previous arithmetic operation ....
							end_var=getSymbolName(aux_p,m_line);

							if(end_var != NULL){

								if((symbol_value = CZetScriptUtils::copyStringFromInterval(aux_p,end_var))==NULL)
									return NULL;

								function_name = symbol_value;

								// check whether parameter name's matches with some global variable...
							}else{
								return NULL;
							}
						}
					}
					aux_p=end_var;
					aux_p=IGNORE_BLANKS(aux_p,m_line);
				}
				else{ //function node
					if(ast_node_to_be_evaluated!=NULL){ // save as function object...
						(*ast_node_to_be_evaluated)->node_type = FUNCTION_OBJECT_NODE;

					}
				}

				if(ast_node_to_be_evaluated!=NULL){
					PASTNode ast_node = NULL;
					// create object function ...
					ast_node=*ast_node_to_be_evaluated;
					ast_node->idxScope = idxScope;

					body_scope=SCOPE_NODE(idxScope)->pushScope(NULL);
				}
				// parse function args...
				if(*aux_p == '('){ // push arguments...

					aux_p++;
					aux_p=IGNORE_BLANKS(aux_p,m_line);

					// grab words separated by ,
					while(*aux_p != 0 && *aux_p != ')' && *aux_p != '{'){

						aux_p=IGNORE_BLANKS(aux_p,m_line);

						if(*aux_p == ')' || *aux_p == ','){
							writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Expected arg");
							return NULL;
						}

						int m_start_arg=m_line;
						end_var=getSymbolName(aux_p,m_line);

						if((symbol_value = CZetScriptUtils::copyStringFromInterval(aux_p,end_var))==NULL)
							return NULL;

						// check if repeats...
						if(ast_node_to_be_evaluated != NULL){
							for(unsigned k = 0; k < args_node->children.size(); k++){
								if(AST_NODE(args_node->children[k])->symbol_value == symbol_value){
									writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Repeated argument '%s' argument ",symbol_value);
									return NULL;
								}
							}

							// check whether parameter name's matches with some global variable...
							if((irv=body_scope->getInfoRegisteredSymbol(symbol_value,-1,false)) != NULL){
								writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Ambiguous symbol argument \"%s\" name with var defined at %i", symbol_value, AST_LINE(irv->idxAstNode));
								return NULL;
							}
							// ok register symbol into the object function ...
						}
						aux_p=end_var;
						aux_p=IGNORE_BLANKS(aux_p,m_line);

						if(*aux_p != ')'){

							if(*aux_p != ','){
								writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Expected ',' ");
								return NULL;
							}
							aux_p++;
						}
						// PUSH NEW ARG...
						if(ast_node_to_be_evaluated != NULL){
							if((arg_node = CASTNode::newASTNode()) == NULL) return NULL;
							arg_node->line_value=m_start_arg;
							arg_node->symbol_value=symbol_value;
							args_node->children.push_back(arg_node->idxAstNode);

							if((body_scope->registerSymbol(symbol_value,arg_node)) == NULL){
								return NULL;
							}
						}
					}

					if(*aux_p != ')'){
						writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Expected ')'");
						return NULL;
					}


					aux_p++;
					aux_p=IGNORE_BLANKS(aux_p,m_line);

					if(*aux_p != '{'){
						writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Expected '{'");
						return NULL;
					}

					// ok let's go to body..
					if((aux_p = parseBlock(
							aux_p,
							m_line,
							ast_node_to_be_evaluated != NULL ? body_scope:NULL ,
							error,
							ast_node_to_be_evaluated != NULL ? &body_node : NULL,
							ast_node_to_be_evaluated != NULL ? *ast_node_to_be_evaluated : NULL,
							false

						)) != NULL){

						if(!error){

							if(ast_node_to_be_evaluated != NULL){

								// link scope / ast
								body_scope->idxAstNode=body_node->idxAstNode;
								body_node->idxScope =body_scope->idxScope;

								// register function symbol...
								int n_params=0;

								if(args_node != NULL){
									n_params=args_node->children.size();
								}

								if(named_function){ // register named function...
									if((irv=SCOPE_NODE(idxScope)->getInfoRegisteredSymbol(function_name,n_params,false)) != NULL){

										if(irv->idxAstNode!=ZS_UNDEFINED_IDX){
											writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Function name \"%s\" is already defined with same args at %s:%i", function_name.c_str(),GET_AST_FILENAME_LINE(irv->idxAstNode));
										}else{
											writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Function name \"%s\" is no allowed it has conflict with name of already registered function in C/C++", function_name.c_str());
										}

										return NULL;
									}

									if((irv=SCOPE_NODE(idxScope)->registerSymbol(function_name,(*ast_node_to_be_evaluated),n_params))==NULL){
										return NULL;
									}

									if((*ast_node_to_be_evaluated) != NULL){
										(*ast_node_to_be_evaluated)->symbol_value=function_name;
									}

								}else{ // register anonymouse function at global scope...
									irv=SCOPE_NODE(IDX_GLOBAL_SCOPE)->registerAnonymouseFunction((*ast_node_to_be_evaluated));
									(*ast_node_to_be_evaluated)->idxScope=IDX_GLOBAL_SCOPE;
									(*ast_node_to_be_evaluated)->symbol_value=irv->name;
								}

								(*ast_node_to_be_evaluated)->children.push_back(body_node->idxAstNode);

								SCOPE_NODE(idxScope)->popScope();
							}

							return aux_p;
						}
					}

					if(ast_node_to_be_evaluated != NULL && function_collection_node !=NULL){
						function_collection_node->children.push_back((*ast_node_to_be_evaluated)->idxAstNode);
					}
				}
				else{
					writeErrorMsg(CURRENT_PARSING_FILENAME,m_line," Expected '('");
				}
			}else{
				writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Expected operator or function operator");
			}
		}
		return NULL;
	}

	char *  CASTNode::parseReturn(const char *s,int & m_line,  CScope *scope_info, PASTNode *ast_node_to_be_evaluated){
		// PRE: **ast_node_to_be_evaluated must be created and is i/o ast pointer variable where to write changes.
		char *aux_p = (char *)s;
		KEYWORD_TYPE key_w;
		string s_aux;

		aux_p=IGNORE_BLANKS(aux_p,m_line);

		key_w = isKeyword(aux_p);

		if(key_w != KEYWORD_TYPE::UNKNOWN_KEYWORD){

			if(key_w == KEYWORD_TYPE::RETURN_KEYWORD){ // possible variable...
				PASTNode child_node=NULL;
				aux_p += strlen(defined_keyword[key_w].str);

				if(ast_node_to_be_evaluated != NULL){ // save
					if((*ast_node_to_be_evaluated = CASTNode::newASTNode()) == NULL) return NULL; // reserve for expression return
					(*ast_node_to_be_evaluated)->node_type = NODE_TYPE::KEYWORD_NODE;
					(*ast_node_to_be_evaluated)->keyword_info = key_w;
					(*ast_node_to_be_evaluated)->line_value = m_line;
				}

				aux_p=IGNORE_BLANKS(aux_p,m_line);

				if((aux_p = parseExpression(aux_p, m_line, scope_info, ast_node_to_be_evaluated != NULL ? &child_node : NULL))!= NULL){

					if(ast_node_to_be_evaluated != NULL){ // return value;
						if(child_node != NULL){
							(*ast_node_to_be_evaluated)->children.push_back(child_node->idxAstNode);
						}
						else{ // return;

						}
					}

					if(*aux_p!=';'){
						writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Expected ';'");
						return NULL;
					}
					aux_p++;
					return aux_p;
				}
			}
		}
		return NULL;
	}

	char * CASTNode::parseWhile(const char *s,int & m_line, CScope *scope_info, PASTNode *ast_node_to_be_evaluated){

		// PRE: **ast_node_to_be_evaluated must be created and is i/o ast pointer variable where to write changes.
		char *aux_p = (char *)s;
		char *end_expr,*start_symbol;
		KEYWORD_TYPE key_w;
		CScope *_currentScope=NULL;

		PASTNode conditional_expression=NULL, while_node=NULL;
		string conditional_str;
		bool error = false;

		aux_p=IGNORE_BLANKS(aux_p,m_line);

		// check for keyword ...
		key_w = isKeyword(aux_p);

		if(key_w != KEYWORD_TYPE::UNKNOWN_KEYWORD){
			if(key_w == KEYWORD_TYPE::WHILE_KEYWORD){

				if(ast_node_to_be_evaluated != NULL){
					if((*ast_node_to_be_evaluated = CASTNode::newASTNode()) == NULL) return NULL;
					(*ast_node_to_be_evaluated)->node_type = KEYWORD_NODE;
					(*ast_node_to_be_evaluated)->keyword_info = key_w;


					_currentScope =scope_info->pushScope(*ast_node_to_be_evaluated); // push current scope

				}

				aux_p += strlen(defined_keyword[key_w].str);

				// evaluate conditional line ...
				aux_p=IGNORE_BLANKS(aux_p,m_line);
				if(*aux_p == '('){

					if((end_expr = parseExpression(aux_p+1,m_line,_currentScope,&conditional_expression)) != NULL){

						if(*end_expr != ')'){
							writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Expected ')'");
							return NULL;
						}
						if((start_symbol = CZetScriptUtils::copyStringFromInterval(aux_p+1, end_expr))==NULL){
							return NULL;
						}

						if( ast_node_to_be_evaluated != NULL){
							PASTNode aux;
							if((aux= CASTNode::newASTNode()) == NULL) return NULL;
							aux->node_type = CONDITIONAL_NODE;
							aux->children.push_back(conditional_expression->idxAstNode);
							(*ast_node_to_be_evaluated)->children.push_back(aux->idxAstNode);
						}
						aux_p=IGNORE_BLANKS(end_expr+1,m_line);
						if(*aux_p != '{'){
							writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Expected while-block open block ('{') ");
							return NULL;
						}
						if((aux_p=parseBlock(aux_p
								,m_line
								,_currentScope
								,error
								,ast_node_to_be_evaluated != NULL ? &while_node : NULL
								,ast_node_to_be_evaluated != NULL ? *ast_node_to_be_evaluated : NULL
								))!= NULL){
							if(!error){
								if(ast_node_to_be_evaluated != NULL){
									(*ast_node_to_be_evaluated)->children.push_back(while_node->idxAstNode);
								}


								scope_info->popScope();

								return aux_p;
							}
						}
					}else{
						writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Expected ')' while ");
						return NULL;
					}

				}else{
					writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Expected '(' while ");
					return NULL;
				}
			}
		}
		return NULL;
	}

	char * CASTNode::parseDoWhile(const char *s,int & m_line, CScope *scope_info, PASTNode *ast_node_to_be_evaluated){

		// PRE: **ast_node_to_be_evaluated must be created and is i/o ast pointer variable where to write changes.

		char *aux_p = (char *)s;
		char *end_expr,*start_symbol;
		KEYWORD_TYPE key_w;
		CScope *_currentScope=NULL;

		PASTNode conditional_expression=NULL, body_node=NULL, while_node=NULL;
		string conditional_str;
		bool error = false;

		aux_p=IGNORE_BLANKS(aux_p,m_line);

		// check for keyword ...
		key_w = isKeyword(aux_p);

		if(key_w != KEYWORD_TYPE::UNKNOWN_KEYWORD){
			if(key_w == KEYWORD_TYPE::DO_WHILE_KEYWORD){

				if(ast_node_to_be_evaluated != NULL){
					if((*ast_node_to_be_evaluated = CASTNode::newASTNode()) == NULL) return NULL;
					(*ast_node_to_be_evaluated)->node_type = KEYWORD_NODE;
					(*ast_node_to_be_evaluated)->keyword_info = key_w;

					_currentScope =scope_info->pushScope(*ast_node_to_be_evaluated); // push current scope
				}

				aux_p += strlen(defined_keyword[key_w].str);

				//1st evaluate body ..
				aux_p=IGNORE_BLANKS(aux_p,m_line);
				if(*aux_p != '{'){
					writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Expected open block ('{') in do-while expression");
					return NULL;
				}
				if((aux_p=parseBlock(aux_p
						,m_line
						,_currentScope
						,error
						,ast_node_to_be_evaluated != NULL ? &body_node : NULL
						,ast_node_to_be_evaluated != NULL ? *ast_node_to_be_evaluated : NULL
						))!= NULL){
					if(!error){

						// Finally evaluate conditional line ...
						aux_p=IGNORE_BLANKS(aux_p,m_line);

						// check for keyword ...
						key_w = isKeyword(aux_p);

						if(key_w!=WHILE_KEYWORD){
							writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"expected while keyword");
							return NULL;
						}

						aux_p += strlen(defined_keyword[key_w].str);

						aux_p=IGNORE_BLANKS(aux_p,m_line);

						if(*aux_p == '('){

							if((end_expr = parseExpression(aux_p+1,m_line,_currentScope,&conditional_expression)) != NULL){

								if(*end_expr != ')'){
									writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Expected ')'");
									return NULL;
								}
								if((start_symbol = CZetScriptUtils::copyStringFromInterval(aux_p+1, end_expr))==NULL){
									return NULL;
								}

								if( ast_node_to_be_evaluated != NULL){

									if((while_node= CASTNode::newASTNode()) == NULL) return NULL;
									while_node->node_type = CONDITIONAL_NODE;
									while_node->children.push_back(conditional_expression->idxAstNode);
								}



							}else{
								writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Expected ')' do-while expression");
								return NULL;
							}

						}else{
							writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Expected '(' do-while expression");
							return NULL;
						}
						/*if(ast_node_to_be_evaluated != NULL){
							(*ast_node_to_be_evaluated)->children.push_back(while_node->idxAstNode);
						}*/

						// push conditional and body nodes...
						if(ast_node_to_be_evaluated != NULL){

							(*ast_node_to_be_evaluated)->children.push_back(while_node->idxAstNode);
							(*ast_node_to_be_evaluated)->children.push_back(body_node->idxAstNode);
						}

						scope_info->popScope();
						return end_expr+1;
					}
				}
			}
		}
		return NULL;
	}

	char * CASTNode::parseIf(const char *s,int & m_line,  CScope *scope_info, PASTNode *ast_node_to_be_evaluated){

		// PRE: **ast_node_to_be_evaluated must be created and is i/o ast pointer variable where to write changes.
		char *aux_p = (char *)s;
		char *end_expr,*start_symbol;
		int dl=-1;
		KEYWORD_TYPE key_w;
		PASTNode conditional=NULL, if_node=NULL, else_node=NULL,block=NULL, group_conditional_nodes = NULL;
		string conditional_str;
		bool error = false;
		int conditional_line;

		aux_p=IGNORE_BLANKS(aux_p,m_line);

		// check for keyword ...
		key_w = isKeyword(aux_p);

		if(key_w != KEYWORD_TYPE::UNKNOWN_KEYWORD){
			if(key_w == KEYWORD_TYPE::IF_KEYWORD){

				if(ast_node_to_be_evaluated != NULL){
					if((*ast_node_to_be_evaluated = CASTNode::newASTNode()) == NULL) return NULL;
					(*ast_node_to_be_evaluated)->node_type = KEYWORD_NODE;
					(*ast_node_to_be_evaluated)->keyword_info = key_w;

					if((group_conditional_nodes = CASTNode::newASTNode()) == NULL) return NULL;
					group_conditional_nodes->node_type = GROUP_IF_NODES;
					(*ast_node_to_be_evaluated)->children.push_back(group_conditional_nodes->idxAstNode);
				}

				do{

					if(ast_node_to_be_evaluated != NULL){
						if((if_node = CASTNode::newASTNode()) == NULL) return NULL;
						if_node->node_type = IF_NODE;
						group_conditional_nodes->children.push_back(if_node->idxAstNode);
					}

					aux_p += strlen(defined_keyword[key_w].str);
					aux_p=IGNORE_BLANKS(aux_p,m_line);

					if(*aux_p != '('){
						writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Expected '(' if");
						return NULL;
					}

					conditional_line=m_line;

					if((end_expr = parseExpression(aux_p+1,m_line,scope_info,ast_node_to_be_evaluated != NULL? &conditional: NULL)) == NULL){
						writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Expected ')' if ");
						return NULL;
					}

					if(*end_expr != ')'){
						writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Expected ')'");
						return NULL;
					}

					if(IGNORE_BLANKS(aux_p+1,dl)==end_expr){
						writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"no conditional expression");
						return NULL;
					}

					if((start_symbol = CZetScriptUtils::copyStringFromInterval(aux_p+1, end_expr))==NULL){
						return NULL;
					}

					conditional_str=start_symbol;

					if(ast_node_to_be_evaluated!=NULL){
						PASTNode aux;
						if((aux = CASTNode::newASTNode()) == NULL) return NULL;
						aux->children.push_back(conditional->idxAstNode);
						aux->node_type = CONDITIONAL_NODE;
						aux->line_value=conditional_line;
						if_node->children.push_back(aux->idxAstNode);
					}

					aux_p=IGNORE_BLANKS(end_expr+1,m_line);
					if(*aux_p != '{'){
						if(!printErrorUnexpectedKeywordOrPunctuator(aux_p, m_line)){
							writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Expected if-block open block ('{')");
						}
						return NULL;
					}


					if((aux_p=parseBlock(aux_p
							,m_line
							,scope_info
							,error
							,ast_node_to_be_evaluated != NULL ? &block : NULL
							,ast_node_to_be_evaluated != NULL ? *ast_node_to_be_evaluated : NULL
							))== NULL){
						return NULL;
					}

					if(error){
						return NULL;
					}

					if(ast_node_to_be_evaluated != NULL){
						if_node->children.push_back(block->idxAstNode);
					}

					aux_p=IGNORE_BLANKS(aux_p,m_line);

					bool else_key = false;
					if((key_w = isKeyword(aux_p)) != KEYWORD_TYPE::UNKNOWN_KEYWORD){
						else_key = (key_w == KEYWORD_TYPE::ELSE_KEYWORD);
					}

					if(else_key){
						aux_p += strlen(defined_keyword[key_w].str);

						if(*aux_p != '{'){
							aux_p++;
						}

						aux_p=IGNORE_BLANKS(aux_p,m_line);

						bool if_key = false;
						if((key_w = isKeyword(aux_p)) != KEYWORD_TYPE::UNKNOWN_KEYWORD){
							if_key = (key_w == KEYWORD_TYPE::IF_KEYWORD);
						}

						if(!if_key){

							if(*aux_p != '{'){
								writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Expected else-block open block ('{')");
								return NULL;
							}

							if((aux_p=parseBlock(aux_p
									,m_line
									,scope_info
									,error
									,ast_node_to_be_evaluated != NULL ? &else_node : NULL
									,ast_node_to_be_evaluated != NULL ? *ast_node_to_be_evaluated : NULL
									))!= NULL){
									if(!error){

										if( ast_node_to_be_evaluated != NULL){
											(*ast_node_to_be_evaluated)->children.push_back(else_node->idxAstNode);
										}

										return aux_p;
									}
									else{
										return NULL;
									}
							}else{
								return NULL;
							}
						} // else keep up parsing if nodes case ...
					}else{
						return aux_p;
					}

				}while(true); // loop
			}
		}
		return NULL;
	}

	char * CASTNode::parseFor(const char *s,int & m_line,  CScope *scope_info, PASTNode *ast_node_to_be_evaluated){

		// PRE: **ast_node_to_be_evaluated must be created and is i/o ast pointer variable where to write changes.
		char *aux_p = (char *)s;
		KEYWORD_TYPE key_w;
		bool error=false;
		PASTNode block_for = NULL,pre_node=NULL,cond_node=NULL,post_node=NULL, pre_node_expression=NULL, cond_node_expression=NULL,post_node_expression=NULL;
		string eval_for;

		//CScope *_localScope =  scope_info != NULL?scope_info->symbol_info.ast->scope_info_ptr:NULL; // gets scope...
		CScope *_currentScope=NULL;

		aux_p=IGNORE_BLANKS(aux_p,m_line);

		// check for keyword ...
		key_w = isKeyword(aux_p);

		if(key_w != KEYWORD_TYPE::UNKNOWN_KEYWORD){
			if(key_w == KEYWORD_TYPE::FOR_KEYWORD){

				if(ast_node_to_be_evaluated != NULL){
					if((*ast_node_to_be_evaluated = CASTNode::newASTNode()) == NULL) return NULL;; // ini, conditional, post
					(*ast_node_to_be_evaluated)->node_type = KEYWORD_NODE;
					(*ast_node_to_be_evaluated)->keyword_info = key_w;

					if((pre_node= CASTNode::newASTNode()) == NULL) return NULL;
					pre_node->node_type = PRE_FOR_NODE;




					// reserve 3 nodes for init/eval/post_op
					(*ast_node_to_be_evaluated)->children.push_back(pre_node->idxAstNode);


				}

				aux_p += strlen(defined_keyword[key_w].str);
				aux_p=IGNORE_BLANKS(aux_p,m_line);

				if(*aux_p == '('){ // ready ...

					// save scope pointer ...
					if(ast_node_to_be_evaluated != NULL){
						_currentScope =scope_info->pushScope(*ast_node_to_be_evaluated); // push current scope
					}

					aux_p=IGNORE_BLANKS(aux_p+1,m_line);

					if(*aux_p != ';'){ // there's some var init...
						// init node ...
						KEYWORD_TYPE key_w = isKeyword(aux_p);

						if(key_w == VAR_KEYWORD){
							if((aux_p = parseVar(aux_p,m_line, _currentScope, ast_node_to_be_evaluated != NULL ? &pre_node_expression: NULL))==NULL){
								return NULL;
							}

							if(ast_node_to_be_evaluated!=NULL){
								pre_node->children.push_back(pre_node_expression->idxAstNode);
							}
							aux_p = aux_p - 1; // redirect aux_p to ';'
						}
						else{

							writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Expected 'var' keyword");
							return NULL;
						}

					}

					aux_p=IGNORE_BLANKS(aux_p,m_line);

					key_w = isKeyword(aux_p);
					if(key_w == KEYWORD_TYPE::IN_KEYWORD){

						PASTNode node_for_in_right_op_expression=NULL;

						aux_p=IGNORE_BLANKS(aux_p+strlen(defined_keyword[KEYWORD_TYPE::IN_KEYWORD].str),m_line);


						if((aux_p = parseExpression((const char *)aux_p,m_line,_currentScope, ast_node_to_be_evaluated != NULL ? &node_for_in_right_op_expression: NULL)) == NULL){
							return NULL;
						}


						if(ast_node_to_be_evaluated!=NULL){
							(*ast_node_to_be_evaluated)->children.push_back(node_for_in_right_op_expression->idxAstNode);
						}

					}
					else{ // expects conditional and post (i.e for(;;) )

						if(ast_node_to_be_evaluated != NULL){
							if((cond_node= CASTNode::newASTNode()) == NULL) return NULL;
							cond_node->node_type = CONDITIONAL_NODE;

							if((post_node= CASTNode::newASTNode()) == NULL) return NULL;
							post_node->node_type = POST_FOR_NODE;

							(*ast_node_to_be_evaluated)->children.push_back(cond_node->idxAstNode);
							(*ast_node_to_be_evaluated)->children.push_back(post_node->idxAstNode);
						}


						if(*aux_p != ';'){
							writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Expected ';'");
							return NULL;

						}

						aux_p=IGNORE_BLANKS(aux_p+1,m_line);

						if(*aux_p != ';'){ // conditional...
							char * end_p=IGNORE_BLANKS(aux_p+1,m_line);

							if(*end_p != ';'){// there's some condition if not, then is like for(X;true;X)

								if((aux_p = parseExpression((const char *)aux_p,m_line,_currentScope, ast_node_to_be_evaluated != NULL ? &cond_node_expression: NULL)) == NULL){
									return NULL;
								}

								if(ast_node_to_be_evaluated!=NULL){
									cond_node->children.push_back(cond_node_expression->idxAstNode);
								}
							}
						}

						aux_p=IGNORE_BLANKS(aux_p,m_line);

						if(*aux_p != ';'){
							writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Expected ';'");
							return NULL;

						}

						aux_p=IGNORE_BLANKS(aux_p+1,m_line);


						if(*aux_p != ')' ){ // finally do post op...

							if(*aux_p == ',' ){
								writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Unexpected ) ");
								return NULL;
							}

							do{
								if((aux_p = parseExpression(aux_p,m_line,_currentScope,(ast_node_to_be_evaluated != NULL ? &post_node_expression : NULL)))==NULL){
									return NULL;
								}

								// push arg into node...
								if(ast_node_to_be_evaluated!=NULL){
									post_node->children.push_back(post_node_expression->idxAstNode);
								}

								if(*aux_p == ',' ){
									aux_p=IGNORE_BLANKS(aux_p+1,m_line);
								}else{
									if(*aux_p != ')' ){
										writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Expected ')'");
										return NULL;
									}
								}

							}while(*aux_p != ')' && *aux_p != 0);
						}
					}

					if(*aux_p != ')'){
						writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Expected ')'");
						return NULL;

					}




					aux_p=IGNORE_BLANKS(aux_p+1,m_line);
					if(*aux_p != '{'){
						writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Expected '{' for-block");
						return NULL;
					}

					// parse block ...
					if((aux_p=parseBlock(aux_p
							,m_line
							,_currentScope
							,error
							,ast_node_to_be_evaluated != NULL ? &block_for : NULL
							,ast_node_to_be_evaluated != NULL ? *ast_node_to_be_evaluated : NULL
						))!= NULL){ // true: We treat declared variables into for as another scope.
						if(!error){

							if(ast_node_to_be_evaluated != NULL) {
								(*ast_node_to_be_evaluated)->children.push_back(block_for->idxAstNode);
								scope_info->popScope(); // push current scope
							}
							return aux_p;
						}

					}
				}else{
					writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Expected '(' for");
					return NULL;
				}
			}
		}
		return NULL;
	}

	/*char * CASTNode::parseForeach(const char *s,int & m_line,  CScope *scope_info, PASTNode *ast_node_to_be_evaluated){

		// PRE: **ast_node_to_be_evaluated must be created and is i/o ast pointer variable where to write changes.
		char buffer[128]={0};
		char *aux_p = (char *)s;
		char * end_p;
		KEYWORD_TYPE key_w;
		bool error=false;
		PASTNode block_for = NULL,node_foreach_expression=NULL,node_foreach_vars=NULL,node_foreach_symbol=NULL;
		string eval_for;
		//CScope *_localScope =  scope_info != NULL?scope_info->symbol_info.ast->scope_info_ptr:NULL; // gets scope...
		CScope *_currentScope=NULL;

		aux_p=IGNORE_BLANKS(aux_p,m_line);

		// check for keyword ...
		key_w = isKeyword(aux_p);

		if(key_w != KEYWORD_TYPE::UNKNOWN_KEYWORD){
			if(key_w == KEYWORD_TYPE::FOREACH_KEYWORD){

				if(ast_node_to_be_evaluated != NULL){
					if((*ast_node_to_be_evaluated = CASTNode::newASTNode()) == NULL) return NULL;; // ini, conditional, post
					(*ast_node_to_be_evaluated)->node_type = KEYWORD_NODE;
					(*ast_node_to_be_evaluated)->keyword_info = key_w;

					// reserve 2 nodes for iterator symbol and vector symbol
					(*ast_node_to_be_evaluated)->children.push_back(ZS_UNDEFINED_IDX); // vars current vector elements + iterators...
					(*ast_node_to_be_evaluated)->children.push_back(ZS_UNDEFINED_IDX); // vector symbol
					//(*ast_node_to_be_evaluated)->children.push_back(ZS_UNDEFINED_IDX);

				}

				aux_p += strlen(defined_keyword[key_w].str);
				aux_p=IGNORE_BLANKS(aux_p,m_line);

				if(*aux_p == '('){ // ready ...

					// save scope pointer ...
					if(ast_node_to_be_evaluated != NULL){
						_currentScope =scope_info->pushScope(*ast_node_to_be_evaluated); // push current scope

					}

					aux_p=IGNORE_BLANKS(aux_p+1,m_line);

					bool parse_var = false;

					// parse var...
					end_p=aux_p;
					key_w = isKeyword(aux_p);
					if(key_w != KEYWORD_TYPE::UNKNOWN_KEYWORD){
						if(key_w == VAR_KEYWORD){
							aux_p=IGNORE_BLANKS(aux_p+strlen(defined_keyword[KEYWORD_TYPE::VAR_KEYWORD].str),m_line);
						}
						else{
							writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Expected 'var' keyword");
							return NULL;
						}
					}

					end_p = getSymbolName(aux_p, m_line);
					if(end_p == NULL){
						return NULL;
					}

					strncpy(buffer,aux_p,(end_p-aux_p));



					if(ast_node_to_be_evaluated != NULL){ // set children element/iterator/vector ...


							if((node_foreach_vars = CASTNode::newASTNode()) == NULL) return NULL;;

							// var current vector element
							(*ast_node_to_be_evaluated)->children[0]=node_foreach_vars->idxAstNode;
							node_foreach_vars->idxScope=_currentScope->idxScope;
							node_foreach_vars->node_type=NODE_TYPE::KEYWORD_NODE;
							node_foreach_vars->keyword_info = KEYWORD_TYPE::VAR_KEYWORD;
							node_foreach_vars->line_value = m_line;

							// var element ...
							if((node_foreach_symbol = CASTNode::newASTNode()) == NULL) return NULL;; // var load symbol...
							node_foreach_vars->children.push_back(node_foreach_symbol->idxAstNode);
							node_foreach_symbol->idxScope=_currentScope->idxScope;
							node_foreach_symbol->node_type=NODE_TYPE::SYMBOL_NODE;
							node_foreach_symbol->symbol_value=buffer;
							node_foreach_symbol->line_value = m_line;

							// register var element and iterator var...
							if(!SCOPE_NODE(_currentScope->idxScope)->registerSymbol(node_foreach_symbol->symbol_value,node_foreach_symbol)){
									return NULL;
							}


					}

					aux_p=IGNORE_BLANKS(end_p,m_line);

					key_w = isKeyword(aux_p);
					if(key_w != KEYWORD_TYPE::IN_KEYWORD){
						writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Expected 'in' keyword");
						return NULL;
					}

					aux_p=IGNORE_BLANKS(aux_p+strlen(defined_keyword[KEYWORD_TYPE::IN_KEYWORD].str),m_line);


					if((aux_p = parseExpression((const char *)aux_p,m_line,_currentScope, ast_node_to_be_evaluated != NULL ? &node_foreach_expression: NULL)) == NULL){
						return NULL;
					}


					if(ast_node_to_be_evaluated!=NULL){
						(*ast_node_to_be_evaluated)->children[1]=node_foreach_expression->idxAstNode;
					}

					aux_p=IGNORE_BLANKS(aux_p,m_line);

					if(*aux_p != ')'){
						writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Expected ')'");
						return NULL;
					}

					aux_p++;



					aux_p=IGNORE_BLANKS(aux_p,m_line);
					if(*aux_p != '{'){
						writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Expected '{' foreach-block");
						return NULL;
					}

					// parse block ...
					if((aux_p=parseBlock(aux_p
							,m_line
							,_currentScope
							,error
							,ast_node_to_be_evaluated != NULL ? &block_for : NULL
							,ast_node_to_be_evaluated != NULL ? *ast_node_to_be_evaluated : NULL
							))!= NULL){ // true: We treat declared variables into for as another scope.
						if(!error){

							if(ast_node_to_be_evaluated != NULL) {
								(*ast_node_to_be_evaluated)->children.push_back(block_for->idxAstNode);
								scope_info->popScope(); // push current scope
							}
							return aux_p;
						}

					}
				}else{
					writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Expected '(' for");
					return NULL;
				}
			}
		}
		return NULL;
	}*/

	char * CASTNode::parseSwitch(const char *s,int & m_line,  CScope *scope_info, PASTNode *ast_node_to_be_evaluated){

		// PRE: **ast_node_to_be_evaluated must be created and is i/o ast pointer variable where to write changes.
		char *aux_p = (char *)s;
		char *end_symbol,*start_symbol;
		PASTNode switch_node=NULL,
					 group_cases=NULL,
					 case_value_node=NULL,
					 default_switch_node=NULL;

		CScope *scope_case=NULL;
		PASTNode body_switch=NULL;

		__PUNCTUATOR_TYPE_OLD__ ip;
		char *value_to_eval;
		string val;
		KEYWORD_TYPE key_w,key_w2;
		CScope *currentScope=scope_info;

		bool error=false;
		int n_cases;

		aux_p=IGNORE_BLANKS(aux_p,m_line);

		// check for keyword ...
		key_w = isKeyword(aux_p);

		if(key_w != KEYWORD_TYPE::UNKNOWN_KEYWORD){
			if(key_w == KEYWORD_TYPE::SWITCH_KEYWORD){

				if( ast_node_to_be_evaluated != NULL){
					if((*ast_node_to_be_evaluated = CASTNode::newASTNode())==NULL) return NULL;
					(*ast_node_to_be_evaluated)->node_type = KEYWORD_NODE;
					(*ast_node_to_be_evaluated)->keyword_info = key_w;
					currentScope=scope_info->pushScope((*ast_node_to_be_evaluated));
				}

				aux_p += strlen(defined_keyword[key_w].str);
				aux_p=IGNORE_BLANKS(aux_p,m_line);

				if(*aux_p == '('){
						aux_p=IGNORE_BLANKS(aux_p+1,m_line);

						// evaluate switch vale expression ...
						PASTNode condition_expression_to_evaluate = NULL;
						//static char * parseExpression_Recursive(const char *s, int & m_line, CScope *scope_info, PASTNode *ast_node_to_be_evaluated=NULL,GROUP_TYPE type_group=GROUP_TYPE::GROUP_0,PASTNode parent=NULL);
						if((aux_p = CASTNode::parseExpression(
								aux_p,
								m_line,
								scope_info,
								ast_node_to_be_evaluated==NULL?NULL:&condition_expression_to_evaluate))==NULL)
						{// getEndWord(aux_p, m_line);
							return NULL;
						}

						if(ast_node_to_be_evaluated != NULL){
							condition_expression_to_evaluate->idxAstParent = (*ast_node_to_be_evaluated)->idxAstNode;
							(*ast_node_to_be_evaluated)->children.push_back(condition_expression_to_evaluate->idxAstNode);
						}

						if(*aux_p != ')'){
							writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Expected ')' switch");
							error = true;
							return NULL;
						}

						aux_p=IGNORE_BLANKS(aux_p+1,m_line);

						if(*aux_p == '{'){

							aux_p++;

							if(ast_node_to_be_evaluated != NULL){
								if((body_switch = CASTNode::newASTNode())==NULL) return NULL;
								body_switch->idxAstParent = (*ast_node_to_be_evaluated)->idxAstNode;
								(*ast_node_to_be_evaluated)->children.push_back(body_switch->idxAstNode);
								body_switch->idxScope = currentScope->idxScope;

							}


							if((aux_p=generateAST_Recursive(aux_p, m_line, currentScope, error, body_switch))==NULL){
								return NULL;
							}

							aux_p=IGNORE_BLANKS(aux_p,m_line);

							if(*aux_p != '}'){
								writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Expected '}' switch");
								return NULL;
							}

							if( ast_node_to_be_evaluated != NULL){
								scope_info->popScope();
							}

							return aux_p+1;
						}
						else{
							writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Expected '{' switch");
							return NULL;
						}
				}
				else{
					writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Expected '(' switch ");
					return NULL;
				}
			}
		}
		return NULL;
	}

	char * CASTNode::parseVar(const char *s,int & m_line,  CScope *scope_info, PASTNode *ast_node_to_be_evaluated){

		// PRE: **ast_node_to_be_evaluated must be created and is i/o ast pointer variable where to write changes.

		char *aux_p = (char *)s;

		KEYWORD_TYPE key_w;
		char *start_var,*end_var;
		string class_name, class_member;
		PASTNode class_node;
		PASTNode var_node;
		PASTNode vars_collection_node=NULL;
		bool error=false;
		int idxScope=ZS_UNDEFINED_IDX;
		string s_aux,variable_name;
		char *symbol_value;
		bool end=false;
		bool allow_for_in=true;

		bool parent_scope_is_class=false;
		int m_startLine=0;

		if(scope_info != NULL){// && class_scope){ // if class scope let's see if is within function member..
			if(scope_info->getIdxBaseScope() != 0) { // if base scope != 0 is a class
				parent_scope_is_class = scope_info->getIdxBaseScope() == scope_info->getCurrentScopePointer()->idxScope;
			}
		}

		aux_p=IGNORE_BLANKS(aux_p,m_line);
		key_w = isKeyword(aux_p);

		if(key_w != KEYWORD_TYPE::UNKNOWN_KEYWORD){
			if(key_w == KEYWORD_TYPE::VAR_KEYWORD){ // possible variable...

				aux_p += strlen(defined_keyword[key_w].str);
				aux_p=IGNORE_BLANKS(aux_p,m_line);

				//
				if(ast_node_to_be_evaluated != NULL){
					//_currentScope=scope_info->getCurrentScopePointer(); // gets current evaluating scope...
					if(((*ast_node_to_be_evaluated) = CASTNode::newASTNode()) == NULL) return NULL;
					(*ast_node_to_be_evaluated)->node_type = KEYWORD_NODE;
					(*ast_node_to_be_evaluated)->keyword_info = key_w;
					(*ast_node_to_be_evaluated)->idxScope =ZS_UNDEFINED_IDX; // assign main scope...
				}

				while(*aux_p != ';' && *aux_p != 0 && !end){ // JE: added multivar feature.

					bool is_class_member=parent_scope_is_class;

					aux_p=IGNORE_BLANKS(aux_p,m_line);
					start_var=aux_p;
					m_startLine = m_line;
					vars_collection_node=NULL;

					//parent_ast_to_insert_var=NULL;
					if(scope_info != NULL){ // main as default
						idxScope=scope_info->getCurrentScopePointer()->idxScope;
					}

					if((end_var=isClassMember(aux_p,m_line,class_name,class_member,class_node, error,key_w))!=NULL){ // check if particular case extension attribute class
						idxScope = class_node->idxScope; // override scope info
						symbol_value = (char *)class_member.c_str();
						variable_name = symbol_value;

						is_class_member=true;


					}
					else{ // causal variable
						if(error){
							return NULL;
						}
						else{ // get normal name...

							m_line = m_startLine;

							// check whwther the function is anonymous with a previous arithmetic operation ....
							end_var=getSymbolName(aux_p,m_line);

							if(end_var != NULL){

								if((symbol_value = CZetScriptUtils::copyStringFromInterval(aux_p,end_var))==NULL)
									return NULL;

								variable_name = symbol_value;

								// check whether parameter name's matches with some global variable...
							}else{
								return NULL;
							}
						}
					}

					KEYWORD_TYPE keyw = isKeyword(variable_name.c_str());

					if(keyw != KEYWORD_TYPE::UNKNOWN_KEYWORD){ // a keyword was detected...
						writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Cannot use reserved word (%s) as var.",defined_keyword[keyw].str);
						return NULL;
					}

					aux_p=end_var;
					aux_p=IGNORE_BLANKS(aux_p,m_line);
					//}
					bool ok_char=*aux_p == ';' || *aux_p == ',' || *aux_p == '=' ;
					if(is_class_member && *aux_p == '='){
						writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Variable member is not assignable on its declaration. Should be initialized within constructor.");
						return NULL;
					}

					if(ok_char){//(*aux_p == ';' || (*aux_p == ',' && !extension_prop))){ // JE: added multivar feature (',)).
						allow_for_in=false;
						//zs_print_debug_cr("registered symbol \"%s\" line %i ",variable_name, m_line);
						var_node = NULL;
						if(ast_node_to_be_evaluated!=NULL){

							if((var_node = CASTNode::newASTNode()) == NULL) return NULL;
							// save symbol in the node ...
							(var_node)->symbol_value = variable_name;
							(var_node)->idxScope = idxScope;
							(var_node)->line_value = m_startLine;

							(*ast_node_to_be_evaluated)->children.push_back(var_node->idxAstNode);
						}

						if(*aux_p == '='){ // only for variables (not class members)

							PASTNode children_node=NULL;


							// try to evaluate expression...
							aux_p=IGNORE_BLANKS(aux_p,m_line);

							if((aux_p = parseExpression(start_var,m_startLine,scope_info,var_node != NULL ? &children_node : NULL)) == NULL){
								return NULL;
							}

							if(var_node != NULL){

								if(children_node==NULL){
									THROW_RUNTIME_ERROR("internal:children node == NULL");
									return NULL;
								}
								var_node->children.push_back(children_node->idxAstNode);
							}

							m_line = m_startLine;
						}

						if(ast_node_to_be_evaluated!=NULL){ // define as many vars is declared within ','

							if(!SCOPE_NODE(idxScope)->registerSymbol(var_node->symbol_value,var_node)){
								return NULL;
							}

							if(vars_collection_node != NULL && ast_node_to_be_evaluated != NULL){
								vars_collection_node->children.push_back(var_node->idxAstNode);
							}

							zs_print_debug_cr("registered symbol \"%s\" line %i ",(var_node)->symbol_value.c_str(), (var_node)->line_value);
						}
					}
					else{

						KEYWORD_TYPE keyw = isKeyword(variable_name.c_str());
						if(keyw == KEYWORD_TYPE::IN_KEYWORD){ // in keyword was detected (return to parser)...
							if(!allow_for_in){
								writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"'in' keyword should be used with an uninitialized variable (example: for ( var e in v) {...} )", *aux_p);
								return NULL;
							}
							end=true;
						}
						else{
							writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"unexpected '%c'", *aux_p);
							return NULL;
						}
					}

					// ignores ';' or ','
					if(*aux_p == ',')
						aux_p++;
				}

				if(*aux_p == ';'){
					aux_p++;
				}
				else{
					writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Expected ';'");
					return NULL;
				}

				return aux_p;
			}
		}
		return NULL;
	}

	char * CASTNode::parseBlock(const char *s,int & m_line,  CScope *scope_info, bool & error,PASTNode *ast_node_to_be_evaluated, PASTNode parent, bool push_scope){
		// PRE: **ast_node_to_be_evaluated must be created and is i/o ast pointer variable where to write changes.
		char *aux_p = (char *)s;

		//CScope *_localScope =  scope_info != NULL ? scope_info->symbol_info.ast->scope_info_ptr: NULL;
		CScope *currentScope=  NULL;
		aux_p=IGNORE_BLANKS(aux_p,m_line);

		// check for keyword ...
		if(*aux_p == '{'){
			aux_p++;

			if(scope_info != NULL){
				if((*ast_node_to_be_evaluated=newASTNode())==NULL) return NULL;
				(*ast_node_to_be_evaluated)->node_type = BODY_BLOCK_NODE;
				(*ast_node_to_be_evaluated)->idxAstParent = parent->idxAstNode;
				currentScope =scope_info->getCurrentScopePointer();
				if(push_scope){
					currentScope = scope_info->pushScope(*ast_node_to_be_evaluated); // special case... ast is created later ...
				}
			}

			if((aux_p = generateAST_Recursive(aux_p, m_line, currentScope,error,ast_node_to_be_evaluated!=NULL?*ast_node_to_be_evaluated:NULL)) != NULL){
				if(error){
					return NULL;
				}

				if(*aux_p != '}'){
					error = true;
					writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Expected } ");
					return NULL;
				}

				if(scope_info != NULL && push_scope){
					scope_info->popScope();
				}

				return aux_p+1;
			}
		}
		return NULL;
	}

	char * CASTNode::isClassMember(const char *s,int & m_line, string & _class_name, string & var_name, PASTNode & _class_node, bool & error, KEYWORD_TYPE kwi){

		char *aux_p = (char *)s;
		char *end_var;
		char *symbol_value;

		error = true;

		aux_p=IGNORE_BLANKS(aux_p,m_line);

		// check whwther the function is anonymous or not.
		end_var=getSymbolName(aux_p,m_line);

		if(end_var != NULL){

			if((symbol_value = CZetScriptUtils::copyStringFromInterval(aux_p,end_var))==NULL)
				return NULL;
		}else{
			return NULL;
		}
		aux_p=end_var;
		aux_p=IGNORE_BLANKS(aux_p,m_line);

		if(*aux_p == ':' && *(aux_p+1)==':'){ // extension class detected...
			_class_name = symbol_value;
			_class_node = findAst(_class_name,NODE_TYPE::KEYWORD_NODE, KEYWORD_TYPE::CLASS_KEYWORD);

			if(_class_node == NULL){
				writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Class \"%s\" is no defined", symbol_value);
				return NULL;
			}

			aux_p=IGNORE_BLANKS(aux_p+2,m_line); // ignore ::

			end_var=getSymbolName(aux_p,m_line);

			if(end_var != NULL){

				if((symbol_value = CZetScriptUtils::copyStringFromInterval(aux_p,end_var))==NULL)
					return NULL;
			}else{
				return NULL;
			}

			var_name = symbol_value;
			aux_p=IGNORE_BLANKS(end_var,m_line);
			error = false;
			return aux_p;
		}else {
			error = false;
			return NULL;
		}
		return NULL;
	}

	PASTNode  CASTNode::findConditionForContinueRecursive(CScope *scope_info){
		PASTNode _ast = AST_NODE(scope_info->idxAstNode);

		if(_ast != NULL){ // some nodes may not initialized...

			if(_ast->node_type == NODE_TYPE::BODY_BLOCK_NODE){

				if(_ast->idxAstParent != ZS_UNDEFINED_IDX){
					PASTNode parent = AST_NODE(_ast->idxAstParent);

					if(
							//parent->keyword_info == KEYWORD_TYPE::FOREACH_KEYWORD
							parent->keyword_info == KEYWORD_TYPE::FOR_KEYWORD
								){
						return parent;
					}
				}
			}
		}

		short idxParent=scope_info->getIdxParent();

		if(idxParent != ZS_UNDEFINED_IDX){
			return findConditionForContinueRecursive(SCOPE_NODE(idxParent));
		}

		return NULL;
	}

	PASTNode  CASTNode::findConditionForContinue(CScope *scope_info){
		return findConditionForContinueRecursive(scope_info);
	}

	PASTNode  CASTNode::findConditionForBreakRecursive(CScope *scope_info){
		PASTNode _ast = AST_NODE(scope_info->idxAstNode);

		if(_ast != NULL){ // some nodes may not initialized...

			if(_ast->keyword_info == KEYWORD_TYPE::SWITCH_KEYWORD){
				return _ast;
			}
			else if(_ast->node_type == NODE_TYPE::BODY_BLOCK_NODE){

				if(_ast->idxAstParent != ZS_UNDEFINED_IDX){
					PASTNode parent = AST_NODE(_ast->idxAstParent);


					if( parent->keyword_info == KEYWORD_TYPE::FUNCTION_KEYWORD){
						return NULL;
					}

					if(			//parent->keyword_info == KEYWORD_TYPE::FOREACH_KEYWORD
								 parent->keyword_info == KEYWORD_TYPE::FOR_KEYWORD
								|| parent->keyword_info == KEYWORD_TYPE::DO_WHILE_KEYWORD
								||parent->keyword_info == KEYWORD_TYPE::WHILE_KEYWORD
								){
						return parent;
					}
				}
			}
		}

		short idxParent=scope_info->getIdxParent();

		if(idxParent != ZS_UNDEFINED_IDX){
			return findConditionForBreakRecursive(SCOPE_NODE(idxParent));
		}

		return NULL;
	}

	PASTNode  CASTNode::findConditionForBreak(CScope *scope_info){
		return findConditionForBreakRecursive(scope_info);

	}

	char *CASTNode::parseKeyword(const char *s, int & m_line, CScope *scope_info, bool & error, PASTNode *ast_node_to_be_evaluated){

		// PRE: **ast_node_to_be_evaluated must be created and is i/o ast pointer variable where to write changes.
		char *aux_p= (char *)s;

		KEYWORD_TYPE keyw=KEYWORD_TYPE::UNKNOWN_KEYWORD,keyw2nd=KEYWORD_TYPE::UNKNOWN_KEYWORD;

		aux_p=IGNORE_BLANKS(aux_p, m_line);

		// check if condition...
		keyw = isKeyword(aux_p);

		if(keyw != KEYWORD_TYPE::UNKNOWN_KEYWORD){ // a keyword was detected...

			aux_p+=strlen(defined_keyword[keyw].str);
			aux_p=IGNORE_BLANKS(aux_p, m_line);
			char *value_to_eval;

			if(keyw == KEYWORD_TYPE::CASE_KEYWORD ||keyw == KEYWORD_TYPE::DEFAULT_KEYWORD){
				PASTNode ast=AST_NODE(scope_info->idxAstNode);
				if(ast->keyword_info != KEYWORD_TYPE::SWITCH_KEYWORD){
					writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"\"case or default\" are allowed only within switch statements");
					error = true;
					return NULL;
				}

				if(keyw == KEYWORD_TYPE::CASE_KEYWORD){

					aux_p=IGNORE_BLANKS(aux_p,m_line);

					// get the symbol...
					char *start_symbol=aux_p;
					__PUNCTUATOR_TYPE_OLD__ ip;

					if((ip = isPunctuator(aux_p)) != __PUNCTUATOR_TYPE_OLD__::UNKNOWN_PUNCTUATOR){
						if(ip == __PUNCTUATOR_TYPE_OLD__::ADD_PUNCTUATOR ||ip == __PUNCTUATOR_TYPE_OLD__::SUB_PUNCTUATOR){
							aux_p+=strlen(defined_operator_punctuator[ip].str);
						}
						else{
							writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"unexpected token %s",defined_operator_punctuator[ip].str);
							error = true;
							return NULL;
						}
					}
					char *end_symbol = getEndWord(aux_p, m_line);
					aux_p=end_symbol;

					value_to_eval = CZetScriptUtils::copyStringFromInterval(start_symbol, end_symbol);

					if(value_to_eval==NULL){ return NULL;}

				}

				aux_p=IGNORE_BLANKS(aux_p,m_line);

				if(*aux_p != ':'){
					writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"Expected  ':' ");
					return NULL;
				}

				if(ast_node_to_be_evaluated!=NULL){
					*ast_node_to_be_evaluated = newASTNode();
					(*ast_node_to_be_evaluated)->node_type=NODE_TYPE::KEYWORD_NODE;
					(*ast_node_to_be_evaluated)->keyword_info=keyw;
					(*ast_node_to_be_evaluated)->idxScope=scope_info->idxScope;

					(*ast_node_to_be_evaluated)->symbol_value=value_to_eval;
					(*ast_node_to_be_evaluated)->line_value = m_line;
				}

				return aux_p+1;

			}else if(keyw == KEYWORD_TYPE::BREAK_KEYWORD){


				if(ast_node_to_be_evaluated!=NULL){
					PASTNode break_ast;
					if((break_ast = findConditionForBreak(scope_info)) == NULL){ // ok break is valid in current scope...

						writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"\"break\" allowed within loop or case-switch statements");
						error = true;
						return NULL;
					}

					*ast_node_to_be_evaluated = newASTNode();
					(*ast_node_to_be_evaluated)->node_type=NODE_TYPE::KEYWORD_NODE;
					(*ast_node_to_be_evaluated)->keyword_info=keyw;
				}

				if(*aux_p != ';'){
					writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"expected ';'");
					error = true;
					return NULL;
				}

				return aux_p+1;
			}else if(keyw == KEYWORD_TYPE::CONTINUE_KEYWORD){

				if(ast_node_to_be_evaluated!=NULL){
					PASTNode continue_ast;
					if((continue_ast = findConditionForContinue(scope_info)) == NULL){ // ok break is valid in current scope...
						writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"\"continue\" allowed within for or foreach loop");
						error = true;
						return NULL;
					}

					if(ast_node_to_be_evaluated!=NULL){
						*ast_node_to_be_evaluated = newASTNode();
						(*ast_node_to_be_evaluated)->node_type=NODE_TYPE::KEYWORD_NODE;
						(*ast_node_to_be_evaluated)->keyword_info=keyw;
					}
				}

				if(*aux_p != ';'){
					writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"expected ';'");
					error = true;
					return NULL;
				}


				return aux_p+1;

			}

			// check if non named function...
			if(keyw == KEYWORD_TYPE::FUNCTION_KEYWORD){
				if( *aux_p == '('){
					// Is no named. No named function is an object and should be processed within parseExpression ...
					return NULL;
				}
			}

			// check if class and is not main class (scope )...
			if(keyw == KEYWORD_TYPE::CLASS_KEYWORD){

				if((aux_p = parseClass(s,m_line,scope_info,ast_node_to_be_evaluated)) != NULL){
					return aux_p;
				}

				error = true;
				return NULL;
			}

			// check if another kwyword is defined ...
			if((keyw2nd = isKeyword(aux_p))!= KEYWORD_TYPE::UNKNOWN_KEYWORD){

				if(
						   (keyw2nd != KEYWORD_TYPE::FUNCTION_KEYWORD)   // list of exceptional keywords...
						&& (keyw2nd != KEYWORD_TYPE::THIS_KEYWORD)   // list of exceptional keywords...
						&& (keyw2nd != KEYWORD_TYPE::NEW_KEYWORD)   // list of exceptional keywords...
				  ){

					writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"unexpected keyword \"%s\"",defined_keyword[keyw2nd].str);
					error = true;
					return NULL;
				}
			}

			if(defined_keyword[keyw].eval_fun != NULL){

				if((aux_p = defined_keyword[keyw].eval_fun(s,m_line,scope_info, ast_node_to_be_evaluated)) != NULL){
					return aux_p;
				}
				error = true;
			}
			// something wrong was happen..
		}

		return NULL;
	}

	void manageOnErrorParse(PASTNode node_to_be_evaluated){

		bool is_main_node=false;
		if(node_to_be_evaluated!= NULL){
			is_main_node=MAIN_AST_NODE==node_to_be_evaluated;
		}

		if(is_main_node){ // remove global variable/function if any

		}
	}

	char * CASTNode::generateAST_Recursive(const char *s, int & m_line, CScope *scope_info, bool & error, PASTNode node_to_be_evaluated){

		// PRE: *node_to_be_evaluated must be created (the pointer is only read mode)

		KEYWORD_TYPE keyw=KEYWORD_TYPE::UNKNOWN_KEYWORD;
		bool custom_quit = false;
		char *aux = (char *)s;
		char *end_expr=0;
		PASTNode children=NULL;
		bool is_main_node = false;
		bool processed_directive=false;

		if(node_to_be_evaluated!= NULL){

			is_main_node=MAIN_AST_NODE==node_to_be_evaluated;

			if(is_main_node){
				char *test = IGNORE_BLANKS(s,DUMMY_LINE);

				// empty script ? return true anyways
				if(test == 0) return NULL;
			}

		}
		aux=IGNORE_BLANKS(aux, m_line);

		while(*aux != 0 && !custom_quit){

			processed_directive=false;
			children = NULL;
			// ignore all ;
			while(*aux==';' && *aux != 0){
				aux =IGNORE_BLANKS(aux+1, m_line);
			}

			if(*aux==0){ // custom case exit..
				return aux;
			}

			if(*aux == '}'){ // trivial cases...
				return aux;
			}else{

				// try directive ...
				DIRECTIVE_TYPE directive = isDirective(aux);
				char *start_var,* end_var,*symbol_name;
				if(directive != DIRECTIVE_TYPE::UNKNOWN_DIRECTIVE){
					switch(directive){
					case INCLUDE_DIRECTIVE:
						aux += strlen(defined_directive[directive].str);
						aux = IGNORE_BLANKS(aux,m_line);
						if(*aux != '\"'){
							writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"expected starting \" directive");
							THROW_SCRIPT_ERROR();
							return NULL;
						}
						aux++;
						start_var=aux;

						while(*aux != '\n' && *aux!=0 && !(*aux=='\"' && *(aux-1)!='\\')) aux++;

						if(*aux != '\"'){
							writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"expected end \" directive");
							THROW_SCRIPT_ERROR();
							return NULL;
						}

						end_var=aux;

						if((symbol_name=CZetScriptUtils::copyStringFromInterval(start_var,end_var)) == NULL){
							THROW_SCRIPT_ERROR();
							return NULL;
						}

						zs_print_debug_cr("include file: %s",symbol_name);

						{

							// save current file info...
							string current_file_str=CASTNode::current_parsing_filename;
							int current_file_idx=CASTNode::current_idx_parsing_filename;
							string file_to_parse=symbol_name;

							if(CZetScript::getInstance()->isFilenameAlreadyParsed(file_to_parse.c_str())){
								writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"\"%s\" already parsed",file_to_parse.c_str());
								THROW_SCRIPT_ERROR();
								return NULL;
							}

							try{
								CZetScript::getInstance()->parse_file(file_to_parse.c_str());
							}catch(script_error & error){
								THROW_SCRIPT_ERROR();
								return NULL;
							}

							//restore current file info...
							CASTNode::current_parsing_filename=current_file_str.c_str();
							CASTNode::current_idx_parsing_filename=current_file_idx;
						}

						aux++;// advance ..
						break;
					default:
						writeErrorMsg(CURRENT_PARSING_FILENAME,m_line,"directive \"%s\" not supported",defined_directive[directive].str);
						break;
					}

					processed_directive = true;
					end_expr=aux;
				}
			}

			// 0st special case member class extension ...
			if(children==NULL && !processed_directive){ // not processed yet ...
				// 1st. check whether parse a keyword...
				if((end_expr = parseKeyword(aux, m_line, scope_info, error, node_to_be_evaluated != NULL ? &children : NULL)) == NULL){

					// If was unsuccessful then try to parse expression.
					if(error){
						manageOnErrorParse(node_to_be_evaluated);
						THROW_SCRIPT_ERROR();
						return NULL;
					}
					// 2nd. check whether parse a block
					if((end_expr = parseBlock(aux
							,m_line
							, scope_info
							, error
							,node_to_be_evaluated != NULL ? &children:NULL
							,node_to_be_evaluated))==NULL){

						// If was unsuccessful then try to parse expression.
						if(error){
							manageOnErrorParse(node_to_be_evaluated);
							THROW_SCRIPT_ERROR();
							return NULL;
						}
						// 2nd. try expression
						int starting_expression=m_line;

						if((end_expr = parseExpression(aux,m_line, scope_info,node_to_be_evaluated != NULL ? &children:NULL)) == NULL){ // something wrong was happen.
							manageOnErrorParse(node_to_be_evaluated);
							THROW_SCRIPT_ERROR();
							return NULL;
						}

						if(*end_expr == ')'){ // unexpected close parenthesis.
							error = true;
							writeErrorMsg(CURRENT_PARSING_FILENAME,starting_expression,"missing '('");
							manageOnErrorParse(node_to_be_evaluated);
							THROW_SCRIPT_ERROR();
							return NULL;
						}

						if(*end_expr != ';'){
							error = true;
							writeErrorMsg(CURRENT_PARSING_FILENAME,starting_expression,"Expected ';'");
							manageOnErrorParse(node_to_be_evaluated);
							THROW_SCRIPT_ERROR();
							return NULL;
						}
						end_expr++;
					}
				}

				// new expression ready to be evaluated...

				if(node_to_be_evaluated != NULL && children != NULL){
					(node_to_be_evaluated)->children.push_back(children->idxAstNode);
					children->idxAstParent = (node_to_be_evaluated)->idxAstNode;


					if(is_main_node){
						astNodeToCompile->push_back({(node_to_be_evaluated)->idxAstNode,children->idxAstNode});
					}
				}
			}

			aux=end_expr;
			aux=IGNORE_BLANKS(aux, m_line);
		}
		return aux;
	}
	//------------------------------------------------------------------------------------------------------------
	//
	// PUBLIC
	//
	void CASTNode::init(){

		if(astNodeToCompile != NULL){
			THROW_RUNTIME_ERROR("CASTNode already initialized");
			return;
		}

		// init operator punctuators...
		defined_operator_punctuator[UNKNOWN_PUNCTUATOR]={UNKNOWN_PUNCTUATOR, "none",NULL};

		defined_operator_punctuator[ADD_PUNCTUATOR]={ADD_PUNCTUATOR, "+",parsePlusPunctuator};
		defined_operator_punctuator[SUB_PUNCTUATOR]={SUB_PUNCTUATOR, "-",parseMinusPunctuator};
		defined_operator_punctuator[MUL_PUNCTUATOR]={MUL_PUNCTUATOR, "*",parseMulPunctuator};
		defined_operator_punctuator[DIV_PUNCTUATOR]={DIV_PUNCTUATOR, "/",parseDivPunctuator};
		defined_operator_punctuator[MOD_PUNCTUATOR]={MOD_PUNCTUATOR, "%",parseModPunctuator};
		defined_operator_punctuator[FIELD_PUNCTUATOR]={FIELD_PUNCTUATOR, ".",parseFieldPunctuator};
		defined_operator_punctuator[TERNARY_IF_PUNCTUATOR]={TERNARY_IF_PUNCTUATOR, "?",parseInlineIfPunctuator};
		defined_operator_punctuator[TERNARY_ELSE_PUNCTUATOR]={TERNARY_ELSE_PUNCTUATOR, ":",parseInlineElsePunctuator};
		defined_operator_punctuator[ASSIGN_PUNCTUATOR]={ASSIGN_PUNCTUATOR, "=",parseAssignPunctuator};
		defined_operator_punctuator[ADD_ASSIGN_PUNCTUATOR]={ADD_ASSIGN_PUNCTUATOR, "+=",parseAddAssignPunctuator};
		defined_operator_punctuator[SUB_ASSIGN_PUNCTUATOR]={SUB_ASSIGN_PUNCTUATOR, "-=",parseSubAssignPunctuator};
		defined_operator_punctuator[MUL_ASSIGN_PUNCTUATOR]={MUL_ASSIGN_PUNCTUATOR, "*=",parseMulAssignPunctuator};
		defined_operator_punctuator[DIV_ASSIGN_PUNCTUATOR]={DIV_ASSIGN_PUNCTUATOR, "/=",parseDivAssignPunctuator};
		defined_operator_punctuator[MOD_ASSIGN_PUNCTUATOR]={MOD_ASSIGN_PUNCTUATOR, "%=",parseModAssignPunctuator};
		defined_operator_punctuator[BINARY_XOR_PUNCTUATOR]={BINARY_XOR_PUNCTUATOR, "^",parseBinaryXorPunctuator};
		defined_operator_punctuator[BINARY_AND_PUNCTUATOR]={BINARY_AND_PUNCTUATOR, "&",parseBinaryAndPunctuator};
		defined_operator_punctuator[BINARY_OR_PUNCTUATOR]={BINARY_OR_PUNCTUATOR, "|",parseBinaryOrPunctuator};
		defined_operator_punctuator[SHIFT_LEFT_PUNCTUATOR]={SHIFT_LEFT_PUNCTUATOR, "<<",parseShiftLeftPunctuator};
		defined_operator_punctuator[SHIFT_RIGHT_PUNCTUATOR]={SHIFT_RIGHT_PUNCTUATOR, ">>",parseShiftRightPunctuator};
		defined_operator_punctuator[LOGIC_AND_PUNCTUATOR]={LOGIC_AND_PUNCTUATOR, "&&",parseLogicAndPunctuator};
		defined_operator_punctuator[LOGIC_OR_PUNCTUATOR]={LOGIC_OR_PUNCTUATOR, "||",parseLogicOrPunctuator};
		defined_operator_punctuator[LOGIC_EQUAL_PUNCTUATOR]={LOGIC_EQUAL_PUNCTUATOR, "==",parseLogicEqualPunctuator};
		defined_operator_punctuator[LOGIC_NOT_EQUAL_PUNCTUATOR]={LOGIC_NOT_EQUAL_PUNCTUATOR, "!=",parseLogicNotEqualPunctuator};
		defined_operator_punctuator[LOGIC_GT_PUNCTUATOR]={LOGIC_GT_PUNCTUATOR, ">",parseLogicGreatherThanPunctuator};
		defined_operator_punctuator[LOGIC_LT_PUNCTUATOR]={LOGIC_LT_PUNCTUATOR, "<",parseLogicLessThanPunctuator};
		defined_operator_punctuator[LOGIC_GTE_PUNCTUATOR]={LOGIC_GTE_PUNCTUATOR, ">=",parseLogicGreatherEqualThanPunctuator};
		defined_operator_punctuator[LOGIC_LTE_PUNCTUATOR]={LOGIC_LTE_PUNCTUATOR, "<=",parseLessEqualThanPunctuator};
		defined_operator_punctuator[INSTANCEOF_PUNCTUATOR]={INSTANCEOF_PUNCTUATOR, "instanceof",parseInstanceOfPunctuator};

		defined_operator_punctuator[LOGIC_NOT_PUNCTUATOR]={LOGIC_NOT_PUNCTUATOR, "!",parseNotPunctuator};
		defined_operator_punctuator[PRE_INC_PUNCTUATOR]={PRE_INC_PUNCTUATOR, "++",parseIncPunctuator};
		defined_operator_punctuator[PRE_DEC_PUNCTUATOR]={PRE_DEC_PUNCTUATOR, "--",parseDecPunctuator};
		defined_operator_punctuator[POST_INC_PUNCTUATOR]={POST_INC_PUNCTUATOR, "++",parseIncPunctuator};
		defined_operator_punctuator[POST_DEC_PUNCTUATOR]={POST_DEC_PUNCTUATOR, "--",parseDecPunctuator};

		// special punctuators...
		defined_operator_punctuator[COMA_PUNCTUATOR]={COMA_PUNCTUATOR, ",",NULL};
		defined_operator_punctuator[SEMICOLON_PUNCTUATOR]={SEMICOLON_PUNCTUATOR, ";",NULL};
		defined_operator_punctuator[OPEN_PARENTHESIS_PUNCTUATOR]={OPEN_PARENTHESIS_PUNCTUATOR, "(",NULL};
		defined_operator_punctuator[CLOSE_PARENTHESIS_PUNCTUATOR]={CLOSE_PARENTHESIS_PUNCTUATOR, ")",NULL};
		defined_operator_punctuator[OPEN_BRAKET_PUNCTUATOR]={OPEN_BRAKET_PUNCTUATOR, "{",NULL};
		defined_operator_punctuator[CLOSE_BRAKET_PUNCTUATOR]={CLOSE_BRAKET_PUNCTUATOR, "}",NULL};
		defined_operator_punctuator[OPEN_SQUARE_BRAKET_PUNCTUATOR]={OPEN_SQUARE_BRAKET_PUNCTUATOR, "[",NULL};
		defined_operator_punctuator[CLOSE_SQUARE_BRAKET_PUNCTUATOR]={CLOSE_SQUARE_BRAKET_PUNCTUATOR, "]",NULL};

		// init special punctuators...
		// init keywords...
		defined_keyword[KEYWORD_TYPE::UNKNOWN_KEYWORD] = {UNKNOWN_KEYWORD, "none",NULL};
		defined_keyword[KEYWORD_TYPE::VAR_KEYWORD] = {VAR_KEYWORD,"var",parseVar};
		defined_keyword[KEYWORD_TYPE::IF_KEYWORD] = {IF_KEYWORD,"if",parseIf};
		defined_keyword[KEYWORD_TYPE::ELSE_KEYWORD] = {ELSE_KEYWORD,"else",NULL};
		defined_keyword[KEYWORD_TYPE::FOR_KEYWORD] = {FOR_KEYWORD,"for",parseFor};
		//defined_keyword[KEYWORD_TYPE::FOREACH_KEYWORD] = {FOREACH_KEYWORD,"foreach",parseForeach};
		defined_keyword[KEYWORD_TYPE::WHILE_KEYWORD] = {WHILE_KEYWORD,"while",parseWhile};
		defined_keyword[KEYWORD_TYPE::DO_WHILE_KEYWORD] = {DO_WHILE_KEYWORD,"do",parseDoWhile}; // while is expected in the end ...

		defined_keyword[KEYWORD_TYPE::SWITCH_KEYWORD] = {SWITCH_KEYWORD,"switch",parseSwitch};
		defined_keyword[KEYWORD_TYPE::CASE_KEYWORD] = {CASE_KEYWORD,"case",NULL};
		defined_keyword[KEYWORD_TYPE::BREAK_KEYWORD] = {BREAK_KEYWORD,"break",NULL};
		defined_keyword[KEYWORD_TYPE::CONTINUE_KEYWORD] = {CONTINUE_KEYWORD,"continue",NULL};
		defined_keyword[KEYWORD_TYPE::DEFAULT_KEYWORD] = {DEFAULT_KEYWORD,"default",NULL};
		defined_keyword[KEYWORD_TYPE::FUNCTION_KEYWORD] = {FUNCTION_KEYWORD,"function",parseFunction};
		defined_keyword[KEYWORD_TYPE::RETURN_KEYWORD] = {RETURN_KEYWORD,"return",parseReturn};
		defined_keyword[KEYWORD_TYPE::THIS_KEYWORD] = {THIS_KEYWORD,"this", NULL};
	//	defined_keyword[KEYWORD_TYPE::SUPER_KEYWORD] = {SUPER_KEYWORD,"super", NULL};
		defined_keyword[KEYWORD_TYPE::CLASS_KEYWORD] = {CLASS_KEYWORD,"class",NULL};
		defined_keyword[KEYWORD_TYPE::NEW_KEYWORD] = {NEW_KEYWORD,"new", NULL};
		defined_keyword[KEYWORD_TYPE::DELETE_KEYWORD] = {DELETE_KEYWORD,"delete",parseDelete};
		defined_keyword[KEYWORD_TYPE::IN_KEYWORD] = {IN_KEYWORD,"in",NULL};

		// DIRECTIVES
		defined_directive[UNKNOWN_DIRECTIVE]={UNKNOWN_DIRECTIVE, NULL};
		defined_directive[INCLUDE_DIRECTIVE]={INCLUDE_DIRECTIVE, "import"};

		astNodeToCompile = new vector<tInfoAstNodeToCompile>();

			// create main ast management
	}

	CASTNode::CASTNode(){
		node_type = UNKNOWN_NODE;
		keyword_info = KEYWORD_TYPE::UNKNOWN_KEYWORD;
		pre_post_operator_info = __PUNCTUATOR_TYPE_OLD__::UNKNOWN_PUNCTUATOR;
		line_value=ZS_UNDEFINED_IDX;
		operator_info=__PUNCTUATOR_TYPE_OLD__::UNKNOWN_PUNCTUATOR;
		symbol_value="";
		idxAstParent=ZS_UNDEFINED_IDX;
		//aux_value=NULL;

		idxAstNode = ZS_UNDEFINED_IDX;
		idxScope = ZS_UNDEFINED_IDX;

		is_packed_node = false;
		idxFilename = ZS_UNDEFINED_IDX;
	}


	CASTNode::~CASTNode(){
		//destroyChildren();
	}

}
