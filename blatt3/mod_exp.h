#ifndef MOD_EXP_H
#define MOD_EXP_H

/**
 * @brief calculates the modular exponentation of a base
 *        where we get b^e mod m in a memory efficient way
 *
 * @param base      the base(b)
 * @param exp       the exponent(e)
 * @param mod       the modulus
 *
 *
 * Because exponential calculations can get very big and an
 * unsigned long long can just save up to numbers with 20 digits
 * this method doesn't need to calculate this big exponential numbers
 *
 * @return c        the correct result of the modular exponentation
 */
unsigned long mod_exp(unsigned short base, unsigned short exp, unsigned short mod)
{
  unsigned int c = 1;
  unsigned short e2 = 0;
  if(mod == 1)
  {
    return 0;
  }
  for(e2 = 1; e2 <= exp; e2++)
  {
    c = (c * base) % mod;
  }
  return c;
}

#endif /*MOD_EXP_H*/
