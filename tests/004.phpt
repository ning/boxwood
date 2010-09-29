--TEST--
Do replacement
--SKIPIF--
<?php if (!extension_loaded("boxwood")) print "skip"; ?>
--FILE--
<?php 
$r = boxwood_new();
$a = boxwood_add_text($r, "monkey");
$b = boxwood_add_text($r, "salad");
$c = boxwood_replace_text($r, "My monkey ate some salad today.","*");
print $c;
?>
--EXPECT--
My m***** ate some s**** today.