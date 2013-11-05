if (this.importScripts) {
    importScripts('../resources/fs-worker-common.js');
    importScripts('../../../resources/js-test.js');
    importScripts('../resources/fs-test-util.js');
}

description("DirectoryEntry.getMetadata required arguments test.");

var fileSystem = null;

function errorCallback(error) {
    debug("Error occured while requesting a TEMPORARY file system:" + error.name);
    finishJSTest();
}

function successCallback(fs) {
    fileSystem = fs;
    debug("Successfully obtained TEMPORARY FileSystem:" + fileSystem.name);
    root = evalAndLog("root = fileSystem.root");
    shouldThrow("root.getMetadata()");
    finishJSTest();
}

var jsTestIsAsync = true;
evalAndLog("webkitRequestFileSystem(TEMPORARY, 100, successCallback, errorCallback);");
var successfullyParsed = true;
