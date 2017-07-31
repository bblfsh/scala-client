name := "client"
organization := "org.bblfsh"
version := "0.0.1"

scalaVersion := "2.11.11"

mainClass in Compile := Some("org.bblfsh.clent.ScalaClient")

PB.targets in Compile := Seq(
  scalapb.gen() -> (sourceManaged in Compile).value
)
PB.protoSources in Compile := Seq(file("src/main/proto"))

libraryDependencies += "com.trueaccord.scalapb" %% "scalapb-runtime" % com.trueaccord.scalapb.compiler.Version.scalapbVersion % "protobuf"

libraryDependencies ++= Seq(
  "org.scalatest" %% "scalatest" % "3.0.1" % "test",

  "io.grpc" % "grpc-netty" % com.trueaccord.scalapb.compiler.Version.grpcJavaVersion,
  "com.trueaccord.scalapb" %% "scalapb-runtime-grpc" % com.trueaccord.scalapb.compiler.Version.scalapbVersion
)
