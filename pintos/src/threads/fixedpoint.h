#ifndef THREADS_FIXEDPOINT_H

#define THREADS_FIXEDPOINT_H

#define fp int
#define P 17
#define Q 14
#define F 1<<(Q)

/* Numbers in 17.14 format (P.Q)
   According to https://jeason.gitbooks.io/pintos-reference-guide-sysu/content/Advanced%20Scheduler.html
   x and y are fp numbers, n is an integer
*/

int int_to_fp(int n);
int fp_to_int(int x);
int fp_to_int_round(int x);
int add_fp(int x, int y);
int add_int(int x, int n);
int sub_fp(int x, int y);
int sub_int(int x, int n);
int mult_fp(int x, int y);
int mult_int(int x, int n);
int div_fp(int x, int y);
int div_int(int x, int n);

/* Convert n to fixed point */
int int_to_fp(int n)
{
  return (n) * (F);
}

/* Convert x to integer (rounding toward zero) */
int fp_to_int(int x)
{
  return (x) / (F);
}

/* Convert x to integer (rounding to nearest)	*/
int fp_to_int_round(int x)
{
  if (x >= 0)
    {
      return ((x) + (F) / 2) / (F);
    }
  else
    {
      return ((x) - (F) / 2) / (F);
    }
}

/* Add x and y	*/
int add_fp(int x, int y)
{
  return (x) + (y);
}

/* Add x and n	*/
int add_int(int x, int n)
{
  return (x) + (n) * (F);
}

/* Subtract y from x */
int sub_fp(int x, int y)
{
  return (x) - (y);
}

/* Subtract n from x */
int sub_int(int x, int n)
{
  return (x) - (n) * (F);
}

/* Multiply x by n */
int mult_fp(int x, int y)
{
  return ((int64_t)(x)) * (y) / (F);
}

/* Multiply x by n */
int mult_int(int x, int n)
{
  return (x) * (n);
}

/* Divide x by n */
int div_fp(int x, int y)
{
  return ((int64_t)(x)) * (F) / (y);
}

/* Divide x by n */
int div_int(int x, int n)
{
  return (x) / (n);
}

#endif /* threads/fixedpoint.h */
