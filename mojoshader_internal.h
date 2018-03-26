#ifndef _INCLUDE_MOJOSHADER_INTERNAL_H_
#define _INCLUDE_MOJOSHADER_INTERNAL_H_

#ifndef __MOJOSHADER_INTERNAL__
#error Do not include this header from your applications.
#endif

// Shader bytecode format is described at MSDN:
//  http://msdn.microsoft.com/en-us/library/ff569705.aspx

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>

#ifndef MSVC
#include <stdbool.h>
#endif

#include "mojoshader.h"

#define DEBUG_LEXER 0
#define DEBUG_PREPROCESSOR 0
#define DEBUG_ASSEMBLER_PARSER 0
#define DEBUG_COMPILER_PARSER 0
#define DEBUG_TOKENIZER \
    (DEBUG_PREPROCESSOR || DEBUG_ASSEMBLER_PARSER || DEBUG_LEXER)

#if (defined(__APPLE__) && defined(__MACH__))
#define PLATFORM_MACOSX 1
#endif

// This is the highest shader version we currently support.

#define MAX_SHADER_MAJOR 3
#define MAX_SHADER_MINOR 255  // vs_3_sw


// If SUPPORT_PROFILE_* isn't defined, we assume an implicit desire to support.
//  You get all the profiles unless you go out of your way to disable them.

#ifndef SUPPORT_PROFILE_D3D
#define SUPPORT_PROFILE_D3D 1
#endif

#ifndef SUPPORT_PROFILE_BYTECODE
#define SUPPORT_PROFILE_BYTECODE 1
#endif

#ifndef SUPPORT_PROFILE_GLSL
#define SUPPORT_PROFILE_GLSL 1
#endif

#ifndef SUPPORT_PROFILE_GLSL120
#define SUPPORT_PROFILE_GLSL120 1
#endif

#ifndef SUPPORT_PROFILE_GLSLES
#define SUPPORT_PROFILE_GLSLES 1
#endif

#ifndef SUPPORT_PROFILE_ARB1
#define SUPPORT_PROFILE_ARB1 1
#endif

#ifndef SUPPORT_PROFILE_ARB1_NV
#define SUPPORT_PROFILE_ARB1_NV 1
#endif

#ifndef SUPPORT_PROFILE_METAL
#define SUPPORT_PROFILE_METAL 1
#endif

#if SUPPORT_PROFILE_ARB1_NV && !SUPPORT_PROFILE_ARB1
#error nv profiles require arb1 profile. Fix your build.
#endif

#if SUPPORT_PROFILE_GLSL120 && !SUPPORT_PROFILE_GLSL
#error glsl120 profile requires glsl profile. Fix your build.
#endif

#if SUPPORT_PROFILE_GLSLES && !SUPPORT_PROFILE_GLSL
#error glsles profile requires glsl profile. Fix your build.
#endif

// Likewise, if SUPPORT_FORMAT_* isn't defined, we assume an implicit desire to support.
// Note that the "default" format is always supported and that format support is a horrible mess.

#ifndef SUPPORT_FORMAT_XENOS
#define SUPPORT_FORMAT_XENOS 1
#endif

// Microsoft's preprocessor has some quirks. In some ways, it doesn't work
//  like you'd expect a C preprocessor to function.
#ifndef MATCH_MICROSOFT_PREPROCESSOR
#define MATCH_MICROSOFT_PREPROCESSOR 1
#endif

// Other stuff you can disable...

#ifdef MOJOSHADER_EFFECT_SUPPORT
void MOJOSHADER_runPreshader(const MOJOSHADER_preshader*, float*);
#endif


// Get basic wankery out of the way here...

#ifdef _WINDOWS
#define ENDLINE_STR "\r\n"
#else
#define ENDLINE_STR "\n"
#endif

typedef unsigned int uint;  // this is a printf() helper. don't use for code.

// Locale-independent float printing replacement for snprintf
size_t MOJOSHADER_printFloat(char *text, size_t maxlen, float arg);

#ifdef _MSC_VER
#include <malloc.h>
#define va_copy(a, b) a = b
#define snprintf _snprintf  // !!! FIXME: not a safe replacement!
#define vsnprintf _vsnprintf  // !!! FIXME: not a safe replacement!
#define strcasecmp stricmp
#define strncasecmp strnicmp
typedef unsigned __int8 uint8;
typedef unsigned __int16 uint16;
typedef unsigned __int32 uint32;
typedef unsigned __int64 uint64;
typedef __int32 int32;
typedef __int64 int64;
#ifdef _WIN64
typedef __int64 ssize_t;
#elif defined _WIN32
typedef __int32 ssize_t;
#else
#error Please define your platform.
#endif
// Warning Level 4 considered harmful.  :)
#pragma warning(disable: 4100)  // "unreferenced formal parameter"
#pragma warning(disable: 4389)  // "signed/unsigned mismatch"
#else
#include <stdint.h>
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef int32_t int32;
typedef int64_t int64;
typedef uint64_t uint64;
#endif

#ifdef sun
#include <alloca.h>
#endif

#ifdef __GNUC__
#define ISPRINTF(x,y) __attribute__((format (printf, x, y)))
#else
#define ISPRINTF(x,y)
#endif

#define STATICARRAYLEN(x) ( (sizeof ((x))) / (sizeof ((x)[0])) )


// Byteswap magic...

// Used only to swap from big to little endian.
// Useful when running on a big endian platform (PowerPC).
#if ((defined __GNUC__) && (defined __POWERPC__))
    static inline uint32 SWAP32(uint32 x)
    {
        __asm__ __volatile__("lwbrx %0,0,%1" : "=r" (x) : "r" (&x));
        return x;
    } // SWAP32
    static inline uint16 SWAP16(uint16 x)
    {
        __asm__ __volatile__("lhbrx %0,0,%1" : "=r" (x) : "r" (&x));
        return x;
    } // SWAP16
#elif defined(__POWERPC__)
    static inline uint32 SWAP32(uint32 x)
    {
        return ( (((x) >> 24) & 0x000000FF) | (((x) >>  8) & 0x0000FF00) |
                 (((x) <<  8) & 0x00FF0000) | (((x) << 24) & 0xFF000000) );
    } // SWAP32
    static inline uint16 SWAP16(uint16 x)
    {
        return ( (((x) >> 8) & 0x00FF) | (((x) << 8) & 0xFF00) );
    } // SWAP16
#else
#   define SWAP16(x) (x)
#   define SWAP32(x) (x)
#endif

#define SWAPDBL(x) (x)  // !!! FIXME

// Used to forcibly swap between big and little endian.
// Useful when reading X360 effects.
#if ((defined __GNUC__) && (defined __POWERPC__))
    static inline uint32 FSWAP32(uint32 x)
    {
        __asm__ __volatile__("lwbrx %0,0,%1" : "=r" (x) : "r" (&x));
        return x;
    } // FSWAP32
    static inline uint16 FSWAP16(uint16 x)
    {
        __asm__ __volatile__("lhbrx %0,0,%1" : "=r" (x) : "r" (&x));
        return x;
    } // FSWAP16
#else
    static inline uint32 FSWAP32(uint32 x)
    {
        return ( (((x) >> 24) & 0x000000FF) | (((x) >>  8) & 0x0000FF00) |
                 (((x) <<  8) & 0x00FF0000) | (((x) << 24) & 0xFF000000) );
    } // FSWAP32
    static inline uint16 FSWAP16(uint16 x)
    {
        return ( (((x) >> 8) & 0x00FF) | (((x) << 8) & 0xFF00) );
    } // FSWAP16
#endif

#define FSWAPDBL(x) (x)  // !!! FIXME

// Helpers to automatically FSWAP based on ctx->big_endian and just SWAP on PowerPC.
#if (defined __POWERPC__)
#define CTXSWAP16(x) (ctx->big_endian ? (x) : SWAP16(x))
#define CTXSWAP32(x) (ctx->big_endian ? (x) : SWAP32(x))
#define CTXSWAPDBL(x) (ctx->big_endian ? (x) : SWAPDBL(x))
#else
#define CTXSWAP16(x) (ctx->big_endian ? FSWAP16(x) : (x))
#define CTXSWAP32(x) (ctx->big_endian ? FSWAP32(x) : (x))
#define CTXSWAPDBL(x) (ctx->big_endian ? FSWAPDBL(x) : (x))
#endif

static inline int Min(const int a, const int b)
{
    return ((a < b) ? a : b);
} // Min


// Hashtables...

typedef struct HashTable HashTable;
typedef uint32 (*HashTable_HashFn)(const void *key, void *data);
typedef int (*HashTable_KeyMatchFn)(const void *a, const void *b, void *data);
typedef void (*HashTable_NukeFn)(const void *key, const void *value, void *data);

HashTable *hash_create(void *data, const HashTable_HashFn hashfn,
                       const HashTable_KeyMatchFn keymatchfn,
                       const HashTable_NukeFn nukefn,
                       const int stackable,
                       MOJOSHADER_malloc m, MOJOSHADER_free f, void *d);
void hash_destroy(HashTable *table);
int hash_insert(HashTable *table, const void *key, const void *value);
int hash_remove(HashTable *table, const void *key);
int hash_find(const HashTable *table, const void *key, const void **_value);
int hash_iter(const HashTable *table, const void *key, const void **_value, void **iter);
int hash_iter_keys(const HashTable *table, const void **_key, void **iter);

uint32 hash_hash_string(const void *sym, void *unused);
int hash_keymatch_string(const void *a, const void *b, void *unused);


// String -> String map ...
typedef HashTable StringMap;
StringMap *stringmap_create(const int copy, MOJOSHADER_malloc m,
                            MOJOSHADER_free f, void *d);
void stringmap_destroy(StringMap *smap);
int stringmap_insert(StringMap *smap, const char *key, const char *value);
int stringmap_remove(StringMap *smap, const char *key);
int stringmap_find(const StringMap *smap, const char *key, const char **_val);


// String caching...

typedef struct StringCache StringCache;
StringCache *stringcache_create(MOJOSHADER_malloc m,MOJOSHADER_free f,void *d);
const char *stringcache(StringCache *cache, const char *str);
const char *stringcache_len(StringCache *cache, const char *str,
                            const unsigned int len);
const char *stringcache_fmt(StringCache *cache, const char *fmt, ...);
int stringcache_iscached(StringCache *cache, const char *str);
void stringcache_destroy(StringCache *cache);


// Error lists...

typedef struct ErrorList ErrorList;
ErrorList *errorlist_create(MOJOSHADER_malloc m, MOJOSHADER_free f, void *d);
int errorlist_add(ErrorList *list, const char *fname,
                      const int errpos, const char *str);
int errorlist_add_fmt(ErrorList *list, const char *fname,
                      const int errpos, const char *fmt, ...) ISPRINTF(4,5);
int errorlist_add_va(ErrorList *list, const char *_fname,
                     const int errpos, const char *fmt, va_list va);
int errorlist_count(ErrorList *list);
MOJOSHADER_error *errorlist_flatten(ErrorList *list); // resets the list!
void errorlist_destroy(ErrorList *list);



// Dynamic buffers...

typedef struct Buffer Buffer;
Buffer *buffer_create(size_t blksz,MOJOSHADER_malloc m,MOJOSHADER_free f,void *d);
char *buffer_reserve(Buffer *buffer, const size_t len);
int buffer_append(Buffer *buffer, const void *_data, size_t len);
int buffer_append_fmt(Buffer *buffer, const char *fmt, ...) ISPRINTF(2,3);
int buffer_append_va(Buffer *buffer, const char *fmt, va_list va);
size_t buffer_size(Buffer *buffer);
void buffer_empty(Buffer *buffer);
char *buffer_flatten(Buffer *buffer);
char *buffer_merge(Buffer **buffers, const size_t n, size_t *_len);
void buffer_destroy(Buffer *buffer);
ssize_t buffer_find(Buffer *buffer, const size_t start,
                    const void *data, const size_t len);



// This is the ID for a D3DXSHADER_CONSTANTTABLE in the bytecode comments.
#define CTAB_ID 0x42415443  // 0x42415443 == 'CTAB'
#define CTAB_SIZE 28  // sizeof (D3DXSHADER_CONSTANTTABLE).
#define CINFO_SIZE 20  // sizeof (D3DXSHADER_CONSTANTINFO).
#define CTYPEINFO_SIZE 16  // sizeof (D3DXSHADER_TYPEINFO).
#define CMEMBERINFO_SIZE 8  // sizeof (D3DXSHADER_STRUCTMEMBERINFO)

// Preshader magic values...
#define PRES_ID 0x53455250  // 0x53455250 == 'PRES'
#define PRSI_ID 0x49535250  // 0x49535250 == 'PRSI'
#define CLIT_ID 0x54494C43  // 0x54494C43 == 'CLIT'
#define FXLC_ID 0x434C5846  // 0x434C5846 == 'FXLC'

// we need to reference these by explicit value occasionally...
#define OPCODE_NOP 0
#define OPCODE_MOV 1
#define OPCODE_MUL 5
#define OPCODE_RET 28
#define OPCODE_IF 40
#define OPCODE_IFC 41
#define OPCODE_ELSE 42
#define OPCODE_ENDIF 43
#define OPCODE_BREAK 44
#define OPCODE_BREAKC 45
#define OPCODE_TEXLD 66
#define OPCODE_SETP 94

#define OPCODE_XENOS_VFETCH 0
#define OPCODE_XENOS_TFETCH 1
#define OPCODE_XENOS_TFETCH_GETBCF 16
#define OPCODE_XENOS_TFETCH_GETCOMPTEXLOD 17
#define OPCODE_XENOS_TFETCH_GETGRADIENTS 18
#define OPCODE_XENOS_TFETCH_GETWEIGHTS 19
#define OPCODE_XENOS_TFETCH_SETTEXLOD 24
#define OPCODE_XENOS_TFETCH_SETGRADIENTH 25
#define OPCODE_XENOS_TFETCH_SETGRADIENTV 26
#define OPCODE_XENOS_TFETCH_UNKNOWN 27

// TEXLD becomes a different instruction with these instruction controls.
#define CONTROL_TEXLD  0
#define CONTROL_TEXLDP 1
#define CONTROL_TEXLDB 2

// #define this to force app to supply an allocator, so there's no reference
//  to the C runtime's malloc() and free()...
#if MOJOSHADER_FORCE_ALLOCATOR
#define MOJOSHADER_internal_malloc NULL
#define MOJOSHADER_internal_free NULL
#else
void * MOJOSHADERCALL MOJOSHADER_internal_malloc(int bytes, void *d);
void MOJOSHADERCALL MOJOSHADER_internal_free(void *ptr, void *d);
#endif

#if MOJOSHADER_FORCE_INCLUDE_CALLBACKS
#define MOJOSHADER_internal_include_open NULL
#define MOJOSHADER_internal_include_close NULL
#else
int MOJOSHADER_internal_include_open(MOJOSHADER_includeType inctype,
                                     const char *fname, const char *parent,
                                     const char **outdata,
                                     unsigned int *outbytes,
                                     MOJOSHADER_malloc m, MOJOSHADER_free f,
                                     void *d);

void MOJOSHADER_internal_include_close(const char *data, MOJOSHADER_malloc m,
                                       MOJOSHADER_free f, void *d);
#endif


// result modifiers.
// !!! FIXME: why isn't this an enum?
#define MOD_SATURATE 0x01
#define MOD_PP 0x02
#define MOD_CENTROID 0x04

typedef enum
{
    REG_TYPE_TEMP = 0,
    REG_TYPE_INPUT = 1,
    REG_TYPE_CONST = 2,
    REG_TYPE_ADDRESS = 3,
    REG_TYPE_TEXTURE = 3,  // ALSO 3!
    REG_TYPE_RASTOUT = 4,
    REG_TYPE_ATTROUT = 5,
    REG_TYPE_TEXCRDOUT = 6,
    REG_TYPE_OUTPUT = 6,  // ALSO 6!
    REG_TYPE_CONSTINT = 7,
    REG_TYPE_COLOROUT = 8,
    REG_TYPE_DEPTHOUT = 9,
    REG_TYPE_SAMPLER = 10,
    REG_TYPE_CONST2 = 11,
    REG_TYPE_CONST3 = 12,
    REG_TYPE_CONST4 = 13,
    REG_TYPE_CONSTBOOL = 14,
    REG_TYPE_LOOP = 15,
    REG_TYPE_TEMPFLOAT16 = 16,
    REG_TYPE_MISCTYPE = 17,
    REG_TYPE_LABEL = 18,
    REG_TYPE_PREDICATE = 19,
    REG_TYPE_MAX = 19
} RegisterType;

typedef enum
{
    TEXTURE_TYPE_2D = 2,
    TEXTURE_TYPE_CUBE = 3,
    TEXTURE_TYPE_VOLUME = 4,
} TextureType;

typedef enum
{
    RASTOUT_TYPE_POSITION = 0,
    RASTOUT_TYPE_FOG = 1,
    RASTOUT_TYPE_POINT_SIZE = 2,
    RASTOUT_TYPE_MAX = 2
} RastOutType;

typedef enum
{
    MISCTYPE_TYPE_POSITION = 0,
    MISCTYPE_TYPE_FACE = 1,
    MISCTYPE_TYPE_MAX = 1
} MiscTypeType;

// source modifiers.
typedef enum
{
    SRCMOD_NONE,
    SRCMOD_NEGATE,
    SRCMOD_BIAS,
    SRCMOD_BIASNEGATE,
    SRCMOD_SIGN,
    SRCMOD_SIGNNEGATE,
    SRCMOD_COMPLEMENT,
    SRCMOD_X2,
    SRCMOD_X2NEGATE,
    SRCMOD_DZ,
    SRCMOD_DW,
    SRCMOD_ABS,
    SRCMOD_ABSNEGATE,
    SRCMOD_NOT,
    SRCMOD_TOTAL
} SourceMod;


typedef struct
{
    const uint32 *token;   // this is the unmolested token in the stream.
    int regnum;
    int relative;
    int writemask;   // xyzw or rgba (all four, not split out).
    int writemask0;  // x or red
    int writemask1;  // y or green
    int writemask2;  // z or blue
    int writemask3;  // w or alpha
    int orig_writemask;   // writemask before mojoshader tweaks it.
    int result_mod;
    int result_shift;
    RegisterType regtype;
} DestArgInfo;

// NOTE: This will NOT know a dcl_psize or dcl_fog output register should be
//        scalar! This function doesn't have access to that information.
static inline int scalar_register(const MOJOSHADER_shaderType shader_type,
                                  const RegisterType regtype, const int regnum)
{
    switch (regtype)
    {
        case REG_TYPE_RASTOUT:
            if (((const RastOutType) regnum) == RASTOUT_TYPE_FOG)
                return 1;
            else if (((const RastOutType) regnum) == RASTOUT_TYPE_POINT_SIZE)
                return 1;
            return 0;

        case REG_TYPE_DEPTHOUT:
        case REG_TYPE_CONSTBOOL:
        case REG_TYPE_LOOP:
            return 1;

        case REG_TYPE_MISCTYPE:
            if ( ((const MiscTypeType) regnum) == MISCTYPE_TYPE_FACE )
                return 1;
            return 0;

        case REG_TYPE_PREDICATE:
            return (shader_type == MOJOSHADER_TYPE_PIXEL) ? 1 : 0;

        default: break;
    } // switch

    return 0;
} // scalar_register


extern MOJOSHADER_error MOJOSHADER_out_of_mem_error;
extern MOJOSHADER_parseData MOJOSHADER_out_of_mem_data;


// preprocessor stuff.

typedef enum
{
    TOKEN_UNKNOWN = 256,  // start past ASCII character values.

    // These are all C-like constructs. Tokens < 256 may be single
    //  chars (like '+' or whatever). These are just multi-char sequences
    //  (like "+=" or whatever).
    TOKEN_IDENTIFIER,
    TOKEN_INT_LITERAL,
    TOKEN_FLOAT_LITERAL,
    TOKEN_STRING_LITERAL,
    TOKEN_RSHIFTASSIGN,
    TOKEN_LSHIFTASSIGN,
    TOKEN_ADDASSIGN,
    TOKEN_SUBASSIGN,
    TOKEN_MULTASSIGN,
    TOKEN_DIVASSIGN,
    TOKEN_MODASSIGN,
    TOKEN_XORASSIGN,
    TOKEN_ANDASSIGN,
    TOKEN_ORASSIGN,
    TOKEN_INCREMENT,
    TOKEN_DECREMENT,
    TOKEN_RSHIFT,
    TOKEN_LSHIFT,
    TOKEN_ANDAND,
    TOKEN_OROR,
    TOKEN_LEQ,
    TOKEN_GEQ,
    TOKEN_EQL,
    TOKEN_NEQ,
    TOKEN_HASH,
    TOKEN_HASHHASH,

    // This is returned if the preprocessor isn't stripping comments. Note
    //  that in asm files, the ';' counts as a single-line comment, same as
    //  "//". Note that both eat newline tokens: all of the ones inside a
    //  multiline comment, and the ending newline on a single-line comment.
    TOKEN_MULTI_COMMENT,
    TOKEN_SINGLE_COMMENT,

    // This is returned at the end of input...no more to process.
    TOKEN_EOI,

    // This is returned for char sequences we think are bogus. You'll have
    //  to judge for yourself. In most cases, you'll probably just fail with
    //  bogus syntax without explicitly checking for this token.
    TOKEN_BAD_CHARS,

    // This is returned if there's an error condition (the error is returned
    //  as a NULL-terminated string from preprocessor_nexttoken(), instead
    //  of actual token data). You can continue getting tokens after this
    //  is reported. It happens for things like missing #includes, etc.
    TOKEN_PREPROCESSING_ERROR,

    // These are all caught by the preprocessor. Caller won't ever see them,
    //  except TOKEN_PP_PRAGMA.
    //  They control the preprocessor (#includes new files, etc).
    TOKEN_PP_INCLUDE,
    TOKEN_PP_LINE,
    TOKEN_PP_DEFINE,
    TOKEN_PP_UNDEF,
    TOKEN_PP_IF,
    TOKEN_PP_IFDEF,
    TOKEN_PP_IFNDEF,
    TOKEN_PP_ELSE,
    TOKEN_PP_ELIF,
    TOKEN_PP_ENDIF,
    TOKEN_PP_ERROR,  // caught, becomes TOKEN_PREPROCESSING_ERROR
    TOKEN_PP_PRAGMA,
    TOKEN_INCOMPLETE_COMMENT,  // caught, becomes TOKEN_PREPROCESSING_ERROR
    TOKEN_PP_UNARY_MINUS,  // used internally, never returned.
    TOKEN_PP_UNARY_PLUS,   // used internally, never returned.
} Token;


// This is opaque.
struct Preprocessor;
typedef struct Preprocessor Preprocessor;

typedef struct Conditional
{
    Token type;
    int linenum;
    int skipping;
    int chosen;
    struct Conditional *next;
} Conditional;

typedef struct Define
{
    const char *identifier;
    const char *definition;
    const char *original;
    const char **parameters;
    int paramcount;
    struct Define *next;
} Define;

typedef struct IncludeState
{
    const char *filename;
    const char *source_base;
    const char *source;
    const char *token;
    unsigned int tokenlen;
    Token tokenval;
    int pushedback;
    const unsigned char *lexer_marker;
    int report_whitespace;
    int report_comments;
    int asm_comments;
    unsigned int orig_length;
    unsigned int bytes_left;
    unsigned int line;
    Conditional *conditional_stack;
    MOJOSHADER_includeClose close_callback;
    struct IncludeState *next;
} IncludeState;

Token preprocessor_lexer(IncludeState *s);

// This will only fail if the allocator fails, so it doesn't return any
//  error code...NULL on failure.
Preprocessor *preprocessor_start(const char *fname, const char *source,
                            unsigned int sourcelen,
                            MOJOSHADER_includeOpen open_callback,
                            MOJOSHADER_includeClose close_callback,
                            const MOJOSHADER_preprocessorDefine *defines,
                            unsigned int define_count, int asm_comments,
                            MOJOSHADER_malloc m, MOJOSHADER_free f, void *d);

void preprocessor_end(Preprocessor *pp);
int preprocessor_outofmemory(Preprocessor *pp);
const char *preprocessor_nexttoken(Preprocessor *_ctx,
                                   unsigned int *_len, Token *_token);
const char *preprocessor_sourcepos(Preprocessor *pp, unsigned int *pos);


void MOJOSHADER_print_debug_token(const char *subsystem, const char *token,
                                  const unsigned int tokenlen,
                                  const Token tokenval);

#endif  // _INCLUDE_MOJOSHADER_INTERNAL_H_


#if MOJOSHADER_DO_INSTRUCTION_TABLE
// These have to be in the right order! Arrays are indexed by the value
//  of the instruction token.

// INSTRUCTION_STATE means this opcode has to update the state machine
//  (we're entering an ELSE block, etc). INSTRUCTION means there's no
//  state, just go straight to the emitters.

// !!! FIXME: Some of these MOJOSHADER_TYPE_ANYs need to have their scope
// !!! FIXME:  reduced to just PIXEL or VERTEX.

INSTRUCTION(NOP, "NOP", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(MOV, "MOV", 1, DS, MOJOSHADER_TYPE_ANY)
INSTRUCTION(ADD, "ADD", 1, DSS, MOJOSHADER_TYPE_ANY)
INSTRUCTION(SUB, "SUB", 1, DSS, MOJOSHADER_TYPE_ANY)
INSTRUCTION(MAD, "MAD", 1, DSSS, MOJOSHADER_TYPE_ANY)
INSTRUCTION(MUL, "MUL", 1, DSS, MOJOSHADER_TYPE_ANY)
INSTRUCTION_STATE(RCP, "RCP", 1, DS, MOJOSHADER_TYPE_ANY)
INSTRUCTION(RSQ, "RSQ", 1, DS, MOJOSHADER_TYPE_ANY)
INSTRUCTION(DP3, "DP3", 1, DSS, MOJOSHADER_TYPE_ANY)
INSTRUCTION_STATE(DP4, "DP4", 1, DSS, MOJOSHADER_TYPE_ANY)
INSTRUCTION(MIN, "MIN", 1, DSS, MOJOSHADER_TYPE_ANY)
INSTRUCTION(MAX, "MAX", 1, DSS, MOJOSHADER_TYPE_ANY)
INSTRUCTION(SLT, "SLT", 1, DSS, MOJOSHADER_TYPE_ANY)
INSTRUCTION(SGE, "SGE", 1, DSS, MOJOSHADER_TYPE_ANY)
INSTRUCTION(EXP, "EXP", 1, DS, MOJOSHADER_TYPE_ANY)
INSTRUCTION_STATE(LOG, "LOG", 1, DS, MOJOSHADER_TYPE_ANY)
INSTRUCTION(LIT, "LIT", 3, DS, MOJOSHADER_TYPE_ANY)
INSTRUCTION(DST, "DST", 1, DSS, MOJOSHADER_TYPE_VERTEX)
INSTRUCTION(LRP, "LRP", 2, DSSS, MOJOSHADER_TYPE_ANY)
INSTRUCTION_STATE(FRC, "FRC", 1, DS, MOJOSHADER_TYPE_ANY)
INSTRUCTION_STATE(M4X4, "M4X4", 4, DSS, MOJOSHADER_TYPE_ANY)
INSTRUCTION_STATE(M4X3, "M4X3", 3, DSS, MOJOSHADER_TYPE_ANY)
INSTRUCTION_STATE(M3X4, "M3X4", 4, DSS, MOJOSHADER_TYPE_ANY)
INSTRUCTION_STATE(M3X3, "M3X3", 3, DSS, MOJOSHADER_TYPE_ANY)
INSTRUCTION_STATE(M3X2, "M3X2", 2, DSS, MOJOSHADER_TYPE_ANY)
INSTRUCTION_STATE(CALL, "CALL", 2, S, MOJOSHADER_TYPE_ANY)
INSTRUCTION_STATE(CALLNZ, "CALLNZ", 3, SS, MOJOSHADER_TYPE_ANY)
INSTRUCTION_STATE(LOOP, "LOOP", 3, SS, MOJOSHADER_TYPE_ANY)
INSTRUCTION_STATE(RET, "RET", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_STATE(ENDLOOP, "ENDLOOP", 2, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_STATE(LABEL, "LABEL", 0, S, MOJOSHADER_TYPE_ANY)
INSTRUCTION_STATE(DCL, "DCL", 0, DCL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_STATE(POW, "POW", 3, DSS, MOJOSHADER_TYPE_ANY)
INSTRUCTION(CRS, "CRS", 2, DSS, MOJOSHADER_TYPE_ANY)
INSTRUCTION(SGN, "SGN", 3, DSSS, MOJOSHADER_TYPE_ANY)
INSTRUCTION(ABS, "ABS", 1, DS, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NRM, "NRM", 3, DS, MOJOSHADER_TYPE_ANY)
INSTRUCTION_STATE(SINCOS, "SINCOS", 8, SINCOS, MOJOSHADER_TYPE_ANY)
INSTRUCTION_STATE(REP, "REP", 3, S, MOJOSHADER_TYPE_ANY)
INSTRUCTION_STATE(ENDREP, "ENDREP", 2, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_STATE(IF, "IF", 3, S, MOJOSHADER_TYPE_ANY)
INSTRUCTION_STATE(IFC, "IF", 3, SS, MOJOSHADER_TYPE_ANY)
INSTRUCTION(ELSE, "ELSE", 1, NULL, MOJOSHADER_TYPE_ANY)  // !!! FIXME: state!
INSTRUCTION(ENDIF, "ENDIF", 1, NULL, MOJOSHADER_TYPE_ANY) // !!! FIXME: state!
INSTRUCTION_STATE(BREAK, "BREAK", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_STATE(BREAKC, "BREAK", 3, SS, MOJOSHADER_TYPE_ANY)
INSTRUCTION_STATE(MOVA, "MOVA", 1, DS, MOJOSHADER_TYPE_VERTEX)
INSTRUCTION_STATE(DEFB, "DEFB", 0, DEFB, MOJOSHADER_TYPE_ANY)
INSTRUCTION_STATE(DEFI, "DEFI", 0, DEFI, MOJOSHADER_TYPE_ANY)
INSTRUCTION(RESERVED, 0, 0, NULL, MOJOSHADER_TYPE_UNKNOWN)
INSTRUCTION(RESERVED, 0, 0, NULL, MOJOSHADER_TYPE_UNKNOWN)
INSTRUCTION(RESERVED, 0, 0, NULL, MOJOSHADER_TYPE_UNKNOWN)
INSTRUCTION(RESERVED, 0, 0, NULL, MOJOSHADER_TYPE_UNKNOWN)
INSTRUCTION(RESERVED, 0, 0, NULL, MOJOSHADER_TYPE_UNKNOWN)
INSTRUCTION(RESERVED, 0, 0, NULL, MOJOSHADER_TYPE_UNKNOWN)
INSTRUCTION(RESERVED, 0, 0, NULL, MOJOSHADER_TYPE_UNKNOWN)
INSTRUCTION(RESERVED, 0, 0, NULL, MOJOSHADER_TYPE_UNKNOWN)
INSTRUCTION(RESERVED, 0, 0, NULL, MOJOSHADER_TYPE_UNKNOWN)
INSTRUCTION(RESERVED, 0, 0, NULL, MOJOSHADER_TYPE_UNKNOWN)
INSTRUCTION(RESERVED, 0, 0, NULL, MOJOSHADER_TYPE_UNKNOWN)
INSTRUCTION(RESERVED, 0, 0, NULL, MOJOSHADER_TYPE_UNKNOWN)
INSTRUCTION(RESERVED, 0, 0, NULL, MOJOSHADER_TYPE_UNKNOWN)
INSTRUCTION(RESERVED, 0, 0, NULL, MOJOSHADER_TYPE_UNKNOWN)
INSTRUCTION(RESERVED, 0, 0, NULL, MOJOSHADER_TYPE_UNKNOWN)
INSTRUCTION_STATE(TEXCRD, "TEXCRD", 1, TEXCRD, MOJOSHADER_TYPE_PIXEL)
INSTRUCTION_STATE(TEXKILL, "TEXKILL", 2, D, MOJOSHADER_TYPE_PIXEL)
INSTRUCTION_STATE(TEXLD, "TEXLD", 1, TEXLD, MOJOSHADER_TYPE_PIXEL)
INSTRUCTION_STATE(TEXBEM, "TEXBEM", 1, DS, MOJOSHADER_TYPE_PIXEL)
INSTRUCTION_STATE(TEXBEML, "TEXBEML", 2, DS, MOJOSHADER_TYPE_PIXEL)
INSTRUCTION(TEXREG2AR, "TEXREG2AR", 1, DS, MOJOSHADER_TYPE_PIXEL)
INSTRUCTION(TEXREG2GB, "TEXREG2GB", 1, DS, MOJOSHADER_TYPE_PIXEL)
INSTRUCTION_STATE(TEXM3X2PAD, "TEXM3X2PAD", 1, DS, MOJOSHADER_TYPE_PIXEL)
INSTRUCTION_STATE(TEXM3X2TEX, "TEXM3X2TEX", 1, DS, MOJOSHADER_TYPE_PIXEL)
INSTRUCTION_STATE(TEXM3X3PAD, "TEXM3X3PAD", 1, DS, MOJOSHADER_TYPE_PIXEL)
INSTRUCTION_STATE(TEXM3X3TEX, "TEXM3X3TEX", 1, DS, MOJOSHADER_TYPE_PIXEL)
INSTRUCTION(RESERVED, 0, 0, NULL, MOJOSHADER_TYPE_UNKNOWN)
INSTRUCTION_STATE(TEXM3X3SPEC, "TEXM3X3SPEC", 1, DSS, MOJOSHADER_TYPE_PIXEL)
INSTRUCTION_STATE(TEXM3X3VSPEC, "TEXM3X3VSPEC", 1, DS, MOJOSHADER_TYPE_PIXEL)
INSTRUCTION(EXPP, "EXPP", 1, DS, MOJOSHADER_TYPE_ANY)
INSTRUCTION_STATE(LOGP, "LOGP", 1, DS, MOJOSHADER_TYPE_ANY)
INSTRUCTION_STATE(CND, "CND", 1, DSSS, MOJOSHADER_TYPE_PIXEL)
INSTRUCTION_STATE(DEF, "DEF", 0, DEF, MOJOSHADER_TYPE_ANY)
INSTRUCTION(TEXREG2RGB, "TEXREG2RGB", 1, DS, MOJOSHADER_TYPE_PIXEL)
INSTRUCTION(TEXDP3TEX, "TEXDP3TEX", 1, DS, MOJOSHADER_TYPE_PIXEL)
INSTRUCTION(TEXM3X2DEPTH, "TEXM3X2DEPTH", 1, DS, MOJOSHADER_TYPE_PIXEL)
INSTRUCTION(TEXDP3, "TEXDP3", 1, DS, MOJOSHADER_TYPE_PIXEL)
INSTRUCTION_STATE(TEXM3X3, "TEXM3X3", 1, DS, MOJOSHADER_TYPE_PIXEL)
INSTRUCTION(TEXDEPTH, "TEXDEPTH", 1, D, MOJOSHADER_TYPE_PIXEL)
INSTRUCTION_STATE(CMP, "CMP", 1, DSSS, MOJOSHADER_TYPE_PIXEL)
INSTRUCTION(BEM, "BEM", 2, DSS, MOJOSHADER_TYPE_PIXEL)
INSTRUCTION_STATE(DP2ADD, "DP2ADD", 2, DSSS, MOJOSHADER_TYPE_PIXEL)
INSTRUCTION(DSX, "DSX", 2, DS, MOJOSHADER_TYPE_PIXEL)
INSTRUCTION(DSY, "DSY", 2, DS, MOJOSHADER_TYPE_PIXEL)
INSTRUCTION(TEXLDD, "TEXLDD", 3, DSSSS, MOJOSHADER_TYPE_PIXEL)
INSTRUCTION_STATE(SETP, "SETP", 1, DSS, MOJOSHADER_TYPE_ANY)
INSTRUCTION_STATE(TEXLDL, "TEXLDL", 2, DSS, MOJOSHADER_TYPE_ANY)
INSTRUCTION_STATE(BREAKP, "BREAKP", 3, S, MOJOSHADER_TYPE_ANY)
#endif

#if MOJOSHADER_DO_INSTRUCTION_X360_CF_TABLE
// These have to be in the right order! Arrays are indexed by the value
//  of the instruction token.

// Similar to MOJOSHADER_DO_INSTRUCTION_TABLE, based on information in Xenia. -ade

// Control flow instructions.
INSTRUCTION(NOP, "NOP", 0, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_EXEC, "EXEC", 0, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_EXECEND, "EXECEND", 0, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_CONDEXEC, "CONDEXEC", 0, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_CONDEXECEND, "CONDEXECEND", 0, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_CONDEXECPRED, "CONDEXECPRED", 0, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_CONDEXECPREDEND, "CONDEXECPREDEND", 0, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "LOOPSTART", 0, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "LOOPEND", 0, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_CONDCALL, "CONDCALL", 0, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(RET, "RET", 0, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_CONDJMP, "CONDJMP", 0, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_ALLOC, "ALLOC", 0, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "CONDEXECPREDCLEAN", 0, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "CONDEXECPREDCLEANEND", 0, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "MARKVFETCHDONE", 0, NULL, MOJOSHADER_TYPE_ANY)
#endif

#if MOJOSHADER_DO_INSTRUCTION_X360_ALUV_TABLE
// These have to be in the right order! Arrays are indexed by the value
//  of the instruction token.

// Similar to MOJOSHADER_DO_INSTRUCTION_TABLE, based on information in Xenia. -ade

// ALU vector instructions.
INSTRUCTION(NOP, "ADD", 2, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_MUL, "MUL", 2, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "MAX", 2, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "MIN", 2, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "SEQ", 2, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "SGT", 2, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "SGE", 2, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "SNE", 2, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "FRC", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "TRUNC", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "FLOOR", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "MAD", 3, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "CNDEQ", 3, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "CNDGE", 3, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "CNDGT", 3, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "DP4", 2, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "DP3", 2, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "DP2ADD", 3, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "CUBE", 2, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "MAX4", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "SETP_EQ_PUSH", 2, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "SETP_NE_PUSH", 2, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "SETP_GT_PUSH", 2, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "SETP_GE_PUSH", 2, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "KILL_EQ", 2, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "KILL_GT", 2, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "KILL_GE", 2, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "KILL_NE", 2, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "DST", 2, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "MAXA", 2, NULL, MOJOSHADER_TYPE_ANY)
#endif
#if false
INSTRUCTION_META(XENOS_ADD, "ADD", 2, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_MUL, "MUL", 2, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_MAX, "MAX", 2, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_MIN, "MIN", 2, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_SEQ, "SEQ", 2, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_SGT, "SGT", 2, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_SGE, "SGE", 2, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_SNE, "SNE", 2, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_FRC, "FRC", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_TRUNC, "TRUNC", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_FLOOR, "FLOOR", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_MAD, "MAD", 3, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_CNDEQ, "CNDEQ", 3, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_CNDGE, "CNDGE", 3, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_CNDGT, "CNDGT", 3, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_DP4, "DP4", 2, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_DP3, "DP3", 2, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_DP2ADD, "DP2ADD", 3, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_CUBE, "CUBE", 2, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_MAX4, "MAX4", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_SETP_EQ_PUSH, "SETP_EQ_PUSH", 2, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_SETP_NE_PUSH, "SETP_NE_PUSH", 2, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_SETP_GT_PUSH, "SETP_GT_PUSH", 2, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_SETP_GE_PUSH, "SETP_GE_PUSH", 2, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_KILL_EQ, "KILL_EQ", 2, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_KILL_GT, "KILL_GT", 2, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_KILL_GE, "KILL_GE", 2, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_KILL_NE, "KILL_NE", 2, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_DST, "DST", 2, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_MAXA, "MAXA", 2, NULL, MOJOSHADER_TYPE_ANY)
#endif

#if MOJOSHADER_DO_INSTRUCTION_X360_ALUS_TABLE
// These have to be in the right order! Arrays are indexed by the value
//  of the instruction token.

// Similar to MOJOSHADER_DO_INSTRUCTION_TABLE, based on information in Xenia. -ade

// ALU scalar instructions.
INSTRUCTION(NOP, "ADDS", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "ADDS_PREV", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "ADDS", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "ADDS_PREV", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "MULS", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "MULS_PREV", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "MULS_PREV2", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "MAXS", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "MINS", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "SEQS", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "SGTS", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "SGES", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "SNES", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "FRCS", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "TRUNCS", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "FLOORS", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "EXP", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "LOGC", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "LOG", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "RCPC", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "RCPF", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "RCP", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "RSQC", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "RSQF", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "RSQ", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "MAXAS", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "MAXASF", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "SUBS", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "SUBS_PREV", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "SETP_EQ", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "SETP_NE", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "SETP_GT", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "SETP_GE", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "SETP_INV", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "SETP_POP", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "SETP_CLR", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "SETP_RSTR", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "KILLS_EQ", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "KILLS_GT", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "KILLS_GE", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "KILLS_NE", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "KILLS_ONE", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "SQRT", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "UNKNOWN", 0, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "MULSC", 2, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "MULSC", 2, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "ADDSC", 2, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "ADDSC", 2, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "SUBSC", 2, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "SUBSC", 2, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "SIN", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "COS", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION(NOP, "RETAIN_PREV", 1, NULL, MOJOSHADER_TYPE_ANY)
#endif
#if false
INSTRUCTION_META(XENOS_ADDS, "ADDS", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_ADDS_PREV, "ADDS_PREV", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_ADDS, "ADDS", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_ADDS_PREV, "ADDS_PREV", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_MULS, "MULS", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_MULS_PREV, "MULS_PREV", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_MULS_PREV2, "MULS_PREV2", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_MAXS, "MAXS", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_MINS, "MINS", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_SEQS, "SEQS", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_SGTS, "SGTS", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_SGES, "SGES", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_SNES, "SNES", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_FRCS, "FRCS", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_TRUNCS, "TRUNCS", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_FLOORS, "FLOORS", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_EXP, "EXP", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_LOGC, "LOGC", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_LOG, "LOG", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_RCPC, "RCPC", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_RCPF, "RCPF", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_RCP, "RCP", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_RSQC, "RSQC", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_RSQF, "RSQF", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_RSQ, "RSQ", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_MAXAS, "MAXAS", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_MAXASF, "MAXASF", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_SUBS, "SUBS", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_SUBS_PREV, "SUBS_PREV", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_SETP_EQ, "SETP_EQ", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_SETP_NE, "SETP_NE", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_SETP_GT, "SETP_GT", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_SETP_GE, "SETP_GE", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_SETP_INV, "SETP_INV", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_SETP_POP, "SETP_POP", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_SETP_CLR, "SETP_CLR", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_SETP_RSTR, "SETP_RSTR", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_KILLS_EQ, "KILLS_EQ", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_KILLS_GT, "KILLS_GT", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_KILLS_GE, "KILLS_GE", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_KILLS_NE, "KILLS_NE", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_KILLS_ONE, "KILLS_ONE", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_SQRT, "SQRT", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_UNKNOWN, "UNKNOWN", 0, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_MULSC, "MULSC", 2, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_MULSC, "MULSC", 2, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_ADDSC, "ADDSC", 2, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_ADDSC, "ADDSC", 2, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_SUBSC, "SUBSC", 2, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_SUBSC, "SUBSC", 2, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_SIN, "SIN", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_COS, "COS", 1, NULL, MOJOSHADER_TYPE_ANY)
INSTRUCTION_META(XENOS_RETAIN_PREV, "RETAIN_PREV", 1, NULL, MOJOSHADER_TYPE_ANY)
#endif

// end of mojoshader_internal.h ...

