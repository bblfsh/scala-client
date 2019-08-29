package org.bblfsh.client.v2

import java.io.Serializable
import java.nio.ByteBuffer

import org.bblfsh.client.v2.libuast.Libuast.UastIterExt

import scala.collection.mutable


/**
  * UAST representation for the nodes originated from the Go side.
  * This is equivalent of pyuast.NodeExt API.
  *
  * @param ctx a reference to the external context
  * @param handle pointer to the native Node
  */
case class NodeExt(ctx: ContextExt, handle: Long) {
  @native def load(): JNode
  @native def filter(query: String): UastIterExt
}


/**
  * UAST representation for the nodes originated from or loaded to the JVM side.
  * Mirrors https://godoc.org/github.com/bblfsh/sdk/uast/nodes
  *
  * This is equivalent of pyuast.Node API.
  */
sealed abstract class JNode {
  import BblfshClient.uastBinary

  def toByteArray(fmt: Int): Array[Byte] = {
    val buf = toByteBuffer(fmt)
    val arr = new Array[Byte](buf.capacity())
    buf.get(arr)
    buf.rewind()
    arr
  }

  /** Use binary UAST format */
  def toByteArray: Array[Byte] = {
    toByteArray(uastBinary)
  }

  def toByteBuffer(fmt: Int): ByteBuffer = {
    val ctx = Context()
    val bb = ctx.encode(this, fmt)
    ctx.dispose()
    bb
  }

  /** Use binary UAST format */
  def toByteBuffer: ByteBuffer = {
    toByteBuffer(uastBinary)
  }

  /* Dynamic dispatch is a convenience to be called from JNI */
  def children: Seq[JNode] = this match {
    case JObject(l) => l map (_._2)
    case JArray(l) => l
    case _ => Seq()
  }

  def size: Int = this match {
    case JObject(l) => l.size
    case JArray(l) => l.size
    case JString(l) => l.length
    case _ => 0
  }

  def keyAt(i: Int): String = this match {
    case o: JObject => o.obj(i)._1
    case _ => ""
  }

  def valueAt(i: Int): JNode = this match {
    case o: JObject => o.obj(i)._2
    case c: JArray => c.arr(i)
    case _ => JNothing
  }

  def apply(k: String): JNode = this match {
    case o: JObject => o.obj.filter(_._1 == k).head._2
    case _ => JNothing
  }
}

object JNode {
  import BblfshClient.uastBinary

  private def decodeFrom(bytes: ByteBuffer, fmt: Int): JNode = {
    val ctx = BblfshClient.decode(bytes, fmt)
    val node = ctx.root().load()
    ctx.dispose()
    node
  }

  /**
    * Decodes UAST from the given Buffer.
    *
    * If the buffer is Direct, it will avoid extra memory allocation,
    * otherwise it will copy the content to a new Direct buffer.
    *
    * @param original UAST encoded in wire format of protocol.v2
    * @return JNode of the UAST root
    */
  def parseFrom(original: ByteBuffer, fmt: Int): JNode = {
    val bufDirect = if (!original.isDirect) {
      val bufDirectCopy = ByteBuffer.allocateDirect(original.capacity())
      original.rewind()
      bufDirectCopy.put(original)
      original.rewind()
      bufDirectCopy.flip()
      bufDirectCopy
    } else {
      original
    }
    decodeFrom(bufDirect, fmt)
  }

  /** Parse from a buffer using binary UAST format */
  def parseFrom(original: ByteBuffer): JNode = {
    parseFrom(original, uastBinary)
  }

  /**
    * Decodes UAST from the given bytes.
    *
    * It will copy memory into temporary Direct buffer,
    * otherwise it will copy the content to a new Direct buffer.
    *
    * @param bytes UAST encoded in wire format of protocol.v2
    * @return JNode of the UAST root
    */
  def parseFrom(bytes: Array[Byte], fmt: Int): JNode = {
    val bufDirect = ByteBuffer.allocateDirect(bytes.size)
    bufDirect.put(bytes)
    bufDirect.flip()
    decodeFrom(bufDirect, fmt)
  }

  /** Parse from an array using binary UAST format */
  def parseFrom(bytes: Array[Byte]): JNode = {
    parseFrom(bytes, uastBinary)
  }
}

case object JNothing extends JNode // 'zero' value for JNode
case class JNull() extends JNode
case class JString(str: String) extends JNode
case class JFloat(num: Double) extends JNode
case class JUint(num: Long) extends JNode {
  def get(): Long = java.lang.Integer.toUnsignedLong(num.toInt)
}
case class JInt(num: Long) extends JNode
case class JBool(value: Boolean) extends JNode

case class JObject(obj: mutable.Buffer[JField]) extends JNode {
  def this() = this(mutable.Buffer[JField]())
  def filter(p: ((String, JNode)) => Boolean) = this.obj.filter(p)
  def add(k: String, v: JNode) = {
    obj += ((k, v))
  }
}
case object JObject {
  def apply[T <: (Product with Serializable with JNode)](ns: (String, T)*) = {
    val jo = new JObject()
    jo.obj ++= ns
    jo
  }
}

case class JArray(arr: mutable.Buffer[JNode]) extends JNode {
  def this(size: Int) = this(new mutable.ArrayBuffer[JNode](size))
  def filter(p: JNode => Boolean) = this.arr.filter(p)
  def add(n: JNode) = {
    arr += n
  }
}
case object JArray {
  /** Helper to construct literals in map-like notation */
  def apply[T <:  (Product with Serializable with JNode)](ns: T *)   = {
    val ja = new JArray(ns.length)
    ja.arr ++= ns
    ja
  }
}
