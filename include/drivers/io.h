#ifndef IO_H_
#define IO_H_

#include <types.h>

/*Input operations*/
static __inline__ uint8_t inb(uint16_t port)
{
   uint8_t ret;
   __asm__ __volatile__ ("inb %1,%0":"=a"(ret):"Nd"(port));
   return ret;
}

static __inline unsigned short int
inw (unsigned short int port)
{
  unsigned short _v;

  __asm__ __volatile__ ("inw %w1,%0":"=a" (_v):"Nd" (port));
  return _v;
}

static __inline unsigned int
inl (unsigned short int port)
{
  unsigned int _v;

  __asm__ __volatile__ ("inl %w1,%0":"=a" (_v):"Nd" (port));
  return _v;
}

/*Output operations*/

static __inline__ void outb(uint16_t port, uint8_t val)
{
	__asm__ __volatile__("outb %0,%1"::"a"(val), "Nd" (port));
}

static __inline void
outw (unsigned short int port, unsigned short int value)
{
  __asm__ __volatile__ ("outw %w0,%w1": :"a" (value), "Nd" (port));

}

static __inline void
outl (unsigned short int port, unsigned int value)
{
  __asm__ __volatile__ ("outl %0,%w1": :"a" (value), "Nd" (port));
}

static __inline void
insb (unsigned short int port, void *addr, unsigned long int count)
{
  __asm__ __volatile__ ("cld ; rep ; insb":"=D" (addr),
            "=c" (count):"d" (port), "0" (addr), "1" (count));
}

static __inline void
insw (unsigned short int port, void *addr, unsigned long int count)
{
  __asm__ __volatile__ ("cld ; rep ; insw":"=D" (addr),
            "=c" (count):"d" (port), "0" (addr), "1" (count));
}

static __inline void
insl (unsigned short int port, void *addr, unsigned long int count)
{
  __asm__ __volatile__ ("cld ; rep ; insl":"=D" (addr),
            "=c" (count):"d" (port), "0" (addr), "1" (count));
}

static __inline void
outsb (unsigned short int port, const void *addr, unsigned long int count)
{
  __asm__ __volatile__ ("cld ; rep ; outsb":"=S" (addr),
            "=c" (count):"d" (port), "0" (addr), "1" (count));
}

static __inline void
outsw (unsigned short int port, const void *addr, unsigned long int count)
{
  __asm__ __volatile__ ("cld ; rep ; outsw":"=S" (addr),
            "=c" (count):"d" (port), "0" (addr), "1" (count));
}

static __inline void
outsl (unsigned short int port, const void *addr, unsigned long int count)
{
  __asm__ __volatile__ ("cld ; rep ; outsl":"=S" (addr),
            "=c" (count):"d" (port), "0" (addr), "1" (count));
}

#endif /*IO_H_*/
