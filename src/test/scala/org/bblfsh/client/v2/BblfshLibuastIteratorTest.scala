package org.bblfsh.client.v2

import org.scalatest.{FlatSpec, Matchers}

import scala.io.Source


class BblfshLibuastIteratorTest extends FlatSpec with Matchers {

  var nativeRootNode: NodeExt = _
  val mangedRootNode: JNode = JArray(
    JObject(
      "k1" -> JString("v1"),
      "k2" -> JObject(
        "k3" -> JInt(24)
      )
    ))


  "Managed UAST iterator" should "return non-empty results on JVM objects" in {
    val it = BblfshClient.iterator(mangedRootNode, BblfshClient.PreOrder)
    val nodes = it.toList

    nodes shouldNot be(empty)
  }


  "Native UAST iterator" should "return non-empty results on decoded objects" in {
    import BblfshClient._ // enables uast.* methods
    val parse = (file: String) => {
      BblfshClient("localhost", 9432)
      .parse(file, Source.fromFile(file).getLines.mkString("\n"))
    }
    val resp = parse("src/test/resources/Tiny.java")
    nativeRootNode = resp.uast.decode.root()

    val it = BblfshClient.iterator(nativeRootNode, BblfshClient.PreOrder)
    val nodes = it.toList

    nodes shouldNot be(empty)
  }

}
