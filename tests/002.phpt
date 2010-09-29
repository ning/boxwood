--TEST--
Create a resource
--SKIPIF--
<?php if (!extension_loaded("boxwood")) print "skip"; ?>
--FILE--
<?php 
$r = boxwood_new();
var_dump($r);
?>
--EXPECTF--
resource(%d) of type (Boxwood)
