name := "bblfsh-client"
organization := "org.bblfsh"

git.useGitDescribe := true
enablePlugins(GitVersioning)

scalaVersion := "2.11.11"
val libuastVersion = "3.4.2"
val sdkMajor = "v3"
val sdkVersion = s"${sdkMajor}.1.0"
val protoDir = "src/main/proto"

target in assembly := file("build")

// Where the .h files will be generated by javah from the sbt-jni plugin
target in javah := file("src/main/native")

libraryDependencies ++= Seq(
  "org.rogach" %% "scallop" % "3.0.3",
  "commons-io" % "commons-io" % "2.5",

  "com.thesamet.scalapb" %% "scalapb-runtime" % scalapb.compiler.Version.scalapbVersion % "protobuf",
  "com.thesamet.scalapb" %% "scalapb-runtime-grpc" % scalapb.compiler.Version.scalapbVersion,
  "io.grpc" % "grpc-netty" % scalapb.compiler.Version.grpcJavaVersion,

  "org.scalatest" %% "scalatest" % "3.0.1" % "test"
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
homepage := Some(url("https://github.com/bblfsh/scala-client"))
scmInfo := Some(ScmInfo(url("https://github.com/bblfsh/scala-client"),
                            "git@github.com:bblfsh/scala-client.git"))
developers += Developer("juanjux",
                        "Juanjo Álvarez",
                        "juanjo@sourced.tech",
                        url("https://github.com/juanjux"))
developers += Developer("bzz",
                        "Alexander Bezzubov",
                        "Alex@sourced.tech",
                        url("https://github.com/bzz"))

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
val JAVA_HOME = scala.util.Properties.envOrElse("JAVA_HOME", "/usr/lib/jvm/java-8-openjdk-amd64")
val CPP_FLAGS = "-shared -Wall -fPIC -O2 -std=c++11"
val LINUX_GCC_FLAGS = "-Wl,-Bsymbolic"

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

    println(s"Downloading and installing SDK$sdkMajor protocol buffer files...")

    val bblfshProto = s"${protoDir}/github.com/bblfsh"
    val sdkProto = s"${bblfshProto}/sdk/${sdkMajor}"

    s"mkdir -p ${sdkProto}/protocol" !

    s"mkdir -p ${sdkProto}/uast/role" !

    val unzip_dir = "sdk-" + sdkVersion.substring(1)

    s"curl -SL https://github.com/bblfsh/sdk/archive/${sdkVersion}.tar.gz" #| "tar xz" #&&
    s"cp ${unzip_dir}/protocol/driver.proto ${sdkProto}/protocol/" #&&
    s"cp ${unzip_dir}/uast/role/generated.proto ${sdkProto}/uast/role" #&&
    s"rm -rf ${unzip_dir}" !

    println("Done unpacking SDK")
}

val getLibuast = TaskKey[Unit]("getLibuast", "Retrieve libuast")
getLibuast := {
    val os = if (System.getProperty("os.name").toLowerCase.contains("mac os x")) "darwin" else "linux"

    downloadUnpackLibuast(os)
}

def downloadUnpackLibuast(os: String) = {
    import sys.process._

    val ghUrl = "https://github.com/bblfsh/libuast"
    val binaryReleaseUrl = s"${ghUrl}/releases/download/v${libuastVersion}/libuast-${os}-amd64.tar.gz"
    println(s"Downloading libuast binary from ${binaryReleaseUrl}")

    s"curl -sL ${binaryReleaseUrl} -o libuast-bin.tar.gz" #&&
    "tar xzf libuast-bin.tar.gz" #&&
    s"mv ${os}-amd64 libuast" #&&
    "mkdir -p src/main/resources" #&&
    "rm -rf src/main/resources/libuast" #&&
    "mv libuast src/main/resources" #&&
    "rm -f src/main/resources/libuast/libuast.so" #&& // always a static build
    "rm -f src/main/resources/libuast/libuast.dylib" #&&
    "rm libuast-bin.tar.gz" !

    "find src/main/resources"!

    "nm src/main/resources/libuast/libuast.a" #| "grep -c UastDecode"!

    "nm src/main/resources/libuast/libuast.a" #| "wc -l"!

    println(s"Done unpacking libuast for ${os}")
}

val compileScalaLibuast = TaskKey[Unit]("compileScalaLibuast", "Compile libScalaUast JNI library")
compileScalaLibuast := {
    import sys.process._

    println("Compiling libuast bindings...")

    "mkdir -p ./src/main/resources/lib/" !

    val nativeSourceFiles = "src/main/native/org_bblfsh_client_v2_libuast_Libuast.cc " +
        "src/main/native/jni_utils.cc "

    compileUnix(nativeSourceFiles)
    crossCompileMacOS(nativeSourceFiles)
}

def compileUnix(sourceFiles: String) = {
  import sys.process._

  val osName = System.getProperty("os.name").toLowerCase()

  if (osName.contains("mac os x")) {
    val cmd:String = "g++" +  " " + CPP_FLAGS + " " +
      "-I/usr/include " +
      "-I" + JAVA_HOME + "/include/ " +
      "-I" + JAVA_HOME + "/include/darwin " +
      "-Isrc/main/resources/libuast " +
      "-o src/main/resources/lib/libscalauast.dylib " + // sic, must be in the classpath for the test
      sourceFiles +
      "src/main/resources/libuast/libuast.a "

    checkedProcess(cmd, "macOS build")
  } else {
    val cmd:String = "g++" + " " + LINUX_GCC_FLAGS + " " + CPP_FLAGS + " " +
      "-I/usr/include " +
      "-I" + JAVA_HOME + "/include/ " +
      "-I" + JAVA_HOME + "/include/linux " +
      "-Isrc/main/resources/libuast " +
      "-o src/main/resources/lib/libscalauast.so " +
      sourceFiles +
      "src/main/resources/libuast/libuast.a "

    checkedProcess(cmd, "Linux build")

    "nm src/main/resources/lib/libscalauast.so" #| "grep -c UastDecode"!
  }
}

def crossCompileMacOS(sourceFiles: String): Unit = {
  val osName = System.getProperty("os.name").toLowerCase()
  if (osName.contains("mac os x")) {
      println("Skipping cross-compilation for macOS on macOS")
      return
  }

  val osxHome = System.getenv("OSXCROSS_PATH")
  // This is defined in .travis.yml
  val sdkVersion = System.getenv("SDK_VERSION")

  if (osxHome == null || osxHome.isEmpty) {
    println("OSXCROSS_PATH variable not defined, not cross-compiling for macOS")
    return
  }

  if (sdkVersion == null || sdkVersion.isEmpty) {
    println("SDK_VERSION variable not defined, it should be to cross-compile for macOS")
    return
  }

  downloadUnpackLibuast("darwin")

  val cmd = osxHome + "/bin/o64-clang++-libc++" + " " + CPP_FLAGS + " " +
      "-I" + osxHome + s"/SDK/MacOSX${sdkVersion}.sdk/usr/include/ " +
      "-I" + JAVA_HOME + "/include " +
      "-I" + JAVA_HOME + "/include/linux " +
      "-Isrc/main/resources/libuast " +
      "-o src/main/resources/lib/libscalauast.dylib " +
      sourceFiles +
      "src/main/resources/libuast/libuast.a "

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

cleanFiles ++= Seq(
  baseDirectory.value / "src/main/resources/libuast",
  baseDirectory.value / "src/main/resources/lib",
  baseDirectory.value / s"${protoDir}/github.com/bblfsh"
)

mainClass := Def.sequential(getProtoFiles, getLibuast, compileScalaLibuast, (mainClass in Compile)).value
