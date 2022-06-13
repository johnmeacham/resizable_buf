# resizable_buf
simple automatically growing buffer for use in C with lots of useful utility
routines

See header file for documentation.

# Example usage


## stack

```c
rb_t rb = RB_BLANK;
RB_PUSH(int, &rb) = 4;
RB_PUSH(char, &rb) = 'x';
RB_PUSH(int, &rb) = 54;
printf("%i", RB_POP(int, &rb)); // prints 54
printf("%c", RB_PEEK(char, &rb)); // prints x
printf("%c", RB_POP(char, &rb)); // prints x
printf("%i", RB_MPOP(int, &rb, 100)); // prints 4
printf("%i", RB_MPOP(int, &rb, 100)); // prints 100
```
## string

many routines maintain a null byte at the end of your buffer that is not
included in the length, so the data may be used directly as c strings.

```c
rb_t rb = RB_BLANK;
rb_printf(&rb, "hello %i %sX", 43, "test");
rb_strcat(&rb, "foobar");
rb_strins(&rb, 3, "TEST");
puts(rb_ptr(&rb));
```
