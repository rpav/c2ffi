# c2ffi

This is a tool for extracting definitions from C headers for use with
foreign function call interfaces.  For instance:

```c
#define FOO (1 << 2)

const int BAR = FOO + 10;

typedef struct my_point {
    int x;
    int y;
    int odd_value[BAR + 1];
} my_point_t;

enum some_values {
    a_value,
    another_value,
    yet_another_value
};

void do_something(my_point_t *p, int x, int y);
```

Running `c2ffi` on this, we can get the following:

```lisp
(const BAR :int 14)

(struct my_point
    (x :int)
    (y :int)
    (odd_value (:array :int 15)))
(typedef my_point_t (:struct my_point))

(enum some_values
    (a_value 0)
    (another_value 1)
    (yet_another_value 2))

(function "do_something" ((p (:pointer my_point_t)) (x :int) (y :int)) :void)

(const FOO __int128_t 4)
```

Because this uses [Clang](http://clang.llvm.org/) as a parser, the C
is fully and correctly parsed, including complex array initializers
and similar.  For output, [JSON](http://json.org/) is the default, but
this is a bit less readable:

```json
[
{ "tag": "constant", "name": "BAR", "type": { "tag": ":int" },
"value": 14 },
{ "tag": "struct", "name": "my_point", "fields": [{ "tag": "field",
"name": "x", "type": { "tag": ":int" } }, { "tag": "field", "name":
"y", "type": { "tag": ":int" } }, { "tag": "field", "name":
"odd_value", "type": { "tag": ":array", "type": { "tag": ":int" },
"size": 15 } }] },
{ "tag": "typedef", "name": "my_point_t", "type": { "tag": ":struct",
"name": "my_point" } },
{ "tag": "enum", "name": "some_values", "fields": [{ "tag": "field",
"name": "a_value", "value": 0 }, { "tag": "field", "name":
"another_value", "value": 1 }, { "tag": "field", "name":
"yet_another_value", "value": 2 }] },
{ "tag": "function", "name": "do_something", "parameters": [{ "tag":
"parameter", "name": "p", "type": { "tag": ":pointer", "type": {
"tag": "my_point_t" } } }, { "tag": "parameter", "name": "x", "type":
{ "tag": ":int" } }, { "tag": "parameter", "name": "y", "type": {
"tag": ":int" } }], "return-type": { "tag": ":void" } }
]
```

## Building

This requires Clang 3.3, which you can [obtain from the
repository](http://clang.llvm.org/get_started.html).  Once that is
built, you should be able to build `c2ffi`:

```console
c2ffi/ $ mkdir build/ && cd build
build/ $ ../configure
   : # lots of output
build/ $ make
build/ $ ./src/c2ffi -h
Usage: c2ffi [options ...] FILE

Options:
      -I, --include        Add a "LOCAL" include path
      -i, --sys-include    Add a <system> include path
      -D, --driver         Specify an output driver (default: json)

      -o, --output         Specify an output file (default: stdout)
      -M, --macro-file     Specify a file for macro definition output

Drivers: json, sexp
```

Now you have a working `c2ffi`.

## Usage

There are generally two steps to using `c2ffi`:

* Generate output for a particular header or file, gathering macro
  definitions (with the `-M <file>.c` parameter)

* Generate output for macro definitions by running `c2ffi` again on
  the *generated* file (without `-M`)

This is due to the preprocessor being a huge hack (see below).
However, once this is done, you should have two files with all the
necessary data for your FFI bindings.

Currently JSON is the default output.  This is in a rather wordy
hierarchical format, with each object having a "tag" field which
describes it.  All objects are contained in an array.  This should
make it fairly easy (or at least far easier than parsing C yourself)
to transform into language-specific bindings.

This format may be documented at some point, but for now, you'll have
to look at the input and the output!  I recommend a pretty-printing
reformatter for the JSON.  Patches to produce prettier output will be
accepted. `;-)`

## New Output Drivers

If you're feeling motivated, it should be fairly simple to produce a
new output driver.  Look in `src/drivers/` and you can see the source
for JSON, Sexp (lisp symbolic expressions), and possibly some others.

You will need to do the following:

* Create a new subclass of OutputDriver; copying one of the existing
  ones is probably the easiest.

* Add the factory function to `src/OutputDriver.cpp`.

* Write your code!

## The Preprocessor

The preprocessor handling is, as was noted, a huge hack.  This is due
entirely to the fact that `#define` macros can contain just about
anything, and thus it's not easy to tell if they are useful values or
syntax hackery.

For this, `c2ffi` uses a simple heuristic:

* If there are arithmetic operators (`+`, `-`, `*`, `<<`, etc),
  parens, numbers, and identifiers, it's treated as "useful".

* If only ints are found, it's treated as an `__int128_t`; if floats are
  found, it's treated as a `double`; if a string is found, a `char*`

Why the odd `__int128_t`?  Because without more parsing (and
technically, without context), it can't be determined as signed or
unsigned.  So this is declared with very large capacity which will
hold the entire range of signed and unsigned 64-bit ints.

If you're dealing with unsigned 128-bit int constants, you'll have to
do it yourself.  I personally haven't seen any.

## License

This is currently GPL2, but it will almost certainly be moved to
LGPL2, as I would like to make a shared library which you can load
definitions at runtime.  It may be moved to BSD or similar at some
point in the future.

**Exception:** Any code generated by `c2ffi` as a part of its regular
output, including any code that may otherwise be contained in or
distributed with the source, should be considered under the public
domain.  I.e., just because this includes a snippet in the output
doesn't make it GPL.  (On the other hand, making it dump its source
doesn't count!)
