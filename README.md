## Babelfish Scala client [![Build Status](https://travis-ci.org/bzz/client-scala.svg?branch=master)](https://travis-ci.org/bzz/client-scala)

This a pure Scala implementation of [Babelfish](https://doc.bblf.sh/) client.
It uses [ScalaPB](https://scalapb.github.io/grpc.html) for Protobuf/gRPC code generation.

### Usage

API
```scala
import scala.io.Source
import org.bblfsh.client.BblfshClient

val client = BblfshClient("0.0.0.0", 9432)

val filename = "/path/to/file.py" // client responsible for encoding it to utf-8
val fileContent = Source.fromFile(filename).getLines.mkString
println(client.parse(filename, fileContent))
```

Command line
```
java -jar ./target/scala-2.11/bblfsh-client-assembly-1.0.0.jar -f file.py
```

### Build

```
./sbt assembly
```

gRPC/protobuf are re-generate from `src/main/proto` on every `./sbt compile` and are stored under `./target/scala-2.11/src_managed/`

### License

Apache 2.0
