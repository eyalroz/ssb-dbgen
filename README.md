 # Star Schema Benchmark data set generator (ssb-dbgen)  

This repository holds the data generation utility for the Star Schema Benchmark (SSB) for DBMS analytics. It generates schema data as table files, in a simple textual format, which can then be loaded into a DBMS for running the benchmark.

Package build status on Travis CI: [![Build Status](https://travis-ci.org/eyalroz/ssb-dbgen.png?branch=master)](https://travis-ci.org/eyalroz/ssb-dbgen)

| Table of contents|
|:----------------|
| ["What? _Another_ fork of ssb-dbgen? Why?"](#another-fork)<br>  [About the Star Schema Benchmark](#about-ssb)<br> [Building the generation utility](#building)<br> [Using the utility to generate data](#using)<br> [Differences of the generated data from the TPC-H schema](#difference-from-tpch)<br>[Trouble building/running?](#trouble)<br> |

## <a name="another-fork">"What? _Another_ fork of ssb-dbgen? Why?"</a>

The `ssb-dbgen` utility is based on the [TPC-H benchmark](http://tpc.org/tpch/)'s data generation utility, also named `dbgen`. The TPC-H benchmark's `dgen` is pretty stable, and is maintained by the TPC, getting updated if bugs or build issues are found (well, sort of; see [this](https://github.com/eyalroz/tpch-dbgen)). As for the Star Schema Benchmark - it does not have an official website; the original code of its own `dbgen` was forked from an older, now out-of-date version of TPC-H `dbgen`; and its resources have not been maintained by the benchmark's creators for a long time now.

The result has seen several repositories here on github with various changes to the code, intended to resolve this or the other issue with compilation or execution, occasionally adding new files (such as scripts for loading the data into a DBMS, generating compressed data files, removing the trailing pipe characters etc.). This means a tree of mostly unsynchronized repositories - with most having been essentially abandoned: Last commits several years ago with more than a couple of issues unresolved.

This effort is an attempt to **unify** all of those disparate repositories, taking all changes to the code which - in my opinion - are generally applicable, and applying them all together while resolving any conflicts. Details of what's already been done can be found on the [Closed Issues Page](https://github.com/eyalroz/ssb-dbgen/issues?q=is%3Aissue+is%3Aclosed) and of course by examining the commit comments.

If you are the author of one of the other repositories - please [contact me](mailto:eyalroz@technion.ac.il) for better coordination of this effort.

## <a name="about-ssb">About the Star Schema Benchmark</a>

The Star Schema Benchmark is a modification of the [TPC-H benchmark](http://tpc.org/tpch/), which is the Transaction Processing Council's (older) benchmark for evaluating the performance of Database Management Systems (DBMSes) on analytic queries - that is, queries which do not modify the data.

The TPC-H has various known issues and deficiencies which are beyond the scope of this document. Researchers [Patrick O'Neil](http://www.cs.umb.edu/~poneil/), [Betty O'Neil](http://www.cs.umb.edu/~eoneil/) and [Xuedong Chen](https://www.linkedin.com/in/xuedong-chen-18414ba/), from the University of Massachusetts Boston, proposed a modification of the TPC-H benchmark which addresses some of these shortcomings, in several papers, the latest and most relevant being [Star Schema Benchmark, Revision 3](http://www.cs.umb.edu/~poneil/StarSchemaB.PDF) published June 2009. One of the key features of the modification is the conversion of the [TPC-H schemata](http://kejser.org/wp-content/uploads/2014/06/image_thumb2.png) to Star Schemata ("Star Schema" is a misnomer), by some denormalizing as well as dropping some of the data; more details appear <a href="#difference-from-tpch">below</a> and even more details in the paper itself.

The benchmark was also accompanied by the initial versions of the code in this repository - a modified utility to generate schema data on which to run the benchmark.

For a recent discussion of the benchmark, you may wish to also read [A Review of Star Schema Benchmark](https://arxiv.org/pdf/1606.00295.pdf), by Jimi Sanchez.

## <a name="building">Building the generation utility</a>

The build is automated using [CMake](https://cmake.org/) now. You can run it in several modes:

+ Default: `$ cmake -B ./build && cmake --build ./build`
* Passing options manually: `$ cmake -B ./build [OPTIONS] && cmake --build ./build`
* Interactive: `$ ccmake -B ./build && cmake --build ./build`

(these examples assume you're running CMake inside the project root directory; otherwise, use `-S` and `-B` to specify the source and build directory paths)

Of course, you should have C language compiler (C99/C2011 support is not necessary), linker, and corresponding make-tool preinstalled in your system. CMake will detect them automatically.

Building process was tested using [Travis CI](https://travis-ci.org/) with [gcc](https://gcc.gnu.org/) and [clang](https://clang.llvm.org/) compilers on Ubuntu Linux, with clang on OS X, and with [MSVC](https://en.wikipedia.org/wiki/Microsoft_Visual_C%2B%2B) and [MinGW](http://www.mingw.org/) on Windows.

#### Available options

| Option | What is it about? | Possible values | Default value |
|----------|------------------------|----------------------|-------------------|
| `CMAKE_BUILD_TYPE` | Predefined CMake option. if `Debug`, then build with debugging symbols and without optimizations; if `Release`, then build with optimizations. | `Release`, `Debug` | `Release` |
| `DATABASE` | DBMS which you are going to benchmark with SSB. This option only affects `qgen`, so if you're only generating data, let it stay at its default value . | `INFORMIX`, `DB2`, `TDAT`, `SQLSERVER`, `SYBASE` | `DB2` |
| `EOL_HANDLING` | If `ON`, then separator is omitted after the last column in all tables.   | `ON`  `OFF` | `OFF` |
| `CSV_OUTPUT_FORMAT` |  Adhere to the CSV format for the output, i.e. use commans (`,`) as a field separator, and enclose strings in double-quotes to ensure any commas within them aren't mis-interpreted. | `ON` `OFF` | `OFF` |
| `WORKLOAD` | As was already mentioned, this generator was created on the base of tpch-dbgen. And formally it supports data generation for TPC-H. But it's strongly recommended to use ssb-dbgen for SSB and tpch-dbgen for TPC-H. | `SSB`, `TPCH` | `SSB` |
| `YMD_DASH_DATE` | When set to `ON`, generates dates with dashes between fields, i.e. `YYYY-MM-DD`; when set to `OFF`, no dashes are included, e.g. `YYYYMMDD`  | `ON`, `OFF` | `OFF` |


<!--2. Set the value of the  variable to `DB2` - or, if you know what you're doing and you have a specific reason to do so, to one of the other databases in the commented list of possibilities.
3. Set `WORKLOAD` to `SSB` (theoretically, `TPCH` might also work and generate TPC-H data, but don't count on it)  -->

## <a name="using">Using the utility to generate data</a>

A typical invocation:

    $ dbgen -b /path/to/dists.dss -v -s 10
    
will create all tables in the current directory, with a scale factor of 10. This will have, for example, 300,000 lines in `customer.tbl`, beginning with something like:
```
1|Customer#000000001|j5JsirBM9P|MOROCCO  0|MOROCCO|AFRICA|25-989-741-2988|BUILDING|
2|Customer#000000002|487LW1dovn6Q4dMVym|JORDAN   1|JORDAN|MIDDLE EAST|23-768-687-3665|AUTOMOBILE|
3|Customer#000000003|fkRGN8n|ARGENTINA7|ARGENTINA|AMERICA|11-719-748-3364|AUTOMOBILE|
4|Customer#000000004|4u58h f|EGYPT    4|EGYPT|MIDDLE EAST|14-128-190-5944|MACHINERY|
```
the fields are separated by a pipe character (`|`), and if `EOL_HANDLING`was set to `OFF` during building there's a trailing separator at the end of the line. 

After generating `.tbl` files for the CUSTOMER, PART, SUPPLIER, DATE, and LINEORDER tables, you should now either load them directly into your DBMS or apply some textual processing to them before loading.

**Notes:** 

* On Unix-like systems, it is also possible to write the generated data into a FIFO filesystem node, reading from the other side with a compression utility, so as to only write compressed data to disk. This may be useful if disk space is limited and you are using a particularly high scale factor.
* If you do not specify the location of `dists.dss` with `-b`, the `dbgen` utility will try locating it in the current working directory.


<br>

## <a name="difference-from-tpch">Differences of the generated data from the TPC-H schema</a>


For a detailed description of the differences between SSB data and its distributions, as well as motivation for the differences, please read the SSB's eponymous [paper](http://www.cs.umb.edu/~poneil/StarSchemaB.PDF).

In a nutshell, the differences are as follows:

1. Removed: Snowflake tables such as `NATION` and `REGION`
2. Removed: The `PARTSUPP` table
3. Denormalized/Removed: The `ORDERS` table - data is denormalized into `LINEORDER`
4. Expanded/Modified/Renamed: The fact table `LINEITEM` is now `LINEORDER`; many of its fields have been added/removed, including fields denormalized from the `ORDERS` table.
5. Added: A `DATE` dimension table
6. Modified: Removed and added fields in existing dimension tables (e.g. `SUPPLIER`)
7. `LINEORDER` now has data cross-reference for supplycost and revenue 

Also, refreshing is only applied to `LINEORDER`.

## <a name="trouble">Trouble building/running?</a>
Have you encountered some other issue with `dbgen` or `qgen`? Please open a new issue on the [Issues Page](https://github.com/eyalroz/ssb-dbgen/issues); be sure to list exactly what you did and enter a copy of the terminal output of the commands you used.


