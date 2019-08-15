## Babelfish Scala client [![Build Status](https://travis-ci.org/bblfsh/scala-client.svg?branch=master)](https://travis-ci.org/bblfsh/scala-client)

This is a Scala/JNI implementation of the [Babelfish](https://doc.bblf.sh/) client.
It uses [ScalaPB](https://scalapb.github.io/grpc.html) for Protobuf/gRPC code
generation and [libuast](https://github.com/bblfsh/libuast) for XPath queries.

### Status

The latest `scala-client` *v2.x* supports the [UASTv2 protocol](https://doc.bblf.sh/uast/uast-specification-v2.html).

### Installation

#### Building from sources
```
git clone https://github.com/bblfsh/scala-client.git
cd scala-client
./sbt assembly
```

gRPC/protobuf files are re-generate from `src/main/proto` on every `./sbt compile`
and are stored under `./target/src_managed/`.

The jar file and the native module are generated in the `build/` directory. If
you move the jar file to some other path, the native (`.so` or `.dylib`)
library must be in the same path.

If the build fails because it can't find the `jni.h` header file, run it with:

```
./sbt -java-home /usr/lib/jvm/java-8-openjdk-amd64 assembly
```

Changing the JDK directory to the one right for your system.

For more developer documentation please check our [CONTRIBUTING](./CONTRIBUTING.md) guideline.

#### Apache Maven

The `bblfsh-client` package is available thorugh [Maven
central](http://search.maven.org/#search%7Cga%7C1%7Cbblfsh), so it can be easily
added as a dependency in various package management systems.  Examples of how to
handle it for most common systems are included below; for other systems just look
at Maven central's dependency information.

```xml
<dependency>
    <groupId>org.bblfsh</groupId>
    <artifactId>bblfsh-client</artifactId>
    <version>${version}</version>
</dependency>
```

#### Scala sbt

```
libraryDependencies += "org.bblfsh" % "bblfsh-client" % version
```

### Dependencies

You need to install the Java SDK and its header files. The command for
Debian and derived distributions would be:

```
sudo apt install openjdk-8 openjdk-8-jdk-headless
```

### Usage

If you don't have a bblfsh server running you can execute it using the following
command:

```
docker run --privileged --rm -it -p 9432:9432 --name bblfsh bblfsh/bblfshd
```

Please, read the [getting started](https://doc.bblf.sh/using-babelfish/getting-started.html)
guide to learn more about how to use and deploy a bblfsh server, install language drivers, etc.

API
```scala
import scala.io.Source
import org.bblfsh.client.BblfshClient
import gopkg.in.bblfsh.sdk.v2.protocol.driver.Mode

val client = BblfshClient("localhost", 9432)

val filename = "/path/to/file.py" // client responsible for encoding it to utf-8
val fileContent = Source.fromFile(filename).getLines.mkString("\n")
val resp = client.parse(filename, fileContent, Mode.SEMANTIC)

// Full response
println(resp.get)

// Filtered response
println(client.filter(resp.get, "//uast:Import"))
```

Command line:

```
java -jar build/bblfsh-client-assembly-*.jar -f <file.py>
```

or if you want to use a XPath query:

```
java -jar build/bblfsh-client-assembly-*.jar -f <file.py> -q "//uast:Import"
```

Please read the [Babelfish clients](https://doc.bblf.sh/user/language-clients.html)
guide section to learn more about babelfish clients and their query language.

### License

Apache 2.0
