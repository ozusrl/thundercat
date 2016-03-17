## Benchmarking and Testing Tools

To use the scripts, you have to define
an environment variable named `MATRICES`
whose value is the path to the folder that contains
the matrix files.

Scripts take matrix names as a list from the
local file named `matrixNames.txt`.
The list in `matrixNames_gpce.txt` is a good candidate for this.

### `genSpMVlibDebugOutput.sh`
Use this to generate the output vector for each
matrix (using the `-debug` flag).
Results are stored in the `../data` folder.
Sample usage:

```bash
$ ./genSpMVlibDebugOutput.sh CSRbyNZ -num_threads 6
$ ./genSpMVlibDebugOutput.sh stencil
```

### `compareSpMVlibAndSpMVlib.sh`
For each matrix,
performs a numerical diff between the outputs of
two specialization methods.
Requires the **numdiff** command (i.e. **cern-ndiff**).
Sample usage:

```bash
$ ./compareSpMVlibAndSpMVlib.sh CSRbyNZ stencil
$ ./compareSpMVlibAndSpMVlib.sh CSRbyNZ-num_threads6 stencil
```

### To be continued...
