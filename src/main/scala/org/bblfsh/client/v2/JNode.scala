package org.bblfsh.client.v2

/** UAST nodes representation on the JVM side.
 *
 *  Mirrors https://godoc.org/github.com/bblfsh/sdk/uast/nodes
 */
sealed abstract class JNode
case class JString(s: String) extends JNode
case class JDouble(num: Double) extends JNode
case class JLong(num: Long) extends JNode
case class JInt(num: BigInt) extends JNode
case class JBool(value: Boolean) extends JNode
case class JObject(obj: List[JField]) extends JNode
case class JArray(arr: List[JNode]) extends JNode

