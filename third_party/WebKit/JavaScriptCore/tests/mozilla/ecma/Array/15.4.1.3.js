/* The contents of this file are subject to the Netscape Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 *
 * The Original Code is Mozilla Communicator client code, released March
 * 31, 1998.
 *
 * The Initial Developer of the Original Code is Netscape Communications
 * Corporation. Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation. All
 * Rights Reserved.
 *
 * Contributor(s): 
 * 
 */
/**
    File Name:          15.4.1.3.js
    ECMA Section:       15.4.1.3 Array()

    Description:        When Array is called as a function rather than as a
                        constructor, it creates and initializes a new array
                        object.  Thus, the function call Array(...) is
                        equivalent to the object creationi new Array(...) with
                        the same arguments.

                        An array is created and returned as if by the
                        expression new Array(len).

    Author:             christine@netscape.com
    Date:               7 october 1997
*/
    var SECTION = "15.4.1.3";
    var VERSION = "ECMA_1";
    startTest();
    var TITLE   = "Array Constructor Called as a Function:  Array()";

    writeHeaderToLog( SECTION + " "+ TITLE);

    var testcases = getTestCases();
    test();


function getTestCases() {
    var array = new Array();
    var item = 0;

    array[item++] = new TestCase(   SECTION,
                                    "typeof Array()",
                                    "object",
                                    typeof Array() );

    array[item++] = new TestCase(   SECTION,
                                    "MYARR = new Array();MYARR.getClass = Object.prototype.toString;MYARR.getClass()",
                                    "[object Array]",
                                    eval("MYARR = Array();MYARR.getClass = Object.prototype.toString;MYARR.getClass()") );

    array[item++] = new TestCase(   SECTION,
                                    "(Array()).length",
                                    0,          (
                                    Array()).length );

    array[item++] = new TestCase(   SECTION,
                                    "Array().toString()",
                                    "",
                                    Array().toString() );


    return ( array );
}
function test() {
    for ( tc=0; tc < testcases.length; tc++ ) {
        testcases[tc].passed = writeTestCaseResult(
                            testcases[tc].expect,
                            testcases[tc].actual,
                            testcases[tc].description +" = "+ testcases[tc].actual );
        testcases[tc].reason += ( testcases[tc].passed ) ? "" : "wrong value ";
    }
    stopTest();
    return ( testcases );
}
