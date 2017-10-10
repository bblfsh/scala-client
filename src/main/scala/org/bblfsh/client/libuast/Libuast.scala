package org.bblfsh.client.libuast

import gopkg.in.bblfsh.sdk.v1.uast.generated.Node
import java.io.File
import java.nio.file.Paths

class Libuast {
  var jarPath = getClass.getProtectionDomain.getCodeSource.getLocation.getPath.toString

  //mainClass returns the jar file in the path, testing doesn't
  if (jarPath.endsWith(".jar")) {
    jarPath = new File(jarPath).getParent()
  }
  val ext = if (System.getProperty("os.name").toLowerCase == "mac") ".dylib" else ".so"
  val libPath = Paths.get(jarPath, "libscalauast" + ext).toString

  System.load(libPath)
  @native def filter(node: Node, query: String): List[Node]
}
