description("Tests that when a request is made on a Geolocation object, permission is denied and its Frame is disconnected before a callback is made, no callbacks are made.");

// Prime the Geolocation instance by denying permission. This makes sure that we execute the
// same code path for both preemptive and non-preemtive permissions policies.
if (window.testRunner) {
    testRunner.setGeolocationPermission(false);
    testRunner.setMockGeolocationPosition(51.478, -0.166, 100);
} else
    debug('This test can not be run without the testRunner');

var error;
function onIframeLoaded() {
    iframeGeolocation = iframe.contentWindow.navigator.geolocation;
    iframeGeolocation.getCurrentPosition(function() {
        testFailed('Success callback invoked unexpectedly');
        finishJSTest();
    }, function(e) {
        error = e;
        shouldBe('error.code', 'error.PERMISSION_DENIED');
        shouldBe('error.message', '"User denied Geolocation"');
        debug('');
        iframe.src = 'data:text/html,This frame should be visible when the test completes';
    });
}

function onIframeUnloaded() {
    // Make another request, with permission already denied.
    iframeGeolocation.getCurrentPosition(function () {
        testFailed('Success callback invoked unexpectedly');
        finishJSTest();
    }, function(e) {
        testFailed('Error callback invoked unexpectedly');
        finishJSTest();
    });
    setTimeout(function() {
        testPassed('No callbacks invoked');
        finishJSTest();
    }, 100);
}

var iframe = document.createElement('iframe');
iframe.src = 'resources/disconnected-frame-inner.html';
document.body.appendChild(iframe);

window.jsTestIsAsync = true;
