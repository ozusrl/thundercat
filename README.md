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
* `profiler.*`: Time measurement support.
* `svmAnalyzer.*`: Feature extration from the matrices. Features are used for autotuning (done separately, not integrated here).
* `plaincsr.*`: SpMV implementation using the CSR format.
* `mkl.*`: SpMV using Intel MKL (calls `mkl_dcsrmv` after setting the parameters).

## Runtime Specialization Methods
 
* `CSRbyNZ`
* `RowPattern`
* `GenOSKI <r> <c>` (Similar to [PBR](http://dl.acm.org/citation.cfm?id=1542294). `GenOSKI44` is for 4x4 block size, `GenOSKI55` is for 5x5.)
* `Unfolding`
* `CSRWithGOTO`
* `UnrollingWithGOTO`

See the papers for details. For each method, there exist a corresponding `.cpp` file.

## How to Compile
Thundercat uses [asmjit](http://github.com/asmjit/asmjit) for
assembling executable code at runtime.
It should be placed under the main `thundercat` folder.
Below are the steps.
In these steps, we use `cmake` to generate build files for [Ninja](https://ninja-build.org).
You may generate build files for other targets as well.
In particular, if you don't have Ninja installed, replace `-G Ninja`
below with `-G "Unix Makefiles"`, and replace `ninja` with `make`.


1.  Clone this git repository.
    ```
    ~ $ git clone https://github.com/ozusrl/thundercat.git
    ```
2.  Clone asmjit under thundercat
    ```
    ~ $ cd thundercat
    ~/thundercat $ git clone https://github.com/asmjit/asmjit.git
    ```
3.  Build asmjit.
    ```
    ~/thundercat $ cd asmjit
    ~/thundercat/asmjit $ mkdir build
    ~/thundercat/asmjit $ cd build
    ~/thundercat/asmjit/build $ cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..
    ~/thundercat/asmjit/build $ ninja
    ~/thundercat/asmjit/build $ cd ../..
    ```
4.  Build thundercat.
    ```
    ~/thundercat $ mkdir build
    ~/thundercat $ cd build
    ~/thundercat/build $ cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ../src
    ~/thundercat/build $ ninja
    ```

This will produce a main executable file, named `thundercat`.
The library runs on Mac OS X and Linux. 

Setting the `CMAKE_BUILD_TYPE` variable to `Debug` is a good idea
for a build that you will use for debugging purposes.
All the benchmarkings, however, should be done using a build
configured as `Release`.

To force a particular compiler, e.g. icc, do the following:

```
~/thundercat/build $ cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=`which icc` -DCMAKE_CXX_COMPILER=`which icc` ../src
```

## How to Run
```
./thundercat <matrixName> <methodName> [optional flags]
```

`<matrixName>` is the path to the `.mtx` file
(i.e. the matrix file as downloaded from the Matrix Market or the U. of Florida collection).
The name should be provided without the `.mtx` extention.
 
The following are recognized as `<methodName>`:
 
* Specialization methods: `CSRbyNZ`, `RowPattern`, `Unfolding`, `GenOSKI33`, `GenOSKI44`, `GenOSKI55`, `UnrollingWithGOTO`, `CSRWithGOTO`
* Non-generative methods: `MKL` and `PlainCSR`

### Optional flags
* `-num_threads <num_threads>`: Number of threads to be used. By default, a single thread is used.
* `-debug`: Output vector is printed.
* `-dump_matrix`: Prints matrix's rows, cols and vals array (in the format required by the chosen method).
* `-dump_object`: Dumps the generated object code to current folder into files named `generated_X`
  where `X` is a number from 0 up to the thread count.
* `-matrix_stats`: Prints the `svmAnalyzer`'s results for the current matrix.

### Examples
Run for the `rajat22` matrix (assuming `rajat22.mtx` exists in the current directory), using the `RowPattern` method with 6 threads:
```
./thundercat rajat22 stencil -num_threads 6
```

### Sample Matrices

There is a small set of matrices at

<http://srl.ozyegin.edu.tr/matrices_debug.tar.gz>

Matrices used in the [GPCE 2014 paper](http://dx.doi.org/10.1145/2658761.2658773)
are available at

<http://srl.ozyegin.edu.tr/matrices_gpce.tar.gz>

