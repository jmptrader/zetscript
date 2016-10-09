#include "core/zg_core.h"
/*
CNumber * CNumber::Parse(const string & s){
	char *p;bool ok=true;
	CNumber *n=NULL;
	
	double numberValue=0;
	if(s!="0") {// trivial case
		//print_info_cr("not %i %i! %s",s[0],s[1]);
	  numberValue=strtod((char *)s.c_str(),&p);
	  ok = *p == '\0';
	}

    if(ok){
		n=new CNumber();
		n->m_value = numberValue;
	}

	// TODO: develop exception handler.
	return n;
}*/

float *  CNumber::Parse(const string & s){
	char *p;bool ok=true;
	float *n=NULL;

	float numberValue=0;
	if(s!="0") {// trivial case
		//print_info_cr("not %i %i! %s",s[0],s[1]);
	  numberValue=strtof((char *)s.c_str(),&p);
	  ok = *p == '\0';
	}

    if(ok){
    	n=new float;
    	*n = numberValue;

	}

	// TODO: develop exception handler.
	return n;
}


CNumber::CNumber(tInfoRegisteredClass *info_registered_class):CScriptVariable(info_registered_class){

  //  m_classStr="number";
   // m_pointerClassStr="number";

	//m_varType = CVariable::VAR_TYPE::NUMBER;
	m_value=0;
	//m_ptr=&m_value;
}

