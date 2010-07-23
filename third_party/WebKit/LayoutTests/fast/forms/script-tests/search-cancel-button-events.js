description('Test for event dipatching by search cancel button.');

var parent = document.createElement('div');
document.body.appendChild(parent);
parent.innerHTML = '<input type=search id=search value=foo><input id=another>';
var search = document.getElementById('search');
var anotherInput = document.getElementById('another');
var inputEventCounter = 0;
var changeEventCounter = 0;

search.onchange = function() { changeEventCounter++; };
search.oninput = function() { inputEventCounter++; };

if (window.eventSender) {
    debug('Initial state');
    shouldBe('changeEventCounter', '0');
    shouldBe('inputEventCounter', '0');

    debug('Click the cancel button');
    eventSender.mouseMoveTo(search.offsetLeft + search.offsetWidth - 8, search.offsetTop + search.offsetHeight / 2);
    eventSender.mouseDown();
    eventSender.mouseUp();
    shouldBe('search.value', '""');
    shouldBe('changeEventCounter', '0');
    shouldBe('inputEventCounter', '1');

    debug('Focus on another field');
    anotherInput.focus();
    shouldBe('changeEventCounter', '1');
    shouldBe('inputEventCounter', '1');

    parent.innerHTML = '';
} else {
  document.getElementById('console').innerHTML = 'No eventSender';
}

var successfullyParsed = true;
