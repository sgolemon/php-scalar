--TEST--
Basic scalar type hints
--SKIPIF--
<?php
if (!defined('E_RECOVERABLE_ERROR')) echo 'skip';
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

function catch_and_release($errno, $errstr) {
  echo "$errstr\n";
  return true;
}
set_error_handler('catch_and_release', E_RECOVERABLE_ERROR);

echo "--basic--\n";
basic(17, 2.17, 'def', true);
basic(42, 3.14, 'abc', false);
echo "--numeric--\n";
numeric(false);
numeric('ghi');
echo "--scalar--\n";
scalar(new stdClass);
scalar(STDIN);
echo "--obj--\n";
obj(true);
echo "--res--\n";
res(array());
--EXPECTF--
--basic--
Argument 1 passed to basic() must be an instance of bool, integer given, called in %s/tests/scalar-error.php on line %d and defined
Argument 2 passed to basic() must be an instance of int, double given, called in %s/tests/scalar-error.php on line %d and defined
Argument 3 passed to basic() must be an instance of float, string given, called in %s/tests/scalar-error.php on line %d and defined
Argument 4 passed to basic() must be an instance of string, boolean given, called in %s/tests/scalar-error.php on line %d and defined
int(17)
float(2.17)
string(3) "def"
bool(true)
Argument 1 passed to basic() must be an instance of bool, integer given, called in %s/tests/scalar-error.php on line %d and defined
Argument 2 passed to basic() must be an instance of int, double given, called in %s/tests/scalar-error.php on line %d and defined
Argument 3 passed to basic() must be an instance of float, string given, called in %s/tests/scalar-error.php on line %d and defined
Argument 4 passed to basic() must be an instance of string, boolean given, called in %s/tests/scalar-error.php on line %d and defined
int(42)
float(3.14)
string(3) "abc"
bool(false)
--numeric--
Argument 1 passed to numeric() must be an instance of num, boolean given, called in %s/tests/scalar-error.php on line %d and defined
bool(false)
Argument 1 passed to numeric() must be an instance of num, string given, called in %s/tests/scalar-error.php on line %d and defined
string(3) "ghi"
--scalar--
Argument 1 passed to scalar() must be an instance of scalar, instance of stdClass given, called in %s/tests/scalar-error.php on line %d and defined
object(stdClass)#1 (0) {
}
Argument 1 passed to scalar() must be an instance of scalar, resource given, called in %s/tests/scalar-error.php on line %d and defined
resource(1) of type (stream)
--obj--
Argument 1 passed to obj() must be an instance of object, boolean given, called in %s/tests/scalar-error.php on line %d and defined
bool(true)
--res--
Argument 1 passed to res() must be an instance of resource, array given, called in %s/tests/scalar-error.php on line %d and defined
array(0) {
}
