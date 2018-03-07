package org.bblfsh.client.libuast

import gopkg.in.bblfsh.sdk.v1.uast.generated.Node
import scala.collection.Iterator
import java.io.File
import java.nio.file.Paths
import java.nio.ByteBuffer

import org.apache.commons.io.{IOUtils, FileUtils}

object Libuast {
  final var loaded = false

  class UastIterator(node: Node, treeOrder: Int) extends Iterator[Node] {

    private var closed = false
    private var iterPtr: ByteBuffer = newIterator(node, treeOrder)

    override def hasNext(): Boolean = {
      !closed
    }

    override def next(): Node = {
      val res = nextIterator(iterPtr)
      if (res == null) {
        close() 
      }
      res
    }

    def close() = {
      if (!closed) {
        disposeIterator(iterPtr)
        closed = true
      }
    }

    @native def newIterator(node: Node, treeOrder: Int): ByteBuffer
    @native def nextIterator(ptr: ByteBuffer): Node
    @native def disposeIterator(ptr: ByteBuffer)
  }

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

  def iterator(node: Node, treeOrder: Int) = {
    new Libuast.UastIterator(node, treeOrder)
  }

  @native def filter(node: Node, query: String): List[Node]
  @native def filterBool(node: Node, query: String): Boolean
  @native def filterNumber(node: Node, query: String): Double
  @native def filterString(node: Node, query: String): String
}
