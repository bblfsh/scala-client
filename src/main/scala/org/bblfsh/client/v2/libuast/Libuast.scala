package org.bblfsh.client.v2.libuast

import org.bblfsh.client.v2.{ContextExt, JNode, NodeExt}

import scala.collection.Iterator
import java.io.File
import java.nio.file.Paths
import java.nio.ByteBuffer

import org.apache.commons.io.{FileUtils, IOUtils}

object Libuast {
  final var loaded = false

  // TODO(bzz): implement
  class UastIter(node: JNode, treeOrder: Int, iter: Long, ctx: Long) extends Iterator[JNode] {
    @native override def hasNext(): Boolean
    @native override def next(): JNode

    @native def nativeInit()
    @native def nativeDispose()
    override def finalize(): Unit = {
        this.nativeDispose()
    }
  }

  object UastIter {
    def apply(node: JNode, treeOrder: Int): UastIter = {
      val it = new UastIter(node, treeOrder, 0, 0)
      it.nativeInit()
      it
    }
  }

  class UastIterExt(var node: NodeExt, var treeOrder: Int, var iter: Long, var ctx: Long) extends Iterator[NodeExt] {
    @native override def hasNext(): Boolean // FIXM(bzz): implement
    @native override def next(): NodeExt // FIXM(bzz): implement

    @native def nativeInit()
    @native def nativeDispose()
    override def finalize(): Unit = {
        this.nativeDispose()
    }
  }

  object UastIterExt {
    def apply(node: NodeExt, treeOrder: Int): UastIterExt = {
      val it = new UastIterExt(node, treeOrder, 0, 0)
      it.nativeInit()
      it
    }
  }

  // FIXM(bzz): remove once hasNext/next are implemented
  //class UastIterExt(node: NodeExt, treeOrder: Int, nIter: Long, nCtx: Long) extends Iterator[NodeExt] {
  //  private var closed = false
  //  private var iterPtr: ByteBuffer = newIterator(node, treeOrder)
  //
  //  override def hasNext(): Boolean = {
  //    !closed
  //  }
  //
  //  override def next(): NodeExt = {
  //    val res = nextIterator(iterPtr)
  //    if (res == null) {
  //      close()
  //    }
  //    res
  //  }
  //
  //  def close() = {
  //    if (!closed) {
  //      disposeIterator(iterPtr)
  //      closed = true
  //    }
  //  }
  //
  //  @native def newIterator(node: NodeExt, treeOrder: Int): ByteBuffer
  //
  //  @native def nextIterator(ptr: ByteBuffer): NodeExt
  //
  //  @native def disposeIterator(ptr: ByteBuffer)
  //}


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

  @native def filter(node: NodeExt, query: String): List[NodeExt]

  //  def iterator(node: JNode, treeOrder: Int) =
  //    new Libuast.UastIter(node, treeOrder)
  //  def iterator(node: NodeExt, treeOrder: Int) =
  //    new Libuast.UastIterExt(node, treeOrder)

  // bblfsh.pyuast
  /** Get an iterator over a node */
  @native def iterator()

  /** Decode UAST from a byte array */
  @native def decode(buf: ByteBuffer): ContextExt
}
