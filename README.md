# graphd -- a graph distance calculator

## What is it?

Primarily a small demonstration project and a learning opportunity for myself.

As of writing, this program supports a very small subset of the [DOT language](https://en.wikipedia.org/wiki/DOT_(graph_description_language)) and allows for
shortest-path finding between two nodes in the graph.

## Building

Assuming Linux, GNU/Make and a recent version of g++ or clang++ is all you need.
Running `make` generates the binary as `bin/graphd`.

## Usage

```
$ bin/graphd
usage: bin/graphd [-f file.dot] from-node to-node
  if no input file is specified, stdin is assumed.
```

Examples:

```
$ cat test/input/small.dot
graph {
    1 -- 2;
    2 -- 3;
    3 -- 4;
    4 -- 5;
    1 -- 5;
}
$ cat test/input/small.dot | bin/graphd 1 4
total distance: 2
1 -> 5 -> 4
$ cat test/input/weighted.dot
strict graph weights {
    foo -- bar [weight=28];
    foo -- baz [weight=2];
    bar -- baz [weight=25.8];
}
$ bin/graphd -f test/input/weighted.dot foo bar
total distance: 27.8
foo -> baz -> bar
$ bin/graphd -f test/input/larger.dot a z
total distance: 8
a -> g -> f -> p -> o -> v -> u -> y -> z
```

Since we're the DOT format, we can easily get graphviz to produce a
visualization for the graph in the third example

```
neato -Tpng -oimg/larger.png test/input/larger.dot
```

resulting in

![larger.dot](/img/larger.png)

The second example about covers the subset of DOT currently supported. There is
no limit on the number of expressions. Attributes other than `weight` are
ignored. Directed graphs are not allowed.

## Why DOT?

Because it is relatively simple and I wanted to test out a few things I had
learned about building parsers. A CSV or otherwise list-based approach would be
much easier to support and that would probably be my choice in any kind of
real-world setting.

Anyway, the grammar reference was taken from the [Graphviz manual](https://www.graphviz.org/doc/info/lang.html)
which is reflected in class names within the code.

## TODOs

Supporting a substantially larger subset of the DOT language is feasible and
would increase usefulness by a lot. Some further bits of the grammar have been
planned and delayed along the way, a few of which are considered essential to
achieve any kind of practicality.

There's a few bugs to fix along the way. Look for `TODO` and `FIXME` tags in the
code.
