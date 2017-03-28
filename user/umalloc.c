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
static struct spinlock baseSpinLock = {0};
static struct spinlock freepSpinLock = {0};

void
free(void *ap)
{
  Header *bp, *p;

  spin_lock(&freepSpinLock);

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

  spin_unlock(&freepSpinLock);
}

static Header*
morecore(uint nu)
{
  char *p;
  Header *hp;

  if(nu < 4096)
    nu = 4096;
  p = sbrk(nu * sizeof(Header));
  if(p == (char*)-1)
    return 0;
  hp = (Header*)p;
  hp->s.size = nu;
  free((void*)(hp + 1));
  return freep;
}

void*
malloc(uint nbytes)
{
  Header *p, *prevp;
  uint nunits;

  spin_lock(&baseSpinLock);

  nunits = (nbytes + sizeof(Header) - 1)/sizeof(Header) + 1;
  
  spin_lock(&freepSpinLock);
  if((prevp = freep) == 0){
    base.s.ptr = freep = prevp = &base;
    base.s.size = 0;
  }
  spin_unlock(&freepSpinLock);

  for(p = prevp->s.ptr; ; prevp = p, p = p->s.ptr){
    if(p->s.size >= nunits){
      if(p->s.size == nunits)
        prevp->s.ptr = p->s.ptr;
      else {
        p->s.size -= nunits;
        p += p->s.size;
        p->s.size = nunits;
      }
      spin_lock(&freepSpinLock);
      freep = prevp;
      spin_unlock(&freepSpinLock);

      spin_unlock(&baseSpinLock);
      return (void*)(p + 1);
    }

    spin_lock(&freepSpinLock);
    if(p == freep){
      spin_unlock(&freepSpinLock);
      if((p = morecore(nunits)) == 0){
        spin_unlock(&baseSpinLock);
        return 0;
      }
    }else{
      spin_lock(&freepSpinLock);
    }
  }

  spin_unlock(&baseSpinLock);

}
