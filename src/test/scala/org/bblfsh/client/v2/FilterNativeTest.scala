package org.bblfsh.client.v2

import org.scalatest.{BeforeAndAfterAll, FlatSpec, Matchers}

import scala.io.Source

class FilterNativeTest extends FlatSpec
  with Matchers
  with BeforeAndAfterAll {

  var nativeRootCtx: ContextExt = _

  override def beforeAll {
    import BblfshClient._ // enables uast.* methods

    val client = BblfshClient("localhost", 9432)
    val parse = (file: String) => {
      client.parse(file, Source.fromFile(file).getLines.mkString("\n"))
    }
    val resp = parse("src/test/resources/Tiny.java")
    client.close()
    nativeRootCtx = resp.uast.decode()
  }

  "XPath filter" should "find all positions under context" in {
    val it = nativeRootCtx.filter("//uast:Position")
    it.hasNext() should be(true)

    val pos = it.toList
    pos should have size (8) // Tiny.java contains 8 nodes with position

    it.close()
    it.hasNext() should be(false)
  }

  "XPath filter" should "find all positions under node" in {
    val it = nativeRootCtx.root().filter("//uast:Position")

    it.hasNext() should be(true)

    val pos = it.toList
    pos should have size (8)  // Tiny.java contains 8 nodes with position
  }

}
