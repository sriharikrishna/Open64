/* C code produced by gperf version 2.7.2 */
/* Command-line: gperf -L C -F ', 0, 0' -p -j1 -i 1 -g -o -t -G -N is_reserved_word -k'1,3,4,5$' c-parse.gperf  */
/* Command-line: gperf -L KR-C -F ', 0, 0' -p -j1 -i 1 -g -o -t -N is_reserved_word -k1,3,$ c-parse.gperf  */ 
struct resword { const char *name; short token; enum rid rid; };

#define TOTAL_KEYWORDS 94
#define MIN_WORD_LENGTH 2
#define MAX_WORD_LENGTH 20
#define MIN_HASH_VALUE 6
#define MAX_HASH_VALUE 269
/* maximum key range = 264, duplicates = 0 */

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
static unsigned int
hash (str, len)
     register const char *str;
     register unsigned int len;
{
  static unsigned short asso_values[] =
    {
      270, 270, 270, 270, 270, 270, 270, 270, 270, 270,
      270, 270, 270, 270, 270, 270, 270, 270, 270, 270,
      270, 270, 270, 270, 270, 270, 270, 270, 270, 270,
      270, 270, 270, 270, 270, 270, 270, 270, 270, 270,
      270, 270, 270, 270, 270, 270, 270, 270, 270, 270,
      270, 270, 270, 270, 270, 270, 270, 270, 270, 270,
      270, 270, 270, 270,  41, 270, 270, 270, 270, 270,
      270, 270, 270, 270, 270, 270, 270, 270, 270, 270,
      270, 270, 270, 270, 270, 270, 270, 270, 270, 270,
      270, 270, 270, 270, 270,   1, 270,  63,  38,  18,
       96,   1,  36,  90,   4,   1, 270,   1,  12,  19,
       32,  13,   1, 270,   1,   7,   1,  62,  17,   2,
       19,  15,   3, 270, 270, 270, 270, 270, 270, 270,
      270, 270, 270, 270, 270, 270, 270, 270, 270, 270,
      270, 270, 270, 270, 270, 270, 270, 270, 270, 270,
      270, 270, 270, 270, 270, 270, 270, 270, 270, 270,
      270, 270, 270, 270, 270, 270, 270, 270, 270, 270,
      270, 270, 270, 270, 270, 270, 270, 270, 270, 270,
      270, 270, 270, 270, 270, 270, 270, 270, 270, 270,
      270, 270, 270, 270, 270, 270, 270, 270, 270, 270,
      270, 270, 270, 270, 270, 270, 270, 270, 270, 270,
      270, 270, 270, 270, 270, 270, 270, 270, 270, 270,
      270, 270, 270, 270, 270, 270, 270, 270, 270, 270,
      270, 270, 270, 270, 270, 270, 270, 270, 270, 270,
      270, 270, 270, 270, 270, 270, 270, 270, 270, 270,
      270, 270, 270, 270, 270, 270
    };
  register int hval = len;

  switch (hval)
    {
      default:
      case 5:
        hval += asso_values[(unsigned char)str[4]];
      case 4:
        hval += asso_values[(unsigned char)str[3]];
      case 3:
        hval += asso_values[(unsigned char)str[2]];
      case 2:
      case 1:
        hval += asso_values[(unsigned char)str[0]];
        break;
    }
  return hval + asso_values[(unsigned char)str[len - 1]];
}

static struct resword wordlist[] =
  {
    {"", 0, 0}, {"", 0, 0}, {"", 0, 0}, {"", 0, 0},
    {"", 0, 0}, {"", 0, 0},
    {"int", TYPESPEC, RID_INT},
    {"", 0, 0}, {"", 0, 0}, {"", 0, 0}, {"", 0, 0},
    {"", 0, 0}, {"", 0, 0}, {"", 0, 0},
    {"else", ELSE, NORID},
    {"__iterator", SCSPEC, RID_ITERATOR},
    {"", 0, 0},
    {"__iterator__", SCSPEC, RID_ITERATOR},
    {"out", TYPE_QUAL, RID_OUT},
    {"restrict", TYPE_QUAL, RID_RESTRICT},
    {"", 0, 0},
    {"__restrict", TYPE_QUAL, RID_RESTRICT},
    {"while", WHILE, NORID},
    {"__restrict__", TYPE_QUAL, RID_RESTRICT},
    {"", 0, 0}, {"", 0, 0}, {"", 0, 0}, {"", 0, 0},
    {"short", TYPESPEC, RID_SHORT},
    {"__typeof__", TYPEOF, NORID},
    {"", 0, 0},
    {"case", CASE, NORID},
    {"", 0, 0}, {"", 0, 0},
    {"strict", TYPE_QUAL, RID_STRICT},
    {"in", TYPE_QUAL, RID_IN},
    {"__extension__", EXTENSION, NORID},
    {"switch", SWITCH, NORID},
    {"", 0, 0},
    {"if", IF, NORID},
    {"", 0, 0},
    {"for", FOR, NORID},
    {"extern", SCSPEC, RID_EXTERN},
    {"", 0, 0}, {"", 0, 0}, {"", 0, 0}, {"", 0, 0},
    {"", 0, 0}, {"", 0, 0}, {"", 0, 0}, {"", 0, 0},
    {"", 0, 0}, {"", 0, 0},
    {"inline", SCSPEC, RID_INLINE},
    {"__volatile", TYPE_QUAL, RID_VOLATILE},
    {"__inline", SCSPEC, RID_INLINE},
    {"__volatile__", TYPE_QUAL, RID_VOLATILE},
    {"__inline__", SCSPEC, RID_INLINE},
    {"typeof", TYPEOF, NORID},
    {"", 0, 0}, {"", 0, 0},
    {"continue", CONTINUE, NORID},
    {"__typeof", TYPEOF, NORID},
    {"__complex__", TYPESPEC, RID_COMPLEX},
    {"const", TYPE_QUAL, RID_CONST},
    {"@selector", SELECTOR, NORID},
    {"sizeof", SIZEOF, NORID},
    {"", 0, 0}, {"", 0, 0},
    {"@private", PRIVATE, NORID},
    {"", 0, 0}, {"", 0, 0},
    {"__const", TYPE_QUAL, RID_CONST},
    {"", 0, 0},
    {"__const__", TYPE_QUAL, RID_CONST},
    {"__real__", REALPART, NORID},
    {"", 0, 0},
    {"@protocol", PROTOCOL, NORID},
    {"__attribute", ATTRIBUTE, NORID},
    {"__complex", TYPESPEC, RID_COMPLEX},
    {"__attribute__", ATTRIBUTE, NORID},
    {"", 0, 0}, {"", 0, 0},
    {"inout", TYPE_QUAL, RID_INOUT},
    {"__real", REALPART, NORID},
    {"", 0, 0},
    {"@interface", INTERFACE, NORID},
    {"char", TYPESPEC, RID_CHAR},
    {"", 0, 0},
    {"__alignof__", ALIGNOF, NORID},
    {"", 0, 0},
    {"bycopy", TYPE_QUAL, RID_BYCOPY},
    {"upc_wait", UPC_WAIT, NORID},
    {"__imag__", IMAGPART, NORID},
    {"auto", SCSPEC, RID_AUTO},
    {"struct", STRUCT, NORID},
    {"static", SCSPEC, RID_STATIC},
    {"@defs", DEFS, NORID},
    {"__asm__", ASM_KEYWORD, NORID},
    {"id", OBJECTNAME, RID_ID},
    {"oneway", TYPE_QUAL, RID_ONEWAY},
    {"@compatibility_alias", ALIAS, NORID},
    {"volatile", TYPE_QUAL, RID_VOLATILE},
    {"return", RETURN, NORID},
    {"asm", ASM_KEYWORD, NORID},
    {"enum", ENUM, NORID},
    {"", 0, 0}, {"", 0, 0},
    {"register", SCSPEC, RID_REGISTER},
    {"break", BREAK, NORID},
    {"__signed__", TYPESPEC, RID_SIGNED},
    {"do", DO, NORID},
    {"@encode", ENCODE, NORID},
    {"", 0, 0},
    {"__asm", ASM_KEYWORD, NORID},
    {"", 0, 0}, {"", 0, 0},
    {"byref", TYPE_QUAL, RID_BYREF},
    {"", 0, 0},
    {"float", TYPESPEC, RID_FLOAT},
    {"@implementation", IMPLEMENTATION, NORID},
    {"goto", GOTO, NORID},
    {"__alignof", ALIGNOF, NORID},
    {"", 0, 0},
    {"__label__", LABEL, NORID},
    {"", 0, 0}, {"", 0, 0},
    {"upc_fence", UPC_FENCE, NORID},
    {"", 0, 0}, {"", 0, 0}, {"", 0, 0},
    {"upc_barrier", UPC_BARRIER, NORID},
    {"upc_elemsizeof", UPC_ELEMSIZEOF, NORID},
    {"", 0, 0}, {"", 0, 0}, {"", 0, 0},
    {"@class", CLASS, NORID},
    {"", 0, 0},
    {"upc_notify", UPC_NOTIFY, NORID},
    {"upc_forall", FORALL, NORID},
    {"", 0, 0}, {"", 0, 0},
    {"typedef", SCSPEC, RID_TYPEDEF},
    {"", 0, 0},
    {"upc_localsizeof", UPC_LOCALSIZEOF, NORID},
    {"union", UNION, NORID},
    {"", 0, 0}, {"", 0, 0}, {"", 0, 0}, {"", 0, 0},
    {"", 0, 0}, {"", 0, 0}, {"", 0, 0}, {"", 0, 0},
    {"", 0, 0}, {"", 0, 0}, {"", 0, 0}, {"", 0, 0},
    {"", 0, 0}, {"", 0, 0}, {"", 0, 0}, {"", 0, 0},
    {"@protected", PROTECTED, NORID},
    {"", 0, 0}, {"", 0, 0}, {"", 0, 0}, {"", 0, 0},
    {"", 0, 0}, {"", 0, 0}, {"", 0, 0},
    {"upc_blocksizeof", UPC_BLOCKSIZEOF, NORID},
    {"", 0, 0}, {"", 0, 0}, {"", 0, 0},
    {"shared", TYPE_QUAL, RID_SHARED},
    {"", 0, 0}, {"", 0, 0}, {"", 0, 0},
    {"@public", PUBLIC, NORID},
    {"", 0, 0},
    {"__imag", IMAGPART, NORID},
    {"", 0, 0}, {"", 0, 0}, {"", 0, 0}, {"", 0, 0},
    {"", 0, 0}, {"", 0, 0}, {"", 0, 0}, {"", 0, 0},
    {"", 0, 0}, {"", 0, 0}, {"", 0, 0}, {"", 0, 0},
    {"", 0, 0}, {"", 0, 0}, {"", 0, 0}, {"", 0, 0},
    {"", 0, 0},
    {"relaxed", TYPE_QUAL, RID_RELAXED},
    {"", 0, 0}, {"", 0, 0}, {"", 0, 0}, {"", 0, 0},
    {"__signed", TYPESPEC, RID_SIGNED},
    {"", 0, 0}, {"", 0, 0}, {"", 0, 0}, {"", 0, 0},
    {"", 0, 0}, {"", 0, 0}, {"", 0, 0}, {"", 0, 0},
    {"", 0, 0}, {"", 0, 0},
    {"void", TYPESPEC, RID_VOID},
    {"double", TYPESPEC, RID_DOUBLE},
    {"", 0, 0}, {"", 0, 0}, {"", 0, 0}, {"", 0, 0},
    {"", 0, 0}, {"", 0, 0}, {"", 0, 0}, {"", 0, 0},
    {"", 0, 0}, {"", 0, 0}, {"", 0, 0}, {"", 0, 0},
    {"long", TYPESPEC, RID_LONG},
    {"", 0, 0}, {"", 0, 0}, {"", 0, 0},
    {"signed", TYPESPEC, RID_SIGNED},
    {"", 0, 0}, {"", 0, 0}, {"", 0, 0}, {"", 0, 0},
    {"", 0, 0}, {"", 0, 0}, {"", 0, 0}, {"", 0, 0},
    {"", 0, 0}, {"", 0, 0}, {"", 0, 0}, {"", 0, 0},
    {"", 0, 0}, {"", 0, 0}, {"", 0, 0}, {"", 0, 0},
    {"", 0, 0}, {"", 0, 0}, {"", 0, 0}, {"", 0, 0},
    {"", 0, 0}, {"", 0, 0}, {"", 0, 0}, {"", 0, 0},
    {"", 0, 0}, {"", 0, 0}, {"", 0, 0}, {"", 0, 0},
    {"", 0, 0}, {"", 0, 0}, {"", 0, 0},
    {"unsigned", TYPESPEC, RID_UNSIGNED},
    {"default", DEFAULT, NORID},
    {"", 0, 0}, {"", 0, 0}, {"", 0, 0},
    {"@end", END, NORID}
  };

#ifdef __GNUC__
__inline
#endif
struct resword *
is_reserved_word (str, len)
     register const char *str;
     register unsigned int len;
{
  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register int key = hash (str, len);

      if (key <= MAX_HASH_VALUE && key >= 0)
        {
          register const char *s = wordlist[key].name;

          if (*str == *s && !strcmp (str + 1, s + 1))
            return &wordlist[key];
        }
    }
  return 0;
}
