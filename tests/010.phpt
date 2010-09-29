--TEST--
Array of words to replace
--SKIPIF--
<?php if (!extension_loaded("boxwood")) print "skip"; ?>
--FILE--
<?php 
$r = boxwood_new();
boxwood_add_text($r, "mon");
boxwood_add_text($r, "monkey");
boxwood_add_text($r, "monotreme");
$a = array('a','mont','monkeys','mono','eat','monotremes');
$c = boxwood_replace_text($r, $a,'*');
print implode('-', $a) . "\n";
print implode('-', $c) . "\n";
--EXPECT--
a-mont-monkeys-mono-eat-monotremes
a-m**t-m*****s-m**o-eat-m********s