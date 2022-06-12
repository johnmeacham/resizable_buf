#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "resizable_buf.h"


int main(int argc, char *argv[])
{
        rb_t rb = RB_BLANK;
        assert(!rb_ptr(&rb));
        assert(!rb_endptr(&rb));
        assert(!rb_len(&rb));
        rb_printf(&rb, "hello %i %sX", 43, "test");
        assert(*RBP(char, &rb) == 'h');
        assert(*RBPE(char, &rb) == 0);
        assert(rb_len(&rb) == 14);
        /* lets pad out with non nulls */
        rb_grow(&rb, 15);
        memset(RBPE(char, &rb) + 1, 'x', 14);
        rb_insert(&rb, 5, ", world", 7);
        puts(rb_ptr(&rb));
        assert(!strcmp(rb_ptr(&rb), "hello, world 43 testX"));
        rb_delete(&rb, 2, 7);
        puts(rb_ptr(&rb));
        rb_delete(&rb, 10, 2);
        puts(rb_ptr(&rb));
        assert(!strcmp(rb_ptr(&rb), "herld 43 ttX"));
        return 0;
}
