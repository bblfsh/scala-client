package org.bblfsh.client

import java.util

case class NodeExt(ctx: Long, handle: Long) {
  @native def load(): util.TreeMap[String, _] //FIXME: scala.collection.immutable.Map
}
