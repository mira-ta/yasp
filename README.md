# yasp

Yet another simple pipe byte seeker. It can write specific amount of bytes from stdin to stdout, also providing offset ability.

## Compile

```sh
inst() {
    install --mode 755 -D $1 $2
}

gcc -xc -lc -O2 -std=c18 yasp.c -o yasp &&
    inst yasp usr/bin/yasp

unset -f inst
```

