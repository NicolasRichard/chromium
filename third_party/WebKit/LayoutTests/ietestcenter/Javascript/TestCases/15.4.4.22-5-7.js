/// Copyright (c) 2009 Microsoft Corporation 
/// 
/// Redistribution and use in source and binary forms, with or without modification, are permitted provided
/// that the following conditions are met: 
///    * Redistributions of source code must retain the above copyright notice, this list of conditions and
///      the following disclaimer. 
///    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and 
///      the following disclaimer in the documentation and/or other materials provided with the distribution.  
///    * Neither the name of Microsoft nor the names of its contributors may be used to
///      endorse or promote products derived from this software without specific prior written permission.
/// 
/// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
/// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
/// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
/// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
/// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
/// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
/// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
/// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 



ES5Harness.registerTest( {
id: "15.4.4.22-5-7",

path: "TestCases/chapter15/15.4/15.4.4/15.4.4.22/15.4.4.22-5-7.js",

description: "Array.prototype.reduceRight throws TypeError if 'length' is 0 (subclassed Array, length overridden with obj w/o valueOf (toString)), no initVal",

test: function testcase() {
  foo.prototype = new Array(1, 2, 3);
  function foo() {}
  var f = new foo();
  
  var o = { toString: function () { return '0';}};
  f.length = o;
  
  // objects inherit the default valueOf method of the Object object;
  // that simply returns the itself. Since the default valueOf() method
  // does not return a primitive value, ES next tries to convert the object
  // to a number by calling its toString() method and converting the
  // resulting string to a number.

  function cb(){}
  try {
    f.reduceRight(cb);
  }
  catch (e) {
    if (e instanceof TypeError) {
      return true;
    }
  }
 },

precondition: function prereq() {
  return fnExists(Array.prototype.reduceRight);
 }
});
