#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <circt-c/Dialect/SV.h>
#include <circt-c/Dialect/HW.h>
#include <mlir-c/IR.h>


/* forward decls */

// Subroutine to get the op I'm having trouble with
MlirOperation get_ifdef_op(MlirOperation top_op);
// Simple callback for MLIR *Print routines. It just prints to the stdout.
void print_callback(MlirStringRef r, void *data);
// Load dialect and error check
MlirDialect load_dialect(MlirContext ctx, MlirDialectHandle handle);

int main(int argc, char **argv)
{
    // Initialize the MLIR context 
    MlirContext ctx = mlirContextCreate();

    // Load the HW and SV Dialects
    MlirDialect hw = load_dialect(ctx, mlirGetDialectHandle__hw__());
    MlirDialect sv = load_dialect(ctx, mlirGetDialectHandle__sv__());

    // Create an MLIR module with an sv.macro.decl op, and one hw.module op containing 
    // one sv.ifdef.procedural op with a cond field referencing the macro
    MlirStringRef src_ref = mlirStringRefCreateFromCString("\
        module {\
            sv.macro.decl @SYNTHESIS\
            hw.module @test1(in %arg0: i1, in %arg1: i1, in %arg8: i8) {\
                sv.always posedge  %arg0 {\
                    sv.ifdef.procedural @SYNTHESIS {\n} else {\n}\
                }\
            }\
        }");
    MlirStringRef file_ref = mlirStringRefCreateFromCString(__FILE__);
    MlirOperation top_op = mlirOperationCreateParse(ctx, src_ref, file_ref);

    // Now that we have the MLIR, navigate to the sv.ifdef.procedural operation
    MlirOperation ifdef_op = get_ifdef_op(top_op);
    printf("Found ifdef op\n");

    // How do we access the cond attr? From SV.td I expect it to be an Inherent Attribute.
    MlirStringRef cond_str = mlirStringRefCreateFromCString("cond");
    if (mlirOperationHasInherentAttributeByName(ifdef_op, cond_str)) {
        printf("op has an inherent attribute named `cond`\n");
    } 
    // Let's try to look at the cond attribute's value
    MlirAttribute cond_attr = mlirOperationGetInherentAttributeByName(ifdef_op, cond_str);
    if (cond_attr.ptr == 0 || mlirAttributeIsNull(cond_attr)) {
        printf("cond_attr is NULL!\n");
        exit(-1);
    } else {
        printf("cond_attr is not NULL\n");
    }

    // I expected that this is an SV Attribute because sv.ifdef.procedural's cond attribute
    // is documented to be of type ::circt::sv::MacroIdentAttr
    // https://circt.llvm.org/docs/Dialects/SV/#svifdefprocedural-circtsvifdefproceduralop 
    if (svAttrIsASVAttributeAttr(cond_attr)) {
        printf("cond is an svAttr\n");
    } else {
        printf("cond is NOT an svAttr!\n");
    }
    // If not an SV Attribute, what type is it?
    MlirType cond_attr_type = mlirAttributeGetType(cond_attr);
    if (mlirTypeIsNull(cond_attr_type)) {
        printf("cond attribute type is NULL!");
    } else {
        printf("cond type: "); fflush(stdout); mlirTypePrint(cond_attr_type, print_callback, 0);
        putchar('\n');
    }
    printf("cond value printed: "); fflush(stdout);mlirAttributePrint(cond_attr, print_callback, 0);
    putchar('\n');

    // Can I write to it?
    if (mlirOperationVerify(ifdef_op)) {
        printf("Op verifies before writing\n");
    } else {
        printf("Op fails to verify even before writing\n");
    }

    printf("Trying to write it ...\n");
    MlirStringRef cond_expr_str = mlirStringRefCreateFromCString("#sv<macro.ident @FOO>");
    MlirAttribute my_cond_attr = svSVAttributeAttrGet(ctx, cond_str, cond_expr_str, false);
    if (mlirAttributeIsNull(my_cond_attr)) {
        printf("Failed to create attr.");
    } else {
        printf("my_cond_attr value printed: "); fflush(stdout);mlirAttributePrint(my_cond_attr, print_callback, 0); putchar('\n');
        mlirOperationSetInherentAttributeByName(ifdef_op, cond_str, my_cond_attr);
        MlirAttribute readback_cond_attr = mlirOperationGetInherentAttributeByName(ifdef_op, cond_str);
        if (mlirAttributeIsNull(readback_cond_attr)) {
            printf("readback_cond_attr is NULL!\n");
            if (mlirOperationVerify(ifdef_op)) {
                printf("Op still verifies\n");
            } else {
                printf("Op fails to verify after writing\n");
            }
        } else {
            printf("readback_cond_attr value printed: "); fflush(stdout);mlirAttributePrint(readback_cond_attr, print_callback, 0);
            putchar('\n');
        }
    }

    // Just trying to assume it is an SV Attribute fails in a class cast expression
    // ifdef: /home/jack/projects/circt/circt/llvm/llvm/include/llvm/Support/Casting.h:566: decltype(auto) llvm::cast(const From&) [with To = circt::sv::SVAttributeAttr; From = mlir::Attribute]: Assertion `isa<To>(Val) && "cast<Ty>() argument of incompatible type!"' failed.
    svSVAttributeAttrGetExpression(cond_attr);
}

// Walk the parsed MLIR to the ifdef_op. This code is specific to the string parsed above.
MlirOperation get_ifdef_op(MlirOperation top_op)
{
    MlirRegion parse_region = mlirOperationGetFirstRegion(top_op);
    MlirBlock parse_block = mlirRegionGetFirstBlock(parse_region);
    MlirOperation macro_decl_op = mlirBlockGetFirstOperation(parse_block);
    MlirOperation module_op = mlirOperationGetNextInBlock(macro_decl_op);
    MlirRegion module_region = mlirOperationGetFirstRegion(module_op);
    MlirBlock module_block = mlirRegionGetFirstBlock(module_region);
    MlirOperation always_op = mlirBlockGetFirstOperation(module_block);
    MlirRegion always_region = mlirOperationGetFirstRegion(always_op);
    MlirBlock always_block = mlirRegionGetFirstBlock(always_region);
    MlirOperation ifdef_op = mlirBlockGetFirstOperation(always_block);

    // Let's verify that we got to the ifdef op
    MlirStringRef found_op_ref = mlirIdentifierStr(mlirOperationGetName(ifdef_op));
    MlirStringRef ifdef_str_ref = mlirStringRefCreateFromCString("sv.ifdef.procedural");
    if (!mlirStringRefEqual(found_op_ref, ifdef_str_ref)) {
        fprintf(stderr, "Expected %s, but found %s", ifdef_str_ref.data, found_op_ref.data);
    }

    return ifdef_op;
}

MlirDialect load_dialect(MlirContext ctx, MlirDialectHandle handle)
{
    MlirDialect hw = mlirContextGetOrLoadDialect(ctx, mlirDialectHandleGetNamespace(handle));
    if (!mlirDialectIsNull(hw)) {
        printf("Failed to GetOrLoadDialect\n");
        exit(-1);
    }

    mlirDialectHandleRegisterDialect(handle, ctx);
  
    return hw;
}

// Simple callback for MLIR *Print routines. It just prints to the stdout.
void print_callback(MlirStringRef r, void *data)
{
    write(0, r.data, r.length);
}
