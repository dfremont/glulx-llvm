; Include file with Glulx opcodes
; Use <!include ":glulx"
; This file is public domain.

; Opcodes
!define "nop" = !op 0x00,0
!define "add" = !op 0x10,3,\*
!define "sub" = !op 0x11,3,\*
!define "mul" = !op 0x12,3,\*
!define "div" = !op 0x13,3,\*
!define "mod" = !op 0x14,3,\*
!define "neg" = !op 0x15,2,\*
!define "bitand" = !op 0x18,3,\*
!define "bitor" = !op 0x19,3,\*
!define "bitxor" = !op 0x1A,3,\*
!define "bitnot" = !op 0x1B,2,\*
!define "shiftl" = !op 0x1C,3,\*
!define "sshiftr" = !op 0x1D,3,\*
!define "ushiftr" = !op 0x1E,3,\*
!define "jump" = !relop 0x20,1,\*
!define "jz" = !relop 0x22,2,\*
!define "jnz" = !relop 0x23,2,\*
!define "jeq" = !relop 0x24,3,\*
!define "jne" = !relop 0x25,3,\*
!define "jlt" = !relop 0x26,3,\*
!define "jge" = !relop 0x27,3,\*
!define "jgt" = !relop 0x28,3,\*
!define "jle" = !relop 0x29,3,\*
!define "jltu" = !relop 0x2A,3,\*
!define "jgeu" = !relop 0x2B,3,\*
!define "jgtu" = !relop 0x2C,3,\*
!define "jleu" = !relop 0x2D,3,\*
!define "call" = !op 0x30,3,\*
!define "return" = !op 0x31,1,\*
!define "catch" = !relop 0x32,2,\*
!define "throw" = !op 0x33,2,\*
!define "tailcall" = !op 0x34,2,\*
!define "copy" = !op 0x40,2,\*
!define "copys" = !op 0x41,2,\*
!define "copyb" = !op 0x42,2,\*
!define "sexs" = !op 0x44,2,\*
!define "sexb" = !op 0x45,2,\*
!define "aload" = !op 0x48,3,\*
!define "aloads" = !op 0x49,3,\*
!define "aloadb" = !op 0x4A,3,\*
!define "aloadbit" = !op 0x4B,3,\*
!define "astore" = !op 0x4C,3,\*
!define "astores" = !op 0x4D,3,\*
!define "astoreb" = !op 0x4E,3,\*
!define "astorebit" = !op 0x4F,3,\*
!define "stkcount" = !op 0x50,1,\*
!define "stkpeek" = !op 0x51,2,\*
!define "stkswap" = !op 0x52,0
!define "stkroll" = !op 0x53,2,\*
!define "stkcopy" = !op 0x54,1,\*
!define "streamchar" = !op 0x70,1,\*
!define "streamnum" = !op 0x71,1,\*
!define "streamstr" = !op 0x72,1,\*
!define "streamunichar" = !op 0x73,1,\*
!define "gestalt" = !op 0x100,3,\*
!define "debugtrap" = !op 0x101,1,\*
!define "getmemsize" = !op 0x102,1,\*
!define "setmemsize" = !op 0x103,2,\*
!define "jumpabs" = !op 0x104,1,\*
!define "random" = !op 0x110,2,\*
!define "setrandom" = !op 0x111,1,\*
!define "quit" = !op 0x120,0
!define "verify" = !op 0x121,1,\*
!define "restart" = !op 0x122,0
!define "save" = !op 0x123,2,\*
!define "restore" = !op 0x124,2,\*
!define "saveundo" = !op 0x125,1,\*
!define "restoreundo" = !op 0x126,1,\*
!define "protect" = !op 0x127,2,\*
!define "hasundo" = !op 0x128,1,\*
!define "discardundo" = !op 0x129,0
!define "glk" = !op 0x130,3,\*
!define "getstringtbl" = !op 0x140,1,\*
!define "setstringtbl" = !op 0x141,1,\*
!define "getiosys" = !op 0x148,2,\*
!define "setiosys" = !op 0x149,2,\*
!define "linearsearch" = !op 0x150,8,\*
!define "binarysearch" = !op 0x151,8,\*
!define "linkedsearch" = !op 0x152,7,\*
!define "callf" = !op 0x160,2,\*
!define "callfi" = !op 0x161,3,\*
!define "callfii" = !op 0x162,4,\*
!define "callfiii" = !op 0x163,5,\*
!define "mzero" = !op 0x170,2,\*
!define "mcopy" = !op 0x171,3,\*
!define "malloc" = !op 0x178,2,\*
!define "mfree" = !op 0x179,1,\*
!define "accelfunc" = !op 0x180,2,\*
!define "accelparam" = !op 0x181,2,\*
!define "numtof" = !op 0x190,2,\*
!define "ftonumz" = !op 0x191,2,\*
!define "ftonumn" = !op 0x192,2,\*
!define "ceil" = !op 0x198,2,\*
!define "floor" = !op 0x199,2,\*
!define "fadd" = !op 0x1A0,3,\*
!define "fsub" = !op 0x1A1,3,\*
!define "fmul" = !op 0x1A2,3,\*
!define "fdiv" = !op 0x1A3,3,\*
!define "fmod" = !op 0x1A4,4,\*
!define "sqrt" = !op 0x1A8,2,\*
!define "exp" = !op 0x1A9,2,\*
!define "log" = !op 0x1AA,2,\*
!define "pow" = !op 0x1AB,3,\*
!define "sin" = !op 0x1B0,2,\*
!define "cos" = !op 0x1B1,2,\*
!define "tan" = !op 0x1B2,2,\*
!define "asin" = !op 0x1B3,2,\*
!define "acos" = !op 0x1B4,2,\*
!define "atan" = !op 0x1B5,2,\*
!define "atan2" = !op 0x1B6,3,\*
!define "jfeq" = !relop 0x1C0,4,\*
!define "jfne" = !relop 0x1C1,4,\*
!define "jflt" = !relop 0x1C2,3,\*
!define "jfle" = !relop 0x1C3,3,\*
!define "jfgt" = !relop 0x1C4,3,\*
!define "jfge" = !relop 0x1C5,3,\*
!define "jisnan" = !relop 0x1C8,2,\*
!define "jisinf" = !relop 0x1C9,2,\*

; Shortcuts
!define "push" = !op 0x40,2,\1,$
!define "pull" = !op 0x40,2,$,\1

; Gestalt selectors
:gs.GlulxVersion !is 0
:gs.TerpVersion !is 1
:gs.ResizeMem !is 2
:gs.Undo !is 3
:gs.IOSystem !is 4
:gs.Unicode !is 5
:gs.MemCopy !is 6
:gs.MAlloc !is 7
:gs.MAllocHeap !is 8
:gs.Acceleration !is 9
:gs.AccelFunc !is 10
:gs.Float !is 11

; Types of objects
:ty.FuncS !is 0xC0
:ty.FuncL !is 0xC1
:ty.String !is 0xE0
:ty.HuffString !is 0xE1
:ty.UniString !is 0xE2

