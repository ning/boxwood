--TEST--
More multiple multibyte replacements
--SKIPIF--
<?php if (!extension_loaded("boxwood")) print "skip"; ?>
--FILE--
<?php 
$spanish = "Inscríbase ahora para la Décima Conferencia";
$words = preg_split('!\s+!u', $spanish, -1, PREG_SPLIT_NO_EMPTY);
$r = boxwood_new();
foreach ($words as $word) {
    boxwood_add_text($r, $word);
}
echo boxwood_replace_text($r, 'hello ' . $spanish . 'world', '*');
--EXPECT--
hello I********* a**** p*** l* D***** C**********world
