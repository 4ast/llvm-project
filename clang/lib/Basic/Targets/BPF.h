//===--- BPF.h - Declare BPF target feature support -------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file declares BPF TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_BASIC_TARGETS_BPF_H
#define LLVM_CLANG_LIB_BASIC_TARGETS_BPF_H

#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/TargetOptions.h"
#include "llvm/Support/Compiler.h"
#include "llvm/TargetParser/Triple.h"

namespace clang {
namespace targets {

static const unsigned BPFAddrSpaceMap[] = {
    0,   // Default
    0,   // opencl_global
    0,   // opencl_local
    0,   // opencl_constant
    0,   // opencl_private
    0,   // opencl_generic
    0,   // opencl_global_device
    0,   // opencl_global_host
    0,   // cuda_device
    0,   // cuda_constant
    0,   // cuda_shared
    0,   // sycl_global
    0,   // sycl_global_device
    0,   // sycl_global_host
    0,   // sycl_local
    0,   // sycl_private
    270, // ptr32_sptr
    271, // ptr32_uptr
    272, // ptr64
    0,   // hlsl_groupshared
    0,   // wasm_funcref
};

class LLVM_LIBRARY_VISIBILITY BPFTargetInfo : public TargetInfo {
  bool HasAlu32 = false;

public:
  BPFTargetInfo(const llvm::Triple &Triple, const TargetOptions &)
      : TargetInfo(Triple) {
    LongWidth = LongAlign = PointerWidth = PointerAlign = 64;
    SizeType = UnsignedLong;
    PtrDiffType = SignedLong;
    IntPtrType = SignedLong;
    IntMaxType = SignedLong;
    Int64Type = SignedLong;
    RegParmMax = 5;
    if (Triple.getArch() == llvm::Triple::bpfeb) {
      resetDataLayout("E-m:e-p:64:64-p271:32:32-i64:64-i128:128-n32:64-S128");
    } else {
      resetDataLayout("e-m:e-p:64:64-p271:32:32-i64:64-i128:128-n32:64-S128");
    }
    MaxAtomicPromoteWidth = 64;
    MaxAtomicInlineWidth = 64;
    TLSSupported = false;
    AddrSpaceMap = &BPFAddrSpaceMap;
  }

  uint64_t getPointerWidthV(LangAS AS) const override {
    unsigned TargetAddrSpace = getTargetAddressSpace(AS);
    if (TargetAddrSpace == 271)
      return 32;
    return PointerWidth;
  }

  uint64_t getPointerAlignV(LangAS AddrSpace) const override {
    return getPointerWidthV(AddrSpace);
  }

  void getTargetDefines(const LangOptions &Opts,
                        MacroBuilder &Builder) const override;

  bool hasFeature(StringRef Feature) const override {
    return Feature == "bpf" || Feature == "alu32" || Feature == "dwarfris";
  }

  void setFeatureEnabled(llvm::StringMap<bool> &Features, StringRef Name,
                         bool Enabled) const override {
    Features[Name] = Enabled;
  }
  bool handleTargetFeatures(std::vector<std::string> &Features,
                            DiagnosticsEngine &Diags) override;

  ArrayRef<Builtin::Info> getTargetBuiltins() const override;

  std::string_view getClobbers() const override { return ""; }

  BuiltinVaListKind getBuiltinVaListKind() const override {
    return TargetInfo::VoidPtrBuiltinVaList;
  }

  bool isValidGCCRegisterName(StringRef Name) const override { return true; }
  ArrayRef<const char *> getGCCRegNames() const override {
    return std::nullopt;
  }

  bool validateAsmConstraint(const char *&Name,
                             TargetInfo::ConstraintInfo &Info) const override {
    switch (*Name) {
    default:
      break;
    case 'w':
      if (HasAlu32) {
        Info.setAllowsRegister();
      }
      break;
    }
    return true;
  }

  ArrayRef<TargetInfo::GCCRegAlias> getGCCRegAliases() const override {
    return std::nullopt;
  }

  bool allowDebugInfoForExternalRef() const override { return true; }

  CallingConvCheckResult checkCallingConvention(CallingConv CC) const override {
    switch (CC) {
    default:
      return CCCR_Warning;
    case CC_C:
    case CC_OpenCLKernel:
      return CCCR_OK;
    }
  }

  bool isValidCPUName(StringRef Name) const override;

  void fillValidCPUList(SmallVectorImpl<StringRef> &Values) const override;

  bool setCPU(const std::string &Name) override {
    if (Name == "v3" || Name == "v4") {
      HasAlu32 = true;
    }

    StringRef CPUName(Name);
    return isValidCPUName(CPUName);
  }
};
} // namespace targets
} // namespace clang
#endif // LLVM_CLANG_LIB_BASIC_TARGETS_BPF_H
