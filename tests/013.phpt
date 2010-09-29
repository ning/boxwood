--TEST--
Multibyte word boundary tests 
--SKIPIF--
<?php if (!extension_loaded("boxwood")) echo "skip"; ?>
--FILE--
<?php 
$r = boxwood_new();
boxwood_add_text($r, "ɹǝqɯǝɯ");

$tests = array(
    'ǝɔuǝıɹǝdxǝ ɹǝqɯǝɯ ɐƃǝɯo',
    'ǝɔuǝıɹǝdxǝɹǝqɯǝɯ ɐƃǝɯo',
    'ǝɔuǝıɹǝdxǝ ɹǝqɯǝɯɐƃǝɯo',
    'ǝɔuǝıɹǝdxǝɹǝqɯǝɯɐƃǝɯo',
    'ǝɔuǝıɹǝdxǝ(ɹǝqɯǝɯ)ɐƃǝɯo',
    'ɹǝqɯǝɯ ɐƃǝɯo',
    'ǝɔuǝıɹǝdxǝ ɹǝqɯǝɯ',
);

foreach ($tests as $test) {
    echo boxwood_replace_text($r, $test, '*', false), "\n";
    echo boxwood_replace_text($r, $test, '*', true), "\n";
    echo "\n";
}
?>
--EXPECT--
ǝɔuǝıɹǝdxǝ ɹ***** ɐƃǝɯo
ǝɔuǝıɹǝdxǝ ɹ***** ɐƃǝɯo

ǝɔuǝıɹǝdxǝɹ***** ɐƃǝɯo
ǝɔuǝıɹǝdxǝɹǝqɯǝɯ ɐƃǝɯo

ǝɔuǝıɹǝdxǝ ɹ*****ɐƃǝɯo
ǝɔuǝıɹǝdxǝ ɹǝqɯǝɯɐƃǝɯo

ǝɔuǝıɹǝdxǝɹ*****ɐƃǝɯo
ǝɔuǝıɹǝdxǝɹǝqɯǝɯɐƃǝɯo

ǝɔuǝıɹǝdxǝ(ɹ*****)ɐƃǝɯo
ǝɔuǝıɹǝdxǝ(ɹ*****)ɐƃǝɯo

ɹ***** ɐƃǝɯo
ɹ***** ɐƃǝɯo

ǝɔuǝıɹǝdxǝ ɹ*****
ǝɔuǝıɹǝdxǝ ɹ*****

