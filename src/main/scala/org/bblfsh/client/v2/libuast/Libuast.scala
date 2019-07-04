package org.bblfsh.client.v2.libuast

import org.bblfsh.client.v2.{ContextExt, JNode, Node}

import scala.collection.Iterator
import java.io.File
import java.nio.file.Paths
import java.nio.ByteBuffer

import org.apache.commons.io.{FileUtils, IOUtils}

object Libuast {
  final var loaded = false

  // TODO(bzz): implement iterators
  // class UastIterator(node: Node, treeOrder: Int) extends Iterator[Node] {

  //   private var closed = false
  //   private var iterPtr: ByteBuffer = newIterator(node, treeOrder)

  //   override def hasNext(): Boolean = {
  //     !closed
  //   }

  //   override def next(): Node = {
  //     val res = nextIterator(iterPtr)
  //     if (res == null) {
  //       close() 
  //     }
  //     res
  //   }

  //   def close() = {
  //     if (!closed) {
  //       disposeIterator(iterPtr)
  //       closed = true
  //     }
  //   }

  //   @native def newIterator(node: Node, treeOrder: Int): ByteBuffer
  //   @native def nextIterator(ptr: ByteBuffer): Node
  //   @native def disposeIterator(ptr: ByteBuffer)
  // }

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

  // TODO(bzz): implement iterators
  // def iterator(node: Node, treeOrder: Int) = {
  //   new Libuast.UastIterator(node, treeOrder)
  // }

  @native def decode(buf: ByteBuffer): ContextExt
  @native def filter(node: Node, query: String): List[Node]
}
