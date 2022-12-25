#include "types.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"
#include "buf.h"
 
struct {
  struct spinlock lock;
  struct buf buf[NBUF];
 
  // Linked list of all buffers, through prev/next.
  // head.next is most recently used.
  struct buf head;
} bcache;
 
void
binit(void)
{
  struct buf *b;
 
  initlock(&bcache.lock, "bcache");
 
//PAGEBREAK!
  // Create linked list of buffers
  bcache.head.prev = &bcache.head;
  bcache.head.next = &bcache.head;
  for(b = bcache.buf; b < bcache.buf+NBUF; b++){
    b->next = bcache.head.next;
    b->prev = &bcache.head;
    b->dev = -1;
    bcache.head.next->prev = b;
    bcache.head.next = b;
  }
}
 
// Look through buffer cache for sector on device dev.
// If not found, allocate fresh block.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint sector)
{
  struct buf *b;
 
  acquire(&bcache.lock);
 
 loop:
  // Try for cached block.
  for(b = bcache.head.next; b != &bcache.head; b = b->next){
    if(b->dev == dev && b->sector == sector){
      if(!(b->flags & B_BUSY)){
        b->flags |= B_BUSY;
        release(&bcache.lock);
        return b;
      }
      sleep(b, &bcache.lock);
      goto loop;
    }
  }
 
  // Allocate fresh block.
  for(b = bcache.head.prev; b != &bcache.head; b = b->prev){
    if((b->flags & B_BUSY) == 0){
      b->dev = dev;
      b->sector = sector;
      b->flags = B_BUSY;
      release(&bcache.lock);
      return b;
    }
  }
  panic("bget: no buffers");
}
 
// Return a B_BUSY buf with the contents of the indicated disk sector.
struct buf*
bread(uint dev, uint sector)
{
  struct buf *b;
 
  b = bget(dev, sector);
  if(!(b->flags & B_VALID))
    iderw(b);
  return b;
}
 
// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if((b->flags & B_BUSY) == 0)
    panic("bwrite");
  b->flags |= B_DIRTY;
  iderw(b);
}
 
// Release the buffer b.
void
brelse(struct buf *b)
{
  if((b->flags & B_BUSY) == 0)
    panic("brelse");
 
  acquire(&bcache.lock);
 
  b->next->prev = b->prev;
  b->prev->next = b->next;
  b->next = bcache.head.next;
  b->prev = &bcache.head;
  bcache.head.next->prev = b;
  bcache.head.next = b;
 
  b->flags &= ~B_BUSY;
  wakeup(b);
 
  release(&bcache.lock);
}