package org.bblfsh.client.v2.libuast

import org.bblfsh.client.v2.{ContextExt, Context, JNode, NodeExt}
import org.bblfsh.client.v2.libuast.Libuast.UastIterExt

import scala.collection.Iterator
import java.io.File
import java.nio.file.Paths
import java.nio.ByteBuffer

import org.apache.commons.io.{FileUtils, IOUtils}

object Libuast {
  final var loaded = false

  if (!loaded) {
    System.err.println("Loading native libscalauast")
    Libuast.loadBinaryLib("libscalauast")
  }

  case class UastFormat(
    uastBinary: Int,
    uastYaml: Int
  )

  case class TreeOrder(
    anyOrder: Int,
    preOrder: Int,
    postOrder: Int,
    levelOrder: Int,
    childrenOrder: Int,
    positionOrder: Int
  )

  /**
    * Skeletal Node iterator implementation that delegates to Libuast.
    *
    * It brides the gap between the contracts of a Scala iterator (.hasNext()/.next()) and
    * a native Libuast iterator (.next() == null at the end).
    **/
  abstract class UastAbstractIter[T >: Null](var node: T, var treeOrder: Int, var iter: Long)
      extends Iterator[T] {
    private var closed = false
    private var nextNode: Option[T] = None

    private def lookahead(): Option[T] = {
      val node = nativeNext(iter)
      if (node == null) {
        close()
        None
      } else {
        Some(node)
      }
    }

    /** True only if the next element is not null */
    override def hasNext(): Boolean = if (closed) {
      false
    } else if (nextNode.isDefined) {
      true
    } else {
      nextNode = lookahead()
      nextNode.isDefined
    }

    override def next(): T = if (hasNext()) {
      val next = nextNode.get
      nextNode = None
      next
    } else {
      null
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

  /** Iterator over children of the given external/native node */
  class UastIterExt(node: NodeExt, treeOrder: Int, iter: Long, var ctx: ContextExt)
    extends UastAbstractIter(node, treeOrder, iter) {
    @native def nativeNext(iterPtr: Long): NodeExt
    @native def nativeInit()
    @native def nativeDispose()
  }

  object UastIterExt {
    def apply(node: NodeExt, treeOrder: Int): UastIterExt = {
      val it = new UastIterExt(node, treeOrder, 0, ContextExt(0))
      it.nativeInit()
      it
    }
  }

  /** Iterator over children of the given managed node */
  class UastIter(node: JNode, treeOrder: Int, iter: Long, var ctx: Context)
    extends UastAbstractIter(node, treeOrder, iter) {
    @native def nativeNext(iterPtr: Long): JNode
    @native def nativeInit()
    @native def nativeDispose()
  }

  object UastIter {
    def apply(node: JNode, treeOrder: Int): UastIter = {
      val it = new UastIter(node, treeOrder, 0, Context(0))
      it.nativeInit()
      it
    }
  }

  // Extract the native module from the jar
  private final def loadBinaryLib(name: String) = {
    val osName = System.getProperty("os.name").toLowerCase
    val ext = osName match {
      case os if (os.contains("mac os")) => ".dylib"
      case os if (os.contains("linux")) => ".so"
      case os if (os.contains("windows")) => ".dll"
    }
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
  Libuast

  /** Decode UAST from a byte array
    * Receives the buffer to decode and the format it is encoded in
    */
  @native def decode(buf: ByteBuffer, fmt: Int): ContextExt

  /** Lifts the tree order values from the libuast */
  @native def getTreeOrders: Libuast.TreeOrder

  /** Lifts the uast decoding / encoding options from the libuast */
  @native def getUastFormats: Libuast.UastFormat
}
