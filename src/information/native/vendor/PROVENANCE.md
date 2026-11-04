# Vendored stb headers

Upstream: https://github.com/nothings/stb

Pinned revision: `d14d46f6b44456eb88beac58cba833a4135d4c89`.
`stb_image.h` is unchanged. In `stb_image_write.h`, the JPEG bit accumulator
and its input shift are unsigned, avoiding signed left-shift overflow found
by this project's UBSAN test. No encoding algorithm or public API is changed.
The table below records the **upstream** hashes, before that small local fix.
See the adjacent upstream `LICENSE`
and each header's licence block. This project uses the MIT licence option.

| File | SHA-256 |
| --- | --- |
| stb_image.h | 594c2fe35d49488b4382dbfaec8f98366defca819d916ac95becf3e75f4200b3 |
| stb_image_write.h | cbd5f0ad7a9cf4468affb36354a1d2338034f2c12473cf1a8e32053cb6914a05 |
| LICENSE | bebfe904b14301657e4e5d655c811d51fd31b97c455b9cc2d8600d6bac6cff63 |
