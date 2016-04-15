# Thundercat
This is a sparse matrix-vector multiplication (SpMV) library that generates SpMV code specialized for a given matrix.
Code is generated and executed dynamically for the X86_64 architecture.
For technical details see:

* Buse Yilmaz, Baris Aktemur, Maria Garzaran, Sam Kamin, Furkan Kirac.  
  [Autotuning Runtime Specialization for Sparse Matrix-Vector Multiplication.](http://dx.doi.org/10.1145/2851500)  
  ACM Transactions on Architecture and Code Optimization (TACO).
  Volume 13, Issue 1, Article 5.
* Sam Kamin, Maria Garzaran, Baris Aktemur, Danqing Xu, Buse Yilmaz, Zhongbo Chen.  
  [Optimization by Runtime Specialization for Sparse Matrix-Vector Multiplication.](http://dx.doi.org/10.1145/2658761.2658773)  
  GPCE 2014: The 13th International Conference on Generative Programming: Concepts & Experiences, Västerås, Sweden.  
  
## Files

* `main.cpp`
* `matrix.*`: Matrix class that keeps matrix information in CSR format.
* `method.*`: Specialization methods.
* `spMVgen.*`: Object stream management via LLVM.
* `profiler.*`: Time measurement support.
* `svmAnalyzer.*`: Feature extration from the matrices. Features are used for autotuning (done separately, not integrated here).
* `plaincsr.*`: SpMV implementation using the CSR format.
* `mkl.*`: SpMV using Intel MKL (calls `mkl_dcsrmv` after setting the parameters).

## Runtime Specialization Methods
 
* CSRbyNZ
* Stencil (Called RowPattern in the TACO paper)
* GenOSKI (Similar to [PBR](http://dl.acm.org/citation.cfm?id=1542294). GenOSKI44 is for 4x4 block size, GenOSKI55 is for 5x5.)
* Unfolding

See the papers for details. For each method, there exist two files. One is for method-related matrix analysis, 
the other is for code generation. E.g. `csrByNZAnalyzer.h/cpp` and `csrByNZ.cpp` for CSRbyNZ.

## How to Compile
You should have a slightly modified version of LLVM 3.5.0.
See the [scripts](scripts/) folder to see how to install LLVM.
Once you have LLVM, use `cmake` to build. E.g.:

```
~ $ cd thundercat
~/thundercat $ mkdir build
~/thundercat $ cd build
~/thundercat/build $ cmake -G Ninja ../src
~/thundercat/build $ ninja
```

This will produce a main executable file, named `spMVgen`.
The library runs on Mac OS X and Linux. 

To force a particular compiler, e.g. icc, do the following:

```
~/thundercat/build $ cmake -G Ninja -DCMAKE_C_COMPILER=`which icc` -DCMAKE_CXX_COMPILER=`which icc` ../src/
```

## How to Run
```
./spMVgen <matrixName> <methodName> -num_threads <numOfThreads>
                                    -debug
                                    -dump_matrix
                                    -dump_object
                                    -matrix_stats
```

The following are recognized as `<methodName>`:
* Specialization methods: `CSRbyNZ`, `stencil`, `unfolding`, `genOSKI33`, `genOSKI44`, `genOSKI55`, `unrollingWithGOTO`
* Non-generative methods: `MKL` and `PlainCSR`

`<matrixName>` is the path to the `.mtx` file
(i.e. the matrix file as downloaded from the Matrix Market or the U. of Florida collection).
The name should be provided without the `.mtx` extention.
 
### Optional flags
* `-num_threads`: Number of threads to be used
* `-debug`: Output vector is printed.
* `-dump_matrix`: Dumps matrix's rows, cols and vals array (in CSR format)
* `-dump_object`: Dumps the generated object code to **std out**. The output
can be diassembled using a disassembler (e.g. `llvm-objdump`) to examine the code.
Also, the output can be separately linked to a main file.
* `-matrix_stats`: Prints the `svmAnalyzer`'s results for the current matrix.

### Examples
Run for the lhr71 matrix (assuming `lhr71.mtx` exists in the current directory), using the stencil method with 6 threads:
```
./spMVgen lhr71 stencil -num_threads 6
```
