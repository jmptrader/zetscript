/*
 *  This file is distributed under the MIT License.
 *  See LICENSE file for details.
 */
#include "CZetScript.h"

using namespace zetscript;

void say_helloworld(){
	printf("Hello World!");
}

int main(){

	int 	int_var = 10;
	float 	float_var = 0.5;
	bool 	bool_var = true;
	string	string_var = "in c++";

	CZetScript *zs = CZetScript::getInstance(); // instance zetscript

	register_C_Variable("int_var",int_var); // it takes int *
	register_C_Variable("float_var",float_var); // it takes float *
	register_C_Variable("bool_var",bool_var); // it takes bool *
	register_C_Variable("string_var",string_var); // it takes string *

	zs->eval(
		"int_var+=5;"
		"float_var+=5;"
		"bool_var=!bool_var;"
		"string_var+=\" and in script\";"
		"print(\"int_var:\"+int_var);"       // prints "int_var:0"
		"print(\"float_var:\"+float_var);"   // prints "float_var:5.500000"
		"print(\"bool_var:\"+bool_var);"     // prints "bool_var:false"
		"print(\"string_var:\"+string_var);" // prints "string_var:in c++ and in script"
	);

	return 0;
}
