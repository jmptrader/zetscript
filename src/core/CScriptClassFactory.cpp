#include "zg_core.h"

 CScriptClassFactory *  	 CScriptClassFactory::scriptClassFactory=NULL;
 CScriptClassFactory::tPrimitiveType CScriptClassFactory::primitiveType[MAX_VAR_C_TYPES];


 //--obj , type convert, ---
 map<string,map<string,fntConversionType>> mapTypeConversion;

 template<typename _S, typename _D, typename _F>
 bool addTypeConversion(_F f){

 	bool valid_type = false;

 	// check if any entry is int, *float, *bool , *string, *int or any from factory. Anyelese will be no allowed!
 	valid_type|=CScriptClassFactory::primitiveType[CScriptClassFactory::VOID_TYPE].type_str==string(typeid(_D).name()); ;//={typeid(void).name(),"void",VOID_TYPE};
 	valid_type|=CScriptClassFactory::primitiveType[CScriptClassFactory::INT_TYPE].type_str==string(typeid(_D).name()); ;//={typeid(int).name(),"int",INT_TYPE};
 	valid_type|=CScriptClassFactory::primitiveType[CScriptClassFactory::INT_PTR_TYPE].type_str==string(typeid(_D).name()); ;//={typeid(int *).name(),"int *",INT_PTR_TYPE};
 	valid_type|=CScriptClassFactory::primitiveType[CScriptClassFactory::FLOAT_PTR_TYPE].type_str==string(typeid(_D).name()); ;//={typeid(float *).name(),"float *",FLOAT_PTR_TYPE};
 	valid_type|=CScriptClassFactory::primitiveType[CScriptClassFactory::STRING_PTR_TYPE].type_str==string(typeid(_D).name()); ;//={typeid(string *).name(),"string *",STRING_PTR_TYPE};
 	valid_type|=CScriptClassFactory::primitiveType[CScriptClassFactory::BOOL_PTR_TYPE].type_str==string(typeid(_D).name()); ;//={typeid(bool *).name(),"bool *",BOOL_PTR_TYPE};

 	if(!valid_type){
 		print_error_cr("Conversion type \"%s\" not valid",typeid(_D).name());
 		return false;
 	}



 	if(mapTypeConversion.count(typeid(_S).name()) == 1){ // create new map...
 		if(mapTypeConversion[typeid(_S).name()].count(typeid(_D).name())==1){
 			print_error_cr("type conversion \"%s\" to \"%s\" already inserted",typeid(_S).name(),typeid(_D).name());
 			return false;
 		}
 	}

 	mapTypeConversion[typeid(_S).name()][typeid(_D).name()]=f;

 	return true;
 	//typeConversion["P7CNumber"]["Ss"](&n);
 }



 fntConversionType CScriptClassFactory::getConversionType(string objectType, string conversionType){

 	if(mapTypeConversion.count(objectType) == 0){
 		print_error_cr("There's no type conversion \"%s\". Add conversion types through \"addTypeConversion\" function",objectType.c_str());
 		return NULL;
 	}

 	if(mapTypeConversion[objectType].count(conversionType) == 0){
 		print_error("There's no CONVERSION from type \"%s\" to type \"%s\"",objectType.c_str(),conversionType.c_str());
 		printf("\n\tAvailable types are:");
 		for(map<string, fntConversionType>::iterator j =mapTypeConversion[objectType].begin() ; j != mapTypeConversion[objectType].end();j++){
 			printf("\n\t\t* \"%s\"", j->first.c_str());
 		}

 		return NULL;
 	}

 	return mapTypeConversion[objectType][conversionType];


 }

 void CScriptClassFactory::registerPrimitiveTypes(){


 }


 CScriptClassFactory*  CScriptClassFactory::getInstance(){

	 if(scriptClassFactory == NULL){
		 scriptClassFactory = new CScriptClassFactory();
	 }

	 return scriptClassFactory;

 }

 void CScriptClassFactory::destroySingletons() {
	 if(scriptClassFactory != NULL){
		 delete scriptClassFactory;
	 }

	 scriptClassFactory=NULL;

 }


 void  print(string * s){
 	print_info_cr("ADADADADADADADA:%s",s->c_str());
 }


 int c_var=0;


 bool CScriptClassFactory::registerBase(){


	 	primitiveType[VOID_TYPE]={typeid(void).name(),"void",VOID_TYPE};
	 	primitiveType[INT_TYPE]={typeid(int).name(),"int",INT_TYPE};
	 	primitiveType[INT_PTR_TYPE]={typeid(int *).name(),"int *",INT_PTR_TYPE};
	 	primitiveType[FLOAT_TYPE]={typeid(float).name(),"float",FLOAT_TYPE};
	 	primitiveType[FLOAT_PTR_TYPE]={typeid(float *).name(),"float *",FLOAT_PTR_TYPE};
	 	primitiveType[STRING_TYPE]={typeid(string).name(),"string",STRING_TYPE};
	 	primitiveType[STRING_PTR_TYPE]={typeid(string *).name(),"string *",STRING_PTR_TYPE};
	 	primitiveType[BOOL_TYPE]={typeid(bool).name(),"bool",BOOL_TYPE};
	 	primitiveType[BOOL_PTR_TYPE]={typeid(bool *).name(),"bool *",BOOL_PTR_TYPE};


		//===> MOVE INTO CScriptClassFactory !!!! ====================================================>

		//-----------------------
		// Conversion from object types to primitive types (move into factory) ...
		addTypeConversion<CInteger *,int>( [] (CScriptVariable *obj){return ((CInteger *)obj)->m_value;});
		addTypeConversion<CInteger *,int *>( [] (CScriptVariable *obj){return (int)&((CInteger *)obj)->m_value;});
		addTypeConversion<CInteger *,string *>( [] (CScriptVariable *obj){obj->m_strValue=CStringUtils::intToString(((CInteger*)obj)->m_value);return (int)&obj->m_strValue;});

		addTypeConversion<CNumber *,float *>( [] (CScriptVariable *obj){return (int)(&((CNumber *)obj)->m_value);});
		addTypeConversion<CNumber *,int>( [] (CScriptVariable *obj){return (int)((CNumber *)obj)->m_value;});
		addTypeConversion<CNumber *,string *>( [] (CScriptVariable *obj){obj->toString();return (int)&obj->m_strValue;});

		addTypeConversion<CBoolean *,bool *>( [] (CScriptVariable *obj){return (int)&((CBoolean *)obj)->m_value;});
		addTypeConversion<CBoolean *,string *>( [] (CScriptVariable *obj){obj->toString();return (int)&obj->m_strValue;});

		addTypeConversion<CString *,string *>( [] (CScriptVariable *obj){return (int)&(((CString *)obj)->m_value);});


		//------------------------------------------------------------------------------------------------------------------------------------------------------------------
		// MAIN CLASS! Is the first entry before anyother
		if((registerScriptClass(MAIN_SCRIPT_CLASS_NAME,"",NULL)) == NULL) return false;
		if((registerFunctionSymbol(MAIN_SCRIPT_CLASS_NAME,MAIN_SCRIPT_FUNCTION_NAME,CAst::getInstance()->getMainAstNode())) == NULL) return false;

		// register c function's
		if(!register_C_Function(print)) return false;
		if(!register_C_Variable(c_var)) return false;

		//------------------------------------------------------------------------------------------------------------------------------------------------------------------


		// register internal classes ...
		if(!register_C_Class<CVoid>("CVoid")) return false;
		if(!register_C_Class<CUndefined>("CUndefined")) return false;
		if(!register_C_Class<CInteger>("CInteger")) return false;
		if(!register_C_Class<CNumber>("CNumber")) return false;
		if(!register_C_Class<CBoolean>("CBoolean")) return false;
		if(!register_C_Class<CString>("CString")) return false;
		if(!register_C_Class<CVector>("CVector")) return false;
		if(!register_C_Class<CFunctor>("CFunctor")) return false;


		if((idxClassVoid = getIdxRegisteredClass("CVoid"))==-1) return false;
		if((idxClassUndefined = getIdxRegisteredClass("CUndefined"))==-1) return false;
		if((idxClassInteger = getIdxRegisteredClass("CInteger"))==-1) return false;
		if((idxClassNumber = getIdxRegisteredClass("CNumber"))==-1) return false;
		if((idxClassString = getIdxRegisteredClass("CBoolean"))==-1) return false;
		if((idxClassVector = getIdxRegisteredClass("CVector"))==-1) return false;
		if((idxClassString = getIdxRegisteredClass("CString"))==-1) return false;
		if((idxClassFunctor = getIdxRegisteredClass("CFunctor"))==-1) return false;


		// register custom functions ...
		if(!register_C_FunctionMember<CInteger>("toString",&CInteger::toString)) return false;
		//register_C_VariableMember<CInteger,CInteger::>("toString");

		return true;
 }


 CScriptClassFactory::CScriptClassFactory(){

	 idxClassInteger = idxClassNumber = idxClassString = idxClassBoolean = idxClassVector = idxClassFunctor = idxClassUndefined= idxClassVoid = -1;

	 if(!registerBase()){
			exit(EXIT_FAILURE);
	 }

 }



 CScriptClassFactory::tPrimitiveType *CScriptClassFactory::getPrimitiveTypeFromStr(const string & str){

 	for(unsigned i=0; i < MAX_VAR_C_TYPES; i++){
 		if(primitiveType[i].type_str == str){
 			return &primitiveType[i];
 		}
 	}

 	print_error_cr("type \"%s\" is not registered",str.c_str());

 	return NULL;
 }


bool CScriptClassFactory::searchVarFunctionSymbol(tScriptFunctionInfo *script_info, tInfoAsmOp *iao){

	int idx=0;
	string symbol_to_find = iao->ast_node->value_symbol;

	SCOPE_TYPE scope_type = iao->scope_type;


	if(scope_type == SCOPE_TYPE::UNKNOWN_SCOPE){ // try deduce local/global

		if(script_info == &m_registeredClass[0]->metadata_info.object_info.local_symbols.m_registeredFunction[0].object_info){
			scope_type = SCOPE_TYPE::GLOBAL_SCOPE;
		}
	}

	 if((idx=getIdxRegisteredFunctionSymbol(script_info,symbol_to_find,false))!=-1){
		 iao->scope_type = scope_type;
		 iao->index_op1 = LOAD_TYPE_FUNCTION;
		 iao->index_op2 = idx;
		 return true;
	 }else if((idx=getIdxRegisteredVariableSymbol(script_info,symbol_to_find, false))!=-1){
		 iao->scope_type = scope_type;
		 iao->index_op1 = LOAD_TYPE_VARIABLE;
		 iao->index_op2 = idx;
		 return true;
	 }

	 return false;
}






 bool CScriptClassFactory::updateReferenceSymbols(){



	 tInfoRegisteredFunctionSymbol  *main_function = &m_registeredClass[0]->metadata_info.object_info.local_symbols.m_registeredFunction[0];
	 print_info_cr("DEFINED CLASSES");
	 vector<tInfoRegisteredFunctionSymbol *>  mrf;
	 // For each class...
	 for(unsigned i = 0; i < m_registeredClass.size(); i++){

		 mrf.clear();
		 if(i==0){ // First entry (MAIN_CLASS), load global functions....
			 mrf.push_back(main_function);//->object_info.local_symbols.m_registeredFunction;
			 for(unsigned h=0; h<  main_function->object_info.local_symbols.m_registeredFunction.size(); h++){
				 mrf.push_back(&main_function->object_info.local_symbols.m_registeredFunction[h]);
			 }
		 }else{ // any other class
			 for(unsigned h=0; h<  m_registeredClass[i]->metadata_info.object_info.local_symbols.m_registeredFunction.size(); h++){
				 mrf.push_back(&m_registeredClass[i]->metadata_info.object_info.local_symbols.m_registeredFunction[h]);
			 }
		 }

		 // For each function ...
		 for(unsigned k=0; k < mrf.size();k++){

			 if(i==0){

				 if(k==0){
					 print_info_cr("Main Function");
				 }else{
					 print_info_cr("Global function %s...",mrf[k]->object_info.symbol_info.symbol_name.c_str());
				 }
			 }else{
				 print_info_cr("Function %s::%s...",m_registeredClass[i]->metadata_info.object_info.symbol_info.symbol_name.c_str(),mrf[k]->object_info.symbol_info.symbol_name.c_str());
			 }
			 vector<tInfoStatementOp> *stat = &mrf[k]->object_info.statment_op;
			 for(unsigned h=0; h < stat->size();h++){
				 for(unsigned idx_op=0; idx_op < stat->at(h).asm_op.size();idx_op++){
					 tInfoAsmOp *iao=stat->at(h).asm_op[idx_op];
					 if(iao->operator_type==ASM_OPERATOR::LOAD){

						 string base_class = m_registeredClass[i]->metadata_info.object_info.symbol_info.symbol_name;

						 tScriptFunctionInfo *sfi=NULL;

						 if(iao->scope_type == SCOPE_TYPE::ACCESS_SCOPE ){
							 iao->index_op1 = LOAD_TYPE_VARIABLE;
							 iao->index_op2 = idx_op-1;
						 }
						 else {

							 if(iao->scope_type == SCOPE_TYPE::THIS_SCOPE ){

								 sfi = &m_registeredClass[i]->metadata_info.object_info;
							 }


							 if(iao->index_op1 == LOAD_TYPE_NOT_DEFINED){

										 string symbol_to_find =iao->ast_node->value_symbol;

										 if(iao->scope_type ==SCOPE_TYPE::ACCESS_SCOPE){

											 print_info_cr("Symbol defined at line %i \"%s\" will solved at run-time",iao->ast_node->definedValueline, iao->ast_node->value_symbol.c_str());
										 }
										 else{

											// this required ...
											 if(sfi != NULL){
												 // search global...
												 if(!searchVarFunctionSymbol(sfi,iao)){
													 print_error_cr("Symbol defined at line %i \"%s::%s\"not found",iao->ast_node->definedValueline, base_class.c_str(),symbol_to_find.c_str());
													 return false;
												 }
											 }
											 else{
												 // search local...
												 if(!searchVarFunctionSymbol(&mrf[k]->object_info,iao)){

													 // search global...
													 if(!searchVarFunctionSymbol(&main_function->object_info,iao)){
														 print_error_cr("Symbol defined at line %i \"%s\"not found",iao->ast_node->definedValueline, symbol_to_find.c_str());
														 return false;
													 }
												 }
											 }
									 }
							 }
					 	 }

					 }else  if(iao->operator_type==ASM_OPERATOR::CALL){ // overrides variable type as function ...
						 // check whether access scope ...
						 if(stat->at(h).asm_op[iao->index_op1]->operator_type ==ASM_OPERATOR::LOAD  && // e
						    stat->at(h).asm_op[iao->index_op1]->scope_type == SCOPE_TYPE::ACCESS_SCOPE){

							 stat->at(h).asm_op[iao->index_op1]->index_op1 = LOAD_TYPE_FUNCTION;
							 stat->at(h).asm_op[iao->index_op1]->index_op2 = iao->index_op1-1;
							 iao->asm_properties = ASM_PROPERTY_CALLING_OBJECT;
						 }

					 }
				 }

			 }
		 }
	 }

	 return true;


}

 //----------------------------------------------------------------------------------------------------------------------------------------------------------------
 // PRINT ASM INFO

 const char * CScriptClassFactory::getStrMovVar(tInfoAsmOp * iao){

 	if(iao->operator_type != MOV){
 		return "ERROR";
 	}

 	string value_symbol="Unknown";

 	if(iao->ast_node != NULL){
 		value_symbol = iao->ast_node->value_symbol;
 	}

 	sprintf(print_aux_load_value,"VAR(%s)",value_symbol.c_str());

 	return print_aux_load_value;
 }

 const char * CScriptClassFactory::getStrTypeLoadValue(vector<tInfoStatementOp> * m_listStatements,int current_statment, int current_instruction){


 	tInfoAsmOp * iao =(*m_listStatements)[current_statment].asm_op[current_instruction];
 	int dddsi=0;


 	if(iao->operator_type != LOAD){
 		return "ERROR";
 	}


 	string value_symbol="Unknown";

 	if(iao->ast_node != NULL){
 		value_symbol = iao->ast_node->value_symbol;
 	}

 	char object_access[512] = "";

 	sprintf(print_aux_load_value,"UNDEFINED");

	if(iao->scope_type == SCOPE_TYPE::ACCESS_SCOPE){
		sprintf(object_access,"[%02i:%02i].",current_statment,iao->index_op2);
	}
	else if(iao->scope_type == SCOPE_TYPE::THIS_SCOPE){
		sprintf(object_access,"this.");
	}

 	/*if(iao->scope_type == SCOPE_TYPE::ACCESS_SCOPE) {
 		sprintf(print_aux_load_value,"[%02i:%02i].%s (solved at run-time)",current_statment,current_instruction-1,value_symbol.c_str());
 	}
 	else*/
 	{

 		switch(iao->index_op1){

		case LOAD_TYPE::LOAD_TYPE_CONSTANT:

			sprintf(print_aux_load_value,"CONST(%s)",value_symbol.c_str());
			break;

		case LOAD_TYPE::LOAD_TYPE_VARIABLE:



			sprintf(print_aux_load_value,"%sVAR(%s)",object_access,value_symbol.c_str());
			break;
		case LOAD_TYPE::LOAD_TYPE_FUNCTION:

			sprintf(print_aux_load_value,"%sFUN(%s)",object_access,value_symbol.c_str());
			break;

		case LOAD_TYPE::LOAD_TYPE_ARGUMENT:
			sprintf(print_aux_load_value,"ARG(%s)",value_symbol.c_str());
			break;
		default:

			break;

		}
 	}

 	return print_aux_load_value;
 }

 void CScriptClassFactory::printGeneratedCode_Recursive(tScriptFunctionInfo *fs){

 	vector<tInfoStatementOp> * m_listStatements = &fs->statment_op;
 	string pre="";
 	string post="";

 	for(unsigned s = 0; s < (*m_listStatements).size();s++){
 		vector<tInfoAsmOp *> * asm_op_statment = &(*m_listStatements)[s].asm_op;

 		//printf("\n[%s]\n\n","file.zs");

 		for(unsigned i = 0; i  <  asm_op_statment->size(); i++){

 			int n_ops=0;
 			int index_op1 = (*asm_op_statment)[i]->index_op1;
 			int index_op2 = (*asm_op_statment)[i]->index_op2;

 			if(index_op1 != -1)
 				n_ops++;

 			 if(index_op2 != -1)
 				 n_ops++;

 			 pre="";
 			 post="";

 				switch((*asm_op_statment)[i]->pre_post_operator_type){
 				case ASM_PRE_POST_OPERATORS::PRE_INC:
 					pre="++";
 					break;
 				case ASM_PRE_POST_OPERATORS::PRE_DEC:
 					pre="--";
 					break;
 				case ASM_PRE_POST_OPERATORS::POST_INC:
 					post="++";
 					break;
 				case ASM_PRE_POST_OPERATORS::POST_DEC:
 					post="--";
 					break;
 				default:
 					break;

 				}
 			switch((*asm_op_statment)[i]->operator_type){
 			case  NEW:
 				printf("[%02i:%02i]\t%s\t%s\n",s,i,CCompiler::def_operator[(*asm_op_statment)[i]->operator_type].op_str,CScriptClassFactory::getInstance()->getNameRegisteredClassByIdx((*asm_op_statment)[i]->index_op1));
 				break;
 			case  LOAD:
 				printf("[%02i:%02i]\t%s\t%s%s%s\n",s,i,
 						CCompiler::def_operator[(*asm_op_statment)[i]->operator_type].op_str,
 						pre.c_str(),
 						getStrTypeLoadValue(m_listStatements,s,i),
 						post.c_str());
 				break;
 			//case  MOV:
 			//	printf("[%02i:%02i]\t%s\t%s,[%02i:%02i]\n",s,i,def_operator[(*asm_op_statment)[i]->operator_type].op_str,getStrMovVar((*asm_op_statment)[i]),s,index_op2);
 			//	break;
 			case JNT:
 			case JT:
 			case JMP:
 				printf("[%02i:%02i]\t%s\t[%04i]\n",s,i,CCompiler::def_operator[(*asm_op_statment)[i]->operator_type].op_str,(*asm_op_statment)[i]->index_op1);
 				break;
 			/*case PRE_INC:
 			case POST_INC:
 			case PRE_DEC:
 			case POST_DEC:
 				printf("[%02i:%02i]\t%s\n",s,i,def_operator[(*asm_op_statment)[i]->operator_type].op_str);
 				break;*/
 			case VGET:
 			case VPUSH:
 				printf("[%02i:%02i]\t%s\t%s[%02i:%02i]%s,[%02i:%02i]\n",s,i,CCompiler::def_operator[(*asm_op_statment)[i]->operator_type].op_str,pre.c_str(),s,index_op1,post.c_str(),s,index_op2);
 				break;
 			default:

 				if(n_ops==0){
 					printf("[%02i:%02i]\t%s\n",s,i,CCompiler::def_operator[(*asm_op_statment)[i]->operator_type].op_str);
 				}else if(n_ops==1){
 					printf("[%02i:%02i]\t%s\t[%02i:%02i]\n",s,i,CCompiler::def_operator[(*asm_op_statment)[i]->operator_type].op_str,s,index_op1);
 				}else{
 					printf("[%02i:%02i]\t%s\t[%02i:%02i],[%02i:%02i]\n",s,i,CCompiler::def_operator[(*asm_op_statment)[i]->operator_type].op_str,s,index_op1,s,index_op2);
 				}
 				break;
 			}
 		}
 	}
 	// and then print its functions ...
 	vector<tInfoRegisteredFunctionSymbol> * m_vf = &fs->local_symbols.m_registeredFunction;

 	for(unsigned j =0; j < m_vf->size(); j++){

 		if(((*m_vf)[j].object_info.symbol_info.properties & PROPERTY_C_OBJECT_REF) != PROPERTY_C_OBJECT_REF){
 			char symbol_ref[1024*8]={0};

 			strcpy(symbol_ref,(*m_vf)[j].object_info.symbol_info.ast->value_symbol.c_str());

 			if(&m_registeredClass[0]->metadata_info.object_info == fs){ // main class (main entry)
 				sprintf(symbol_ref,"MAIN_ENTRY (MainClass)");
 			}
 			else if(&m_registeredClass[0]->metadata_info.object_info.local_symbols.m_registeredFunction[0].object_info!=fs){ // main function (main entry)
 				sprintf(symbol_ref,"%s::%s",fs->symbol_info.symbol_name.c_str(),(*m_vf)[j].object_info.symbol_info.ast->value_symbol.c_str());
 			}

 			printf("-------------------------------------------------------\n");
 			printf("Code for function \"%s\"\n",symbol_ref);
 			printGeneratedCode_Recursive(&m_vf->at(j).object_info);
 		}
 	}
 }

 void CScriptClassFactory::printGeneratedCode(tScriptFunctionInfo *fs){
 	printGeneratedCode_Recursive(fs);
 }

 void CScriptClassFactory::printGeneratedCodeAllClasses(){

	 // for all classes print code...
	 for(unsigned i = 0; i < m_registeredClass.size(); i++){
		 printGeneratedCode(&m_registeredClass[i]->metadata_info.object_info);
	 }

 }

 // PRINT ASM INFO
 //----------------------------------------------------------------------------------------------------------------------------------------------------------------


 tInfoRegisteredClass * CScriptClassFactory::registerScriptClass(const string & class_name, const string & base_class_name, PASTNode _ast){
	int index;
	tInfoRegisteredClass *irv=NULL;

	if(class_name.empty()){
		print_error_cr("Class name empty");
		return NULL;
	}

	tInfoRegisteredClass *base_class=NULL;

	if(base_class_name != ""){
		if((base_class = this->getRegisteredClass(base_class_name)) == NULL){
			return NULL;
		}
	}


	if((index = getIdxRegisteredClass_Internal(class_name))==-1){ // check whether is local var registered scope ...


		irv = new tInfoRegisteredClass;
		irv->class_idx=m_registeredClass.size();
		irv->baseClass=base_class;
		irv->idx_constructor_function=-1;
		irv->metadata_info.object_info.symbol_info.symbol_name = class_name;
		irv->metadata_info.object_info.symbol_info.ast=_ast;
		m_registeredClass.push_back(irv);

		return irv;

	}else{
		print_error_cr("error class \"%s\" already registered at line %i!", class_name.c_str(), m_registeredClass[index]->metadata_info.object_info.symbol_info.ast->definedValueline);
	}

	return NULL;
}


 CScriptVariable 		 * CScriptClassFactory::newClass(const string & class_name){

	 CScriptVariable *class_object=NULL;

	 // 0. Search class info ...
	 tInfoRegisteredClass *rc = getRegisteredClass(class_name);

	 if(rc != NULL){

		 // 1. Create the object ...
		 class_object = new CScriptVariable(rc);


	 }

	 return class_object;

 }


 CScriptVariable 		 * CScriptClassFactory::newClassByIdx(unsigned idx){

	 CScriptVariable *class_object=NULL;

	 // 0. Search class info ...
	 tInfoRegisteredClass *rc = getRegisteredClassByIdx(idx);

	 if(rc != NULL){

		 // 1. Create the object ...
		 class_object = new CScriptVariable(rc);


	 }

	 return class_object;

 }

/**
 * Register C variable
 */
bool  CScriptClassFactory::register_C_VariableInt(const string & var_name,void * var_ptr, const string & var_type)
{
	//CScopeInfo *scope;
	tInfoRegisteredVariableSymbol irs;

	if(var_ptr==NULL){
		print_error_cr("cannot register var \"%s\" with NULL reference value", var_name.c_str());
		return false;
	}

	tInfoRegisteredFunctionSymbol * mainFunctionInfo = getRegisteredFunctionSymbol(MAIN_SCRIPT_CLASS_NAME,MAIN_SCRIPT_FUNCTION_NAME);
	//tInfoRegisteredClass *rc = getRegisteredClass(class_name);

	if(mainFunctionInfo == NULL){
		print_error_cr("main function is not created");
		return false;
	}



	// init struct...
	irs.ast = NULL;
	irs.class_info=NULL;
	irs.properties = ::PROPERTY_C_OBJECT_REF;
	irs.symbol_name = var_name;
	irs.ref_ptr=(int)var_ptr;

	if((irs.info_var_scope=CAst::getInstance()->getRootScopeInfo()->registerSymbol(var_name))==NULL){
		return false;
	}



	mainFunctionInfo->object_info.local_symbols.m_registeredVariable.push_back(irs);

	print_info_cr("Registered function name: %s",var_name);

	return true;

}




tInfoRegisteredVariableSymbol * CScriptClassFactory::registerVariableSymbol(const string & class_name,const string & var_name,PASTNode  ast){

	tInfoRegisteredClass *rc = getRegisteredClass(class_name);


	if(rc != NULL){

		tScriptFunctionInfo *object_info=&rc->metadata_info.object_info;

		tInfoRegisteredVariableSymbol info_var;
		info_var.class_info = rc;
		info_var.ast = ast;
		info_var.symbol_name =var_name;
		object_info->local_symbols.m_registeredVariable.push_back(info_var);

		return &object_info->local_symbols.m_registeredVariable[object_info->local_symbols.m_registeredVariable.size()-1];
	}else{
		print_error_cr("object_info null!");
		return NULL;
	}

	return NULL;
}

tInfoRegisteredVariableSymbol *  CScriptClassFactory::getRegisteredVariableSymbol(const string & class_name,const string & function_name){

	tInfoRegisteredClass *rc = getRegisteredClass(class_name);

	if(rc != NULL){

		tScriptFunctionInfo *object_info=&rc->metadata_info.object_info;

		for(unsigned i = 0; i < object_info->local_symbols.m_registeredVariable.size(); i++){
			if(object_info->local_symbols.m_registeredVariable[i].symbol_name == function_name){
				return &object_info->local_symbols.m_registeredVariable[i];
			}
		}
	/*if((index = getIdxRegisteredFunctionSymbol_Internal(class_name))!=-1){ // check whether is local var registered scope ...

		return m_registeredClass[index];
	}else{
		if(print_msg){
			print_error_cr("class \"%s\" doesn't exist!", class_name.c_str());
		}*/
	}

	print_error_cr("variable member %s::%s doesn't exist",class_name.c_str(),function_name.c_str());

	return NULL;
}

int CScriptClassFactory::getIdxRegisteredVariableSymbol(tScriptFunctionInfo *script_info ,const string & function_name, bool show_msg){

	for(unsigned i = 0; i < script_info->local_symbols.m_registeredVariable.size(); i++){
		if(script_info->local_symbols.m_registeredVariable[i].symbol_name == function_name){
			return i;
		}
	}

	if(show_msg){
		print_error_cr("variable member %s::%s doesn't exist",script_info->symbol_info.symbol_name.c_str(),function_name.c_str());
	}

	return -1;
}




//-------

tInfoRegisteredFunctionSymbol * CScriptClassFactory::registerFunctionSymbol(const string & class_name, const string & fun_name, PASTNode  ast){

	tInfoRegisteredClass *rc = getRegisteredClass(class_name);


	if(rc != NULL){
		if(getRegisteredFunctionSymbol(class_name,fun_name, false)==NULL){


			tScriptFunctionInfo *object_info=&rc->metadata_info.object_info;

			tInfoRegisteredFunctionSymbol irs;

			irs.object_info.symbol_info.class_info = rc;
			irs.object_info.symbol_info.symbol_name = fun_name;
			irs.object_info.symbol_info.ast = ast;
			irs.object_info.symbol_info.properties = 0;

			//rc->idx_constructor_function =-1; // by default has no constructor...


			if(fun_name == class_name){
				if(rc->idx_constructor_function == -1){
					rc->idx_constructor_function = object_info->local_symbols.m_registeredFunction.size();
				}else{
					print_error_cr("Constructor \"%s:%s\" already defined",fun_name.c_str(),fun_name.c_str());
					return NULL;
				}
			}



			object_info->local_symbols.m_registeredFunction.push_back(irs);

			return &object_info->local_symbols.m_registeredFunction[object_info->local_symbols.m_registeredFunction.size()-1];
		}
		else{
			print_error_cr("function member %s::%s already registered",class_name.c_str(),fun_name.c_str());
		}
	}else{
		//print_error_cr("object info NULL");
	}

	return NULL;
}


tInfoRegisteredFunctionSymbol * CScriptClassFactory::getRegisteredFunctionSymbol(const string & class_name,const string & function_name, bool show_errors){

	tInfoRegisteredClass *rc = getRegisteredClass(class_name);

	if(rc != NULL){

		tScriptFunctionInfo *object_info=&rc->metadata_info.object_info;

		for(unsigned i = 0; i < object_info->local_symbols.m_registeredFunction.size(); i++){
			if(object_info->local_symbols.m_registeredFunction[i].object_info.symbol_info.symbol_name == function_name){
				return &object_info->local_symbols.m_registeredFunction[i];
			}
		}
	/*if((index = getIdxRegisteredFunctionSymbol_Internal(class_name))!=-1){ // check whether is local var registered scope ...

		return m_registeredClass[index];
	}else{
		if(print_msg){
			print_error_cr("class \"%s\" doesn't exist!", class_name.c_str());
		}*/
	}

	if(show_errors){
		print_error_cr("function member %s::%s doesn't exist",class_name.c_str(),function_name.c_str());
	}

	return NULL;
}


int CScriptClassFactory::getIdxRegisteredFunctionSymbol(tScriptFunctionInfo *script_info,const string & function_name, bool show_msg){

	//tInfoRegisteredClass *rc = getRegisteredClass(class_name);

	//if(rc != NULL)
	//{

		//tScriptFunctionInfo *script_info=&rc->metadata_info.object_info;

		for(unsigned i = 0; i < script_info->local_symbols.m_registeredFunction.size(); i++){
			if(script_info->local_symbols.m_registeredFunction[i].object_info.symbol_info.symbol_name == function_name){
				return i;
			}
		}
	//}

	if(show_msg){
		print_error_cr("function member %s::%s doesn't exist",script_info->symbol_info.symbol_name.c_str(),function_name.c_str());
	}

	return -1;
}



//-----

bool CScriptClassFactory::addArgumentFunctionSymbol(const string & class_name,const string & function_name, const string & arg_name){


	tInfoRegisteredFunctionSymbol *object_info = getRegisteredFunctionSymbol(class_name,function_name);


	if(object_info!=NULL){

		object_info->m_arg.push_back(arg_name);

	}else{
		//print_error_cr("object info NULL");
	}

	return false;
}

int CScriptClassFactory::getIdxRegisteredClass_Internal(const string & class_name){

	for(unsigned i = 0; i < m_registeredClass.size(); i++){
		if(class_name == m_registeredClass[i]->metadata_info.object_info.symbol_info.symbol_name){
			return i;
		}
	}

	return -1;
}

tInfoRegisteredClass *CScriptClassFactory::getRegisteredClass(const string & class_name, bool print_msg){

	int index;
	if((index = getIdxRegisteredClass_Internal(class_name))!=-1){ // check whether is local var registered scope ...

		return m_registeredClass[index];
	}else{
		if(print_msg){
			print_error_cr("class \"%s\" doesn't exist!", class_name.c_str());
		}
	}

	return NULL;

}


tInfoRegisteredClass *CScriptClassFactory::getRegisteredClassByIdx(unsigned index){

	if(index > m_registeredClass.size()){
		print_error_cr("index out of bound");
	}

	return m_registeredClass[index];

}



int CScriptClassFactory::getIdxRegisteredClass(const string & v){
	int index= getIdxRegisteredClass_Internal(v);

	if(index == -1){
		print_error_cr("class %s not registered",v.c_str());
	}

	return index;
}

bool CScriptClassFactory::isClassRegistered(const string & v){
	return getIdxRegisteredClass_Internal(v) != -1;
}

const char * CScriptClassFactory::getNameRegisteredClassByIdx(int idx){
	if(idx != -1){
		return m_registeredClass[idx]->metadata_info.object_info.symbol_info.symbol_name.c_str();
	}
	 return "unknow";

}

CScriptClassFactory::~CScriptClassFactory() {

	for(unsigned i = 0;i<m_registeredClass.size();i++){

			tInfoRegisteredClass *irv = m_registeredClass[i];
		    for(unsigned i = 0; i < irv->metadata_info.object_info.local_symbols.m_registeredFunction.size();i++){
		    	for(unsigned j = 0; j < irv->metadata_info.object_info.local_symbols.m_registeredFunction[i].object_info.statment_op.size(); j++){
		    		for(unsigned a = 0; a  <irv->metadata_info.object_info.local_symbols.m_registeredFunction[i].object_info.statment_op[j].asm_op.size(); a++){

		    			delete irv->metadata_info.object_info.local_symbols.m_registeredFunction[i].object_info.statment_op[j].asm_op[a];
		    		}

		    	}
		    }


		    // delete tInfoRegisteredClass
			delete irv;
	}
}
