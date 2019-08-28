resolvers += Resolver.jcenterRepo
addSbtPlugin("com.eed3si9n" % "sbt-assembly" % "0.14.5")
addSbtPlugin("net.virtual-void" % "sbt-dependency-graph" % "0.8.2")
addSbtPlugin("com.jsuereth" % "sbt-pgp" % "1.0.0")
addSbtPlugin("org.xerial.sbt" % "sbt-sonatype" % "1.1")
addSbtPlugin("com.thesamet" % "sbt-protoc" % "0.99.16")
addSbtPlugin("com.typesafe.sbt" % "sbt-git" % "0.9.3")
addSbtPlugin("ch.jodersky" % "sbt-jni" % "1.2.6")
libraryDependencies += "com.thesamet.scalapb" %% "compilerplugin" % "0.8.3"
