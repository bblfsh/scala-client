name := "bblfsh-client"
organization := "org.bblfsh"
version := "1.0.0"

scalaVersion := "2.11.11"
val libuastVersion = "v1.0.1"


mainClass in Compile := Some("org.bblfsh.client.cli.ScalaClientCLI")

target in assembly := file("build")

PB.targets in Compile := Seq(
  scalapb.gen() -> (sourceManaged in Compile).value
)
PB.protoSources in Compile := Seq(file("src/main/proto"))

libraryDependencies += "com.trueaccord.scalapb" %% "scalapb-runtime" % com.trueaccord.scalapb.compiler.Version.scalapbVersion % "protobuf"
libraryDependencies ++= Seq(
  "org.scalatest" %% "scalatest" % "3.0.1" % "test",

  "io.grpc" % "grpc-netty" % com.trueaccord.scalapb.compiler.Version.grpcJavaVersion,
  "com.trueaccord.scalapb" %% "scalapb-runtime-grpc" % com.trueaccord.scalapb.compiler.Version.scalapbVersion,

  "org.rogach" %% "scallop" % "3.0.3"
)

assemblyMergeStrategy in assembly := {
  case "META-INF/io.netty.versions.properties" => MergeStrategy.last
  case x =>
    val oldStrategy = (assemblyMergeStrategy in assembly).value
    oldStrategy(x)
}

test in assembly := {}

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

val getLibuast = TaskKey[Unit]("getLibuast", "Retrieve libuast")
getLibuast := {
    import sys.process._

    f"curl -SL https://github.com/bblfsh/libuast/releases/download/$libuastVersion%s/libuast-$libuastVersion%s.tar.gz -o libuast.tar.gz" #&&
    "tar zxf libuast.tar.gz" #&&
    f"mv libuast-$libuastVersion%s libuast" #&&
    "rm -rf src/libuast-native" #&&
    "mv libuast/src/ src/libuast-native" #&&
    "rm -rf libuast" #&&
    "rm libuast.tar.gz" !
}

// TODO: MacOS support.
val compileLibuast = TaskKey[Unit]("compileLibuast", "Compile libUAST")
compileLibuast := {
    import sys.process._

    var javaHome = System.getenv("JAVA_HOME")
    if (javaHome == null) 
        javaHome = "/usr/lib/jvm/java-8-openjdk-amd64"
        
    val xml2Conf = "xml2-config --cflags --libs" !!

    "gcc -shared -Wall -fPIC -O2 " +
        "-I/usr/include " +
        "-I" + javaHome + "/include/ " +
        "-I" + javaHome + "/include/linux " +
        "-Isrc/libuast-native/  " +
        "-o build/libscalauast.so " + 
        "src/main/scala/org/bblfsh/client/libuast/org_bblfsh_client_libuast_Libuast.c " +
        "src/main/scala/org/bblfsh/client/libuast/utils.c " +
        "src/main/scala/org/bblfsh/client/libuast/nodeiface.c " +
        "src/libuast-native/uast.c " +
        "src/libuast-native/roles.c " +
        xml2Conf + " " !
}
mainClass := ((mainClass in Compile) dependsOn (getLibuast, compileLibuast)).value

val testsAddLib = TaskKey[Unit]("testAddLib", "Copy native lib to test")
testsAddLib := {
    import sys.process._

    val scalaMinor = scalaVersion.value.slice(0,4)
    val testDir = "target/scala-" + scalaMinor + "/classes"

    f"cp build/libscalauast.so $testDir%s" !!

    "cp build/libscalauast.so target/" !!
}
test := ((test in Test) dependsOn testsAddLib).value
