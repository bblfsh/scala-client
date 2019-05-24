package org.bblfsh.client.v2
	
case class Node(ctx: Long, handle: Long) {
  @native def load(): Map[String, _]
}