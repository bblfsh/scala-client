package org.bblfsh.client.v2

case class Node(ctx: Long, handle: Long) {
  // TODO(bzz) make sure single string value or an array can also be loaded
  @native def load(): Map[String, _]
}
