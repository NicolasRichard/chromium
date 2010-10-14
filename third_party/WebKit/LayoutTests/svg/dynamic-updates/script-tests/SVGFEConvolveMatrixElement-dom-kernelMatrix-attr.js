// [Name] SVGFEConvolveMatrixElement-dom-kernelMatrix-attr.js
// [Expected rendering result] An image with feConvolveMatrix filter - and a series of PASS messages

description("Tests dynamic updates of the 'kernelMatrix' attribute of the SVGFEConvolveMatrixElement object")
createSVGTestCase();

var convolveMatrixElement = createSVGElement("feConvolveMatrix");
convolveMatrixElement.setAttribute("in", "SourceGraphic");
convolveMatrixElement.setAttribute("order", "3");
convolveMatrixElement.setAttribute("kernelMatrix", "-2 0 0 0 1 0 0 0 2");
convolveMatrixElement.setAttribute("preserveToAlpha", "true");

var filterElement = createSVGElement("filter");
filterElement.setAttribute("id", "myFilter");
filterElement.setAttribute("filterUnits", "userSpaceOnUse");
filterElement.setAttribute("x", "0");
filterElement.setAttribute("y", "0");
filterElement.setAttribute("width", "200");
filterElement.setAttribute("height", "200");
filterElement.appendChild(convolveMatrixElement);

var defsElement = createSVGElement("defs");
defsElement.appendChild(filterElement);

rootSVGElement.appendChild(defsElement);
rootSVGElement.setAttribute("width", "700");
rootSVGElement.setAttribute("height", "200");

var imageElement = createSVGElement("image");
imageElement.setAttributeNS(xlinkNS, "xlink:href", "../W3C-SVG-1.1/resources/convolveImage.png");
imageElement.setAttribute("x", "0");
imageElement.setAttribute("y", "0");
imageElement.setAttribute("width", "200");
imageElement.setAttribute("height", "200");
imageElement.setAttribute("filter", "url(#myFilter)");
rootSVGElement.appendChild(imageElement);

shouldBeEqualToString("convolveMatrixElement.getAttribute('kernelMatrix')", "-2 0 0 0 1 0 0 0 2");

function executeTest() {
    convolveMatrixElement.setAttribute("kernelMatrix", "3 0 3 0 0 0 3 0 3");
    shouldBeEqualToString("convolveMatrixElement.getAttribute('kernelMatrix')", "3 0 3 0 0 0 3 0 3");

    completeTest();
}

startTest(rootSVGElement, 100, 100);

var successfullyParsed = true;