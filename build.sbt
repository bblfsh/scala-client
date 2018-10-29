import sys.process._

name := "bblfsh-client"
organization := "org.bblfsh"
version := "3.0.0"

scalaVersion := "2.11.11"
val libuastVersion = "3.0.0-rc2"
val sdkVersion = "v2.6.1"
val sdkMajor = "v2"
val protoDir = "src/main/proto"

mainClass in Compile := Some("org.bblfsh.client.cli.ScalaClientCLI")

target in assembly := file("build")

libraryDependencies += "com.thesamet.scalapb" %% "scalapb-runtime" % scalapb.compiler.Version.scalapbVersion % "protobuf"
libraryDependencies += "commons-io" % "commons-io" % "2.5"
libraryDependencies ++= Seq(
  "org.scalatest" %% "scalatest" % "3.0.1" % "test",
  "io.grpc" % "grpc-netty" % scalapb.compiler.Version.grpcJavaVersion,
  "com.thesamet.scalapb" %% "scalapb-runtime-grpc" % scalapb.compiler.Version.scalapbVersion,
  "org.rogach" %% "scallop" % "3.0.3"
)

assemblyMergeStrategy in assembly := {
  case "META-INF/io.netty.versions.properties" => MergeStrategy.last
  case x =>
    val oldStrategy = (assemblyMergeStrategy in assembly).value
    oldStrategy(x)
}

test in assembly := {}

PB.targets in Compile := Seq(
    scalapb.gen() -> (sourceManaged in Compile).value
)
PB.protoSources in Compile := Seq(file(protoDir))

sonatypeProfileName := "org.bblfsh"

// pom settings for sonatype
homepage := Some(url("https://github.com/bblfsh/client-scala"))
scmInfo := Some(ScmInfo(url("https://github.com/bblfsh/client-scala"),
                            "git@github.com:bblfsh/client-scala.git"))
developers += Developer("juanjux",
                        "Juanjo Álvarez",
                        "juanjo@sourced.tech",
                        url("https://github.com/juanjux"))
licenses += ("Apache-2.0", url("http://www.apache.org/licenses/LICENSE-2.0"))
pomIncludeRepository := (_ => false)

crossPaths := false
publishMavenStyle := true
exportJars := true

val SONATYPE_USERNAME = scala.util.Properties.envOrElse("SONATYPE_USERNAME", "NOT_SET")
val SONATYPE_PASSWORD = scala.util.Properties.envOrElse("SONATYPE_PASSWORD", "NOT_SET")
credentials += Credentials(
  "Sonatype Nexus Repository Manager",
  "oss.sonatype.org",
  SONATYPE_USERNAME,
  SONATYPE_PASSWORD)

val SONATYPE_PASSPHRASE = scala.util.Properties.envOrElse("SONATYPE_PASSPHRASE", "not set")

useGpg := false
pgpSecretRing := baseDirectory.value / "project" / ".gnupg" / "secring.gpg"
pgpPublicRing := baseDirectory.value / "project" / ".gnupg" / "pubring.gpg"
pgpPassphrase := Some(SONATYPE_PASSPHRASE.toArray)

isSnapshot := version.value endsWith "SNAPSHOT"

publishTo := {
  val nexus = "https://oss.sonatype.org/"
  if (isSnapshot.value)
    Some("snapshots" at nexus + "content/repositories/snapshots")
  else
    Some("releases" at nexus + "service/local/staging/deploy/maven2")
}

val getProtoFiles = TaskKey[Unit]("getProtoFiles", "Retrieve protobuf files")
getProtoFiles := {
    import sys.process._

    println("Downloading and installing SDK protocol buffer files...")

    val bblfshProto = f"$protoDir%s/gopkg.in/bblfsh"
    val sdkProto = f"$bblfshProto%s/sdk.$sdkMajor%s"

    f"mkdir -p $sdkProto%s/protocol" !

    f"mkdir -p $sdkProto%s/uast" !

    val unzip_dir = "sdk-" + sdkVersion.substring(1)

    f"curl -SL https://github.com/bblfsh/sdk/archive/$sdkVersion%s.tar.gz" #| "tar xz" #&&
    f"cp $unzip_dir%s/protocol/generated.proto $sdkProto%s/protocol/" #&&
    f"cp $unzip_dir%s/uast/generated.proto $sdkProto%s/uast/" #&&
    f"rm -rf $unzip_dir%s" !
}

val getLibuast = TaskKey[Unit]("getLibuast", "Retrieve libuast")
getLibuast := {
    import sys.process._

    val releaseUrl = s"https://github.com/bblfsh/libuast/archive/v$libuastVersion.tar.gz"
    println(s"Downloading libuast from $releaseUrl")

    f"curl -SL $releaseUrl -o libuast.tar.gz" #&&
    "tar zxf libuast.tar.gz" #&&
    f"mv libuast-$libuastVersion libuast" #&&
    "rm -rf src/libuast-native" #&&
    "mv libuast/src/ src/libuast-native" #&&
    "rm -rf libuast" #&&
    "rm libuast.tar.gz" !
}

val compileScalaLibuast = TaskKey[Unit]("compileScalaLibuast", "Compile libUAST wrapper for JNI")
compileScalaLibuast := {
    import sys.process._

    println("Compiling libuast bindings...")

    "mkdir ./src/main/resources/lib/" !

    val sourceFiles = "src/main/scala/org/bblfsh/client/libuast/org_bblfsh_client_libuast_Libuast.cc " +
        "src/main/scala/org/bblfsh/client/libuast/jni_utils.cc " +
        "src/main/scala/org/bblfsh/client/libuast/memtracker.cc "

    compileUnix(sourceFiles)
    crossCompileMacOS(sourceFiles)
}

def compileUnix(sourceFiles: String) = {
  var javaHome = System.getenv("JAVA_HOME")
  if (javaHome == null) {
    javaHome = "/usr/lib/jvm/java-8-openjdk-amd64"
  }

  val osName = System.getProperty("os.name").toLowerCase()
  if (osName.contains("mac os x")) {
    val cmd:String = "g++ -shared -Wall -fPIC -O2 -std=c++11 " +
      "-I/usr/include " +
      "-I" + javaHome + "/include/ " +
      "-I" + javaHome + "/include/darwin " +
      "-Isrc/libuast-native/ " +
      "-Lsrc/libuast-native/ " +
      "-l uast " +
      "-o src/main/resources/lib/libscalauast.dylib " +
      sourceFiles

    checkedProcess(cmd, "macOS build")
  } else {
    val cmd:String = "g++ -shared -Wall -fPIC -O2 -std=c++11 " +
      "-I/usr/include " +
      "-I" + javaHome + "/include/ " +
      "-I" + javaHome + "/include/linux " +
      "-Isrc/libuast-native/ " +
      "-Lsrc/libuast-native/ " +
      "-l uast " +
      "-o src/main/resources/lib/libscalauast.so " +
      sourceFiles

    checkedProcess(cmd, "Linux build")
  }
}

def crossCompileMacOS(sourceFiles: String): Unit = {
  val osxHome = System.getenv("OSXCROSS_PATH")

  if (osxHome == null || osxHome.isEmpty) {
    println("OSXCROSS_PATH variable not defined, not cross-compiling for macOS")
    return
  }

  val cmd = osxHome + "/bin/o64-clang++-libc++ -shared -Wall -fPIC -O2 -lxml2 -std=c++11 " +
      "-I" + osxHome + "/SDK/MacOSX10.11.sdk/usr/include/libxml2/ " +
      "-I" + osxHome + "/SDK/MacOSX10.11.sdk/usr/include/ " +
      "-I/usr/lib/jvm/java-8-openjdk-amd64/include " +
      "-I/usr/lib/jvm/java-8-openjdk-amd64/include/linux " +
      "-Isrc/libuast-native/ -o src/main/resources/lib/libscalauast.dylib " +
      sourceFiles

  checkedProcess(cmd, "macOS cross-compile build")
} 

def checkedProcess(cmd: String, name: String) {
  import sys.process._

  println(cmd)
  val out = cmd !

  if (out != 0) {
    throw new IllegalStateException(name + " failed (see previous messages)")
  }
}


mainClass := Def.sequential(getProtoFiles, /*getLibuast,*/ compileScalaLibuast, (mainClass in Compile)).value
