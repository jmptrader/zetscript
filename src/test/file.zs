

print("hola2_merda"+0);


/*
function bb(a,b){
	return a+b;
}

*/

//-----------------------------------------------------------------------------

/*

var c=0;
c=c-1;

c++;

c=[0,1];
c[0]=0;

c[0] = c[0]++ - --c[0]; // l-value dec/inc not allowed.


var d = ++[0,1,2,3][1] + ++[[0,4]][0][1]; // a= 2+5= 7


function(a,b){ return a+b;}(1,2) + function(n,m){ return 0;}(0,0); // ok functions, error c+d not declared.




function aaa(a,b){
	a=1; // incorrect not ref
	b=2; // correct (pass by ref)
}

//aaa(0); // warning (2nd parameter is not declared)
//aaa(0,1); // error. 2nd parameter must be a variable (ref)
//aaa(0,c); // error. 2nd parameter doesn't match vector with integer.

//-----------------------------------------------------------------------------




var a1 = function(a,b){ 
	return 5+6;
};

a1(0, ++c[0]);

var g1 = 0;

var d1=++g1;

//var d1=0+1+2+3;
//d1=-(--d1);



var cc=[ [2+3,2+5+6], [0+1] ];

var i = 0+12+3+4+5+6;

cc[ 0][0]+4+5;

cc[0][0]++;



//1*bb(0+1,0)+bb(0,1)+0;


var j=0;

function my_function(a,b){

	return a+b;

}


//print("result:"+my_function(1+0, 2*5*10));
//print("result:"+my_function("1+0", 2*5*10));



//((j+1)==0)?1+1:0+1;

if( (0+1) != (0+0) )
 {
	j=1;
}

*/
/*
function isprime(n)
{

	
	  for (var j = 2; (j < n); ++j)
	  {
	    if (n % j == 0) {return false;}
	  }	

	
 //var gg=0;
 
 //gg++;

 return true;
}


function primes(n)
{
  var count = 0;
  for (var i = 2; i <= n; ++i)
  {
	 // isprime(i);
	  if (isprime(i)) {++count;}
	  //if(true) {++count;}
	  var found = true;
	  for (var j = 2; (j < i) && !found; ++j)
	  {
	    if (i % j == 0) {found=true;}
	  }
	  
	  if(found){
		  count++;
	  }
  }

  return count;
}


var N = 5000;

primes(N);*/
//print("primes: " + primes(N).to_string())





/*
var sum = 0.0;
for (var i = 1; i <= 100000; ++i) {
    if (i % 2 == 0) {
        sum = 1.0 / i;
    }
    else {
        sum = sum + (1.0 / ((i) * i));
    }
}
*/
/*
function fact(n){
	if( n == 0) {
		return 1;
	}
	else{
		return n * fact(n-1);
	}
}

fact(1000);
*/
/*

var kk=0+1;

switch(0){
case 111:
case 200:
default:
kk=11;
break;
case -1:
break;
case 0:
case 4:
 kk=9;
 break;
case 1:
kk=10;
break;


}




for(var i = 0; i < 10; i++){
	if(i==0){
		2*i;
	}else{
		3*i;
	}
}



// var j="hola que hace"+0+0+1;




var h,r,g,e=0,q,tt,t=1; // multivar declaration


var vv= 1+function(){

return 1;

}();



var jj=1+[[1,2], [0,1] ][0][0];



//var a = fun(1,2);



var fun = function (l,d5,k){
	
	var b=0;
	
	function hola2(k1,l1){
		var ff=0;
	}
	
	// hola2(1+1,2+2,3+3); // invalid: hola2 only takes 2 parameters
	
	return b;
};



var i=3+( fun(1
+
2
+
3
*7
,0,0)+2  * 5 + ( 7
+
6)); // ok but j & k are not defined.



var array=[0,1,1];

array[0]++;


var f=0;

while(f < 10){

++f;

}

if(f == 10 ) {
	f++;
}else{
	f--;
}




//print("Hola:"+array[0]++);

for(var j2=0; j2 < 10 ; j2=j2+1){
	var ll=0;
	
	ll = ll + j2;
		
	
}


*/

/*
 
 var m,n;
 
 m=n=0; // multi var assigment.

// NOT IMPLEMENTED YET!

// conditional if...
var hh = 1 + 0==1 ? 2: 3*2 + 10 ? 0 : 1;


class my_class{

	var hh;
	var jj;
	
	function myclass(){
		this.jj=0;
		this;
		
		i=0;
	}
	
	function hola3(){
		this
		 .
		 
		 hh=0;
	}

	
	function hola2(){
		this.i = this.i + 1 + 0;
		hola2();
		this.hola3();
	}
	

};

new my_class();





// my derivated class gets all elements from my_class
class my_class_derivated:my_class{

	function my_extension_class(){
		this.jj = 0;
	}
};

*/
