// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COURGETTE_ENCODED_PROGRAM_H_
#define COURGETTE_ENCODED_PROGRAM_H_

#include <vector>

#include "base/basictypes.h"
#include "courgette/disassembler.h"
#include "courgette/memory_allocator.h"
#include "courgette/types_elf.h"

namespace courgette {

// Stream indexes.
const int kStreamMisc = 0;
const int kStreamOps = 1;
const int kStreamBytes = 2;
const int kStreamAbs32Indexes = 3;
const int kStreamRel32Indexes = 4;
const int kStreamAbs32Addresses = 5;
const int kStreamRel32Addresses = 6;
const int kStreamCopyCounts = 7;
const int kStreamOriginAddresses = kStreamMisc;

const int kStreamLimit = 9;

class SinkStream;
class SinkStreamSet;
class SourceStreamSet;

// An EncodedProgram is a set of tables that contain a simple 'binary assembly
// language' that can be assembled to produce a sequence of bytes, for example,
// a Windows 32-bit executable.
//
class EncodedProgram {
 public:
  EncodedProgram();
  ~EncodedProgram();

  // Generating an EncodedProgram:
  //
  // (1) The image base can be specified at any time.
  void set_image_base(uint64 base) { image_base_ = base; }

  // (2) Address tables and indexes defined first.
  CheckBool DefineRel32Label(int index, RVA address) WARN_UNUSED_RESULT;
  CheckBool DefineAbs32Label(int index, RVA address) WARN_UNUSED_RESULT;
  void EndLabels();

  // (3) Add instructions in the order needed to generate bytes of file.
  // NOTE: If any of these methods ever fail, the EncodedProgram instance
  // has failed and should be discarded.
  CheckBool AddOrigin(RVA rva) WARN_UNUSED_RESULT;
  CheckBool AddCopy(size_t count, const void* bytes) WARN_UNUSED_RESULT;
  CheckBool AddRel32(int label_index) WARN_UNUSED_RESULT;
  CheckBool AddRel32ARM(uint16 op, int label_index) WARN_UNUSED_RESULT;
  CheckBool AddAbs32(int label_index) WARN_UNUSED_RESULT;
  CheckBool AddAbs64(int label_index) WARN_UNUSED_RESULT;
  CheckBool AddPeMakeRelocs(ExecutableType kind) WARN_UNUSED_RESULT;
  CheckBool AddElfMakeRelocs() WARN_UNUSED_RESULT;
  CheckBool AddElfARMMakeRelocs() WARN_UNUSED_RESULT;

  // (3) Serialize binary assembly language tables to a set of streams.
  CheckBool WriteTo(SinkStreamSet* streams) WARN_UNUSED_RESULT;

  // Using an EncodedProgram to generate a byte stream:
  //
  // (4) Deserializes a fresh EncodedProgram from a set of streams.
  bool ReadFrom(SourceStreamSet* streams);

  // (5) Assembles the 'binary assembly language' into final file.
  CheckBool AssembleTo(SinkStream* buffer) WARN_UNUSED_RESULT;

 private:
  // Binary assembly language operations.
  // These are part of the patch format. Reusing an existing value will
  // break backwards compatibility.
  enum OP {
    ORIGIN = 0,    // ORIGIN <rva> - set address for subsequent assembly.
    COPY = 1,      // COPY <count> <bytes> - copy bytes to output.
    COPY1 = 2,     // COPY1 <byte> - same as COPY 1 <byte>.
    REL32 = 3,     // REL32 <index> - emit rel32 encoded reference to address at
                   // address table offset <index>
    ABS32 = 4,     // ABS32 <index> - emit abs32 encoded reference to address at
                   // address table offset <index>
    MAKE_PE_RELOCATION_TABLE = 5,  // Emit PE base relocation table blocks.
    MAKE_ELF_RELOCATION_TABLE = 6, // Emit Elf relocation table for X86
    MAKE_ELF_ARM_RELOCATION_TABLE = 7, // Emit Elf relocation table for ARM
    MAKE_PE64_RELOCATION_TABLE = 8, // Emit PE64 base relocation table blocks.
    ABS64 = 9,     // ABS64 <index> - emit abs64 encoded reference to address at
                   // address table offset <index>
    // ARM reserves 0x1000-LAST_ARM, bits 13-16 define the opcode
    // subset, and 1-12 are the compressed ARM op.
    REL32ARM8   = 0x1000,
    REL32ARM11  = 0x2000,
    REL32ARM24  = 0x3000,
    REL32ARM25  = 0x4000,
    REL32ARM21  = 0x5000,
    LAST_ARM    = 0x5FFF,
  };

  typedef NoThrowBuffer<RVA> RvaVector;
  typedef NoThrowBuffer<size_t> SizeTVector;
  typedef NoThrowBuffer<uint32> UInt32Vector;
  typedef NoThrowBuffer<uint8> UInt8Vector;
  typedef NoThrowBuffer<OP> OPVector;

  void DebuggingSummary();
  CheckBool GeneratePeRelocations(SinkStream *buffer,
                                  uint8 type) WARN_UNUSED_RESULT;
  CheckBool GenerateElfRelocations(Elf32_Word pending_elf_relocation_table,
                                   SinkStream *buffer) WARN_UNUSED_RESULT;
  CheckBool DefineLabelCommon(RvaVector*, int, RVA) WARN_UNUSED_RESULT;
  void FinishLabelsCommon(RvaVector* addresses);

  // Decodes and evaluates courgette ops for ARM rel32 addresses.
  CheckBool EvaluateRel32ARM(OP op, size_t& ix_rel32_ix, RVA& current_rva,
                             SinkStream* output);

  // Binary assembly language tables.
  uint64 image_base_;
  RvaVector rel32_rva_;
  RvaVector abs32_rva_;
  OPVector ops_;
  RvaVector origins_;
  SizeTVector copy_counts_;
  UInt8Vector copy_bytes_;
  UInt32Vector rel32_ix_;
  UInt32Vector abs32_ix_;

  // Table of the addresses containing abs32 relocations; computed during
  // assembly, used to generate base relocation table.
  UInt32Vector abs32_relocs_;

  DISALLOW_COPY_AND_ASSIGN(EncodedProgram);
};

}  // namespace courgette
#endif  // COURGETTE_ENCODED_PROGRAM_H_
