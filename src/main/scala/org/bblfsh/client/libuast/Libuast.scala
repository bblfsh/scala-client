package org.bblfsh.client.libuast

import gopkg.in.bblfsh.sdk.v1.uast.generated.Node
import java.io.File
import java.nio.file.Paths

import org.apache.commons.io.{IOUtils, FileUtils}

object Libuast {
  var loaded = false

  // Extract the native module from the jar
  private def loadBinaryLib(name: String) = {
    val tempDir = System.getProperty("java.io.tmpdir")
    val ext = if (System.getProperty("os.name").toLowerCase == "mac") ".dylib" else ".so"
    val fullLibName = name + ext
    val outPath = Paths.get(tempDir, fullLibName).toString

    val in = getClass.getResourceAsStream(Paths.get("/lib", fullLibName).toString)
    val fout = new File(outPath)
    val out = FileUtils.openOutputStream(fout)

    try {
      IOUtils.copy(in, out)
      System.load(outPath)
      loaded = true
    } finally {
      in.close()
      out.close()
    }
  }

}

class Libuast {
  private val libName = "libscalauast"
  if (!Libuast.loaded) {
    Libuast.loadBinaryLib(libName)
  }

  @native def filter(node: Node, query: String): List[Node]
}
