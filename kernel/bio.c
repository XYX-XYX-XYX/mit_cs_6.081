// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

struct bucket{
  struct spinlock lock;
  struct buf head;
};

struct {
  struct spinlock lock;
  struct buf buf[NBUF];
  struct bucket bucket[NBUCKET];
  // Linked list of all buffers, through prev/next.
  // Sorted by how recently the buffer was used.
  // head.next is most recent, head.prev is least.
  //struct buf head;
} bcache;

void
binit(void)
{
  struct buf *b;

  initlock(&bcache.lock, "bcache");
  for(int i = 0; i < NBUCKET; i++){
    initlock(&bcache.bucket[i].lock, "bucket");
    bcache.bucket[i].head.prev = &bcache.bucket[i].head;
    bcache.bucket[i].head.next = &bcache.bucket[i].head;
  };

  // all buff asigned to bucket 0 initially
  for(b = bcache.buf; b < bcache.buf+NBUF; b++){
    b->next = bcache.bucket[0].head.next;
    b->prev = &bcache.bucket[0].head;
    initsleeplock(&b->lock,"buffer");
    bcache.bucket[0].head.next->prev = b;
    bcache.bucket[0].head.next = b;
  }
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;

  //acquire(&bcache.lock);
  acquire(&bcache.bucket[blockno % NBUCKET].lock);

  //Is the block already cached?
  for(b = bcache.bucket[blockno % NBUCKET].head.next; b != &bcache.bucket[blockno % NBUCKET].head; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&bcache.bucket[blockno % NBUCKET].lock);
      acquiresleep(&b->lock);
      return b;
    }
  }

  //not cached 
  for(int i = 0; i < NBUCKET; i++){
    int nextid = (blockno + i) % NBUCKET;
    if(i != 0)
      acquire(&bcache.bucket[nextid].lock);
    //struct buf *pre = bcache.bucket[nextid].head;
    for(b = bcache.bucket[nextid].head.next; b != &bcache.bucket[nextid].head; b = b->next){
      if(b->refcnt == 0){
        b->dev = dev;
        b->blockno = blockno;
        b->valid = 0;
        b->refcnt = 1;
        if(i == 0){
          release(&bcache.bucket[blockno % NBUCKET].lock);
          acquiresleep(&b->lock);
          return b;
        }else{
          b->prev->next = b->next;
          b->next->prev = b->prev;
          b->next = bcache.bucket[blockno % NBUCKET].head.next;
          b->prev = &bcache.bucket[blockno % NBUCKET].head;
          bcache.bucket[blockno % NBUCKET].head.next->prev = b;
          bcache.bucket[blockno % NBUCKET].head.next = b; 
          release(&bcache.bucket[nextid].lock);
          release(&bcache.bucket[blockno % NBUCKET].lock);
          acquiresleep(&b->lock);
          return b;
        }
      }
    }
    if(i != 0)
      release(&bcache.bucket[nextid].lock);
  }
  panic("bget: no buffers");
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);
  acquire(&bcache.bucket[b->blockno % NBUCKET].lock);
  b->refcnt--;
  
  release(&bcache.bucket[b->blockno % NBUCKET].lock);
}

void
bpin(struct buf *b) {
  acquire(&bcache.bucket[b->blockno % NBUCKET].lock);
  b->refcnt++;
  release(&bcache.bucket[b->blockno % NBUCKET].lock);
}

void
bunpin(struct buf *b) {
  acquire(&bcache.bucket[b->blockno % NBUCKET].lock);
  b->refcnt--;
  release(&bcache.bucket[b->blockno % NBUCKET].lock);
}


