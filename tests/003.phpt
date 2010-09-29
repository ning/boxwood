--TEST--
Add a word
--SKIPIF--
<?php if (!extension_loaded("boxwood")) print "skip"; ?>
--FILE--
<?php 
$r = boxwood_new();
$a = boxwood_add_text($r, "monkey");
$b = boxwood_add_text($r, "salad");
print "$a,$b";
?>
--EXPECT--
6,5
