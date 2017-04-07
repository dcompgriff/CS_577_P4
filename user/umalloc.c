#include "types.h"
#include "stat.h"
#include "user.h"
#include "param.h"

// Memory allocator by Kernighan and Ritchie,
// The C programming Language, 2nd ed.  Section 8.7.

typedef long Align;

union header {
  struct {
    union header *ptr;
    uint size;
  } s;
  Align x;
};

typedef union header Header;

static Header base;
static Header *freep;

struct spinlock mallocLock = {.locked = 0};

void
free(void *ap)
{
  Header *bp, *p;

  //printf(1, "free called.\n");

  spin_lock(&mallocLock);

  bp = (Header*)ap - 1;
  for(p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
    if(p >= p->s.ptr && (bp > p || bp < p->s.ptr))
      break;
  if(bp + bp->s.size == p->s.ptr){
    bp->s.size += p->s.ptr->s.size;
    bp->s.ptr = p->s.ptr->s.ptr;
  } else
    bp->s.ptr = p->s.ptr;
  if(p + p->s.size == bp){
    p->s.size += bp->s.size;
    p->s.ptr = bp->s.ptr;
  } else
    p->s.ptr = bp;
  freep = p;
  
  spin_unlock(&mallocLock);
}

// Free function that assumes mallocLock is held.
void
freewlock(void *ap)
{
  Header *bp, *p;

  //printf(1, "freewlock called.\n");

  bp = (Header*)ap - 1;
  for(p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
    if(p >= p->s.ptr && (bp > p || bp < p->s.ptr))
      break;
  if(bp + bp->s.size == p->s.ptr){
    bp->s.size += p->s.ptr->s.size;
    bp->s.ptr = p->s.ptr->s.ptr;
  } else
    bp->s.ptr = p->s.ptr;
  if(p + p->s.size == bp){
    p->s.size += bp->s.size;
    p->s.ptr = bp->s.ptr;
  } else
    p->s.ptr = bp;
  freep = p;
}

static Header*
morecore(uint nu)
{
  char *p;
  Header *hp;

  if(nu < 4096)
    nu = 4096;
  //printf(1, "mallocLock before sbrk is %d.\n", mallocLock.locked);
  p = sbrk(nu * sizeof(Header));
  if(p == (char*)-1)
    return 0;
  hp = (Header*)p;
  hp->s.size = nu;
  freewlock((void*)(hp + 1));
  return freep;
}

void*
malloc(uint nbytes)
{
  Header *p, *prevp;
  uint nunits;
  static int count = 0;

  spin_lock(&mallocLock);
  count++;
  //printf(1, "malloc called %d times.\n", count);
  //printf(1, "mallocLock aquired.\n");

  nunits = (nbytes + sizeof(Header) - 1)/sizeof(Header) + 1;

  if((prevp = freep) == 0){
    base.s.ptr = freep = prevp = &base;
    base.s.size = 0;
  }
  for(p = prevp->s.ptr; ; prevp = p, p = p->s.ptr){
    if(p->s.size >= nunits){
      if(p->s.size == nunits)
        prevp->s.ptr = p->s.ptr;
      else {
        p->s.size -= nunits;
        p += p->s.size;
        p->s.size = nunits;
      }
      freep = prevp;
      //printf(1, "mallocLock released.\n");
      spin_unlock(&mallocLock);
      return (void*)(p + 1);
    }
    if(p == freep){
      if((p = morecore(nunits)) == 0){
        //printf(1, "mallocLock released.\n");
        spin_unlock(&mallocLock);
        return 0;
      }
    }
  }
  //printf(1, "mallocLock released.\n");
  spin_unlock(&mallocLock);

}
