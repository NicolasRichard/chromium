description("Tests that watchPosition correctly reports position updates and errors from the Geolocation service.");

var mockLatitude = 51.478;
var mockLongitude = -0.166;
var mockAccuracy = 100.0;

var mockMessage = 'test';

var position;
var error;

function checkPosition(p) {
    position = p;
    shouldBe('position.coords.latitude', 'mockLatitude');
    shouldBe('position.coords.longitude', 'mockLongitude');
    shouldBe('position.coords.accuracy', 'mockAccuracy');
    debug('');
}

function checkError(e) {
    error = e;
    shouldBe('error.code', 'error.POSITION_UNAVAILABLE');
    shouldBe('error.message', 'mockMessage');
    debug('');
}

if (!window.testRunner || !window.internals)
    debug('This test can not run without testRunner or internals');

internals.setGeolocationClientMock(document);
internals.setGeolocationPermission(document, true);
internals.setGeolocationPosition(document, mockLatitude, mockLongitude, mockAccuracy);

var state = 0;
navigator.geolocation.watchPosition(function(p) {
    switch (state++) {
        case 0:
            checkPosition(p);
            internals.setGeolocationPosition(document, ++mockLatitude, ++mockLongitude, ++mockAccuracy);
            break;
        case 1:
            checkPosition(p);
            internals.setGeolocationPositionUnavailableError(document, mockMessage);
            break;
        case 3:
            checkPosition(p);
            finishJSTest();
            break;
        default:
            testFailed('Success callback invoked unexpectedly');
            finishJSTest();
    }
}, function(e) {
    switch (state++) {
        case 2:
            checkError(e);
            internals.setGeolocationPosition(document, ++mockLatitude, ++mockLongitude, ++mockAccuracy);
            break;
        default:
            testFailed('Error callback invoked unexpectedly');
            finishJSTest();
    }
});

window.jsTestIsAsync = true;
