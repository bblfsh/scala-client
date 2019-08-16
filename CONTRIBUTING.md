# Contribution Guidelines

As all source{d} projects, this project follows the
[source{d} Contributing Guidelines](https://github.com/src-d/guide/blob/master/engineering/documents/CONTRIBUTING.md).


# Developer documentation

This section has more extended documentation for a brave developer willing to
contribute by diving into and debugging the native glue code in `src/main/native`.

## Build libuast with debug symbols
This is not strictly necessary, but will help you to navigate though the stack-traces.

Clone [libuast](https://github.com/bblfsh/libuast) locally and from the project root do:
```
CGO_ENABLED=1 go build -buildmode=c-archive -gcflags "-N -l" -o=libuast.a ./src
mv libuast.a ../scala-client/src/main/resources/libuast
```

Sadly, on non-linux OSes you might still experience issues with CGO due to https://github.com/golang/go/issues/5221.


## Build libscalauast with debug symbols
Compiler flags need to be `-g -O0` used instead of `-fPIC -O2` that is used for releases:
```
$ g++ -shared -Wall -g -std=c++11 -O0 \
      -I/usr/include \
      "-I${JAVA_HOME}/include" \
      "-I${JAVA_HOME}/include/${platform}" \
      -Isrc/main/resources/libuast \
      -o "src/main/resources/lib/libscalauast${platform_ext}" \
      src/main/native/org_bblfsh_client_v2_libuast_Libuast.cc \
      src/main/native/jni_utils.cc src/main/resources/libuast/libuast.a
```

## Run a single test under debugger
To run a single test from CLI one can:

```
./sbt 'testOnly org.bblfsh.client.v2.libuast.IteratorNativeTest -- -z "Native UAST iterator should return non-empty results on decoded objects"'
```

When using `lldb`, the classpath needs to be manually set for the `java` executable:

```
PATH="/usr/bin:$PATH" lldb -- java -ea -Xcheck:jni -Djava.library.path=src/main/resources -cp "target/classes:target/test-classes:src/main/resources:${HOME}/.ivy2/cache/org.scalatest/scalatest_2.11/bundles/scalatest_2.11-3.0.1.jar:${HOME}/.ivy2/cache/org.scala-lang/scala-library/jars/scala-library-2.11.11.jar:${HOME}/.ivy2/cache/org.scala-lang.modules/scala-xml_2.11/bundles/scala-xml_2.11-1.0.5.jar:${HOME}/.ivy2/cache/org.scalactic/scalactic_2.11/bundles/scalactic_2.11-3.0.1.jar:${HOME}/.ivy2/cache/commons-io/commons-io/jars/commons-io-2.5.jar:build/bblfsh-client-assembly-2.0.0-SNAPSHOT.jar:target/*" \
  org.scalatest.tools.Runner \
  -s org.bblfsh.client.v2.libuast.IteratorNativeTest \
  -z "Native UAST iterator" \
  -f iterator-native-test.txt
```
Actual test output will be saved in `iterator-native-test.txt`.

## When inside the debugger

These instructions are for `lldb`, but the steps should be similar in `gdb`.
To load the debug symbols do:

```
run
continue
continue
target symbols add src/main/resources/lib/libscalauast.dylib.dSYM
```

If that does not load the symbols, you have to make sure `libscalauast` library has
already been loaded to the process by `target modules list`.
If the library loads but symbols are not correctly displayed, it probably means
the library is at the wrong filesystem path.
Stop the debugger and do `rm -rf ./target/classes/lib/libscalauast.dylib*`

To actually debug, set a breakpoint like:
```
br s -r Java_org_bblfsh_client_v2_libuast_Libuast_00024UastIter_nativeNext
run
c
```

Check the stack trace and print values do:
```
bt
p *node
```

Check [official LLDB documentation](https://lldb.llvm.org/use/map.html) for more
use cases and instructions.

## More tips on JNI debugging

A small curated list of really useful resources on Go&JNI debugging:
 - https://github.com/facebook/rocksdb/wiki/JNI-Debugging
 - https://gist.github.com/dwbuiten/c9865c4afb38f482702e#2-debugging-tips
 - Go and LLDB http://ribrdb.github.io/lldb/

## JNI details

Here are some resources to understand the JNI machinery and its best practices:
 - https://docs.oracle.com/javase/8/docs/technotes/guides/jni/spec/design.html
 - https://docs.oracle.com/javase/8/docs/technotes/guides/jni/spec/functions.html
 - https://developer.android.com/training/articles/perf-jni
 - https://www.artima.com/insidejvm/ed2/jvm9.html
 - http://blog.jamesdbloom.com/JVMInternals.html
