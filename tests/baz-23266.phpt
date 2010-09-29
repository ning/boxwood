--TEST--
Multiple multibyte replacements
--SKIPIF--
<?php if (!extension_loaded("boxwood")) print "skip"; ?>
--FILE--
<?php 
$korean = '유니코드 국제 회의가 1997년';
$words = preg_split('!\s+!u', $korean, -1, PREG_SPLIT_NO_EMPTY);
$r = boxwood_new();
boxwood_add_text($r, $words[0]);
boxwood_add_text($r, $words[1]);
boxwood_add_text($r, $words[2]);
boxwood_add_text($r, $words[3]);
echo boxwood_replace_text($r, 'hello ' . $korean . 'world', '*');
--EXPECT--
hello 유*** 국* 회** 1****world