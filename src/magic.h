#define SimMagic2(cmd, arg0, arg1) ({        \
   unsigned long _cmd = (cmd), _arg0 = (arg0), _arg1 = (arg1), _res; \
   __asm__ __volatile__ (                    \
   "mov %1, %%" MAGIC_REG_A "\n"             \
   "\tmov %2, %%" MAGIC_REG_B "\n"           \
   "\tmov %3, %%" MAGIC_REG_C "\n"           \
   "\txchg %%bx, %%bx\n"                     \
   : "=a" (_res)           /* output    */   \
   : "g"(_cmd),                              \
     "g"(_arg0),                             \
     "g"(_arg1)            /* input     */   \
   : "%" MAGIC_REG_B, "%" MAGIC_REG_C ); /* clobbered */ \
   _res;                                     \
})

#define SimUser(a,b,c) SimMagic2(a,b,c)