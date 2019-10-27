## Babelfish Scala client [![Build Status](https://travis-ci.org/bblfsh/scala-client.svg?branch=master)](https://travis-ci.org/bblfsh/scala-client)

This is a Scala/JNI implementation of the [Babelfish](https://doc.bblf.sh/) client.
It uses [ScalaPB](https://scalapb.github.io/grpc.html) for Protobuf/gRPC code
generation and [libuast](https://github.com/bblfsh/libuast) for XPath queries.

### Status

The latest `scala-client` *v2.x* supports the [UASTv2 protocol](https://doc.bblf.sh/uast/uast-specification-v2.html).

### Installation

#### Requirements

##### Java SDK 8

You need to install the Java SDK and its header files. The command for Debian and derived distributions would be:

```
sudo apt install openjdk-8 openjdk-8-jdk-headless
```

##### Scala 2.11

Currently we support [Scala](https://www.scala-lang.org/) *2.11*. We recommend using `scalaVersion := "2.11.11"` or above in your `build.sbt` file. This library is not compatible with Scala *2.12* or *2.13*.

#### Building from sources

```
git clone https://github.com/bblfsh/scala-client.git
cd scala-client
./build.sh --clean --get-dependencies --all
```

This command would fetch dependencies for the package, compile native code (JNI) and the Scala code.
gRPC/protobuf files are re-generate from `src/main/proto` on every `./sbt compile` and are stored under
`./target/src_managed/`.

The jar file and the native module are generated in the `build/` directory. If
you move the jar file to some other path, the native (`.so` or `.dylib`)
library must be in the same path.

If the build fails because it can't find the `jni.h` header file, run it setting `JAVA_HOME` (appropriately
setting it to the right location for your system):

```
export JAVA_HOME=/usr/lib/jvm/java-8-openjdk-amd64
./build.sh --clean --get-dependencies --all
```

If the build fails with: `build.sh: Permission denied`, add execution permissions to the script:

```
chmod +x ./build.sh
```

`build.sh` script has been tested in Unix (Linux and macOS) and with [Git Bash](https://gitforwindows.org/) in Windows.

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
import org.bblfsh.client.v2.BblfshClient, BblfshClient._
import gopkg.in.bblfsh.sdk.v2.protocol.driver.Mode

val client = BblfshClient("localhost", 9432)

val filename = "/path/to/file.py" // client responsible for encoding it to utf-8
val fileContent = Source.fromFile(filename).getLines.mkString("\n")
val resp = client.parse(filename, fileContent, Mode.SEMANTIC)

// Full response
println(resp.get)

// Filtered response
val it = client.filter(resp.get, "//uast:Identifier")
it.foreach(println)
```

Please read the [Babelfish clients](https://doc.bblf.sh/user/language-clients.html)
guide section to learn more about babelfish clients and their query language.

### License

Apache 2.0
