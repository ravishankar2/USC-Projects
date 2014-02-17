/*
 * Author:      William Chia-Wei Cheng (bill.cheng@acm.org)
 *:
 * @(#)$Id: cs402.h,v 1.1 2013/01/10 18:02:43 william Exp $
 */

#ifndef _CS402_H_
#define _CS402_H_

#ifndef NULL
#define NULL 0L
#endif /* ~NULL */

#ifndef TRUE
#define FALSE 0
#define TRUE 1
#endif /* ~TRUE */

#ifdef WIN32
#define DIR_SEP '\\'
#else /* ~WIN32 */
#define DIR_SEP '/'
#endif /* WIN32 */

#ifndef max
#define max(A,B) (((A)>(B)) ? (A) : (B))
#define min(A,B) (((A)>(B)) ? (B) : (A))
#endif /* ~max */

#ifndef round
#define round(X) (((X) >= 0) ? (int)((X)+0.5) : (int)((X)-0.5))
#endif /* ~round */

#ifndef MAXPATHLENGTH
#define MAXPATHLENGTH 256
#endif /* ~MAXPATHLENGTH */

#endif /*_CS402_H_*/






