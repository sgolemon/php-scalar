--TEST--
Basic scalar type hints
--FILE--
<?php
function basic(bool $bool, int $iNum, float $fNum,
               string $str) {
  var_dump($bool, $iNum, $fNum, $str);
}

function numeric(num $num) {
  var_dump($num);
}

function scalar(scalar $scalar) {
  var_dump($scalar);
}

function obj(object $obj) {
  var_dump($obj);
}

function res(resource $res) {
  var_dump($res);
}

echo "--basic--\n";
basic(true, 17, 2.17, 'def');
basic(false, 42, 3.14, 'abc');
echo "--numeric--\n";
numeric(123);
numeric(456.789);
echo "--scalar--\n";
scalar(true);
scalar(123);
scalar(456.789);
scalar('abc');
echo "--obj--\n";
obj(new stdClass());
echo "--res--\n";
res(STDIN);
--EXPECTF--
--basic--
bool(true)
int(17)
float(2.17)
string(3) "def"
bool(false)
int(42)
float(3.14)
string(3) "abc"
--numeric--
int(123)
float(456.789)
--scalar--
bool(true)
int(123)
float(456.789)
string(3) "abc"
--obj--
object(stdClass)#%d (0) {
}
--res--
resource(%d) of type (stream)
