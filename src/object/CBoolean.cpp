#include "script/zg_script.h"

CBoolean * CBoolean::Parse(const string & s){



		if(CStringUtils::toLower(s)=="true"){
			CBoolean *b=new CBoolean();
			b->m_value=true;
			return b;
			
		}else if(CStringUtils::toLower(s)=="false"){
			CBoolean *b=new CBoolean();
			b->m_value=false;
			return b;
		}

		// TODO: develop exception handler.
		return NULL;
}

bool * CBoolean::ParsePrimitive(const string & s){

	if(CStringUtils::toLower(s)=="true"){
		bool *b=new bool;
		*b=true;
		return b;

	}else if(CStringUtils::toLower(s)=="false"){
		bool *b=new bool;
		*b=false;
		return b;
	}

	// TODO: develop exception handler.
	return NULL;
}


CBoolean::CBoolean(){

    m_classStr=typeid(CBoolean).name();
    m_pointerClassStr=typeid(CBoolean *).name();

	m_varType = CVariable::VAR_TYPE::BOOLEAN;
	m_value = false;
	m_ptr=&m_value;
}


string CBoolean::toString(){string s; return s;}
