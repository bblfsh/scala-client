## Babelfish Scala client [![Build Status](https://travis-ci.org/bzz/client-scala.svg?branch=master)](https://travis-ci.org/bzz/client-scala)

This a pure Scala implementation of [Babelfish](https://doc.bblf.sh/) client.
It uses [ScalaPB](https://scalapb.github.io/grpc.html) for Protobuf/gRPC code generation.

### Installation

#### Manual
```
git clone https://github.com/bblfsh/client-scala.git
cd client-scala
./sbt assembly
```

gRPC/protobuf are re-generate from `src/main/proto` on every `./sbt compile` and
are stored under `./target/src_managed/`. 

The jar file and the native module are generated in the `build/` directory. If 
you move the jar file to some other path, the native (`.so` or `.dylib`) 
library must be in the same path.

If the build fails because it can't find the `jni.h` header file, run it with:

```
./sbt -java_path /usr/lib/jvm/java-8-openjdk-amd64 assembly
```

Changing the JDK directory to the one right for your system.

*Note: currently only Linux is supported, check [#7](https://github.com/bblfsh/client-scala/issues/7) for MacOS support*.

#### Apache Maven

`bblfsh-client` package is available thorugh [maven central](http://search.maven.org/#search%7Cga%7C1%7Cbblfsh),
so it be used easily added as a dependency in various package management systems.
Examples of how to handle it for most commons systems are included below,
for other systems just look at maven central's dependency information.

```xml
<dependency>
    <groupId>org.bblfsh</groupId>
    <artifactId>bblfsh-client</artifactId>
    <version>${client_version}</version>
</dependency>
```

#### Scala SBT

```
libraryDependencies += "org.bblfsh" % "bblfsh-client" % version
```

### Dependencies

You need to install libxml2, the Java SDK and its header files. The command for 
Debian and derived distributions would be:

```
sudo apt install libxml2-dev openjdk-8 openjdk-8-jdk-headless
```

### Usage


A small example of how to parse a Python file and extract the import declarations from the UAST.

If you don't have a bblfsh server running you can execute it using the following command:

```
docker run --privileged --rm -it -p 9432:9432 --name bblfsh bblfsh/server
```   

Please, read the [getting started](https://doc.bblf.sh/user/getting-started.html) 
guide to learn more about how to use and deploy a bblfsh server.

API
```scala
import scala.io.Source
import org.bblfsh.client.BblfshClient

val client = BblfshClient("0.0.0.0", 9432)

val filename = "/path/to/file.py" // client responsible for encoding it to utf-8
val fileContent = Source.fromFile(filename).getLines.mkString
val resp = client.parse(filename, fileContent)
// Full response
println(resp.uast.get)
// Filtered response
println(client.filter(resp.uast.get, "//Import[@roleImport]"))
```

Command line
```
java -jar build/bblfsh-client-assembly-1.0.0.jar -f file.py
```

or if you want to use a XPath query:

```
java -jar build/bblfsh-client-assembly-1.0.0.jar -f file.py -q "//Import[@roleImport]" 
```
### License

Apache 2.0
