php-scalar
==========

Scalar type hinting for PHP

```php
function foo(int $bar) {
  echo "bar is $bar\n";
}
foo(123); // works
foo(4.56); // throws an error
```

Types:
  * `bool`
  * `int`
  * `float`
  * `num` (either int of float)
  * `string`
  * `scalar` (any of the above types)
  * `object` (of any class)
  * `resource`

Note that you cannot specify default values for scalar types because the parser doesn't support it.
This extension was a quick and dirty proof-of-concept.  A prper implementation will require altering
the parser which can't be done from an extension (without stupid amounts of work).

