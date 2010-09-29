--TEST--
Word boundary tests with alternate boundary set
--SKIPIF--
<?php if (!extension_loaded("boxwood")) echo "skip"; ?>
--FILE--
<?php 
$def = boxwood_new();
$alt = boxwood_new();
boxwood_set_word_boundary_bytes($alt, "()");
boxwood_add_text($def, "xxx");
boxwood_add_text($alt, "xxx");

$tests = array(
    'aaaxxx',
    'aaa xxx bbb',
    'aaa(xxx)bbb',
    'xxx',
    'aaa-xxx-bbb',
);

foreach ($tests as $test) {
    echo boxwood_replace_text($def, $test, '*', false), "\n";
    echo boxwood_replace_text($def, $test, '*', true), "\n";
    echo boxwood_replace_text($alt, $test, '*', false), "\n";
    echo boxwood_replace_text($alt, $test, '*', true), "\n";
    echo "\n";
}
?>
--EXPECT--
aaax**
aaaxxx
aaax**
aaaxxx

aaa x** bbb
aaa x** bbb
aaa x** bbb
aaa xxx bbb

aaa(x**)bbb
aaa(x**)bbb
aaa(x**)bbb
aaa(x**)bbb

x**
x**
x**
x**

aaa-x**-bbb
aaa-x**-bbb
aaa-x**-bbb
aaa-xxx-bbb
