package org.bblfsh.client.libuast

import gopkg.in.bblfsh.sdk.v1.uast.generated.Node

class Libuast {
  @native def filter(node: Node, query: String): List[Node]
}
