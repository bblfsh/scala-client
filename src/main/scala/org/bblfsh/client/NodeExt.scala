package org.bblfsh.client

case class NodeExt(ctx: Long, handle: Long) {
  @native def load(): Map[String, _]
}
