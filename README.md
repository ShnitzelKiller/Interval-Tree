This is an implementation of the interval tree data structure for fast intersection of a point with a large number of potentially overlapping intervals. The class `IntervalTree` acts as a multi-map with intersecting intervals as the keys.

For more information see https://en.wikipedia.org/wiki/Interval_tree.

## Usage
See `example.cpp` for an example usage. To build the example, run
```
g++ -std=c++17 example.cpp -o example
```