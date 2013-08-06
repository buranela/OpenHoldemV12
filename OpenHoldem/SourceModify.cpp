//1. delete or rename stdafx.h, then compile. This will regenerate the stdafx.h compatible for VS2012
//2. poker_defs.h has an include definition which is not compatible with 2012 Commetn define inline on ln 29. Remember to compile Pokereval first to get the library
//3. change inline to __inline beacause its a C source at Pokereval folder for eval.h ln 72/73, eval_low.h ln 34/50
//4. project properties, framework and rerferences, add a reference poker_query_definitions dll project
// if step 4 does not work, compile pokertrackquerydef as library release

// change Reference keyboard DLL.dll to keyboard.dll and mouse dll.dll to mouse.dll as these are referenced as dlls for this project

//renamed release/1. openholdem.lib to openholdem.lib
//copy OpenHoldem.dll from release folder to debug folder. This is not produced because this project is release optimized
#include "stdafx.h"
#include <iostream>

void help(){

}