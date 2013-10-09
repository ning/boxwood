--TEST--
Test EXISTS function
--SKIPIF--
<?php if (!extension_loaded("boxwood")) echo "skip"; ?>
--FILE--
<?php 
$r = boxwood_new();
boxwood_add_text($r, "airplane");
boxwood_add_text($r, "train");

$tests = array(
    'my train airplane norway',
    'my norway',
    'toast ting',
    'no way',
    'drive my norway',
    'fly airplane'
);

foreach ($tests as $test) {
    echo (boxwood_exists($r, $test) === true ? "yes" : "no"), "\n";
}
?>
--EXPECT--
yes
yes
no
no
yes
yes
