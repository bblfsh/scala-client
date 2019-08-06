package org.bblfsh.client.v2.libuast

import org.bblfsh.client.v2.{ContextExt, JNode, NodeExt}

import scala.collection.Iterator
import java.io.File
import java.nio.file.Paths
import java.nio.ByteBuffer

import org.apache.commons.io.{FileUtils, IOUtils}

object Libuast {
  final var loaded = false

  abstract class UastAbstractIter[T](var node: T, var treeOrder: Int, var iter: Long, var ctx: Long)
    extends Iterator[T] {
    private var closed = false
    private var lookedAhead = false
    private var nextNode: T = _

    private def lookahead(): T = {
      lookedAhead = true
      val node = nativeNext(iter)
      if (node == null) {
        close()
      }
      node
    }

    override def hasNext(): Boolean = {
      if (closed) {
        return false
      }
      if (lookedAhead) {
        return true
      }
      nextNode = lookahead()
      nextNode != null
    }

    override def next(): T = {
      if (!lookedAhead) {
        nextNode = lookahead()
      }
      lookedAhead = false
      nextNode // never null, iff called after .hasNext
    }

    def close() = {
      if (!closed) {
        nativeDispose()
        closed = true
      }
    }

    def nativeNext(iterPtr: Long): T
    def nativeInit()
    def nativeDispose()

    override def finalize(): Unit = {
      this.nativeDispose()
    }
  }

  /** Iterator over the given external/native node */
  class UastIterExt(node: NodeExt, treeOrder: Int, iter: Long, ctx: Long)
    extends UastAbstractIter(node, treeOrder, iter, ctx) {
    @native def nativeNext(iterPtr: Long): NodeExt
    @native def nativeInit()
    @native def nativeDispose()
  }

  object UastIterExt {
    def apply(node: NodeExt, treeOrder: Int): UastIterExt = {
      val it = new UastIterExt(node, treeOrder, 0, 0)
      it.nativeInit()
      it
    }
  }

  /** Iterator over the given managed node */
  class UastIter(node: JNode, treeOrder: Int, iter: Long, ctx: Long)
    extends UastAbstractIter(node, treeOrder, iter, ctx) {
    @native def nativeNext(iterPtr: Long): JNode
    @native def nativeInit()
    @native def nativeDispose()
  }

  object UastIter {
    def apply(node: JNode, treeOrder: Int): UastIter = {
      val it = new UastIter(node, treeOrder, 0, 0)
      it.nativeInit()
      it
    }
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

  // TODO(#83): implement
  @native def filter(node: NodeExt, query: String): List[NodeExt]

  /** Decode UAST from a byte array */
  @native def decode(buf: ByteBuffer): ContextExt
}
