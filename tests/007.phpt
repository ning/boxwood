--TEST--
Replacement, UTF-8, case-sensitive
--SKIPIF--
<?php if (!extension_loaded("boxwood")) print "skip"; ?>
--FILE--
<?php 
$r = boxwood_new(true);
boxwood_add_text($r, "monkey");
boxwood_add_text($r, "salad");
boxwood_add_text($r, "ŚÄłäđ");
$c = boxwood_replace_text($r, "My monkey ate some salad today and also some ŚÄłäđ and śäłäđ.","^");
print $c;
?>
--EXPECT--
My m^^^^^ ate some s^^^^ today and also some Ś^^^^ and śäłäđ.