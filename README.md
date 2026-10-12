# libjstd

Java 语言 / JVM 语义的 C++ header 库，命名空间 `jstd`。为需要与 JVM 行为
逐位一致的 C++ 代码提供一份权威实现。

## 模块

- `jstd/arith.hpp` —— `jstd::arith`：int32/int64 溢出回绕四则（含 `INT_MIN/-1`
  特例、除法向零截断、取模符号跟随被除数）、`neg`/`abs`、移位
  `shl`/`shr`/`ushr`（掩码取模 + 逻辑右移）、窄化 `i2b`/`i2c`/`i2s`/`l2i`。
- `jstd/random.hpp` —— `jstd::Random`：`java.util.Random` 位级复刻
  （48-bit LCG；`next`/`nextInt`/`nextLong`/`nextDouble`/`setSeed` 等）。
- `jstd/string_hash.hpp` —— `java_string_hash`：Java `String.hashCode`
  （31 进多项式，i32 回绕）。

## 构建

依赖 libca（core 模块，基础整数类型）。

```sh
xmake repo --add luiox-repo https://github.com/luiox/luiox-repo.git
xmake f --with_tests=y
xmake build libjstd_unittest
xmake run libjstd_unittest
```

## License

MIT —— 见 [LICENSE](LICENSE)。
