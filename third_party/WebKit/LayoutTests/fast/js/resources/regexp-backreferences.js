description("Test to ensure correct behaviour when using backreferences in a RegExp");

shouldBeTrue("/(...)\\1$/.test('abcabc')");
shouldBeFalse("/(...)\\1$/.test('abcdef')");
shouldBeFalse("/(...)\\2$/.test('abcabc')");
shouldBeFalse("/(...)\\2$/.test('abc')");
shouldBeTrue("/\\1(...)$/.test('abcabc')");
shouldBeTrue("/\\1(...)$/.test('abcdef')");
shouldBeFalse("/\\2(...)$/.test('abcabc')");
shouldBeFalse("/\\2(...)$/.test('abc')");
shouldBeTrue("/\\1?(...)$/.test('abc')");
shouldBeTrue("/\\1?(...)$/.test('abc')");

var successfullyParsed = true;
