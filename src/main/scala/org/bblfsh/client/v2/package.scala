package org.bblfsh.client

/** Bblfsh client for protocol v2.
  * See https://github.com/bblfsh/sdk/blob/v3.1.0/protocol/driver.proto
  *
  * The main class to use is [[org.bblfsh.client.v2.BblfshClient]]
  */
package object v2 {
  /** Key, Value representation of [[org.bblfsh.client.v2.JObject]] */
  type JField = (String, JNode)

  import BblfshClient._

  /** Allow to use methods
    * f(fmt: UastFormat) as f(0) or f(1)
    * g(fmt: Int) as g(UastBinary) or g(UastYaml)
    */
  implicit def formatToInt(fmt: UastFormat): Int = {
    fmt.toInt
  }

  implicit def intToFormat(x: Int): UastFormat = {
    x match {
      case UastBinary.toInt => UastBinary
      case UastYaml.toInt => UastYaml
      case _ =>
        println("warning: not valid numeric format, using UastBinary")
        UastBinary
    }
  }

  /** Allow to use methods
    * f(order: UastOrder) as f(0), f(1), f(2), ...
    * g(order: Int) as g(AnyOrder), g(PreOrder), g(PostOrder)
    */
  implicit def orderToInt(order: TreeOrder): Int = {
    order.toInt
  }

  implicit def intToOrder(x: Int): TreeOrder = {
    x match {
      case AnyOrder.toInt => AnyOrder
      case PreOrder.toInt => PreOrder
      case PostOrder.toInt => PostOrder
      case LevelOrder.toInt => LevelOrder
      case ChildrenOrder.toInt => ChildrenOrder
      case PositionOrder.toInt => PositionOrder
      case _ =>
        println("warning: not valid numeric order, using AnyOrder")
        AnyOrder
    }
  }
}
