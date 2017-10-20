package org.bblfsh.client.libuast

import gopkg.in.bblfsh.sdk.v1.uast.generated.Node
import java.io.File
import java.nio.file.Paths

import org.apache.commons.io.{IOUtils, FileUtils}

object Libuast {
  final var loaded = false

  // Extract the native module from the jar
  private final def loadBinaryLib(name: String) = {
    val ext = if (System.getProperty("os.name").toLowerCase == "mac os x") ".dylib" else ".so"
    val fullLibName = name + ext
    val in = getClass.getResourceAsStream(Paths.get("/lib", fullLibName).toString)

    val prefix = "libscalauast_"
    val fout = File.createTempFile(prefix, ext)
    val out = FileUtils.openOutputStream(fout)

    try {
      IOUtils.copy(in, out)
    } finally {
      in.close()
      out.close()
    }

    System.load(fout.getAbsolutePath)
    loaded = true
  }
}

class Libuast {
  initialize()

  // Note: moving this to the Object doesn't synchronize correctly
  private def initialize() = Libuast.synchronized {
    if (!Libuast.loaded) {
      Libuast.loadBinaryLib("libscalauast")
    }
  }

  @native def filter(node: Node, query: String): List[Node]
}
