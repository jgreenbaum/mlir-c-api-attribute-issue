# Problems accessing Dialect Attributes via MLIR C API

This repo demonstrates an issue trying to access the `cond`
attribute of the CIRCT SV MLIR Dialect [`sv.ifdef.procedural`](https://circt.llvm.org/docs/Dialects/SV/#svifdefprocedural-circtsvifdefproceduralop) operator.

See [this post](https://discourse.llvm.org/t/dialect-attributes-in-mlir-c-api/87800) on the LLVM forums for a full description.

# Building

You need a build of MLIR + CIRCT Dialects. My most recent builds have used the [manual compilation directions for the CIRCT Python Bindings](https://circt.llvm.org/docs/PythonBindings/#manual-compilation) since the Python bindings also sit on top of the MLIR C API. 

I've tested this program on several CIRCT commits as far back as August 2023. The most recent I've tried is https://github.com/llvm/circt/commit/2f011299e72ca7291d6f46fb8f01bda160d03be1.

```
mkdir build
cd build
cmake -G Ninja -DMLIR_INSTALL_DIR=/path/to/mlir+circt/install
```

# Running

```
build/ifdef
```

# Output

Here is the output of the program.

```
Found ifdef op
op has an inherent attribute named `cond`
cond_attr is not NULL
cond is NOT an svAttr!
cond type: none
cond value printed: #sv<macro.ident @SYNTHESIS>
ifdef: /home/jack/projects/circt/circt/llvm/llvm/include/llvm/Support/Casting.h:566: decltype(auto) llvm::cast(const From&) [with To = circt::sv::SVAttributeAttr; From = mlir::Attribute]: Assertion `isa<To>(Val) && "cast<Ty>() argument of incompatible type!"' failed.
Aborted (core dumped)
```
