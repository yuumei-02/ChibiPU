// Copyright (c) 2026 yuumei-02. All Rights Reserved.
// See the LICENSE file for more information.

#include <mcu/core.h>
#include <mcu/io.h>
#include <mcu/containers.h>
#include <mcu/memory.h>

#include <string.h>
#include <errno.h>

#include "utils.h"
#include "cpu.h"
#include "assembler.h"

static inline Instr* ptr_from_write_head(Assembler* assembler) {
   return (Instr*) (assembler->memory + assembler->write_head);
}

static inline void advance_write_head(Assembler* assembler, u32 slots) {
   assembler->write_head += sizeof(u32) * slots;
   if (assembler->write_head >= assembler->memory_size)
      panic("OOM");
}

void load_nn_instr(Assembler* self, InstrKind instr_kind) {
   mcu_assert(self != nullptr, "self can't be null");

   *ptr_from_write_head(self) = (Instr) { .kind = instr_kind };
   advance_write_head(self, 1);
}

void load_rr_instr(Assembler* self, InstrKind instr_kind, InstrRegister arg0, InstrRegister arg1) {
   mcu_assert(self != nullptr, "self can't be null");

   Instr* instr = ptr_from_write_head(self);
   instr->kind = instr_kind;
   instr->variant = set_nibble(instr->variant, 0, IV_RR);
   instr->variant = set_nibble(instr->variant, 4, arg0);
   instr->variant = set_nibble(instr->variant, 8, arg1);
   advance_write_head(self, 1);
}

void load_rv_instr(Assembler* self, InstrKind instr_kind, InstrRegister arg0, u32 arg1) {
   mcu_assert(self != nullptr, "self can't be null");

   Instr* instr = ptr_from_write_head(self);
   instr->kind = instr_kind;
   instr->variant = set_nibble(instr->variant, 0, IV_RV);
   instr->variant = set_nibble(instr->variant, 4, arg0);
   advance_write_head(self, 1);

   *((u32*) ptr_from_write_head(self)) = arg1;
   advance_write_head(self, 1);
}

void load_rn_instr(Assembler* self, InstrKind instr_kind, InstrRegister arg0) {
   mcu_assert(self != nullptr, "self can't be null");

   Instr* instr = ptr_from_write_head(self);
   instr->kind = instr_kind;
   instr->variant = set_nibble(instr->variant, 0, IV_RN);
   instr->variant = set_nibble(instr->variant, 4, arg0);
   advance_write_head(self, 1);
}

void load_vn_instr(Assembler* self, InstrKind instr_kind, u32 arg0) {
   mcu_assert(self != nullptr, "self can't be null");

   Instr* instr = ptr_from_write_head(self);
   instr->kind = instr_kind;
   instr->variant = set_nibble(instr->variant, 0, IV_VN);
   advance_write_head(self, 1);

   *((u32*) ptr_from_write_head(self)) = arg0;
   advance_write_head(self, 1);
}

HashMap_hdr(InstrKind)
HashMap_hdr(InstrRegister)

HashMap_impl(InstrKind)
HashMap_impl(InstrRegister)

static HashMap(InstrKind) G_instructions;
static HashMap(InstrRegister) G_registers;
static bool G_instructions_defined = false;
static bool G_registers_defined    = false;

static void check_define_instructions() {
   if (G_instructions_defined) return;

   G_instructions = HashMap_new(InstrKind)();
   HashMap_put(InstrKind)(&G_instructions, "halt", IK_Halt);
   HashMap_put(InstrKind)(&G_instructions, "mov",  IK_Mov);
   HashMap_put(InstrKind)(&G_instructions, "add",  IK_Add);
   HashMap_put(InstrKind)(&G_instructions, "sub",  IK_Sub);
   HashMap_put(InstrKind)(&G_instructions, "div",  IK_Div);
   HashMap_put(InstrKind)(&G_instructions, "mul",  IK_Mul);
   HashMap_put(InstrKind)(&G_instructions, "test", IK_Test);
   HashMap_put(InstrKind)(&G_instructions, "jnz",  IK_Jnz);
   HashMap_put(InstrKind)(&G_instructions, "jz",   IK_Jz);
   G_instructions_defined = true;
}

static void check_define_registers() {
   if (G_registers_defined) return;

   G_registers = HashMap_new(InstrRegister)();
   HashMap_put(InstrRegister)(&G_registers, "ra0", IR_RA0);
   HashMap_put(InstrRegister)(&G_registers, "ra1", IR_RA1);
   HashMap_put(InstrRegister)(&G_registers, "ra2", IR_RA2);
   HashMap_put(InstrRegister)(&G_registers, "ra3", IR_RA3);
   HashMap_put(InstrRegister)(&G_registers, "ra4", IR_RA4);
   HashMap_put(InstrRegister)(&G_registers, "ra5", IR_RA5);
   G_registers_defined = true;
}

typedef enum : i32 {
   TT_Eof,
   TT_Instr,
   TT_Reg,
   TT_IntLiteral,
   TT_Identifier
} TokenType;

typedef struct {
   TokenType type;
   u32 z;

   union {
      StringView identifier;
      InstrKind instr;
      InstrRegister reg;
      i64 int_literal;
   };
} Token;

typedef enum {
   LM_Trim,
   LM_Identifier,
   LM_IntLiteral,
   LM_HexLiteral,
   LM_Comment
} LexerMode;

typedef struct {
   Vector new_line_offsets;
   cstr file_path;
   cstr file_contents;
   u32 z;
} Lexer;

typedef struct {
   u32 y;
   u32 x;
} Loc;

static Lexer Lexer_new(cstr path) {
   mcu_assert(path != nullptr, "path can't be null");

   Lexer self = {
      .new_line_offsets = Vector_new(sizeof(u32)),
      .file_path = path,
   };
   Vector_push_create(&self.new_line_offsets, ((u32) 0));

   FILE* file_handle = fopen(self.file_path, "rb");
   if (file_handle == nullptr || fseek(file_handle, 0, SEEK_END))
      goto file_io_failure;

   isize file_size_tmp = (isize) ftell(file_handle);
   if (file_size_tmp < 0) goto file_io_failure;
   rewind(file_handle);

   usize file_size = (usize) file_size_tmp;
   self.file_contents = mcu_malloc(file_size + 1);
   self.file_contents[file_size] = EOF;
   if (fread(self.file_contents, 1, file_size, file_handle) < file_size)
      goto file_io_failure;
   fclose(file_handle);

   check_define_instructions();
   check_define_registers();

   return self;

file_io_failure:
   panic("[!] Failed to read from file \"%s\", reason: \"%s\"", strerror(errno));
   return self;
}

static void Lexer_delete(Lexer* self) {
   mcu_assert(self != nullptr, "self can't be null");

   Vector_free(&self->new_line_offsets);
   mcu_free(self->file_contents);
   *self = (Lexer) {0};
}

static bool is_identifier_allowed(char c) {
   return
      (c >= 'a' && c <= 'z') ||
      (c >= 'A' && c <= 'Z') ||
      (c >= '0' && c <= '9');
}

static Token Lexer_next(Lexer* self) {
   mcu_assert(self != nullptr, "self can't be null");

   LexerMode mode = LM_Trim;
   StringView accumulated;
   Token token = {
      .type = TT_Eof,
      .z = self->z
   };

   bool int_as_negative = false;

   loop {
      char current = self->file_contents[self->z++];
      char peek = self->file_contents[self->z];
      if (current == EOF) break;
      if (current == '\n') {
         Vector_push_create(&self->new_line_offsets, (self->z - 1));
      }

   reparse_char:
      switch (mode) {
         case LM_Trim: {
            token.z = self->z - 1;
            switch (current) {
               case ',':  break;
               case ' ':  break;
               case '\n': break;

               case '/': {
                  if (peek == '/')
                     mode = LM_Comment;
               } break;

               case '-': {
                  if (peek >= '0' && peek <= '9') {
                     mode = LM_IntLiteral;
                     int_as_negative = true;
                  }
               } break;

               default: {
                  if (current >= '0' && current <= '9') {
                     if (peek == 'x') {
                        mode = LM_HexLiteral;
                        self->z++;
                        continue;
                     } else {
                        mode = LM_IntLiteral;
                     }
                     int_as_negative = false;
                  } else {
                     mode = LM_Identifier;
                     accumulated = (StringView) {
                        .chars = self->file_contents + token.z
                     };
                  }
                  goto reparse_char;
               }
            }
         } continue;

         case LM_Identifier: {
            accumulated.length++;

            if (!is_identifier_allowed(peek)) {
               char tmp = accumulated.chars[accumulated.length];
               accumulated.chars[accumulated.length] = '\0';

               InstrKind* instr = HashMap_get(InstrKind)(&G_instructions, accumulated.chars);
               if (instr != nullptr) {
                  accumulated.chars[accumulated.length] = tmp;
                  token.type = TT_Instr,
                  token.instr = *instr;
                  return token;
               }

               InstrRegister* reg = HashMap_get(InstrRegister)(&G_registers, accumulated.chars);
               if (reg != nullptr) {
                  accumulated.chars[accumulated.length] = tmp;
                  token.type = TT_Reg,
                  token.reg = *reg;
                  return token;
               }

               accumulated.chars[accumulated.length] = tmp;
               token.type = TT_Identifier;
               token.identifier = accumulated;
               return token;
            }
         } continue;

         case LM_IntLiteral: {
            token.int_literal *= 10;
            token.int_literal += current - '0';

            if (!(peek >= '0' && peek <= '9')) {
               token.type = TT_IntLiteral;
               if (int_as_negative)
                  token.int_literal = -token.int_literal;
               return token;
            }
         } continue;

         case LM_HexLiteral: {
            token.int_literal *= 16;
            if      (current >= '0' && current <= '9') token.int_literal += (current - '0');
            else if (current >= 'a' && current <= 'f') token.int_literal += (current - 'a') + 10;
            else if (current >= 'A' && current <= 'F') token.int_literal += (current - 'a') + 10;

            if (!(peek >= '0' && peek <= '9') && !(peek >= 'a' && peek <= 'f') && !(peek >= 'A' && peek <= 'F')) {
               token.type = TT_IntLiteral;
               if (int_as_negative)
                  token.int_literal = -token.int_literal;
               return token;
            }
         } continue;

         case LM_Comment: {
            if (current == '\n')
               mode = LM_Trim;
         } continue;
      }

      panic("unreachable");
   }

   return token;
}

static Loc Loc_from_offset(Lexer* lexer, u32 z) {
   mcu_assert(lexer != nullptr, "lexer can't be null");

   Loc self = {
      .y = 1,
      .x = z
   };

   u32 prev_i = 0;
   foreach (lexer->new_line_offsets, i) {
      u32 zi = *(u32*) Vector_get(&lexer->new_line_offsets, i);

      if (z < zi) {
         self.y = i + 1;
         self.x -= prev_i;
         return self;
      }

      if (i + 1 >= lexer->new_line_offsets.length) {
         self.y = i + 1;
         self.x -= zi;
         return self;
      }

      prev_i = zi;
   }

   return self;
}

static void Token_debug_print(Token self, Lexer* lexer) {
   mcu_assert(lexer != nullptr, "lexer can't be null");

   Loc loc = Loc_from_offset(lexer, self.z);
   printf("%s:%u:%u: info: ", lexer->file_path, loc.y, loc.x);

   switch (self.type) {
      case TT_Eof:        println("Eof");                                       return;
      case TT_Instr:      println("Instr (%s)", InstrKind_to_cstr(self.instr)); return;
      case TT_Reg:        println("Reg (%s)", InstrRegister_to_cstr(self.reg)); return;
      case TT_IntLiteral: println("IntLiteral (%ld)", self.int_literal);        return;
      case TT_Identifier: {
         println("Identifier (%.*s)", (i32) self.identifier.length, self.identifier.chars);
      } return;
   }

   panic("unreachable");
}

void load_program_from_file(Assembler* self, cstr path, u32 offset) {
   mcu_assert(self != nullptr, "self can't be null");
   mcu_assert(path != nullptr, "path can't be null");

   self->write_head += offset;
   Lexer lexer = Lexer_new(path);

   Token token;
   do {
      token = Lexer_next(&lexer);
      Token_debug_print(token, &lexer);
   } while(token.type != TT_Eof);

   Lexer_delete(&lexer);
   return;
}

