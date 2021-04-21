#ifndef RESIZABLE_BUF_H
#define RESIZABLE_BUF_H

/* simple automatically resizing buffers and a fifo implementation.
 * these grow as needed with sublinear cost with minimal overhead. */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <inttypes.h>

typedef struct rb rb_t;

struct rb {
        void *buf;
        unsigned len;
        unsigned size;
};

/* This should be used to initialize new buffers.
 * rb_t rb = RB_BLANK;
 * zero initialized memory also works.
 */
#define RB_BLANK        {NULL, 0, 0}


/*
 * The macro versions accept a type as an argument and cast results
 * to it properly and work on units of that type.
 * The functional versions all work on bytes.
 *
 * The macro versions can also be used as lvalues for setting entries.
 *
 * example:
 * int *iptr = RBP(int, &rb)
 *
 * */

#define RBP(t,rb)          ((t *)((rb)->buf))
#define RBPE(t,rb)         ((t *)((rb)->buf + (rb)->len))
#define RB_NITEMS(t,rb)    ((rb)->len/sizeof(t))
#define RB_LAST(t,rb)      ((t *)((rb)->buf + (rb)->len - sizeof(t)))
#define RB_FIRST(t, b)     (RBP(t,b))

/* stack operations, these may be used directly as values or assigned to.
 *
 * int value = RB_PEEK(int, &rb);
 * RB_PUSH(long long, &rb) = 389;
 *
 */
#define RB_PEEK(t,rb)      *(t *)(rb_peek(rb, sizeof(t)))
#define RB_POP(t,rb)       *(t *)(rb_pop(rb, sizeof(t)))
#define RB_PUSH(t,rb)      *(t *)(rb_push(rb, sizeof(t)))
#define RB_PUSHN(t,rb,n)   (t *)(rb_push(rb, (n)*sizeof(t)))

// these require the argument be addressable but don't need the type explicitly
// passed.
#define RB_LPUSH(rb, x)      rb_append(rb, &(x), sizeof(x))
#define RB_LPUSHN(rb, x, n)  rb_append(rb, x, sizeof(*(x))*(n))

#define RB_LPOP(v, rb)       memcpy(&(v), rb_pop(rb, sizeof(v)))
#define RB_LPOPN(v, rb, n)   memcpy(v, rb_pop(rb, (n)*sizeof(*(v))))
#define RB_LPEEK(v, rb)      memcpy(&(v), rb_peek(rb, sizeof(v)))
#define RB_LPEEKN(v, rb, n)  memcpy(v, rb_peek(rb, (n)*sizeof(*(v))))

// these assume v is a ptr and assign it to directly point at the value in the
// rb
#define RB_APOPN(v, rb, n)   do { v = rb_pop(rb, (n)*sizeof(*(v))); } while (0)
#define RB_APEEKN(v, rb, n)  do { v = rb_peek(rb, (n)*sizeof(*(v))); } while (0)

/* sort and search */
typedef int (*rb_cmpfunc)(const void *, const void *);
#define RB_QSORT(t,rb,cfunc) qsort(rb_ptr(rb), RB_NITEMS(rb), (rb_cmpfunc)(cfunc))
#define RB_BSEARCH(t,k,rb,cfunc) bsearch(k, rb_ptr(rb), RB_NITEMS(rb), (rb_cmpfunc)(cfunc))

/*
 * for loop over rb values.
 *
 * unsigned sum = 0;
 * RB_FOR(unsigned, ptr, &rb) {
 *     sum += *ptr;
 * }
 */
#define RB_FOR(t,v,rb)     for (t *v = rb_ptr(rb); (void *)v < rb_ptr(rb) + rb_len(rb); v++)

inline void *rb_ptr(const rb_t *rb)
{
        return rb->buf;
}
inline void *rb_endptr(const rb_t *rb)
{
        return rb->buf + rb->len;
}
inline unsigned rb_len(const rb_t *rb)
{
        return rb->len;
}
inline void rb_clear(rb_t *rb)
{
        rb->len = 0;
}

char *rb_stringize(rb_t *rb);
void *rb_append(rb_t *rb, void *data, size_t len);
void *rb_extract(rb_t *rb, const rb_t *rb2, unsigned loc, size_t len);
void *rb_insert(rb_t *rb, unsigned loc, char *data, size_t len);
void *rb_insert_space(rb_t *rb, unsigned loc, size_t len);
void *rb_memset(rb_t *rb, char what, unsigned loc, size_t len);
void *rb_peek(rb_t *rb, int n);
void *rb_pop(rb_t *rb, int n);
void *rb_push(rb_t *rb, int n);
void *rb_set(rb_t *rb, void *data, size_t len);
void rb_delete(rb_t *rb, unsigned loc, size_t len);
void rb_free(rb_t *rb);
void rb_resize(rb_t *rb, size_t len, bool preserve);
void rb_resize_fill(rb_t *rb, size_t len, char fillvalue);


/* file operations.
 * These return the number of bytes copied or -(bytes copied + 1) on error. */
ssize_t rb_read_file(rb_t *rb, char *fname);
ssize_t rb_write_file(rb_t *rb, char *fname, bool append);

/* you can pass in -1 to the length to read and write the entire amount of data
 * available */
ssize_t rb_fwrite(rb_t *rb, FILE *fh, size_t n);
ssize_t rb_fread(rb_t *rb, FILE *fh, size_t n);

/* arguments reversed to match putc */
int rb_putc(char ch, rb_t *rb);
int rb_puts(char *str, rb_t *rb);
/* assumes unicode codepoints and utf8 encoding. */
int rb_putwc(uint32_t, rb_t *rb);

/* string operations, these operate with null terminated strings rather than
 * length delimited data. they always leve the data in the buffer null
 * terminated and return the number of characters operated on. the null
 * terminator is not counted as part of the length of the buffer just
 * like stringize */
char *rb_strcpy(rb_t *rb, char *str);
char *rb_strcat(rb_t *rb, char *str);
int rb_printf(rb_t *rb, char *fmt, ...);

/* fifo are a first in first out buffer. these reset storage when the fifo is
 * empty, so only use when the fifo is periodically emptied.
 * they are just a resizable buffer with an offset into it. */

typedef struct fifo fifo_t;

struct fifo {
        struct rb rb;
        unsigned offset;
};

#define FIFO_BLANK  {RB_BLANK, 0}

inline int fifo_len(const fifo_t *fifo)
{
        return fifo->rb.len - fifo->offset;
}
inline void *fifo_head(const fifo_t *fifo)
{
        return rb_ptr(&fifo->rb) + fifo->offset;
}
inline bool fifo_is_empty(const fifo_t *fifo)
{
        return !fifo_len(fifo);
}
inline void fifo_clear(fifo_t *fifo)
{
        rb_clear(&fifo->rb);
        fifo->offset = 0;
}

// #define FIFO_POP(f, l) do { if (l == FIFO_LEN(f)) {(f).offset = 0; RB_CLEAR((f).rb); } else (f).offset += l;  } while (0)

void *fifo_append(fifo_t *fifo, void *data, size_t len);
void *fifo_dequeue(fifo_t *fifo, size_t len);

#endif
