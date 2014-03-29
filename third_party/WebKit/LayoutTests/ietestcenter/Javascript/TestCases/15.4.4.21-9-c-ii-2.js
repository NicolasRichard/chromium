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
id: "15.4.4.21-9-c-ii-2",

path: "TestCases/chapter15/15.4/15.4.4/15.4.4.21/15.4.4.21-9-c-ii-2.js",

description: "Array.prototype.reduce - callbackfn called with correct parameters (initialvalue passed)",

test: function testcase() { 
 
  var bParCorrect = false;
  function callbackfn(prevVal, curVal, idx, obj)
  {
    if(idx === 0 && obj[idx] === curVal && prevVal === initialValue)
      return curVal;
    else if(idx > 0 && obj[idx] === curVal && obj[idx-1] === prevVal)
      return curVal;
    else
      return false;
  }

  var arr = [0,1,true,null,new Object(),"five"];
  var initialValue = 5.5;
  if( arr.reduce(callbackfn,initialValue) === "five") 
    return true;
 },

precondition: function prereq() {
  return fnExists(Array.prototype.reduce);
 }
});
