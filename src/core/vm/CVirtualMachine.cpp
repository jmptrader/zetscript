#include "core/zg_core.h"

//CVirtualMachine * CVirtualMachine::m_virtualMachine = NULL;
//vector<CVirtualMachine::CVirtualMachine> CVirtualMachine::ALE;


// static: only defined in this module...
/*
CVirtualMachine * CVirtualMachine::getInstance(){
	if(m_virtualMachine == NULL){
		m_virtualMachine = new CVirtualMachine();
	}

	return m_virtualMachine;
}

void CVirtualMachine::destroySingletons(){
	if(m_virtualMachine != NULL){
		delete m_virtualMachine;
		m_virtualMachine = NULL;
	}


}
*/
CVirtualMachine::CVirtualMachine(){

	basePtrLocalVar=NULL;
	idxStkCurrentLocalVar=0;


	startIdxStkNumber=

	startIdxStkString=
	startIdxStkResultInstruction=0;

	// push indexes ...

	reset();
}

void CVirtualMachine::reset(){

	// deallocate allocated aux vectors...

	idxStkCurrentNumber=startIdxStkNumber;

	idxStkCurrentString=startIdxStkString;
	idxStkCurrentResultInstruction=startIdxStkResultInstruction;
	//memset(stkResultInstruction,0,sizeof(stkResultInstruction));
	m_functionArgs.clear();
}



#ifdef __DEBUG__ // incoment __VERBOSE_MESSAGE__ to print all messages (wrning is going to be slow because of the prints)
//#define __VERBOSE_MESSAGE__
#endif



#ifdef  __VERBOSE_MESSAGE__

#define print_vm_cr print_info_cr
#else
#define print_vm_cr(s,...)
#endif



CScriptVariable * CVirtualMachine::execute(tInfoRegisteredFunctionSymbol *info_function, CScriptVariable *this_object, vector<CScriptVariable *> * argv, int stk){

	print_info_cr("Executing function %s ...",info_function->symbol_name.c_str());
	//tInfoRegisteredFunctionSymbol *irsf=sf->getFunctionInfo();

	//tInfoRegisteredFunctionSymbol *function_info =function_object->getFunctionInfo();

	if((info_function->object_info.symbol_info.properties & SYMBOL_INFO_PROPERTIES::C_OBJECT_REF) == SYMBOL_INFO_PROPERTIES::C_OBJECT_REF){ // C-Call

			int result=0;

			if(!CZG_ScriptCore::call_C_function(info_function,result,argv)){
				 return NULL;
			}

			//TODO: create primitive object if needed ...
			return CScriptVariable::VoidSymbol;
			//function_object->setReturnObject(CScriptVariable::VoidSymbol);
	}



	CScriptVariable *ret=CScriptVariable::VoidSymbol;
	vector<tInfoStatementOp> * m_listStatements = &info_function->object_info.statment_op;
	//bool conditional_jmp=false;
	int jmp_to_statment = -1;
	bool end=false;

	/*if(argv != NULL){

		if((*argv).size() != function_info->m_arg.size()){
			print_error_cr("calling function s from line . Expected %i but passed %i",function_info->m_arg.size(),(*argv).size());
			return false;
		}

		//print_info_cr("assign local symbol the passing args...");
		for(unsigned i = 0; i < (*argv).size(); i++){
			//print_info_cr("%s=%s <cr>",sf->getArgVector()->at(i).c_str(),(*argv).at(i)->getPointerClassStr().c_str());
			(*this_object->getArgSymbol(i)).object=(*argv).at(i);
		}

	}*/



	//CVirtualMachine ALE; // new ale ?

	// reserve vars...

	pushStack(info_function->object_info.local_symbols.m_registeredVariable.size());


	unsigned n_stats=(*m_listStatements).size();


	for(unsigned s = 0; s < n_stats && !end;){

		//conditional_jmp = false;
		jmp_to_statment = -1;
		tInfoStatementOp * current_statment = &(*m_listStatements)[s];
		vector<tInfoAsmOp *> * asm_op_statment = &current_statment->asm_op;
		unsigned n_asm_op= asm_op_statment->size();

		//if(stk==2){
		//	s++;
		//	continue;
		//}

		if(n_asm_op>0){

			// clear previous ALE information stack..
			//if(stk!=2){
			reset();
			//}

			//vector<CCompiler::tInfoAsmOp *> * asm_op_statment = &(*m_listStatements)[s].asm_op;



			//CCompiler::tInfoAsmOp * instruction=NULL;


			for(unsigned i = 0; i  <  n_asm_op && (jmp_to_statment==-1); i++){ // for each code-instruction execute it.
				print_vm_cr("executing instruction  [%02i:%02i]...", s,i);
				//print_vm_cr("executing code...%i/%i",s,i);
				if( s==1 && i==6){
					int hhh=0;
					hhh++;
				}



				//if(stk!=2){

					//return true;


					if(!performInstruction(asm_op_statment->at(i),jmp_to_statment,info_function,this_object,argv,stk)){
						return NULL;
					}

					if(asm_op_statment->at(i)->operator_type == ASM_OPERATOR::RET){ // return...



						ret=createVarFromResultInstruction(&stkResultInstruction[asm_op_statment->at(i)->index_op1+startIdxStkResultInstruction]); // return last ALE value
						end=true;
					}

				//}


				//previous_instruction = instruction;
			}

			if(jmp_to_statment != -1){
				s=jmp_to_statment;
			}
			else{ // next statment ...
				s++;
			}


		}
	}

	popStack(info_function->object_info.local_symbols.m_registeredVariable.size());
	return ret;

}


#include "core/zg_core.h"

/*
CVirtualMachine::tAleObjectInfo  	CVirtualMachine::stack[VM_LOCAL_VAR_MAX_STACK];
int 					CVirtualMachine::stkInteger[VM_ALE_OPERATIONS_MAX_STACK];
float 					CVirtualMachine::stkNumber[VM_ALE_OPERATIONS_MAX_STACK];
bool					CVirtualMachine::stkBoolean[VM_ALE_OPERATIONS_MAX_STACK];
string 					CVirtualMachine::stkString[VM_ALE_OPERATIONS_MAX_STACK];
CScriptVariable *			CVirtualMachine::stkLocalVar[VM_ALE_OPERATIONS_MAX_STACK];

//CScriptVariable  **stkObject[MAX_PER_TYPE_OPERATIONS]; // all variables references to this ...
//CVector	 * vector[MAX_OPERANDS];

int CVirtualMachine::idxStkCurrentInteger=0;
int CVirtualMachine::idxStkCurrentNumber=0;
int CVirtualMachine::idxStkCurrentBoolean=0;
int CVirtualMachine::idxStkCurrentString=0;
int CVirtualMachine::idxStkCurrentLocalVar=0;

CVirtualMachine::tAleObjectInfo CVirtualMachine::stkResultInstruction[VM_ALE_OPERATIONS_MAX_STACK];


CVirtualMachine::tAleObjectInfo *CVirtualMachine::basePtrLocalVar=NULL;
int CVirtualMachine::idxStkCurrentLocalVar=0;
*/

CVirtualMachine::tAleObjectInfo *CVirtualMachine::pushStack(unsigned n_local_vars){

	if((idxStkCurrentLocalVar+n_local_vars) >=  VM_LOCAL_VAR_MAX_STACK){
		print_error_cr("Error MAXIMUM stack size reached");
		exit(EXIT_FAILURE);
	}


	basePtrLocalVar=&stack[CVirtualMachine::idxStkCurrentLocalVar];

	// init vars ...
	for(unsigned i = 0; i < n_local_vars; i++){
		basePtrLocalVar[i].stkObject=CScriptVariable::UndefinedSymbol;
		basePtrLocalVar[i].type = INS_TYPE_UNDEFINED;
		basePtrLocalVar[i].ptrAssignableVar = NULL;
	}


	CVirtualMachine::idxStkCurrentLocalVar+=n_local_vars;

	// save current aux vars ...
	vecIdxStkNumber.push(startIdxStkNumber);
	vecIdxStkString.push(startIdxStkString);
	vecIdxStkResultInstruction.push(startIdxStkResultInstruction);



	startIdxStkNumber=idxStkCurrentNumber;
	startIdxStkString=idxStkCurrentString;
	startIdxStkResultInstruction=idxStkCurrentResultInstruction+1;


	return basePtrLocalVar;
}

CVirtualMachine::tAleObjectInfo *CVirtualMachine::popStack(unsigned n_local_vars){

	if((idxStkCurrentLocalVar-n_local_vars) <  0){
		print_error_cr("Error MINIMUM stack size reached");
		exit(EXIT_FAILURE);
	}

	CVirtualMachine::idxStkCurrentLocalVar-=n_local_vars;
	basePtrLocalVar=&stack[CVirtualMachine::idxStkCurrentLocalVar];

	// restore last current instruction...
	idxStkCurrentResultInstruction=startIdxStkResultInstruction-1;

	// save current aux vars ...
	startIdxStkNumber = vecIdxStkNumber.top();
	startIdxStkString=vecIdxStkString.top();
	startIdxStkResultInstruction=vecIdxStkResultInstruction.top();



	vecIdxStkNumber.pop();
	vecIdxStkString.pop();
	vecIdxStkResultInstruction.pop();

	return basePtrLocalVar;
}


// general
/*
#define CHECK_VALID_INDEXES \
if(!(index_op1 >= 0 && index_op1 <=idxStkCurrentResultInstruction)) { print_error_cr("instruction 1 out of bounds"); return false;} \
if(!(index_op2 >= 0 && index_op2 <=idxStkCurrentResultInstruction)) { print_error_cr("instruction 2 out of bounds"); return false;} \
if(!(index_op2 >= index_op1 )) { print_error_cr("invalid indexes"); return false;}
*/

#define LOAD_NUMBER_OP(ptr_result_instruction) \
		*(((float *)(ptr_result_instruction->stkObject)))

#define LOAD_INT_OP(ptr_result_instruction) \
		(((int)(ptr_result_instruction->stkObject)))




#define LOAD_BOOL_OP(ptr_result_instruction) \
		(((bool)(ptr_result_instruction->stkObject)))


#define LOAD_STRING_OP(ptr_result_instruction) \
		*(((string *)(ptr_result_instruction->stkObject)))



// Check types
#define IS_NUMBER(ptr_result_instruction) \
(ptr_result_instruction->type == INS_TYPE_NUMBER)


#define IS_INT(ptr_result_instruction) \
(ptr_result_instruction->type == INS_TYPE_INTEGER)


#define IS_STRING(ptr_result_instruction) \
(ptr_result_instruction->type == INS_TYPE_STRING)

#define IS_BOOLEAN(ptr_result_instruction) \
(ptr_result_instruction->type == INS_TYPE_BOOLEAN)

#define IS_UNDEFINED(ptr_result_instruction) \
(ptr_result_instruction->type == INS_TYPE_UNDEFINED)

#define IS_FUNCTION(ptr_result_instruction) \
(ptr_result_instruction->type == INS_TYPE_FUNCTION)

#define IS_VAR(ptr_result_instruction) \
(ptr_result_instruction->type == INS_TYPE_VAR)

#define IS_VECTOR(ptr_result_instruction) \
(( ptr_result_instruction->type == INS_TYPE_VAR) &&\
 (((CScriptVariable *)(ptr_result_instruction->stkObject))->getIdxClass()==CScriptClassFactory::getInstance()->getIdxClassVector()))

#define IS_GENERIC_NUMBER(ptr_result_instruction) \
((ptr_result_instruction->type == INS_TYPE_INTEGER) ||\
(ptr_result_instruction->type == INS_TYPE_NUMBER))


#define OP1_AND_OP2_ARE_NUMBERS \
(IS_GENERIC_NUMBER(ptrResultInstructionOp1) && IS_GENERIC_NUMBER(ptrResultInstructionOp2))

#define OP1_IS_STRING_AND_OP2_IS_NUMBER \
(ptrResultInstructionOp1->type == INS_TYPE_STRING) && \
IS_GENERIC_NUMBER(ptrResultInstructionOp2)

#define OP1_IS_STRING_AND_OP2_IS_BOOLEAN \
(ptrResultInstructionOp1->type == INS_TYPE_STRING) && \
(ptrResultInstructionOp2->type == INS_TYPE_BOOLEAN)


#define OP1_AND_OP2_ARE_BOOLEANS \
(ptrResultInstructionOp1->type == INS_TYPE_BOOLEAN) && \
(ptrResultInstructionOp2->type == INS_TYPE_BOOLEAN)

#define OP1_AND_OP2_ARE_STRINGS \
(ptrResultInstructionOp1->type == INS_TYPE_STRING) && \
(ptrResultInstructionOp2->type == INS_TYPE_STRING)

#define PROCESS_NUM_OPERATION(__OVERR_OP__)\
					if (IS_INT(ptrResultInstructionOp1) && IS_INT(ptrResultInstructionOp2)){\
						if(!pushInteger(LOAD_INT_OP(ptrResultInstructionOp1) __OVERR_OP__ LOAD_INT_OP(ptrResultInstructionOp2))){\
							return false;\
						}\
					}else if (IS_INT(ptrResultInstructionOp1) && IS_NUMBER(ptrResultInstructionOp2)){\
						if(!pushInteger(LOAD_INT_OP(ptrResultInstructionOp1) __OVERR_OP__ LOAD_NUMBER_OP(ptrResultInstructionOp2))) {\
							return false;\
						}\
					}else if (IS_NUMBER(ptrResultInstructionOp1) && IS_INT(ptrResultInstructionOp2)){\
						if(!pushNumber(LOAD_NUMBER_OP(ptrResultInstructionOp1) __OVERR_OP__ LOAD_INT_OP(ptrResultInstructionOp2))) {\
							return false;\
						}\
					}else {\
						if(!pushNumber(LOAD_NUMBER_OP(ptrResultInstructionOp1) __OVERR_OP__ LOAD_NUMBER_OP(ptrResultInstructionOp2))) {\
							return false;\
						}\
					}


string CVirtualMachine::STR_GET_TYPE_VAR_INDEX_INSTRUCTION(tAleObjectInfo *ptr_info_ale){
	string result="undefined";
	if(IS_INT(ptr_info_ale))
		result= "int";
	else if(IS_NUMBER(ptr_info_ale))
		result= "number";
	else if(IS_BOOLEAN(ptr_info_ale))
		result= "bool";
	else if(IS_STRING(ptr_info_ale))
		result= "string";
	else if(IS_FUNCTION(ptr_info_ale))
		result= "function";
	else if(IS_VAR(ptr_info_ale)){
		result=((CScriptVariable *)ptr_info_ale->stkObject)->getClassName();
	}


	return result;

}

// NUMBER result behaviour.
// this is the combination for number operations:
//
// op1 | op2 |  R
// ----+-----+----
//  i  |  i  |  i
//  i  |  f  |  i
//  f  |  i  |  f
//  f  |  f  |  f




bool CVirtualMachine::pushInteger(int  init_value, CScriptVariable ** ptrAssignable){

	stkResultInstruction[idxStkCurrentResultInstruction]={INS_TYPE_INTEGER,(void*)init_value,ptrAssignable};

	return true;
}

bool CVirtualMachine::pushBoolean(bool init_value, CScriptVariable ** ptrAssignable, int n_stk){


	stkResultInstruction[idxStkCurrentResultInstruction]={INS_TYPE_BOOLEAN,(void *)init_value,ptrAssignable};

	return true;
}

bool CVirtualMachine::pushNumber(float init_value, CScriptVariable ** ptrAssignable){
	if(idxStkCurrentNumber ==VM_ALE_OPERATIONS_MAX_STACK){
		print_error_cr("Reached max number operations");
		return false;
	}

	stkNumber[idxStkCurrentNumber]=init_value;
	stkResultInstruction[idxStkCurrentResultInstruction]={INS_TYPE_NUMBER,&stkNumber[idxStkCurrentNumber],ptrAssignable};
	idxStkCurrentNumber++;

	return true;
}

bool CVirtualMachine::pushString(const string & init_value, CScriptVariable ** ptrAssignable){
	if(idxStkCurrentString ==VM_ALE_OPERATIONS_MAX_STACK){
		print_error_cr("Reached max string operations");
		return false;
	}


	stkString[idxStkCurrentString]=init_value;
	stkResultInstruction[idxStkCurrentResultInstruction]={INS_TYPE_STRING,&stkString[idxStkCurrentString],ptrAssignable};
	idxStkCurrentString++;

	return true;

}

bool CVirtualMachine::pushFunction(tInfoRegisteredFunctionSymbol * init_value, CScriptVariable ** ptrAssignable){

	stkResultInstruction[idxStkCurrentResultInstruction]={INS_TYPE_FUNCTION,init_value,ptrAssignable};
	return true;
}



bool CVirtualMachine::pushVar(CScriptVariable * init_value, CScriptVariable ** ptrAssignableVar){


	int idxClass = init_value->getIdxClass();


	// finally assign the value ...
	if(idxClass == CScriptClassFactory::getInstance()->getIdxClassInteger()){
		return pushInteger(((CInteger *)init_value)->m_value,ptrAssignableVar);
	}else if(idxClass == CScriptClassFactory::getInstance()->getIdxClassNumber()){
		return pushNumber(((CNumber *)init_value)->m_value,ptrAssignableVar);
	}else if(idxClass == CScriptClassFactory::getInstance()->getIdxClassString()){
		return pushString(((CString *)init_value)->m_value,ptrAssignableVar);
	}else if(idxClass == CScriptClassFactory::getInstance()->getIdxClassBoolean()){
		return pushBoolean(((CBoolean *)init_value)->m_value,ptrAssignableVar);
	}else if(idxClass == CScriptClassFactory::getInstance()->getIdxClassFunctor()){
		return pushFunction(((CFunctor *)init_value)->m_value,ptrAssignableVar);
	}else{
		if(idxStkCurrentLocalVar ==VM_ALE_OPERATIONS_MAX_STACK){
			print_error_cr("Reached max object operations");
			return false;
		}

		stkResultInstruction[idxStkCurrentResultInstruction]={INS_TYPE_VAR,init_value, ptrAssignableVar};


	}

	return true;

}


CScriptVariable * CVirtualMachine::createVarFromResultInstruction(tAleObjectInfo * ptr_instruction){
	CScriptVariable *obj = NULL;


	// check second operand valid object..
	switch(ptr_instruction->type){
	default:
		print_error_cr("(internal) cannot determine var type");
		return NULL;
		break;
	case VALUE_INSTRUCTION_TYPE::INS_TYPE_UNDEFINED:
		obj=CScriptVariable::UndefinedSymbol;
		break;
	case VALUE_INSTRUCTION_TYPE::INS_TYPE_INTEGER:
		obj= NEW_INTEGER_VAR;//CScriptClassFactory::getInstance()->newClassByIdx(CScriptClassFactory::getInstance()->getIdxClassInteger());
		((CInteger *)obj)->m_value = ((int)(ptr_instruction->stkObject));
		break;
	case VALUE_INSTRUCTION_TYPE::INS_TYPE_NUMBER:
		obj= NEW_NUMBER_VAR;//CScriptClassFactory::getInstance()->newClassByIdx(CScriptClassFactory::getInstance()->getIdxClassNumber());
		((CNumber *)obj)->m_value = *((float *)(ptr_instruction->stkObject));
		break;
	case VALUE_INSTRUCTION_TYPE::INS_TYPE_STRING:
		obj= NEW_STRING_VAR;//=CScriptClassFactory::getInstance()->newClassByIdx(CScriptClassFactory::getInstance()->getIdxClassString());
		((CString *)obj)->m_value = *((string *)(ptr_instruction->stkObject));
		break;
	case VALUE_INSTRUCTION_TYPE::INS_TYPE_BOOLEAN:
		obj= NEW_BOOLEAN_VAR;//=CScriptClassFactory::getInstance()->newClassByIdx(CScriptClassFactory::getInstance()->getIdxClassBoolean());
		((CBoolean *)obj)->m_value = ((bool)(ptr_instruction->stkObject));
		break;
	case VALUE_INSTRUCTION_TYPE::INS_TYPE_FUNCTION:
		obj = NEW_FUNCTOR_VAR(((tInfoRegisteredFunctionSymbol *)ptr_instruction->stkObject));
		break;
	case VALUE_INSTRUCTION_TYPE::INS_TYPE_VAR:
		obj = (CScriptVariable *)ptr_instruction->stkObject;
		break;

	}

	return obj;
}
/*
bool CVirtualMachine::performPreOperator(ASM_PRE_POST_OPERATORS pre_post_operator_type, CScriptVariable *var){
	// ok from here, let's check preoperator ...

	unsigned idx_class=var->getIdxClass();

	if(idx_class==CScriptClassFactory::getInstance()->getIdxClassInteger()){
		if(pre_post_operator_type == ASM_PRE_POST_OPERATORS::PRE_INC)
			((CInteger *)var)->m_value++;
		else //dec
			((CInteger *)var)->m_value--;

	}else if(idx_class == CScriptClassFactory::getInstance()->getIdxClassNumber()){
		if(pre_post_operator_type == ASM_PRE_POST_OPERATORS::PRE_INC)
			((CNumber *)var)->m_value++;
		else // dec
			((CNumber *)var)->m_value--;

	}else{

		print_error_cr("Cannot perform preoperator ?? because is not number");
		return false;
	}

	return true;


}

bool CVirtualMachine::performPostOperator(ASM_PRE_POST_OPERATORS pre_post_operator_type, CScriptVariable *var){
	// ok from here, let's check preoperator ...

	unsigned idx_class=var->getIdxClass();

	if(idx_class==CScriptClassFactory::getInstance()->getIdxClassInteger()){
		if(pre_post_operator_type == ASM_PRE_POST_OPERATORS::POST_INC)
			((CInteger *)var)->m_value++;
		else //dec
			((CInteger *)var)->m_value--;
	}else if(idx_class == CScriptClassFactory::getInstance()->getIdxClassNumber()){
		if(pre_post_operator_type == ASM_PRE_POST_OPERATORS::POST_INC)
			((CNumber *)var)->m_value++;
		else // dec
			((CNumber *)var)->m_value--;
	}else{
		print_error_cr("Cannot perform postoperator ?? because is not number");
		return false;
	}

	return true;
}
*/

bool CVirtualMachine::loadVariableValue(tInfoAsmOp *iao, tInfoRegisteredFunctionSymbol *info_function,CScriptVariable *this_object, int n_stk){

	if(iao->index_op1 != LOAD_TYPE_VARIABLE){
		print_error_cr("expected load type variable.");
		return false;
	}

	//CScriptVariable *this_object = function_object->getThisObject();
	CScriptVariable::tSymbolInfo *si;
	CScriptVariable **ptr_var_object=NULL;
	CScriptVariable *var_object = NULL;

	bool push_object=false;
	bool is_valid_variable = true;
	//bool function_struct_type = false;



	switch(iao->scope_type){
	default:
		print_error_cr("unknow scope type");
		break;
	case SCOPE_TYPE::THIS_SCOPE:

		// get var from object ...
		if((si = this_object->getVariableSymbolByIndex(iao->index_op2))==NULL){
			print_error_cr("cannot find symbol \"%s\"",iao->ast_node->value_symbol.c_str());
			return false;
		}

		ptr_var_object = (CScriptVariable **)(&si->object);
		var_object = (CScriptVariable *)(si->object);

		break;

	case SCOPE_TYPE::LOCAL_SCOPE:

		if(iao->index_op2 >=  CVirtualMachine::idxStkCurrentLocalVar){
			print_error_cr("internal error: index out of stack");
			return false;
		}

		// get var from base stack ...
		ptr_var_object = (CScriptVariable **)(&CVirtualMachine::basePtrLocalVar[iao->index_op2].stkObject);
		var_object = (CScriptVariable *)(CVirtualMachine::basePtrLocalVar[iao->index_op2].stkObject);


		/*if((si = this_object->getVariableSymbolByIndex(iao->index_op2))==NULL){
			print_error_cr("cannot find symbol \"%s\"",iao->ast_node->value_symbol.c_str());

			return false;
		}*/

		break;


	}


	is_valid_variable = (var_object != CScriptVariable::UndefinedSymbol);// && (!function_struct_type);

	push_object = true;


	if(is_valid_variable){

		int idx_class =  var_object->getIdxClass();

		//CScriptVariable **ptr_var_object = (CScriptVariable **)(&si->object);
		//CScriptVariable *var_object = (CScriptVariable *)(si->object);
		if(iao->pre_post_operator_type == ASM_PRE_POST_OPERATORS::PRE_DEC || iao->pre_post_operator_type == ASM_PRE_POST_OPERATORS::PRE_INC){

			/*if(!performPreOperator(iao->pre_post_operator_type, var_object)){
				return false;
			}*/
			if(idx_class==CScriptClassFactory::getInstance()->getIdxClassInteger()){
				if(iao->pre_post_operator_type == ASM_PRE_POST_OPERATORS::PRE_INC)
					((CInteger *)var_object)->m_value++;
				else //dec
					((CInteger *)var_object)->m_value--;

			}else if(idx_class == CScriptClassFactory::getInstance()->getIdxClassNumber()){
				if(iao->pre_post_operator_type == ASM_PRE_POST_OPERATORS::PRE_INC)
					((CNumber *)var_object)->m_value++;
				else // dec
					((CNumber *)var_object)->m_value--;

			}else{

				print_error_cr("internal error:Cannot perform preoperator ?? because is not number");
				return false;
			}
		}

		if(iao->pre_post_operator_type == ASM_PRE_POST_OPERATORS::POST_DEC || iao->pre_post_operator_type == ASM_PRE_POST_OPERATORS::POST_INC){

			push_object = false; // first will push the value and after will increment ...
			// 1. Load value as constant value
			//if(!loadConstantValue(var_object,n_stk)){
			if(CScriptClassFactory::getInstance()->getIdxClassInteger() == idx_class){
				// 1. first load ...
				pushInteger(((CInteger *)var_object)->m_value);
				// 2. increment ...
				if(iao->pre_post_operator_type == ASM_PRE_POST_OPERATORS::POST_INC)
					((CInteger *)var_object)->m_value++;
				else
					((CInteger *)var_object)->m_value--;
			}else if(CScriptClassFactory::getInstance()->getIdxClassNumber() == var_object->getIdxClass()){
				// 1. first load ...
				pushNumber(((CNumber *)var_object)->m_value);
				// 2. increment ...
				if(iao->pre_post_operator_type == ASM_PRE_POST_OPERATORS::POST_INC)
					((CNumber *)var_object)->m_value++;
				else
					((CNumber *)var_object)->m_value--;
			}else{
				print_error_cr("internal error:cannot postoperator ?? because is not number");
				return false;
			}

			// 2. then perform post operation ...
			/*if(!performPostOperator(iao->pre_post_operator_type, var_object)){
				return false;
			}*/

		}
	}


	// generic object pushed ...
	if(push_object){
		if(!pushVar(var_object,ptr_var_object)) {
				return false;
		}
	}

	return true;
}

bool CVirtualMachine::loadFunctionValue(tInfoAsmOp *iao,tInfoRegisteredFunctionSymbol *local_function, CScriptVariable *this_object, int n_stk){

	if(iao->index_op1 != LOAD_TYPE_FUNCTION){
		print_error_cr("expected load type function.");
		return false;
	}


	tInfoRegisteredFunctionSymbol *info_function=NULL;

	CScriptVariable::tSymbolInfo *si;

	//CScriptVariable *var_object = NULL;
	//tInfoRegisteredFunctionSymbol *info_function = (tInfoRegisteredFunctionSymbol *)(si->object);
	//CScriptVariable *this_object = function_object->getThisObject();
	//tInfoRegisteredFunctionSymbol *si;

	/*if((si = this_object->getFunctionSymbol(iao->index_op2))==NULL){
		print_error_cr("cannot find function info \"%s\"",iao->ast_node->value_symbol.c_str());
		return false;
	}*/

	switch(iao->scope_type){
	default:
		print_error_cr("unknow scope type");
		break;
	case SCOPE_TYPE::THIS_SCOPE:

		// get var from object ...
		if((si = this_object->getFunctionSymbolByIndex(iao->index_op2))==NULL){
			print_error_cr("cannot find symbol \"%s\"",iao->ast_node->value_symbol.c_str());
			return false;
		}

		info_function =(tInfoRegisteredFunctionSymbol *)si->object;

		break;

	case SCOPE_TYPE::LOCAL_SCOPE:
		info_function = &local_function->object_info.local_symbols.m_registeredFunction[iao->index_op2];
		break;


	}

	// generic object pushed ...
	if(!pushFunction(info_function)) {
		return false;
	}
	//stkResultInstruction[idxStkCurrentResultInstruction]={CScriptVariable::FUNCTION,(CScriptVariable **)si, false};


	return true;
}

bool CVirtualMachine::loadConstantValue(CCompiler::tInfoConstantValue *info_constant, int n_stk){



	switch(info_constant->type){
		default:
			print_error_cr("Invalid load constant value as %i",info_constant->type);
			return false;
			break;
		case INS_TYPE_UNDEFINED:

			stkResultInstruction[idxStkCurrentResultInstruction]={INS_TYPE_UNDEFINED,NULL,NULL};

			break;
		case INS_TYPE_INTEGER:
			if(!pushInteger(*((int *)info_constant->ptr))) return false;
			break;
		case INS_TYPE_BOOLEAN:
			if(!pushBoolean(*((bool *)info_constant->ptr))) return false;
			break;
		case INS_TYPE_STRING:
			if(!pushString((*((string *)info_constant->ptr)))) return false;
			break;
		case INS_TYPE_NUMBER:
			if(!pushNumber(*((float *)info_constant->ptr))) return false;
			break;
		}

		return true;


}





bool CVirtualMachine::assignVarFromResultInstruction(CScriptVariable **var, tAleObjectInfo  * ptr_instruction){

	CScriptVariable *aux_var=NULL;


	// if undefined, create new by default ...
	if(*var == NULL){

		if((*var = createVarFromResultInstruction(ptr_instruction)) == NULL){
			return false;
		}
	}

	int idxClass=-1;

	if(*var!=CScriptVariable::UndefinedSymbol){
		idxClass = (*var)->getIdxClass();
	}

	//tInfoRegisteredFunctionSymbol * init_value;
	bool create_from_index=false;

	// finally assign the value ...
	switch(ptr_instruction->type){

		case INS_TYPE_UNDEFINED:
			*var = CScriptVariable::UndefinedSymbol;
			break;

		case INS_TYPE_INTEGER:
			if(idxClass == CScriptClassFactory::getInstance()->getIdxClassInteger()){
				((CInteger *)(*var))->m_value=((CInteger *)(ptr_instruction->stkObject))->m_value;
			/*}else if((*var)->getIdxClass() == CScriptClassFactory::getInstance()->getIdxClassNumber()){
				((CNumber *)(*var))->m_value=((CInteger *)(stkResultInstruction[index].stkObject))->m_value;*/
			}else
			{
				create_from_index=true;
			}
			break;
		case INS_TYPE_NUMBER:

			/*if(idxClass == CScriptClassFactory::getInstance()->getIdxClassInteger()){
				((CInteger *)aux_var)->m_value=((CNumber *)(stkResultInstruction[index].stkObject))->m_value;
			}else*/
			if(idxClass == CScriptClassFactory::getInstance()->getIdxClassNumber()){
				((CNumber *)(*var))->m_value=((CNumber *)(ptr_instruction->stkObject))->m_value;
			}else
			{
				create_from_index=true;
			}
			break;
		case INS_TYPE_STRING:

			if(idxClass == CScriptClassFactory::getInstance()->getIdxClassString()){
				((CString  *)(*var))->m_value= ((CString *)(ptr_instruction->stkObject))->m_value;
			}else
			{
				create_from_index=true;
			}

			break;
		case INS_TYPE_BOOLEAN:
			if(idxClass == CScriptClassFactory::getInstance()->getIdxClassBoolean()){
				((CBoolean  *)aux_var)->m_value= ((CBoolean *)(ptr_instruction->stkObject))->m_value;
			}else
			{
				create_from_index=true;
			}
			break;
		case INS_TYPE_FUNCTION: // function object

			if(idxClass == CScriptClassFactory::getInstance()->getIdxClassFunctor()){
				((CFunctor  *)aux_var)->m_value= ((CFunctor *)(ptr_instruction->stkObject))->m_value;
			}else{
				create_from_index=true;
			}
			break;
		case INS_TYPE_VAR: // generic object, assign pointer ...
			*var = (CScriptVariable *)(ptr_instruction->stkObject);
			break;

		default:
				print_error_cr("internal error: unknow assignment %i!",ptr_instruction->type);
				return false;
				break;
		}

	if(create_from_index){
		if((*var = createVarFromResultInstruction(ptr_instruction)) == NULL){
			return false;
		}
	}


	return true;
}

bool CVirtualMachine::performInstruction( tInfoAsmOp * instruction, int & jmp_to_statment,tInfoRegisteredFunctionSymbol *info_function,CScriptVariable *this_object,vector<CScriptVariable *> * argv, int n_stk){


	string 	aux_string;
	bool	aux_boolean;
	string symbol;
	CScriptVariable **obj=NULL;
	tInfoRegisteredFunctionSymbol * aux_function_info=NULL;
	CScriptVariable *ret_obj;



	jmp_to_statment=-1;

	//idxStkCurrentResultInstruction = idx_instruction;

	//CScopeInfo *_lc = instruction->ast_node->scope_info_ptr;

	int index_op1 = instruction->index_op1;
	int index_op2 = instruction->index_op2;
	tAleObjectInfo *ptrResultInstructionOp1=&stkResultInstruction[index_op1+startIdxStkResultInstruction];
	tAleObjectInfo *ptrResultInstructionOp2=&stkResultInstruction[index_op2+startIdxStkResultInstruction];
	tAleObjectInfo *ptrResultLastInstruction=&stkResultInstruction[idxStkCurrentResultInstruction+startIdxStkResultInstruction-1];






	switch(instruction->operator_type){
	default:
		print_error_cr("operator type(%s) not implemented",CCompiler::def_operator[instruction->operator_type].op_str);
		break;
	case NOP: // ignore ...
		break;
	case LOAD: // load value in function of value/constant ...
		/*if(!loadValue(instruction, n_stk)){
			return false;
		}*/
		//sprintf(print_aux_load_value,"UNDEFINED");
		switch(instruction->index_op1){
		case LOAD_TYPE::LOAD_TYPE_CONSTANT:

			if(!loadConstantValue((CCompiler::tInfoConstantValue *)instruction->index_op2, n_stk)){
				return false;
			}

			//sprintf(print_aux_load_value,"CONST(%s)",value_symbol.c_str());
			break;
		case LOAD_TYPE::LOAD_TYPE_VARIABLE:

			if(!loadVariableValue(instruction, info_function,this_object, n_stk)){
				return false;
			}

			break;
		case LOAD_TYPE::LOAD_TYPE_FUNCTION:
			if(!loadFunctionValue(instruction,info_function, this_object, n_stk)){
				return false;
			}

			break;
		case LOAD_TYPE::LOAD_TYPE_ARGUMENT:

			if(argv!=NULL){
				if(index_op2<(int)argv->size()){
					CScriptVariable *var=(*argv)[index_op2];

					pushVar(var,NULL);
				}else{
					print_error_cr("index out of bounds");
					return false;
				}
			}
			else{
				print_error_cr("argv null");
				return false;
			}

			//sprintf(print_aux_load_value,"ARG(%s)",value_symbol.c_str());
			break;
		default:
			print_error_cr("no load defined type");
			return false;
			break;

		}

		break;
		case MOV: // mov value expression to var

			// ok load object pointer ...
			if((obj = ptrResultInstructionOp1->ptrAssignableVar) != NULL) {// == CScriptVariable::VAR_TYPE::OBJECT){

				// get pointer object (can be assigned)
				//obj = stkResultInstruction[index_op1].stkObject;



				if(!assignVarFromResultInstruction(obj,ptrResultInstructionOp2))
						return false;

			}else{
				print_error_cr("Expected object l-value mov");
				return false;
			}

			break;
		case EQU:  // == --> boolean && boolean or string && string or number && number

			if(OP1_AND_OP2_ARE_BOOLEANS) {
				if(!pushBoolean(LOAD_BOOL_OP(ptrResultInstructionOp1) == LOAD_BOOL_OP(ptrResultInstructionOp2))) return false;
			}else if(OP1_AND_OP2_ARE_STRINGS){
				if(!pushBoolean(LOAD_STRING_OP(ptrResultInstructionOp1) == LOAD_STRING_OP(ptrResultInstructionOp2))) return false;
			}else if (IS_INT(ptrResultInstructionOp1) && IS_INT(ptrResultInstructionOp2)){
				if(!pushBoolean(LOAD_INT_OP(ptrResultInstructionOp1) == LOAD_INT_OP(ptrResultInstructionOp2))) return false;
			}else if (IS_INT(ptrResultInstructionOp1) && IS_NUMBER(ptrResultInstructionOp2)){
				if(!pushBoolean(LOAD_INT_OP(ptrResultInstructionOp1) == LOAD_NUMBER_OP(ptrResultInstructionOp2))) return false;
			}else if (IS_NUMBER(ptrResultInstructionOp1) && IS_INT(ptrResultInstructionOp2)){
				if(!pushBoolean(LOAD_NUMBER_OP(ptrResultInstructionOp1) == LOAD_INT_OP(ptrResultInstructionOp2))) return false;
			}else if (IS_NUMBER(ptrResultInstructionOp1) && IS_NUMBER(ptrResultInstructionOp2)){
				if(!pushBoolean(LOAD_NUMBER_OP(ptrResultInstructionOp1) == LOAD_NUMBER_OP(ptrResultInstructionOp2))) return false;
			}else{
				print_error_cr("Expected both operands as string, number or boolean!");
				return false;
			}

			break;

		case NOT_EQU:  // == --> boolean && boolean or string && string or number && number

			if(OP1_AND_OP2_ARE_BOOLEANS) {
				if(!pushBoolean(LOAD_BOOL_OP(ptrResultInstructionOp1) != LOAD_BOOL_OP(ptrResultInstructionOp2))) return false;
			}else if(OP1_AND_OP2_ARE_STRINGS){
				if(!pushBoolean(LOAD_STRING_OP(ptrResultInstructionOp1) != LOAD_STRING_OP(ptrResultInstructionOp2))) return false;
			}else if (IS_INT(ptrResultInstructionOp1) && IS_INT(ptrResultInstructionOp2)){
				if(!pushBoolean(LOAD_INT_OP(ptrResultInstructionOp1) != LOAD_INT_OP(ptrResultInstructionOp2))) return false;
			}else if (IS_INT(ptrResultInstructionOp1) && IS_NUMBER(ptrResultInstructionOp2)){
				if(!pushBoolean(LOAD_INT_OP(ptrResultInstructionOp1) != LOAD_NUMBER_OP(ptrResultInstructionOp2))) return false;
			}else if (IS_NUMBER(ptrResultInstructionOp1) && IS_INT(ptrResultInstructionOp2)){
				if(!pushBoolean(LOAD_NUMBER_OP(ptrResultInstructionOp1) != LOAD_INT_OP(ptrResultInstructionOp2))) return false;
			}else if (IS_NUMBER(ptrResultInstructionOp1) && IS_NUMBER(ptrResultInstructionOp2)){
				if(!pushBoolean(LOAD_NUMBER_OP(ptrResultInstructionOp1) != LOAD_NUMBER_OP(ptrResultInstructionOp2))) return false;
			}else{
				print_error_cr("Expected both operands as string, number or boolean!");
				return false;
			}

			break;
		case LT:  // <
			if (IS_INT(ptrResultInstructionOp1) && IS_INT(ptrResultInstructionOp2)){
				if(!pushBoolean(LOAD_INT_OP(ptrResultInstructionOp1) < LOAD_INT_OP(ptrResultInstructionOp2))) return false;
			}else if (IS_INT(ptrResultInstructionOp1) && IS_NUMBER(ptrResultInstructionOp2)){
				if(!pushBoolean(LOAD_INT_OP(ptrResultInstructionOp1) < LOAD_NUMBER_OP(ptrResultInstructionOp2))) return false;
			}else if (IS_NUMBER(ptrResultInstructionOp1) && IS_INT(ptrResultInstructionOp2)){
				if(!pushBoolean(LOAD_NUMBER_OP(ptrResultInstructionOp1) < LOAD_INT_OP(ptrResultInstructionOp2))) return false;
			}else if (IS_NUMBER(ptrResultInstructionOp1) && IS_NUMBER(ptrResultInstructionOp2)){
				if(!pushBoolean(LOAD_NUMBER_OP(ptrResultInstructionOp1) < LOAD_NUMBER_OP(ptrResultInstructionOp2))) return false;
			}else{
				print_error_cr("Expected both operands as number!");
				return false;
			}
			break;
		case LTE:  // <=

			if (IS_INT(ptrResultInstructionOp1) && IS_INT(ptrResultInstructionOp2)){
				if(!pushBoolean(LOAD_INT_OP(ptrResultInstructionOp1) <= LOAD_INT_OP(ptrResultInstructionOp2))) return false;
			}else if (IS_INT(ptrResultInstructionOp1) && IS_NUMBER(ptrResultInstructionOp2)){
				if(!pushBoolean(LOAD_INT_OP(ptrResultInstructionOp1) <= LOAD_NUMBER_OP(ptrResultInstructionOp2))) return false;
			}else if (IS_NUMBER(ptrResultInstructionOp1) && IS_INT(ptrResultInstructionOp2)){
				if(!pushBoolean(LOAD_NUMBER_OP(ptrResultInstructionOp1) <= LOAD_INT_OP(ptrResultInstructionOp2))) return false;
			}else if (IS_NUMBER(ptrResultInstructionOp1) && IS_NUMBER(ptrResultInstructionOp2)){
				if(!pushBoolean(LOAD_NUMBER_OP(ptrResultInstructionOp1) <= LOAD_NUMBER_OP(ptrResultInstructionOp2))) return false;
			}else{
				print_error_cr("Expected both operands as number!");
				return false;
			}

			break;
		case NOT: // !
			if (ptrResultInstructionOp1->type == INS_TYPE_BOOLEAN){
				if(!pushBoolean(!LOAD_BOOL_OP(ptrResultInstructionOp1))) return false;
			}else{
				print_error_cr("Expected operands 1 as boolean!");
				return false;
			}
			break;
		case NEG: // !
			if (IS_GENERIC_NUMBER(ptrResultInstructionOp1)){
				if(ptrResultInstructionOp1->type == INS_TYPE_INTEGER){ // operation will result as integer.
					if(!pushInteger(-LOAD_INT_OP(ptrResultInstructionOp1))) {
						return false;
					}
				}
				else{
					if(!pushNumber(-LOAD_NUMBER_OP(ptrResultInstructionOp2))){
						return false;
					}
				}

			}else{
					print_error_cr("Expected operands 1 as number or integer!");
					return false;
			}
			break;

		case GT:  // >
			if (IS_INT(ptrResultInstructionOp1) && IS_INT(ptrResultInstructionOp2)){
				if(!pushBoolean(LOAD_INT_OP(ptrResultInstructionOp1) > LOAD_INT_OP(ptrResultInstructionOp2))) return false;
			}else if (IS_INT(ptrResultInstructionOp1) && IS_NUMBER(ptrResultInstructionOp2)){
				if(!pushBoolean(LOAD_INT_OP(ptrResultInstructionOp1) > LOAD_NUMBER_OP(ptrResultInstructionOp2))) return false;
			}else if (IS_NUMBER(ptrResultInstructionOp1) && IS_INT(ptrResultInstructionOp2)){
				if(!pushBoolean(LOAD_NUMBER_OP(ptrResultInstructionOp1) > LOAD_INT_OP(ptrResultInstructionOp2))) return false;
			}else if (IS_NUMBER(ptrResultInstructionOp1) && IS_NUMBER(ptrResultInstructionOp2)){
				if(!pushBoolean(LOAD_NUMBER_OP(ptrResultInstructionOp1) > LOAD_NUMBER_OP(ptrResultInstructionOp2))) return false;
			}else{
				print_error_cr("Expected both operands as number!");
				return false;
			}
			break;
		case GTE: // >=
			if (IS_INT(ptrResultInstructionOp1) && IS_INT(ptrResultInstructionOp2)){
				if(!pushBoolean(LOAD_INT_OP(ptrResultInstructionOp1) >= LOAD_INT_OP(ptrResultInstructionOp2))) return false;
			}else if (IS_INT(ptrResultInstructionOp1) && IS_NUMBER(ptrResultInstructionOp2)){
				if(!pushBoolean(LOAD_INT_OP(ptrResultInstructionOp1) >= LOAD_NUMBER_OP(ptrResultInstructionOp2))) return false;
			}else if (IS_NUMBER(ptrResultInstructionOp1) && IS_INT(ptrResultInstructionOp2)){
				if(!pushBoolean(LOAD_NUMBER_OP(ptrResultInstructionOp1) >= LOAD_INT_OP(ptrResultInstructionOp2))) return false;
			}else if (IS_NUMBER(ptrResultInstructionOp1) && IS_NUMBER(ptrResultInstructionOp2)){
				if(!pushBoolean(LOAD_NUMBER_OP(ptrResultInstructionOp1) >= LOAD_NUMBER_OP(ptrResultInstructionOp2))) return false;
			}else{
				print_error_cr("Expected both operands as number!");
				return false;
			}
			break;

		case ADD: // +

			// get indexes and check whether is possible or not its calculation.
			// check indexes
			//CHECK_VALID_INDEXES;

			// check types ...
			if (IS_STRING(ptrResultInstructionOp1) && (IS_UNDEFINED(ptrResultInstructionOp2) || IS_VAR(ptrResultInstructionOp2))){

				string result = "undefined";
				if(IS_VAR(ptrResultInstructionOp2)){
					result = ((CScriptVariable *)(ptrResultInstructionOp2->stkObject))->getClassName();
				}

				if(!pushString(LOAD_STRING_OP(ptrResultInstructionOp1)+result)) return false;
			}else if (IS_INT(ptrResultInstructionOp1) && IS_INT(ptrResultInstructionOp2)){
				if(!pushInteger(LOAD_INT_OP(ptrResultInstructionOp1) + LOAD_INT_OP(ptrResultInstructionOp2))) return false;
			}else if (IS_INT(ptrResultInstructionOp1) && IS_NUMBER(ptrResultInstructionOp2)){
				if(!pushInteger(LOAD_INT_OP(ptrResultInstructionOp1) + LOAD_NUMBER_OP(ptrResultInstructionOp2))) return false;
			}else if (IS_NUMBER(ptrResultInstructionOp1) && IS_INT(ptrResultInstructionOp2)){
				if(!pushNumber(LOAD_NUMBER_OP(ptrResultInstructionOp1) + LOAD_INT_OP(ptrResultInstructionOp2))) return false;
			}else if (IS_NUMBER(ptrResultInstructionOp1) && IS_NUMBER(ptrResultInstructionOp2)){
				if(!pushNumber(LOAD_NUMBER_OP(ptrResultInstructionOp1) + LOAD_NUMBER_OP(ptrResultInstructionOp2))) return false;
			}else if(OP1_IS_STRING_AND_OP2_IS_NUMBER){ // concatenate string + number

				aux_string =  LOAD_STRING_OP(ptrResultInstructionOp1);

				if(ptrResultInstructionOp2->type == INS_TYPE_INTEGER)
					aux_string = aux_string + CStringUtils::intToString(LOAD_INT_OP(ptrResultInstructionOp2));
				else
					aux_string = aux_string + CStringUtils::intToString(LOAD_NUMBER_OP(ptrResultInstructionOp2));

				if(!pushString(aux_string)) return false;
			}else if(OP1_IS_STRING_AND_OP2_IS_BOOLEAN){ // concatenate string + boolean

				aux_string =  LOAD_STRING_OP(ptrResultInstructionOp1);
				aux_boolean =  LOAD_BOOL_OP(ptrResultInstructionOp2);

				if(aux_boolean)
					aux_string = aux_string + "true";
				else
					aux_string = aux_string + "false";

				if(!pushString(aux_string)) return false;

			}else if(OP1_AND_OP2_ARE_STRINGS){ // concatenate string + boolean

				if(!pushString(LOAD_STRING_OP(ptrResultInstructionOp1)+LOAD_STRING_OP(ptrResultInstructionOp2))) return false;

			}else{

				// full error description ...

				string var_type1=STR_GET_TYPE_VAR_INDEX_INSTRUCTION(ptrResultInstructionOp1),
					   var_type2=STR_GET_TYPE_VAR_INDEX_INSTRUCTION(ptrResultInstructionOp2);


				//print_error_cr("Expected operands as number+number, string+string, string+number or string + boolean!");


				print_error_cr("Error at line %i cannot perform operator \"%s\" +  \"%s\"",
						instruction->ast_node->definedValueline,
						var_type1.c_str(),
						var_type2.c_str());
				return false;
			}

			break;

		case LOGIC_AND: // &&
			if(OP1_AND_OP2_ARE_BOOLEANS) {
				if(!pushBoolean(LOAD_BOOL_OP(ptrResultInstructionOp1) && LOAD_BOOL_OP(ptrResultInstructionOp2))) return false;
			}else{
				print_error_cr("Expected both operands boolean!");
				return false;
			}
			break;
		case LOGIC_OR:  // ||
			if(OP1_AND_OP2_ARE_BOOLEANS) {
				if(!pushBoolean(LOAD_BOOL_OP(ptrResultInstructionOp1) || LOAD_BOOL_OP(ptrResultInstructionOp2))) return false;
			}else{
				print_error_cr("Expected both operands boolean!");
				return false;
			}
			break;
		case DIV: // /
			if(OP1_AND_OP2_ARE_NUMBERS) {

				if(IS_INT(ptrResultInstructionOp2)){
					if(LOAD_INT_OP(ptrResultInstructionOp2) == 0) {
						print_error_cr("Divide by 0 at line %i.",instruction->ast_node->definedValueline);
						return false;
					}
				}else{
					if(LOAD_NUMBER_OP(ptrResultInstructionOp2) == 0) {
						print_error_cr("Divide by 0 at line %i.",instruction->ast_node->definedValueline);
						return false;
					}
				}

				PROCESS_NUM_OPERATION(/);
			}else{
				print_error_cr("Expected both operands as number!");
				return false;
			}

			break;
		case MUL: // *
			if(OP1_AND_OP2_ARE_NUMBERS) {
					PROCESS_NUM_OPERATION(*);

			}else{
				print_error_cr("Expected both operands as number!");
				return false;
			}
			break;
		case MOD:  // %
			if(OP1_AND_OP2_ARE_NUMBERS) {

				if(IS_INT(ptrResultInstructionOp2)){
					if(LOAD_INT_OP(ptrResultInstructionOp2) == 0) {
						print_error_cr("Divide by 0 at line %i.",instruction->ast_node->definedValueline);
						return false;
					}
				}else{
					if(LOAD_NUMBER_OP(ptrResultInstructionOp2) == 0) {
						print_error_cr("Divide by 0 at line %i.",instruction->ast_node->definedValueline);
						return false;
					}
				}
				//PROCESS_NUM_OPERATION(%);

				if (IS_INT(ptrResultInstructionOp1) && IS_INT(ptrResultInstructionOp2)){
					if(!pushInteger(LOAD_INT_OP(ptrResultInstructionOp1) % LOAD_INT_OP(ptrResultInstructionOp2))){
						return false;\
					}
				}else if (IS_INT(ptrResultInstructionOp1) && IS_NUMBER(ptrResultInstructionOp2)){
					if(!pushInteger(LOAD_INT_OP(ptrResultInstructionOp1) % ((int) LOAD_NUMBER_OP(ptrResultInstructionOp2)))) {
						return false;\
					}
				}else if (IS_NUMBER(ptrResultInstructionOp1) && IS_INT(ptrResultInstructionOp2)){
					if(!pushNumber(fmod(LOAD_NUMBER_OP(ptrResultInstructionOp1), LOAD_INT_OP(ptrResultInstructionOp2)))) {
						return false;
					}
				}else {
					if(!pushNumber(fmod(LOAD_NUMBER_OP(ptrResultInstructionOp1) , LOAD_NUMBER_OP(ptrResultInstructionOp2)))) {
						return false;
					}
				}

			}else{
				print_error_cr("Expected both operands as number!");
				return false;
			}

			break;
		case AND: // bitwise logic and
			if((ptrResultInstructionOp1->type == INS_TYPE_INTEGER) && (ptrResultInstructionOp2->type == INS_TYPE_INTEGER)){
				if(!pushInteger(LOAD_INT_OP(ptrResultInstructionOp1) & LOAD_INT_OP(ptrResultInstructionOp2))) return false;
			}else{
				print_error_cr("Expected both operands as integer types!");
				return false;
			}
			break;
		case OR: // bitwise logic or
			if((ptrResultInstructionOp1->type == INS_TYPE_INTEGER) && (ptrResultInstructionOp2->type == INS_TYPE_INTEGER)){
				if(!pushInteger(LOAD_INT_OP(ptrResultInstructionOp1) | LOAD_INT_OP(ptrResultInstructionOp2))) return false;
			}else{
				print_error_cr("Expected both operands as integer types!");
				return false;
			}

			break;
		case XOR: // logic xor
			if((ptrResultInstructionOp1->type == INS_TYPE_INTEGER) && (ptrResultInstructionOp2->type == INS_TYPE_INTEGER)){
				if(!pushInteger(LOAD_INT_OP(ptrResultInstructionOp1) ^ LOAD_INT_OP(ptrResultInstructionOp2))) return false;
			}else{
				print_error_cr("Expected both operands as integer types!");
				return false;
			}
			break;
		case SHL: // shift left
			if((ptrResultInstructionOp1->type == INS_TYPE_INTEGER) && (ptrResultInstructionOp2->type == INS_TYPE_INTEGER)){
				if(!pushInteger(LOAD_INT_OP(ptrResultInstructionOp1) << LOAD_INT_OP(ptrResultInstructionOp2))) return false;
			}else{
				print_error_cr("Expected both operands as integer types!");
				return false;
			}
			break;
		case SHR: // shift right
			if((ptrResultInstructionOp1->type == INS_TYPE_INTEGER) && (ptrResultInstructionOp2->type == INS_TYPE_INTEGER)){
				if(!pushInteger(LOAD_INT_OP(ptrResultInstructionOp1) >> LOAD_INT_OP(ptrResultInstructionOp2))) return false;
			}else{
				print_error_cr("Expected both operands as integer types!");
				return false;
			}
			break;
		// special internal ops...
		case JMP:
			jmp_to_statment = index_op1;
			break;
		case JNT: // goto if not true ... goes end to conditional.

			// load boolean var and jmp if true...
			if(idxStkCurrentResultInstruction > 0){

				if(ptrResultLastInstruction->type == INS_TYPE_BOOLEAN){

					if(!(*((CBoolean **)ptrResultLastInstruction->stkObject))->m_value){
						jmp_to_statment = index_op1;
					}
				}
			}else{
				print_error_cr("No boolean elements");
				return false;
			}
			break;
		case JT: // goto if true ... goes end to conditional.
			if(idxStkCurrentResultInstruction > 0){

				if(ptrResultLastInstruction->type == INS_TYPE_BOOLEAN){

					if((*((CBoolean **)ptrResultLastInstruction->stkObject))->m_value){
						jmp_to_statment = index_op1;
					}
				}
			}else{
				print_error_cr("No boolean elements");
				return false;
			}
			break;
		case CALL: // calling function after all of args are processed...

			// check whether signatures matches or not ...
			// 1. get function object ...
			aux_function_info=(tInfoRegisteredFunctionSymbol *)ptrResultInstructionOp1->stkObject;
			if(ptrResultInstructionOp1->type != INS_TYPE_FUNCTION){
				if(ptrResultInstructionOp1->type == INS_TYPE_VAR && ((CScriptVariable *)ptrResultInstructionOp1->stkObject)->getIdxClass() == CScriptClassFactory::getInstance()->getIdxClassFunctor()){
					aux_function_info = ((CFunctor *)ptrResultInstructionOp1->stkObject)->m_value;
				}else {
					print_error_cr("object \"%s\" is not function at line %i",instruction->ast_node->value_symbol.c_str(), instruction->ast_node->definedValueline);
					return false;
				}
			}


			// by default virtual machine gets main object class in order to run functions ...
			if((ret_obj=CVirtualMachine::execute(aux_function_info,this_object,&m_functionArgs, n_stk+1))==NULL){
				return false;
			}

			// finally set result value into stkObject...
			if(!pushVar(ret_obj)){
				return false;
			}
			break;
		case PUSH: // push arg instruction will creating object ensures not to have e/s...
			m_functionArgs.push_back(createVarFromResultInstruction(ptrResultInstructionOp1));
			break;
		case CLR: // clear args
			m_functionArgs.clear();
			break;
		case VGET: // vector access after each index is processed...
			// index_op1 is vector, index op2 is index...
			if(IS_VECTOR(ptrResultInstructionOp1)){
				if(IS_INT(ptrResultInstructionOp2)){
					// determine object ...
					CVector * vec = (CVector *)(ptrResultInstructionOp1->stkObject);//[stkInteger[stkResultInstruction[index_op2].index]];
					int v_index = LOAD_INT_OP(ptrResultInstructionOp2);

					print_info_cr("%i",v_index);

					// check indexes ...
					if(v_index < 0 || v_index >= (int)vec->m_value.size()){
						print_error_cr("Line %i. Index vector out of bounds!",instruction->ast_node->definedValueline);
						return false;
					}

					if(!pushVar(vec->m_value[v_index],&vec->m_value[v_index])){
						return false;
					}

				}else{
					print_error_cr("Expected vector-index as integer");
					return false;
				}
			}
			else{
				print_error_cr("Expected operand 1 as vector");
				return false;
			}

			break;
		case VPUSH: // Value push for vector
			if(IS_VECTOR(ptrResultInstructionOp1)){
				CVector * vec = (CVector *)(ptrResultInstructionOp1->stkObject);
				vec->m_value.push_back(createVarFromResultInstruction(ptrResultInstructionOp2));
			}else{
				print_error_cr("Expected operand 1 as vector");
				return false;
			}
			break;
		case VEC: // Create new vector object...
			pushVar(CScriptClassFactory::getInstance()->newClassByIdx(CScriptClassFactory::getInstance()->getIdxClassVector()));
			break;

		case RET:

			/*if(!assignObjectFromIndex(function_object->getReturnObjectPtr(),instruction->index_op1)){
				return false;
			}*/

			break;

	}

	idxStkCurrentResultInstruction++;

	return true;
}





CVirtualMachine::~CVirtualMachine(){
	reset();
}
