# graphd -- a very incomplete graph distance calculator

## What is it?

Primarily a small demonstration project and a learning opportunity for myself.

As of writing, this program supports a very small subset of the [DOT language](https://en.wikipedia.org/wiki/DOT_(graph_description_language)) and allows for
shortest-path finding between two nodes in the graph:

```
$ cat test/input/small.dot
graph {
    1 -- 2;
    2 -- 3;
    3 -- 4;
    4 -- 5;
    1 -- 5;
}
$ bin/graphd -f test/input/small.dot 1 4
total distance: 2
1 -> 5 -> 4
$ bin/graphd -f test/input/larger.dot a z
total distance: 8
a -> g -> f -> p -> o -> v -> u -> y -> z
```

The currently supported subset of DOT is in fact so small, that a scanf-based
approach would be easiest. However, the parsing process was kept flexible enough
that a larger subset could conceivably be supported in the future.

## Why DOT?

Because it is relatively simple and I wanted to test out a few things I had
learned about building parsers. A CSV or otherwise list-based approach would be
much easier to support and that would probably be my choice in any kind of
real-world setting.

## Building from source

Assuming Linux, GNU/Make and a recent version of g++ or clang++ is all you need.
Running `make` generates the binary as `bin/graphd`.

## TODOs

Supporting a substantially larger subset of the DOT language is feasible and
would increase usefulness by a lot. Some further bits of the grammar have been
planned and delayed along the way, a few of which are considered essential to
achieve any kind of practicality.

There's a few bugs to fix along the way. Look for `TODO` and `FIXME` tags in the
code.
