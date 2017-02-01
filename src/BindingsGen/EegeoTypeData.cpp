#include "EegeoTypeData.h"

// Declares clang::SyntaxOnlyAction.
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"

// Declares llvm::cl::extrahelp.
#include "llvm/Support/CommandLine.h"

using namespace clang;
using namespace clang::tooling;
using namespace clang::ast_matchers;
using namespace llvm;

namespace Eegeo {
    StringRef Type::GetTypeName(types param) {
        switch (param) {
        case Void:
            return "Void";

        case Floating:
            return "Floating";

        case Integral:
            return "Integral";

        case Boolean:
            return "Boolean";

        case Pointer:
            return "Pointer";

        case FunctionPointer:
            return "FunctionPointer";

        case Array:
            return "Array";

        case Struct:
            return "Struct";

        case String:
            return "String";

        default:
            //TODO(vim): Error
            return "";
        }
    }

    std::tuple<bool, std::string, Eegeo::Type::types> CheckBuiltinType(QualType CanonicalType, ASTContext& Context) {
        std::string keyword;
        Eegeo::Type::types returnTypeName = Eegeo::Type::Void;

        bool typeFound = false;

        if (!CanonicalType->isBuiltinType()) {
            return { typeFound, keyword, returnTypeName };
        }

        typeFound = true;

        keyword = CanonicalType.getAsString();

        auto BT = dyn_cast<BuiltinType>(CanonicalType);

        if (keyword == "_Bool") {
            keyword = "bool";
            returnTypeName = Eegeo::Type::Boolean;
        }
        else if (CanonicalType->isVoidType()) {
            returnTypeName = Eegeo::Type::Void;
        }
        else if (CanonicalType->isIntegralType(Context)) {
            returnTypeName = Eegeo::Type::Integral;
        }
        else if (CanonicalType->isFloatingType()) {
            auto kind = BT->getKind();

            //For future use case
            if (kind == BuiltinType::Kind::Double || kind == BuiltinType::Kind::LongDouble)
                returnTypeName = Eegeo::Type::Floating;
            else
                returnTypeName = Eegeo::Type::Floating;
        }
        else {
            //TODO(vim): Error
            llvm::errs() << "Unsupported Builtin Type Found!!" << endl;
            typeFound = false;
        }

        return { typeFound, keyword, returnTypeName };
    }

    Type ProcessReturnType(QualType type, ASTContext& context) {
        auto& OS = llvm::errs();

        auto canonType = type.getDesugaredType(context).getCanonicalType();
        canonType.removeLocalCVRQualifiers(Qualifiers::CVRMask);

        auto unqual = type.getTypePtr();

        std::string keyword;

        Eegeo::Type::types returnTypeName = Eegeo::Type::Void;

        if (unqual->isBuiltinType()) {

            keyword = canonType.getAsString();

            auto BT = dyn_cast<BuiltinType>(canonType);

            if (keyword == "_Bool") {
                keyword = "bool";
                returnTypeName = Eegeo::Type::Boolean;
            }
            else if (unqual->isVoidType()) {
                returnTypeName = Eegeo::Type::Void;
            }
            else if (unqual->isIntegralType(context)) {
                returnTypeName = Eegeo::Type::Integral;
            }
            else if (unqual->isFloatingType()) {
                auto kind = BT->getKind();

                //For future use case
                if (kind == BuiltinType::Kind::Double || kind == BuiltinType::Kind::LongDouble)
                    returnTypeName = Eegeo::Type::Floating;
                else
                    returnTypeName = Eegeo::Type::Floating;
            }
            else {
                //TODO(vim): Error
                OS << "Unsupported Builtin Type Found!!" << endl;
            }
        }
        else if (unqual->isFunctionPointerType()) {
            keyword = canonType.getAsString();
        }
        else if (unqual->isPointerType()) {
            auto PT = dyn_cast<PointerType>(canonType);

            returnTypeName = Eegeo::Type::Pointer;

            auto pointee = PT->getPointeeType();

            if (pointee->isBuiltinType()) {
                keyword = pointee.getAsString();
            }
            else if (pointee->isRecordType()) {
                auto record = pointee->getAsCXXRecordDecl();

                //if (!record->isPOD()) {
                //    //TODO(vim): Error
                //    OS << "Non POD Pointer Type Found!!" << endl;
                //}

                keyword = record->getName();
            }
            else {
                //TODO(vim): Error
                OS << "Unsupported Pointer Type Found!!" << endl;
            }
        }
        else if (unqual->isRecordType()) {
            auto RT = unqual->getAsCXXRecordDecl();

            returnTypeName = Eegeo::Type::Struct;

            if (RT->isPOD())
                keyword = RT->getName();
            else {
                //TODO(vim): Error
                OS << "Unsupported Value return type found!!" << endl;
            }
        }
        else {
            //TODO(vim): Error
            OS << "Unsupported Param Type Found!!" << endl;
        }

        return { std::move(keyword), Eegeo::Type::GetTypeName(returnTypeName), "" };
    }

    Type ProcessParamType(QualType type, ASTContext& context) {
        auto& OS = llvm::errs();

        auto canonType = type.getDesugaredType(context).getCanonicalType();
        canonType.removeLocalCVRQualifiers(Qualifiers::CVRMask);

        auto unqual = type.getTypePtr();

        std::string keyword;
        std::string name;

        Eegeo::Type::types returnTypeName = Eegeo::Type::Void;

        if (unqual->isBuiltinType()) {

            keyword = canonType.getAsString();

            auto BT = dyn_cast<BuiltinType>(canonType);

            if (keyword == "_Bool") {
                keyword = "bool";
                returnTypeName = Eegeo::Type::Boolean;
            }
            else if (unqual->isVoidType()) {
                returnTypeName = Eegeo::Type::Void;
                //TODO(vim): Error
            }
            else if (unqual->isIntegralType(context)) {
                returnTypeName = Eegeo::Type::Integral;
            }
            else if (unqual->isFloatingType()) {
                auto kind = BT->getKind();

                //For future use case
                if (kind == BuiltinType::Kind::Double || kind == BuiltinType::Kind::LongDouble)
                    returnTypeName = Eegeo::Type::Floating;
                else
                    returnTypeName = Eegeo::Type::Floating;
            }
            else {
                //TODO(vim): Error
                OS << "Unsupported Builtin Type Found!!" << endl;
            }
        }
        else if (unqual->isFunctionPointerType()) {
            returnTypeName = Eegeo::Type::FunctionPointer;
            keyword = canonType.getAsString();
        }
        else if (unqual->isPointerType()) {

            if (const auto DT = dyn_cast<DecayedType>(unqual)) {
                auto original = DT->getOriginalType();

                if (const auto AT = dyn_cast<ArrayType>(original.getTypePtr())) {
                    if (AT->getSizeModifier() == ArrayType::ArraySizeModifier::Normal) {
                        const auto CAT = dyn_cast<ConstantArrayType>(AT);
                        returnTypeName = Eegeo::Type::Array;
                        keyword = original.getAsString();
                    }
                    else {
                        //TODO(vim): Error
                        OS << "Unbounded array types are not supported!!";
                    }
                }

            }
            else {

                auto PT = dyn_cast<PointerType>(canonType);

                returnTypeName = Eegeo::Type::Pointer;

                auto pointee = PT->getPointeeType();

                if (pointee->isBuiltinType()) {
                    keyword = pointee.getAsString();
                }
                else if (pointee->isRecordType()) {
                    auto record = pointee->getAsCXXRecordDecl();

                    if (!record->isPOD()) {
                        //TODO(vim): Error
                        OS << "Non POD Pointer Type Found!!" << endl;
                    }

                    keyword = record->getName();
                }
                else {
                    //TODO(vim): Error
                    OS << "Unsupported Pointer Type Found!!" << endl;
                }
            }
        }
        else if (unqual->isRecordType()) {
            auto RT = unqual->getAsCXXRecordDecl();

            returnTypeName = Eegeo::Type::Struct;

            if (RT->isPOD())
                keyword = RT->getName();
            else {
                //TODO(vim): Error Example: std::string*
                OS << "Unsupported Value return type found!!" << endl;
            }
        }
        else if (unqual->isReferenceType()) {
            auto RT = dyn_cast<ReferenceType>(unqual);
            auto pointee = RT->getPointeeType();

            if (pointee->isRecordType() && pointee->getAsCXXRecordDecl()->getDeclName().getAsString() == "basic_string") {
                keyword = "char";
                returnTypeName = Eegeo::Type::String;
            }
            else {
                OS << "Unsupported reference type. Use a pointer instead!" << endl;
            }

        }
        else {
            //TODO(vim): Error
            OS << "Unsupported Return Type Found!!" << endl;
            type.dump();
        }

        return { std::move(keyword), Eegeo::Type::GetTypeName(returnTypeName), std::move(name) };
    }
}