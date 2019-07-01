package org.bblfsh.client.v2

import scala.collection.mutable

/** UAST nodes representation on the JVM side.
 *
 *  Mirrors https://godoc.org/github.com/bblfsh/sdk/uast/nodes
 */
sealed abstract class JNode {
  def children: Seq[JNode] = this match {
    case JObject(l) => l map (_._2)
    case JArray(l) => l
    case _ => Seq()
  }

  def size: Int = this match {
    case JObject(l) => l.size
    case JArray(l) => l.size
    case JString(l) => l.length
    case JNothing => 0
    case _ => 1
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
    case o: JObject => o.obj.filter( _._1 == k ).head._2
    case _ => JNothing
  }

}

case object JNothing extends JNode // 'zero' value for JNode
case class JNull() extends JNode
case class JString(str: String) extends JNode
case class JFloat(num: Double) extends JNode
case class JUint(num: Long) extends JNode
case class JInt(num: Long) extends JNode
case class JBool(value: Boolean) extends JNode
case class JObject(obj: mutable.Buffer[JField]) extends JNode {
  def this() = this(mutable.Buffer[JField]())
  def add(k: String, v: JNode) = {
    obj += ((k, v))
  }
}
case class JArray(arr: mutable.Buffer[JNode]) extends JNode {
  def this(size: Int) = this(new mutable.ArrayBuffer[JNode](size))
  def add(v: JNode) = {
    arr += v
  }
}
