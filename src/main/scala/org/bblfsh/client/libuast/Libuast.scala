package org.bblfsh.client.libuast

import java.io.File
import java.nio.ByteBuffer
import java.nio.file.Paths

import gopkg.in.bblfsh.sdk.v2.uast.nodes.nodes.Node
import org.apache.commons.io.{FileUtils, IOUtils}
import org.bblfsh.client.{ContextExt, NodeExt}

import scala.collection.Iterator

object Libuast {
  final var loaded = false

  class UastIterator(node: NodeExt, treeOrder: Int) extends Iterator[NodeExt] {

    private var closed = false
    private var iterPtr: ByteBuffer = newIterator(node, treeOrder)

    override def hasNext(): Boolean = {
      !closed
    }

    override def next(): NodeExt = {
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

    @native def newIterator(node: NodeExt, treeOrder: Int): ByteBuffer
    @native def nextIterator(ptr: ByteBuffer): NodeExt
    @native def disposeIterator(ptr: ByteBuffer)
  }

  // Extract the native module from the jar
  private final def loadBinaryLib(name: String) = {
    val ext = if (System.getProperty("os.name").toLowerCase == "mac os x") ".dylib" else ".so"
    val fullLibName = name + ext
    val path = Paths.get("lib", fullLibName).toString
    val in = getClass.getClassLoader.getResourceAsStream(path)
    if (null == in) {
      val msg = s"Failed to load library '$name' from '$path'"
      println(msg)
      throw new RuntimeException(msg)
    }

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

  def iterator(node: NodeExt, treeOrder: Int) = {
    new Libuast.UastIterator(node, treeOrder)
  }

  @native def decode(buf: ByteBuffer): ContextExt                   //done
  //@native def uast(): Context

  @native def filter(node: NodeExt, query: String): List[NodeExt]
  @native def filterBool(node: NodeExt, query: String): Boolean
  @native def filterNumber(node: NodeExt, query: String): Double
  @native def filterString(node: NodeExt, query: String): String
}
