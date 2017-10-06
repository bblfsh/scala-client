package org.bblfsh.client.libuast

import gopkg.in.bblfsh.sdk.v1.uast.generated.Node

class Libuast {
  //@native def filter(node: Node, query: String): List[Node]
  @native def filter(node: Int, query: String): Int
  // Stub XXX remove
  //def filter(node: Node, query: String): List[Node] = {
    //val fake: List[Node] = List()
    //fake
  //}
  @native def readfield(node: Node, field: String): String
  @native def readlen(node: Node, field: String): Int

}
