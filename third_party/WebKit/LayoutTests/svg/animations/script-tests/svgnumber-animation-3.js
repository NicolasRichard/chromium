description("Test for SVGNumber animation with invalid units.");
createSVGTestCase();

// Setup test document
var rect = createSVGElement("rect");
rect.setAttribute("id", "rect");
rect.setAttribute("x", "0");
rect.setAttribute("width", "100");
rect.setAttribute("height", "100");
rect.setAttribute("fill", "green");
rect.setAttribute("opacity", "0");
rect.setAttribute("onclick", "executeTest()");

var animate = createSVGElement("animate");
animate.setAttribute("id", "animation");
animate.setAttribute("attributeName", "opacity");
animate.setAttribute("begin", "click");
animate.setAttribute("dur", "4s");
animate.setAttribute("from", "0px");
animate.setAttribute("to", "1px");
rect.appendChild(animate);
rootSVGElement.appendChild(rect);

var computedStyle = rect.ownerDocument.defaultView.getComputedStyle(rect);

// Setup animation test
function sample1() {
    // Check initial/end conditions
    shouldBe("computedStyle.getPropertyCSSValue('opacity').getFloatValue(CSSPrimitiveValue.CSS_NUMBER)", "0");
}

function sample2() {
    shouldBe("computedStyle.getPropertyCSSValue('opacity').getFloatValue(CSSPrimitiveValue.CSS_NUMBER)", "0");
}

function sample3() {
    shouldBe("computedStyle.getPropertyCSSValue('opacity').getFloatValue(CSSPrimitiveValue.CSS_NUMBER)", "0");
}

function executeTest() {
    const expectedValues = [
        // [animationId, time, elementId, sampleCallback]
        ["animation", 0,      "rect", sample1],
        ["animation", 2.0,    "rect", sample2],
        ["animation", 3.9999, "rect", sample3],
        ["animation", 4,      "rect", sample1]
    ];

    runAnimationTest(expectedValues);
}

// Begin test async
window.setTimeout("triggerUpdate(50, 30)", 0);
var successfullyParsed = true;
