#ifndef Q_STATIC_ASSERT_H
#define Q_STATIC_ASSERT_H

/* There are many tricks to use a constant expression to yield an
   illegal type or expression at compile time, such as zero-sized
   arrays and duplicate case or enum labels. So this is but one of the
   many tricks. */

#define Q_STATIC_ASSERT_CODE(pred) do { switch(0) { case 0: case pred: ; } } while (0)

#endif /* Q_STATIC_ASSERT_H */

/* SHA1 not available (unoffical build.) */
