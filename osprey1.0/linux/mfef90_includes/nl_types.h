
/* NOTE: These are stubs that exist to support builds on systems that
   do not have the gencat, catopen(), etc. functionality.  Examples:
   cygwin and MacOS.
   
   If the real <nl_types.h> exists (usually in /usr/include) it should
   be found before this file.
*/

#ifndef _NL_TYPES_STUB_H
#define _NL_TYPES_STUB_H 1


#if (defined(__CYGWIN__) || defined(__MACH__))

/* ************************************************************************ */

/* The default message set used by the gencat program.  */
#define NL_SETD 1

/* Value for FLAG parameter of `catgets' to say we want XPG4 compliance.  */
#define NL_CAT_LOCALE 1


#ifdef __cplusplus
extern "C" {
#endif

/* Message catalog descriptor type.  */
typedef void *nl_catd;

/* Type used by `nl_langinfo'.  */
typedef int nl_item;

/* Open message catalog for later use, returning descriptor.  */
  /* extern nl_catd catopen(const char *cat_name, int flag); */

#define catopen(cat_name, flag) \
  ((nl_catd)42)

/* Return translation with NUMBER in SET of CATALOG; if not found
   return STRING.  */
  /* extern char *catgets(nl_catd catalog, int set, int number,
                          const char *string); */

#define catgets(catalog, set, number, string) \
  ((char*)"catgets: Bogus message.")

/* Close message CATALOG.  */
  /* extern int catclose(nl_catd catalog); */

#define catclose(catalog) \
  ((int)0)

#ifdef __cplusplus
}
#endif

#else
/* ************************************************************************ */

#include <../../usr/include/nl_types.h>

#endif

#endif /* nl_types.h  */
