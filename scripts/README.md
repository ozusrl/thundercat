Use the `setup_llvm.sh` file to install a slightly
modified version of LLVM 3.5.0.
The modifications are shown in the `llvm_350_thundercat_patch.diff`
file in this folder.

The script will download the LLVM code from LLVM's svn repository,
apply the patch,
then build the framework.
Note that this is a time- and resource-consuming process.
